/*
 * Copyright 2023-2024 Dozingfiretruck
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "nes.h"

typedef struct {
    void (*instruction)(nes_t* nes);      //instructions 
    uint16_t (*addressing_mode)(nes_t* nes);  //addressing_mode
    const uint8_t ticks;
} nes_opcode_t;

static const nes_opcode_t nes_opcode_table[256];

static inline uint8_t nes_read_joypad(nes_t* nes,uint16_t address){
    uint8_t state = 0;
    if (address == 0x4016){
        state = (nes->nes_cpu.joypad.joypad & (0x8000 >> (nes->nes_cpu.joypad.offset1 & nes->nes_cpu.joypad.mask))) ? 1 : 0;
        nes->nes_cpu.joypad.offset1++;
    }else if(address == 0x4017){
        state = (nes->nes_cpu.joypad.joypad & (0x80 >> (nes->nes_cpu.joypad.offset2 & nes->nes_cpu.joypad.mask))) ? 1 : 0;
        nes->nes_cpu.joypad.offset2++;
    }
    // NES_LOG_DEBUG("nes_read joypad %04X %d %02X %d\n",address,nes->nes_cpu.joypad.mask,nes->nes_cpu.joypad.joypad,state);
    return state;
}

static inline void nes_write_joypad(nes_t* nes,uint8_t data){
    nes->nes_cpu.joypad.mask = (data & 1)?0x00:0x07;
    if (data & 1)
        nes->nes_cpu.joypad.offset1 = nes->nes_cpu.joypad.offset2 = 0;
    // NES_LOG_DEBUG("nes_write joypad %04X %02X %d\n",address,data,nes->nes_cpu.joypad.mask);
}

static inline uint8_t nes_read_cpu(nes_t* nes,uint16_t address){
    switch (address & 0xE000){
        case 0x0000://$0000-$1FFF 2KB internal RAM + Mirrors of $0000-$07FF
            return nes->nes_cpu.cpu_ram[address & (uint16_t)0x07ff];
        case 0x2000://$2000-$3FFF NES PPU registers + Mirrors of $2000-2007 (repeats every 8 bytes)
            return nes_read_ppu_register(nes,address);
        case 0x4000://$4000-$5FFF NES APU and I/O registers
            if (address == 0x4016 || address == 0x4017) // I/O registers
                return nes_read_joypad(nes, address);
            else if (address < 0x4016){                 // APU registers
#if (NES_ENABLE_SOUND == 1)
                return nes_read_apu_register(nes, address);
#endif
            }else{
                NES_LOG_ERROR("nes_read address %04X not sPport\n",address);
            }
            return 0;
        case 0x6000://$6000-$7FFF SRAM
#if (NES_USE_SRAM == 1)
            return nes->nes_rom.sram[address & (uint16_t)0x1fff];
#else
            return 0;
#endif
        case 0x8000: case 0xA000: case 0xC000: case 0xE000:
            return nes->nes_cpu.prg_banks[(address >> 13)-4][address & (uint16_t)0x1fff];
        default :
            NES_LOG_ERROR("nes_read_cpu error %04X\n",address);
            return address >> 8;
    }
}

static inline uint16_t nes_readw_cpu(nes_t* nes,uint16_t address){
    return nes_read_cpu(nes,address) | (uint16_t)(nes_read_cpu(nes,address + 1)) << 8;
}

static inline const uint8_t* nes_get_dma_address(nes_t* nes,uint8_t data) {
    switch (data >> 5){
        case 0:
            return nes->nes_cpu.cpu_ram + ((uint16_t)(data & 0x07) << 8);
        case 4: case 5: case 6: case 7:// 高一位为1, [$8000, $10000) PRG-ROM
            return nes->nes_cpu.prg_banks[(data >> 4)&0x03] + ((uint16_t)(data & 0x0f) << 8);
        default:
            NES_LOG_ERROR("nes_get_dma_address error %02X\n",data);
            return NULL;
    }
}

static inline void nes_write_cpu(nes_t* nes,uint16_t address, uint8_t data){
    switch (address & 0xE000){
        case 0x0000://$0000-$1FFF 2KB internal RAM + Mirrors of $0000-$07FF
            nes->nes_cpu.cpu_ram[address & (uint16_t)0x07ff] = data;
            return;
        case 0x2000://$2000-$3FFF NES PPU registers + Mirrors of $2000-2007 (repeats every 8 bytes)
            nes_write_ppu_register(nes,address, data);
            return;
        case 0x4000://$4000-$5FFF NES APU and I/O registers
            if (address == 0x4016)
                nes_write_joypad(nes,data);
            else if (address == 0x4014){
                // NES_LOG_DEBUG("nes_write DMA data:0x%02X oam_addr:0x%02X\n",data,nes->nes_ppu.oam_addr);
                if (nes->nes_ppu.oam_addr) {
                    uint8_t* dst = nes->nes_ppu.oam_data;
                    const uint8_t len = nes->nes_ppu.oam_addr;
                    const uint8_t* src = nes_get_dma_address(nes,data);
                    nes_memcpy(dst, src + len, len);
                    nes_memcpy(dst + len, src, NES_PPU_OAM_SIZE - len);
                } else {
                    nes_memcpy(nes->nes_ppu.oam_data, nes_get_dma_address(nes,data), NES_PPU_OAM_SIZE);
                }
                nes->nes_cpu.cycles += 513;
                nes->nes_cpu.cycles += nes->nes_cpu.cycles & 1; //奇数周期需要多sleep 1个CPU时钟周期
            }else if (address < 0x4016 || address == 0x4017){
#if (NES_ENABLE_SOUND == 1)
                nes_write_apu_register(nes, address,data);
#endif
            }else{
                NES_LOG_ERROR("nes_write address %04X not suport\n",address);
            }
            return;
        case 0x6000://$6000-$7FFF SRAM
#if (NES_USE_SRAM == 1)
            nes->nes_rom.sram[address & (uint16_t)0x1fff] = data;
#endif
            return;
        case 0x8000: case 0xA000: case 0xC000: case 0xE000: // $8000-$FFFF PRG-ROM
            nes->nes_mapper.mapper_write(nes, address, data);
            return;
        default :
            NES_LOG_ERROR("nes_write_cpu error %04X %02X\n",address,data);
            return;
    }
}

// 入栈
#define NES_PUSH(nes,data)      (nes->nes_cpu.cpu_ram + 0x100)[nes->nes_cpu.SP--] = (uint8_t)(data)
#define NES_PUSHW(nes,data)     NES_PUSH(nes, ((data) >> 8) ); NES_PUSH(nes, ((data) & 0xff))
// 出栈
#define NES_POP(nes)            ((nes->nes_cpu.cpu_ram + 0x100)[++nes->nes_cpu.SP])
#define NES_POPW(nes)           ((uint16_t)NES_POP(nes)|(uint16_t)(NES_POP(nes) << 8))
// 状态寄存器检查位
#define NES_CHECK_N(x)          nes->nes_cpu.N = ((uint8_t)(x) >> 7) & 0x01
#define NES_CHECK_Z(x)          nes->nes_cpu.Z = ((uint8_t)(x) == 0)
#define NES_CHECK_NZ(x)         NES_CHECK_N(x);NES_CHECK_Z(x)

static inline void nes_dummy_read(nes_t* nes){
    nes_read_cpu(nes,nes->nes_cpu.PC);
}

// https://www.nesdev.org/6502_cn.txt

/* 
    Adressing modes:
    https://www.nesdev.org/wiki/CPU_addressing_modes
*/

/*
    Implicit:Instructions like RTS or CLC have no address operand, the destination of results are implied.
*/

/*
    #v:Immediate: Uses the 8-bit operand itself as the value for the operation, 
                    rather than fetching a value from a memory address.
*/
static inline uint16_t nes_imm(nes_t* nes){
    return nes->nes_cpu.PC++;
}

/*
    label:Relative::Branch instructions (e.g. BEQ, BCS) have a relative addressing mode 
                    that specifies an 8-bit signed offset relative to the current PC.
*/
static inline uint16_t nes_rel(nes_t* nes){
    const int8_t data = (int8_t)nes_read_cpu(nes,nes->nes_cpu.PC++);
    return nes->nes_cpu.PC + data;
}

/*
    a:Absolute::Fetches the value from a 16-bit address anywhere in memory.
*/
static inline uint16_t nes_abs(nes_t* nes){
    const uint8_t low_byte = nes_read_cpu(nes, nes->nes_cpu.PC++);
    const uint16_t high_byte = nes_read_cpu(nes, nes->nes_cpu.PC++) << 8;
    return high_byte | low_byte;
}

/*
    a,x:Absolute indexed:val = PEEK(arg + X)
*/
static inline uint16_t nes_abx(nes_t* nes){
    const uint16_t base_address = nes_abs(nes);
    const uint16_t address = base_address + nes->nes_cpu.X;
    if (nes_opcode_table[nes->nes_cpu.opcode].ticks==4){
        if ((address>>8) != (base_address>>8))nes->nes_cpu.cycles++;
    }
    return address;
}

/*
    a,y:Absolute indexed:val = PEEK(arg + Y)
*/
static inline uint16_t nes_aby(nes_t* nes){
    const uint16_t base_address = nes_abs(nes);
    const uint16_t address = base_address + nes->nes_cpu.Y;
    if (nes_opcode_table[nes->nes_cpu.opcode].ticks==4){
        if ((address>>8) != (base_address>>8))nes->nes_cpu.cycles++;
    }
    return address;
}

/*
    d:Zero page:Fetches the value from an 8-bit address on the zero page.
*/
static inline uint16_t nes_zp(nes_t* nes){
    // return nes->nes_cpu.cpu_ram[nes->nes_cpu.PC++ & (uint16_t)0x07ff];
    return nes_read_cpu(nes, nes->nes_cpu.PC++);
}

/*
    d,x:Zero page indexed:val = PEEK((arg + X) % 256)
*/
static inline uint16_t nes_zpx(nes_t* nes){
    return (nes_zp(nes) + nes->nes_cpu.X) & 0x00FF;
}

/*
    d,y:Zero page indexed:val = PEEK((arg + Y) % 256)
*/
static inline uint16_t nes_zpy(nes_t* nes){
    return (nes_zp(nes) + nes->nes_cpu.Y) & 0x00FF;
}

/*
    (d,x):Indexed indirect:val = PEEK(PEEK((arg + X) % 256) + PEEK((arg + X + 1) % 256) * 256)
*/
static inline uint16_t nes_izx(nes_t* nes){
    const uint8_t address = (uint8_t)nes_zp(nes) + nes->nes_cpu.X;
    return nes_read_cpu(nes,address)|(uint16_t)nes_read_cpu(nes,(uint8_t)(address + 1)) << 8;
}

/*
    (d),y:Indexed indirect:val = PEEK(PEEK(arg) + PEEK((arg + 1) % 256) * 256 + Y)
*/
static inline uint16_t nes_izy(nes_t* nes){
    const uint8_t value = (uint8_t)nes_zp(nes);
    const uint16_t address = nes_read_cpu(nes,value)|(uint16_t)nes_read_cpu(nes,(uint8_t)(value + 1)) << 8;
    if (nes_opcode_table[nes->nes_cpu.opcode].ticks==5){
        if ((address>>8) != ((address+nes->nes_cpu.Y)>>8))nes->nes_cpu.cycles++;
    }
    return address + nes->nes_cpu.Y;
}

/*
    (a):Indirect:The JMP instruction has a special indirect addressing mode 
                    that can jump to the address stored in a 16-bit pointer anywhere in memory.
*/
static inline uint16_t nes_ind(nes_t* nes){
    // 6502 BUG
    const uint16_t address = nes_abs(nes);
    return nes_read_cpu(nes,address) | (uint16_t)(nes_read_cpu(nes,(uint16_t)((address & (uint16_t)0xFF00)|((address + 1) & (uint16_t)0x00FF)))) << 8;
}

/* 6502/6510/8500/8502 Opcode matrix: https://www.oxyron.de/html/opcodes02.html */
/* Logical and arithmetic commands: */

/* 
    A :=A or {adr}
    N  V  U  B  D  I  Z  C
    *                 *
*/
static inline void nes_ora(nes_t* nes){
    nes->nes_cpu.A |= nes_read_cpu(nes,nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes));
    NES_CHECK_NZ(nes->nes_cpu.A);
}

/* 
    A := A & {adr}
    N  V  U  B  D  I  Z  C
    *                 *
*/
static inline void nes_and(nes_t* nes){
    nes->nes_cpu.A &= nes_read_cpu(nes,nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes));
    NES_CHECK_NZ(nes->nes_cpu.A);
}

/* 
    A := A exor {adr}
    N  V  U  B  D  I  Z  C
    *                 *
*/
static inline void nes_eor(nes_t* nes){
    nes->nes_cpu.A ^= nes_read_cpu(nes,nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes));
    NES_CHECK_NZ(nes->nes_cpu.A);
}

/* 
    A:=A+{adr}
    N  V  U  B  D  I  Z  C
    *  *              *  *
*/
static inline void nes_adc(nes_t* nes){
    const uint8_t src = nes_read_cpu(nes,nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes));
    const uint16_t result16 = nes->nes_cpu.A + src + nes->nes_cpu.C;
    nes->nes_cpu.C = result16 >> 8;
    const uint8_t result8 = (uint8_t)result16;

    if (!((nes->nes_cpu.A ^ src) & 0x80) && ((nes->nes_cpu.A ^ result8) & 0x80)) nes->nes_cpu.V = 1;
    else nes->nes_cpu.V = 0;

    nes->nes_cpu.A = result8;
    NES_CHECK_NZ(nes->nes_cpu.A);
}

/* 
    A:=A-{adr}
    N  V  U  B  D  I  Z  C
    *  *              *  *
*/
static inline void nes_sbc(nes_t* nes){
    const uint8_t src = nes_read_cpu(nes,nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes));
    const uint16_t result16 = nes->nes_cpu.A - src - !nes->nes_cpu.C;
    nes->nes_cpu.C = !(result16 >> 8);
    const uint8_t result8 = (uint8_t)result16;

    if (((nes->nes_cpu.A ^ src) & 0x80) && ((nes->nes_cpu.A ^ result8) & 0x80)) nes->nes_cpu.V = 1;
    else nes->nes_cpu.V = 0;

    nes->nes_cpu.A = result8;
    NES_CHECK_NZ(nes->nes_cpu.A);
}

/*
    A-{adr}
    N  V  U  B  D  I  Z  C
    *                 *  *
*/
static inline void nes_cmp(nes_t* nes){
    const uint16_t value = (uint16_t)nes->nes_cpu.A - (uint16_t)nes_read_cpu(nes,nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes));
    nes->nes_cpu.C = !(value >> 15);
    NES_CHECK_NZ((uint8_t)value);
}

/*
    X-{adr}
    N  V  U  B  D  I  Z  C
    *                 *  *
*/
static inline void nes_cpx(nes_t* nes){
    const uint16_t value = (uint16_t)nes->nes_cpu.X - (uint16_t)nes_read_cpu(nes,nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes));
    nes->nes_cpu.C = !(value >> 15);
    NES_CHECK_NZ((uint8_t)value);
}

/*
    Y-{adr}
    N  V  U  B  D  I  Z  C
    *                 *  *
*/
static inline void nes_cpy(nes_t* nes){
    const uint16_t value = (uint16_t)nes->nes_cpu.Y - (uint16_t)nes_read_cpu(nes,nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes));
    nes->nes_cpu.C = !(value >> 15);
    NES_CHECK_NZ((uint8_t)value);
}

/*
    {adr}:={adr}-1
    N  V  U  B  D  I  Z  C
    *                 *  
*/
static inline void nes_dec(nes_t* nes){
    uint16_t address = nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes);
    uint8_t data = nes_read_cpu(nes,address)-1;
    nes_write_cpu(nes,address, data);
    NES_CHECK_NZ(data);
}

/*
    X:=X-1
    N  V  U  B  D  I  Z  C
    *                 *  
*/
static inline void nes_dex(nes_t* nes){
    nes->nes_cpu.X--;
    NES_CHECK_NZ(nes->nes_cpu.X);
}

/*
    Y:=Y-1
    N  V  U  B  D  I  Z  C
    *                 *  
*/
static inline void nes_dey(nes_t* nes){
    nes->nes_cpu.Y--;
    NES_CHECK_NZ(nes->nes_cpu.Y);
}

/*
    {adr}:={adr}+1
    N  V  U  B  D  I  Z  C
    *                 *  
*/
static inline void nes_inc(nes_t* nes){
    uint16_t address = nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes);
    uint8_t data = nes_read_cpu(nes,address)+1;
    nes_write_cpu(nes,address,data);
    NES_CHECK_NZ(data);
}

/*
    X:=X+1
    N  V  U  B  D  I  Z  C
    *                 *  
*/
static inline void nes_inx(nes_t* nes){
    nes->nes_cpu.X++;
    NES_CHECK_NZ(nes->nes_cpu.X);
}

/*  
    Y:=Y+1
    N  V  U  B  D  I  Z  C
    *                 *  
*/
static inline void nes_iny(nes_t* nes){
    nes->nes_cpu.Y++;
    NES_CHECK_NZ(nes->nes_cpu.Y);
}

/*
    {adr}:={adr}*2
    N  V  U  B  D  I  Z  C
    *                 *  *
*/
static inline void nes_asl(nes_t* nes){
    if (nes_opcode_table[nes->nes_cpu.opcode].addressing_mode){
        uint16_t address = nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes);
        uint8_t data = nes_read_cpu(nes,address);
        nes->nes_cpu.C = data >> 7;
        data <<= 1;
        nes_write_cpu(nes,address,data);
        NES_CHECK_NZ(data);
    }else{
        nes->nes_cpu.C = nes->nes_cpu.A >> 7;
        nes->nes_cpu.A <<= 1;
        NES_CHECK_NZ(nes->nes_cpu.A);
    }
}

/*
    {adr}:={adr}*2+C
    N  V  U  B  D  I  Z  C
    *                 *  *
*/
static inline void nes_rol(nes_t* nes){
    if (nes_opcode_table[nes->nes_cpu.opcode].addressing_mode){
        uint16_t address = nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes);
        uint16_t data = nes_read_cpu(nes,address);
        data <<= 1;
        data |= nes->nes_cpu.C;
        nes->nes_cpu.C = (uint8_t)(data>>8);
        nes_write_cpu(nes,address,(uint8_t)data);
        NES_CHECK_NZ(data);
    }else{
        uint16_t data = nes->nes_cpu.A;
        data <<= 1;
        data |= nes->nes_cpu.C;
        nes->nes_cpu.C = (uint8_t)(data>>8);
        nes->nes_cpu.A = (uint8_t)data;
        NES_CHECK_NZ(nes->nes_cpu.A);
    }
}

/*
    {adr}:={adr}/2
    N  V  U  B  D  I  Z  C
    *                 *  *
*/
static inline void nes_lsr(nes_t* nes){
    if (nes_opcode_table[nes->nes_cpu.opcode].addressing_mode){
        uint16_t address = nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes);
        uint8_t value = nes_read_cpu(nes,address);
        nes->nes_cpu.C = value & 0x01;
        value >>= 1;
        NES_CHECK_NZ(value);
        nes_write_cpu(nes,address,value);
    }else{
        nes->nes_cpu.C = nes->nes_cpu.A & 0x01;
        nes->nes_cpu.A >>= 1;
        NES_CHECK_NZ(nes->nes_cpu.A);
    }
}

/*
    {adr}:={adr}/2+C*128
    N  V  U  B  D  I  Z  C
    *                 *  *
*/
static inline void nes_ror(nes_t* nes){
    if (nes_opcode_table[nes->nes_cpu.opcode].addressing_mode) {
        uint16_t address = nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes);
        uint16_t data = nes_read_cpu(nes,address);
        data |= (uint16_t)nes->nes_cpu.C << 8;
        nes->nes_cpu.C = data & 0x01;
        data >>= 1;
        nes_write_cpu(nes,address,(uint8_t)data);
        NES_CHECK_NZ(data);
    }else{
        uint16_t data = nes->nes_cpu.A;
        data |= (uint16_t)nes->nes_cpu.C << 8;
        nes->nes_cpu.C = data & 0x01;
        data >>= 1;
        nes->nes_cpu.A = (uint8_t)data;
        NES_CHECK_NZ(nes->nes_cpu.A);
    }
}

/* Move commands: */

/*
    A:={adr}
    N  V  U  B  D  I  Z  C
    *                 *  
*/
static inline void nes_lda(nes_t* nes){
    nes->nes_cpu.A = nes_read_cpu(nes,nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes));
    NES_CHECK_NZ(nes->nes_cpu.A);
}

/*
    {adr}:=A
    N  V  U  B  D  I  Z  C

*/
static inline void nes_sta(nes_t* nes){
    nes_write_cpu(nes,nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes),nes->nes_cpu.A);
}

/*
    X:={adr}
    N  V  U  B  D  I  Z  C
    *                 *  
*/
static inline void nes_ldx(nes_t* nes){
    nes->nes_cpu.X = nes_read_cpu(nes,nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes));
    NES_CHECK_NZ(nes->nes_cpu.X);
}

/*
    {adr}:=X
    N  V  U  B  D  I  Z  C

*/
static inline void nes_stx(nes_t* nes){
    nes_write_cpu(nes,nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes),nes->nes_cpu.X);
}

/*
    Y:={adr}
    N  V  U  B  D  I  Z  C
    *                 *  
*/
static inline void nes_ldy(nes_t* nes){
    nes->nes_cpu.Y = nes_read_cpu(nes,nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes));
    NES_CHECK_NZ(nes->nes_cpu.Y);
}

/*
    {adr}:=Y
    N  V  U  B  D  I  Z  C

*/
static inline void nes_sty(nes_t* nes){
    nes_write_cpu(nes,nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes),nes->nes_cpu.Y);
}

/*
    X:=A
    N  V  U  B  D  I  Z  C
    *                 *  
*/
static inline void nes_tax(nes_t* nes){
    nes->nes_cpu.X=nes->nes_cpu.A;
    NES_CHECK_NZ(nes->nes_cpu.X);
}

/*
    A:=X
    N  V  U  B  D  I  Z  C
    *                 *  
*/
static inline void nes_txa(nes_t* nes){
    nes->nes_cpu.A=nes->nes_cpu.X;
    NES_CHECK_NZ(nes->nes_cpu.A);
}

/*
    Y:=A
    N  V  U  B  D  I  Z  C
    *                 *  
*/
static inline void nes_tay(nes_t* nes){
    nes->nes_cpu.Y=nes->nes_cpu.A;
    NES_CHECK_NZ(nes->nes_cpu.Y);
}

/*
    A:=Y
    N  V  U  B  D  I  Z  C
    *                 *  
*/
static inline void nes_tya(nes_t* nes){
    nes->nes_cpu.A=nes->nes_cpu.Y;
    NES_CHECK_NZ(nes->nes_cpu.A);
}

/*
    X:=S
    N  V  U  B  D  I  Z  C
    *                 *  
*/
static inline void nes_tsx(nes_t* nes){
    nes->nes_cpu.X=nes->nes_cpu.SP;
    NES_CHECK_NZ(nes->nes_cpu.X);
}

/*
    S:=X
    N  V  U  B  D  I  Z  C

*/
static inline void nes_txs(nes_t* nes){
    nes->nes_cpu.SP=nes->nes_cpu.X;
}

/*
    A:=+(S)
    N  V  U  B  D  I  Z  C
    *                 *  
*/
static inline void nes_pla(nes_t* nes){
    nes_dummy_read(nes);
    nes->nes_cpu.A = NES_POP(nes);
    NES_CHECK_NZ(nes->nes_cpu.A);
}

/*
    (S)-:=A
    N  V  U  B  D  I  Z  C

*/
static inline void nes_pha(nes_t* nes){
    NES_PUSH(nes,nes->nes_cpu.A);
}

/*
    P:=+(S)
    N  V  U  B  D  I  Z  C
    *  *        *  *  *  *
*/
static inline void nes_plp(nes_t* nes){
    nes_dummy_read(nes);
    nes->nes_cpu.P = NES_POP(nes);
    // nes->nes_cpu.B = 0;
    if (!nes->nes_cpu.I){
        /* code */
    }
}

/*
    (S)-:=P
    N  V  U  B  D  I  Z  C

*/
static inline void nes_php(nes_t* nes){
    nes->nes_cpu.U = 1;
    nes->nes_cpu.B = 1;
    NES_PUSH(nes,nes->nes_cpu.P);
    // nes->nes_cpu.B = 0;
}

// Jump/Flag commands:

static inline void nes_branch(nes_t* nes,const uint16_t address) {
    nes_dummy_read(nes);
    const uint16_t pc_old = nes->nes_cpu.PC;
    nes->nes_cpu.PC = address;
    nes->nes_cpu.cycles++;
    if ((nes->nes_cpu.PC ^ pc_old) >> 8){
        nes_dummy_read(nes);
        nes->nes_cpu.cycles++;
    }
}

/*
    branch on N=0
    N  V  U  B  D  I  Z  C

*/
static inline void nes_bpl(nes_t* nes){
    const uint16_t address = nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes);
    if (nes->nes_cpu.N==0) nes_branch(nes,address);
}

/*
    branch on N=1
    N  V  U  B  D  I  Z  C

*/
static inline void nes_bmi(nes_t* nes){
    const uint16_t address = nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes);
    if (nes->nes_cpu.N)nes_branch(nes,address);
}

/*
    branch on V=0
    N  V  U  B  D  I  Z  C

*/
static inline void nes_bvc(nes_t* nes){
    const uint16_t address = nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes);
    if (nes->nes_cpu.V==0) nes_branch(nes,address);
}

/*
    branch on V=1
    N  V  U  B  D  I  Z  C

*/
static inline void nes_bvs(nes_t* nes){
    const uint16_t address = nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes);
    if (nes->nes_cpu.V) nes_branch(nes,address);
}

/*
    branch on C=0
    N  V  U  B  D  I  Z  C

*/
static inline void nes_bcc(nes_t* nes){
    const uint16_t address = nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes);
    if (nes->nes_cpu.C==0) nes_branch(nes,address);
}

/*
    branch on C=1
    N  V  U  B  D  I  Z  C

*/
static inline void nes_bcs(nes_t* nes){
    const uint16_t address = nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes);
    if (nes->nes_cpu.C) nes_branch(nes,address);
}

/*
    branch on Z=0
    N  V  U  B  D  I  Z  C

*/
static inline void nes_bne(nes_t* nes){
    const uint16_t address = nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes);
    if (nes->nes_cpu.Z==0) nes_branch(nes,address);
}

/*
    branch on Z=1
    N  V  U  B  D  I  Z  C

*/
static inline void nes_beq(nes_t* nes){
    const uint16_t address = nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes);
    if (nes->nes_cpu.Z) nes_branch(nes,address);
}

/*
    (S)-:=PC,P PC:=($FFFE)
    N  V  U  B  D  I  Z  C
             1     1
*/
static inline void nes_brk(nes_t* nes){
    nes->nes_cpu.PC++;
    NES_PUSHW(nes,nes->nes_cpu.PC);
    nes->nes_cpu.B = 1;
    NES_PUSH(nes,nes->nes_cpu.P);
    nes->nes_cpu.I = 1;
    nes->nes_cpu.PC = nes_readw_cpu(nes,NES_VERCTOR_IRQBRK);
}

/*
    P,PC:=+(S)
    N  V  U  B  D  I  Z  C
    *  *        *  *  *  *
*/
static inline void nes_rti(nes_t* nes){
    nes_dummy_read(nes);
    // P:=+(S)
    nes->nes_cpu.P = NES_POP(nes);
    nes->nes_cpu.U = 1;
    nes->nes_cpu.B = 1;
    // PC:=+(S)
    const uint8_t low_byte = (nes->nes_cpu.cpu_ram + 0x100)[++nes->nes_cpu.SP];
    const uint8_t high_byte = (nes->nes_cpu.cpu_ram + 0x100)[++nes->nes_cpu.SP];
    nes->nes_cpu.PC =  (uint16_t)high_byte << 8 | low_byte;
    // 清计数
    
}


/*
    (S)-:=PC PC:={adr}
    N  V  U  B  D  I  Z  C

*/
static inline void nes_jsr(nes_t* nes){
    const uint16_t address = nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes);
    nes_dummy_read(nes);
    NES_PUSHW(nes,nes->nes_cpu.PC-1);
    nes->nes_cpu.PC = address;
}


/*
    PC:=+(S)
    N  V  U  B  D  I  Z  C

*/
static inline void nes_rts(nes_t* nes){
    const uint8_t low_byte = (nes->nes_cpu.cpu_ram + 0x100)[++nes->nes_cpu.SP];
    const uint8_t high_byte = (nes->nes_cpu.cpu_ram + 0x100)[++nes->nes_cpu.SP];
    nes->nes_cpu.PC =  (uint16_t)high_byte << 8 | low_byte;
    nes_dummy_read(nes);
    nes_dummy_read(nes);
    nes->nes_cpu.PC++;
}

/*
    PC:={adr}
    N  V  U  B  D  I  Z  C

*/
static inline void nes_jmp(nes_t* nes){
    nes->nes_cpu.PC = nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes);
}

/*
    N:=b7 V:=b6 Z:=A&{adr}
    N  V  U  B  D  I  Z  C
    *  *              *  
*/
static inline void nes_bit(nes_t* nes){
    const uint8_t value = nes_read_cpu(nes,nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes));
    nes->nes_cpu.N = (value >> 7) & 1;
    nes->nes_cpu.V = (value >> 6) & 1;
    NES_CHECK_Z(value & nes->nes_cpu.A);
}

/*
    C:=0
    N  V  U  B  D  I  Z  C
                         0
*/
static inline void nes_clc(nes_t* nes){
    nes->nes_cpu.C=0;
}

/*
    C:=1
    N  V  U  B  D  I  Z  C
                         1
*/
static inline void nes_sec(nes_t* nes){
    nes->nes_cpu.C=1;
}

/*
    D:=0
    N  V  U  B  D  I  Z  C
                0         
*/
static inline void nes_cld(nes_t* nes){
    nes->nes_cpu.D=0;
}

/*
    D:=1
    N  V  U  B  D  I  Z  C
                1         
*/
static inline void nes_sed(nes_t* nes){
    nes->nes_cpu.D=1;
}

/*
    I:=0
    N  V  U  B  D  I  Z  C
                   0      
*/
static inline void nes_cli(nes_t* nes){
    nes->nes_cpu.I=0;
    // irq_counter
}

/*
    I:=1
    N  V  U  B  D  I  Z  C
                   1      
*/
static inline void nes_sei(nes_t* nes){
    nes->nes_cpu.I=1;
}

/*
    V:=0
    N  V  U  B  D  I  Z  C
       0                  
*/
static inline void nes_clv(nes_t* nes){
    nes->nes_cpu.V=0;
}

/*
    
    N  V  U  B  D  I  Z  C

*/
static inline void nes_nop(nes_t* nes){
    if (nes_opcode_table[nes->nes_cpu.opcode].addressing_mode) nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes);
}

/* Illegal opcodes: */

/*
    {adr}:={adr}*2 A:=A or {adr}	
    N  V  U  B  D  I  Z  C
    *                 *  *
*/
static inline void nes_slo(nes_t* nes){
    uint16_t address = nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes);
    uint8_t data = nes_read_cpu(nes,address);
    // asl
    nes->nes_cpu.C = data >> 7;
    data <<= 1;
    nes_write_cpu(nes,address,data);
    // ora
    nes->nes_cpu.A |= data;
    NES_CHECK_NZ(nes->nes_cpu.A);
}

/*
    {adr}:={adr}rol A:=A and {adr}
    N  V  U  B  D  I  Z  C
    *                 *  *
*/
static inline void nes_rla(nes_t* nes){
    uint16_t address = nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes);
    uint16_t data = nes_read_cpu(nes,address);
    // rol
    data <<= 1;
    data |= nes->nes_cpu.C;
    nes->nes_cpu.C = data >> 8;
    nes_write_cpu(nes,address,(uint8_t)data);
    // and
    nes->nes_cpu.A &= (uint8_t)data;
    NES_CHECK_NZ(nes->nes_cpu.A);
}

/*
    {adr}:={adr}/2 A:=A exor {adr}
    N  V  U  B  D  I  Z  C
    *                 *  *
*/
static inline void nes_sre(nes_t* nes){
    uint16_t address = nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes);
    uint8_t data = nes_read_cpu(nes,address);
    // lsr
    nes->nes_cpu.C = data & 0x01;
    data >>= 1;
    nes_write_cpu(nes,address,data);
    // eor
    nes->nes_cpu.A ^= data;
    NES_CHECK_NZ(nes->nes_cpu.A);
}

/*
    {adr}:={adr}ror A:=A adc {adr}
    N  V  U  B  D  I  Z  C
    *  *              *  *
*/
static inline void nes_rra(nes_t* nes){
    uint16_t address = nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes);
    uint16_t data = nes_read_cpu(nes,address);
    // ror
    data |= (nes->nes_cpu.C << 8);
    uint8_t cflag = data & 0x01;
    data >>= 1;
    nes_write_cpu(nes,address,(uint8_t)data);
    // adc
    const uint16_t data1 = nes->nes_cpu.A + data + cflag;
    nes->nes_cpu.C = data1 >> 8;
    if (!((nes->nes_cpu.A ^ data) & 0x80) && ((nes->nes_cpu.A ^ (uint8_t)data1) & 0x80)) nes->nes_cpu.V = 1;
    else nes->nes_cpu.V = 0;
    nes->nes_cpu.A = (uint8_t)data1;
    NES_CHECK_NZ(nes->nes_cpu.A);
}

/*
    {adr}:=A&X
    N  V  U  B  D  I  Z  C

*/
static inline void nes_sax(nes_t* nes){
    nes_write_cpu(nes,nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes),nes->nes_cpu.A & nes->nes_cpu.X);
}

/*
    A,X:={adr}
    N  V  U  B  D  I  Z  C
    *                 *  
*/
static inline void nes_lax(nes_t* nes){
    nes->nes_cpu.X = nes->nes_cpu.A = nes_read_cpu(nes,nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes));
    NES_CHECK_NZ(nes->nes_cpu.X);
}

/*
    {adr}:={adr}-1 A-{adr}
    N  V  U  B  D  I  Z  C
    *                 *  *
*/
static inline void nes_dcp(nes_t* nes){
    uint16_t address = nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes);
    uint8_t data = nes_read_cpu(nes,address);
    // dec
    data--;
    nes_write_cpu(nes,address,data);
    // cmp
    const uint16_t data1 = (uint16_t)nes->nes_cpu.A - (uint16_t)data;
    nes->nes_cpu.C = !(data1 >> 15);
    NES_CHECK_NZ((uint8_t)data1);
}

/*
    {adr}:={adr}+1 A:=A-{adr}
    N  V  U  B  D  I  Z  C
    *  *              *  *
*/
static inline void nes_isc(nes_t* nes){
    uint16_t address = nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes);
    uint8_t data = nes_read_cpu(nes,address);
    // inc
    nes_write_cpu(nes,address,++data);
    // sbc
    const uint16_t data1 = nes->nes_cpu.A - data - !nes->nes_cpu.C;
    nes->nes_cpu.C = !(data1 >> 8);
    const uint8_t data2 = (uint8_t)data1;

    if (((nes->nes_cpu.A ^ data) & 0x80) && ((nes->nes_cpu.A ^ data2) & 0x80)) nes->nes_cpu.V = 1;
    else nes->nes_cpu.V = 0;

    nes->nes_cpu.A = data2;
    NES_CHECK_NZ(nes->nes_cpu.A);
}

/*
    A:=A&#{imm}
    N  V  U  B  D  I  Z  C
    *                 *  *
*/
static inline void nes_anc(nes_t* nes){
    nes->nes_cpu.A &= nes_read_cpu(nes,nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes));
    NES_CHECK_NZ(nes->nes_cpu.A);
    nes->nes_cpu.C = nes->nes_cpu.A >> 7;
}

/*
    A:=(A&#{imm})/2
    N  V  U  B  D  I  Z  C
    *                 *  *
*/
static inline void nes_alr(nes_t* nes){
    nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes);
}

/*
    A:=(A&#{imm})/2
    N  V  U  B  D  I  Z  C
    *  *              *  *
*/
static inline void nes_arr(nes_t* nes){
    nes->nes_cpu.A &= nes_read_cpu(nes,nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes));
    nes->nes_cpu.A = (nes->nes_cpu.A>>1)|(nes->nes_cpu.C<<7);
    NES_CHECK_NZ(nes->nes_cpu.A);
    nes->nes_cpu.C = (nes->nes_cpu.A >> 6)&1;
    if (((nes->nes_cpu.A >> 5) ^ (nes->nes_cpu.A >> 6)) & 1) nes->nes_cpu.V = 1;
    else nes->nes_cpu.V = 0;
}

/*
    A:=X&#{imm}
    N  V  U  B  D  I  Z  C
    *                 *  
*/
static inline void nes_xaa(nes_t* nes){
    nes->nes_cpu.A = nes->nes_cpu.X & nes_read_cpu(nes,nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes));
    NES_CHECK_NZ(nes->nes_cpu.A);
    nes->nes_cpu.C = nes->nes_cpu.A >> 7;
}

/*
    X:=A&X-#{imm}
    N  V  U  B  D  I  Z  C
    *                 *  *
*/
static inline void nes_axs(nes_t* nes){
    uint16_t data = (nes->nes_cpu.A & nes->nes_cpu.X) - nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes);
    nes->nes_cpu.X = (uint8_t)data;
    NES_CHECK_NZ(nes->nes_cpu.X);
    nes->nes_cpu.C = !(data >> 15);
}

/*
    {adr}:=A&X&H
    N  V  U  B  D  I  Z  C

*/
static inline void nes_ahx(nes_t* nes){
    nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes);
}

/*
    {adr}:=Y&H
    N  V  U  B  D  I  Z  C

*/
static inline void nes_shy(nes_t* nes){
    nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes);
}

/*
    {adr}:=X&H
    N  V  U  B  D  I  Z  C

*/
static inline void nes_shx(nes_t* nes){
    nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes);
}

/*
    S:=A&X {adr}:=S&H
    N  V  U  B  D  I  Z  C

*/
static inline void nes_tas(nes_t* nes){
    nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes);
    
}

/*
    A,X,S:={adr}&S
    N  V  U  B  D  I  Z  C
    *                 *  
*/
static inline void nes_las(nes_t* nes){
    nes_opcode_table[nes->nes_cpu.opcode].addressing_mode(nes);
}

/*
    Hardware vectors:
    $FFFA = NMI vector (NMI=not maskable interupts)
    $FFFC = Reset vector
    $FFFE = IRQ vector
*/

static inline void nes_nmi(nes_t* nes){
    NES_PUSHW(nes,nes->nes_cpu.PC);
    nes->nes_cpu.U = 1;
    nes->nes_cpu.B = 0;
    NES_PUSH(nes,nes->nes_cpu.P);
    nes->nes_cpu.I = 1;
    nes->nes_cpu.PC = nes_readw_cpu(nes,NES_VERCTOR_NMI);
    nes->nes_cpu.cycles += 7;
}

void nes_cpu_irq(nes_t* nes){
    if (nes->nes_cpu.I==0){
        NES_PUSHW(nes,nes->nes_cpu.PC);
        NES_PUSH(nes,nes->nes_cpu.P);
        // nes->nes_cpu.B = 0;
        nes->nes_cpu.I = 1;
        nes->nes_cpu.PC = nes_readw_cpu(nes,NES_VERCTOR_IRQBRK);
        nes->nes_cpu.cycles += 7;
    }
}

// https://www.nesdev.org/wiki/CPU_power_up_state#After_reset
void nes_cpu_reset(nes_t* nes){
    nes->nes_cpu.I = 1;                 // The I (IRQ disable) flag was set to true
    nes->nes_cpu.SP -= 3;               // S was decremented by 3 (but nothing was written to the stack)
    nes_write_cpu(nes,0x4015, 0x00);    // APU was silenced ($4015 = 0)

    nes->nes_cpu.PC = nes_readw_cpu(nes,NES_VERCTOR_RESET);
    nes->nes_cpu.cycles = 7;
}

// https://www.nesdev.org/wiki/CPU_power_up_state#At_power-up
void nes_cpu_init(nes_t* nes){
    // Status: Carry, Zero, Decimal, Overflow, Negative clear. Interrupt Disable set.
    // A, X, Y = 0
    nes->nes_cpu.A = nes->nes_cpu.X = nes->nes_cpu.Y = nes->nes_cpu.P = 0;
    nes->nes_cpu.U = nes->nes_cpu.I = 1;
    nes->nes_cpu.SP = 0x00;             // reset: S = $00-$03 = $FD
}

// https://www.nesdev.org/wiki/CPU_unofficial_opcodes
// https://www.oxyron.de/html/opcodes02.html

static const nes_opcode_t nes_opcode_table[256] = {
    {nes_brk,	NULL,	    7   },      // 0x00     BRK         7
    {nes_ora,   nes_izx,    6   },      // 0x01     ORA IZX     6
    {NULL,	    NULL,	    0   },      // 0x02     KIL         0
    {nes_slo,	nes_izx,	8   },      // 0x03     SLO IZX     8
    {nes_nop,	nes_zp,	    3   },      // 0x04     NOP ZP      3
    {nes_ora,	nes_zp,	    3   },      // 0x05     ORA ZP      3
    {nes_asl,	nes_zp,	    5   },      // 0x06     ASL ZP      5
    {nes_slo,	nes_zp,	    5   },      // 0x07     SLO ZP      5
    {nes_php,	NULL,	    3   },      // 0x08     PHP         3
    {nes_ora,	nes_imm,	2   },      // 0x09     ORA IMM     2
    {nes_asl,	NULL,	    2   },      // 0x0A     ASL         2
    {nes_anc,	nes_imm,	2   },      // 0x0B     ANC IMM     2
    {nes_nop,	nes_abs,	4   },      // 0x0C     NOP ABS     4
    {nes_ora,	nes_abs,	4   },      // 0x0D     ORA ABS     4
    {nes_asl,	nes_abs,	6   },      // 0x0E     ASL ABS     6
    {nes_slo,	nes_abs,	6   },      // 0x0F     SLO ABS     6
    {nes_bpl,	nes_rel,	2   },      // 0x10     BPL REL     2*
    {nes_ora,   nes_izy,    5   },      // 0x11     ORA IZY     5*
    {NULL,      NULL,	    0   },      // 0x12     KIL         0
    {nes_slo,	nes_izy,	8   },      // 0x13     SLO IZY     8
    {nes_nop,	nes_zpx,	4   },      // 0x14     NOP ZPX     4
    {nes_ora,	nes_zpx,	4   },      // 0x15     ORA ZPX     4
    {nes_asl,	nes_zpx,	6   },      // 0x16     ASL ZPX     6
    {nes_slo,	nes_zpx,	6   },      // 0x17     SLO ZPX     6
    {nes_clc,	NULL,	    2   },      // 0x18     CLC         2
    {nes_ora,	nes_aby,	4   },      // 0x19     ORA ABY     4*
    {nes_nop,	NULL,	    2   },      // 0x1A     NOP         2
    {nes_slo,	nes_aby,	7   },      // 0x1B     SLO ABY     7
    {nes_nop,	nes_abx,	4   },      // 0x1C     NOP ABX     4*
    {nes_ora,	nes_abx,	4   },      // 0x1D     ORA ABX     4*
    {nes_asl,	nes_abx,	7   },      // 0x1E     ASL ABX     7
    {nes_slo,	nes_abx,	7   },      // 0x1F     SLO ABX     7
    {nes_jsr,	nes_abs,	6   },      // 0x20     JSR ABS     6
    {nes_and,   nes_izx,    6   },      // 0x21     AND IZX     6
    {NULL,      NULL,	    0   },      // 0x22     KIL         0
    {nes_rla,	nes_izx,	8   },      // 0x23     RLA IZX     8
    {nes_bit,	nes_zp,	    3   },      // 0x24     BIT ZP      3
    {nes_and,	nes_zp,     3   },      // 0x25     AND ZP      3
    {nes_rol,	nes_zp,     5   },      // 0x26     ROL ZP      5
    {nes_rla,	nes_zp,     5   },      // 0x27     RLA ZP      5
    {nes_plp,	NULL,	    4   },      // 0x28     PLP         4
    {nes_and,	nes_imm,	2   },      // 0x29     AND IMM     2
    {nes_rol,	NULL,	    2   },      // 0x2A     ROL         2
    {nes_anc,	nes_imm,	2   },      // 0x2B     ANC IMM     2
    {nes_bit,	nes_abs,	4   },      // 0x2C     BIT ABS     4
    {nes_and,	nes_abs,	4   },      // 0x2D     AND ABS     4
    {nes_rol,	nes_abs,	6   },      // 0x2E     ROL ABS     6
    {nes_rla,	nes_abs,	6   },      // 0x2F     RLA ABS     6
    {nes_bmi,	nes_rel,	2   },      // 0x30     BMI REL     2*
    {nes_and,   nes_izy,    5   },      // 0x31     AND IZY     5*
    {NULL,      NULL,	    0   },      // 0x32     KIL         0
    {nes_rla,	nes_izy,	8   },      // 0x33     RLA IZY     8
    {nes_nop,	nes_zpx,	4   },      // 0x34     NOP ZPX     4
    {nes_and,	nes_zpx,    4   },      // 0x35     AND ZPX     4
    {nes_rol,	nes_zpx,    6   },      // 0x36     ROL ZPX     6
    {nes_rla,	nes_zpx,    6   },      // 0x37     RLA ZPX     6
    {nes_sec,	NULL,	    2   },      // 0x38     SEC         2
    {nes_and,	nes_aby,	4   },      // 0x39     AND ABY     4*
    {nes_nop,	NULL,	    2   },      // 0x3A     NOP         2
    {nes_rla,	nes_aby,	7   },      // 0x3B     RLA ABY     7
    {nes_nop,	nes_abx,	4   },      // 0x3C     NOP ABX     4*
    {nes_and,	nes_abx,	4   },      // 0x3D     AND ABX     4*
    {nes_rol,	nes_abx,	7   },      // 0x3E     ROL ABX     7
    {nes_rla,	nes_abx,	7   },      // 0x3F     RLA ABX     7
    {nes_rti,	NULL,	    6   },      // 0x40     RTI         6
    {nes_eor,   nes_izx,    6   },      // 0x41     EOR IZX     6
    {NULL,      NULL,	    0   },      // 0x42     KIL         0
    {nes_sre,	nes_izx,	8   },      // 0x43     SRE IZX     8
    {nes_nop,	nes_zp,	    3   },      // 0x44     NOP ZP      3
    {nes_eor,	nes_zp,     3   },      // 0x45     EOR ZP      3
    {nes_lsr,	nes_zp,     5   },      // 0x46     LSR ZP      5
    {nes_sre,	nes_zp,     5   },      // 0x47     SRE ZP      5
    {nes_pha,	NULL,	    3   },      // 0x48     PHA         3
    {nes_eor,	nes_imm,	2   },      // 0x49     EOR IMM     2
    {nes_lsr,	NULL,	    2   },      // 0x4A     LSR         2
    {nes_alr,	nes_imm,	2   },      // 0x4B     ALR IMM     2
    {nes_jmp,	nes_abs,	3   },      // 0x4C     JMP ABS     3
    {nes_eor,	nes_abs,	4   },      // 0x4D     EOR ABS     4
    {nes_lsr,	nes_abs,	6   },      // 0x4E     LSR ABS     6
    {nes_sre,	nes_abs,	6   },      // 0x4F     SRE ABS     6
    {nes_bvc,	nes_rel,	2   },      // 0x50     BVC REL     2*
    {nes_eor,   nes_izy,    5   },      // 0x51     EOR IZY     5*
    {NULL,      NULL,	    0   },      // 0x52     KIL         0
    {nes_sre,	nes_izy,	8   },      // 0x53     SRE IZY     8
    {nes_nop,	nes_zpx,	4   },      // 0x54     NOP ZPX     4
    {nes_eor,	nes_zpx,    4   },      // 0x55     EOR ZPX     4
    {nes_lsr,	nes_zpx,    6   },      // 0x56     LSR ZPX     6
    {nes_sre,	nes_zpx,    6   },      // 0x57     SRE ZPX     6
    {nes_cli,	NULL,	    2   },      // 0x58     CLI         2
    {nes_eor,	nes_aby,	4   },      // 0x59     EOR ABY     4*
    {nes_nop,	NULL,	    2   },      // 0x5A     NOP         2
    {nes_sre,	nes_aby,	7   },      // 0x5B     SRE ABY     7
    {nes_nop,	nes_abx,	4   },      // 0x5C     NOP ABX     4*
    {nes_eor,	nes_abx,	4   },      // 0x5D     EOR ABX     4*
    {nes_lsr,	nes_abx,	7   },      // 0x5E     LSR ABX     7
    {nes_sre,	nes_abx,	7   },      // 0x5F     SRE ABX     7
    {nes_rts,	NULL,   	6   },      // 0x60     RTS         6
    {nes_adc,   nes_izx,    6   },      // 0x61     ADC IZX     6
    {NULL,      NULL,	    0   },      // 0x62     KIL 
    {nes_rra,	nes_izx,	8   },      // 0x63     RRA IZX     8
    {nes_nop,	nes_zp,	    3   },      // 0x64     NOP ZP      3
    {nes_adc,	nes_zp,     3   },      // 0x65     ADC ZP      3
    {nes_ror,	nes_zp,     5   },      // 0x66     ROR ZP      5
    {nes_rra,	nes_zp,     5   },      // 0x67     RRA ZP      5
    {nes_pla,	NULL,	    4   },      // 0x68     PLA         4
    {nes_adc,	nes_imm,	2   },      // 0x69     ADC IMM     2
    {nes_ror,	NULL,	    2   },      // 0x6A     ROR         2
    {nes_arr,	nes_imm,	2   },      // 0x6B     ARR IMM     2
    {nes_jmp,	nes_ind,	5   },      // 0x6C     JMP IND     5
    {nes_adc,	nes_abs,	4   },      // 0x6D     ADC ABS     4
    {nes_ror,	nes_abs,	6   },      // 0x6E     ROR ABS     6
    {nes_rra,	nes_abs,	6   },      // 0x6F     RRA ABS     6
    {nes_bvs,	nes_rel,   	2   },      // 0x70     BVS REL     2*
    {nes_adc,   nes_izy,    5   },      // 0x71     ADC IZY     5*
    {NULL,      NULL,	    0   },      // 0x72     KIL         0
    {nes_rra,	nes_izy,	8   },      // 0x73     RRA IZY     8
    {nes_nop,	nes_zpx,	4   },      // 0x74     NOP ZPX     4
    {nes_adc,	nes_zpx,    4   },      // 0x75     ADC ZPX     4
    {nes_ror,	nes_zpx,    6   },      // 0x76     ROR ZPX     6
    {nes_rra,	nes_zpx,    6   },      // 0x77     RRA ZPX     6
    {nes_sei,	NULL,	    2   },      // 0x78     SEI         2
    {nes_adc,	nes_aby,	4   },      // 0x79     ADC ABY     4*
    {nes_nop,	NULL,	    2   },      // 0x7A     NOP         2
    {nes_rra,	nes_aby,	7   },      // 0x7B     RRA ABY     7
    {nes_nop,	nes_abx,	4   },      // 0x7C     NOP ABX     4*
    {nes_adc,	nes_abx,	4   },      // 0x7D     ADC ABX     4*
    {nes_ror,	nes_abx,	7   },      // 0x7E     ROR ABX     7
    {nes_rra,	nes_abx,	7   },      // 0x7F     RRA ABX     7
    {nes_nop,	nes_imm,   	2   },      // 0x80     NOP IMM     2
    {nes_sta,   nes_izx,    6   },      // 0x81     STA IZX     6
    {nes_nop,   nes_imm,	2   },      // 0x82     NOP IMM     2
    {nes_sax,	nes_izx,	6   },      // 0x83     SAX IZX     6
    {nes_sty,	nes_zp,	    3   },      // 0x84     STY ZP      3
    {nes_sta,	nes_zp,     3   },      // 0x85     STA ZP      3
    {nes_stx,	nes_zp,     3   },      // 0x86     STX ZP      3
    {nes_sax,	nes_zp,     3   },      // 0x87     SAX ZP      3
    {nes_dey,	NULL,	    2   },      // 0x88     DEY         2
    {nes_nop,	nes_imm,	2   },      // 0x89     NOP IMM     2
    {nes_txa,	NULL,	    2   },      // 0x8A     TXA         2
    {nes_xaa,	nes_imm,	2   },      // 0x8B     XAA IMM     2
    {nes_sty,	nes_abs,	4   },      // 0x8C     STY ABS     4
    {nes_sta,	nes_abs,	4   },      // 0x8D     STA ABS     4
    {nes_stx,	nes_abs,	4   },      // 0x8E     STX ABS     4
    {nes_sax,	nes_abs,	4   },      // 0x8F     SAX ABS     4
    {nes_bcc,	nes_rel,   	2   },      // 0x90     BCC REL     2*
    {nes_sta,   nes_izy,    6   },      // 0x91     STA IZY     6
    {NULL,      NULL,	    0   },      // 0x92     KIL         0
    {nes_ahx,	nes_izy,	6   },      // 0x93     AHX IZY     6
    {nes_sty,	nes_zpx,	4   },      // 0x94     STY ZPX     4
    {nes_sta,	nes_zpx,    4   },      // 0x95     STA ZPX     4
    {nes_stx,	nes_zpy,    4   },      // 0x96     STX ZPY     4
    {nes_sax,	nes_zpy,    4   },      // 0x97     SAX ZPY     4
    {nes_tya,	NULL,	    2   },      // 0x98     TYA         2
    {nes_sta,	nes_aby,	5   },      // 0x99     STA ABY     5
    {nes_txs,	NULL,	    2   },      // 0x9A     TXS         2
    {nes_tas,	nes_aby,	5   },      // 0x9B     TAS ABY     5
    {nes_shy,	nes_abx,	5   },      // 0x9C     SHY ABX     5
    {nes_sta,	nes_abx,	5   },      // 0x9D     STA ABX     5
    {nes_shx,	nes_aby,	5   },      // 0x9E     SHX ABY     5
    {nes_ahx,	nes_aby,	5   },      // 0x9F     AHX ABY     5
    {nes_ldy,	nes_imm,   	2   },      // 0xA0     LDY IMM     2
    {nes_lda,   nes_izx,    6   },      // 0xA1     LDA IZX     6
    {nes_ldx,   nes_imm,	2   },      // 0xA2     LDX IMM     2
    {nes_lax,	nes_izx,	6   },      // 0xA3     LAX IZX     6
    {nes_ldy,	nes_zp,	    3   },      // 0xA4     LDY ZP      3
    {nes_lda,	nes_zp,     3   },      // 0xA5     LDA ZP      3
    {nes_ldx,	nes_zp,     3   },      // 0xA6     LDX ZP      3
    {nes_lax,	nes_zp,     3   },      // 0xA7     LAX ZP      3
    {nes_tay,	NULL,	    2   },      // 0xA8     TAY         2
    {nes_lda,	nes_imm,	2   },      // 0xA9     LDA IMM     2
    {nes_tax,	NULL,	    2   },      // 0xAA     TAX         2
    {nes_lax,	nes_imm,	2   },      // 0xAB     LAX IMM     2
    {nes_ldy,	nes_abs,	4   },      // 0xAC     LDY ABS     4
    {nes_lda,	nes_abs,	4   },      // 0xAD     LDA ABS     4
    {nes_ldx,	nes_abs,	4   },      // 0xAE     LDX ABS     4
    {nes_lax,	nes_abs,	4   },      // 0xAF     LAX ABS     4
    {nes_bcs,	nes_rel,   	2   },      // 0xB0     BCS REL     2*
    {nes_lda,   nes_izy,    5   },      // 0xB1     LDA IZY     5*
    {NULL,      NULL,	    0   },      // 0xB2     KIL         0
    {nes_lax,	nes_izy,	5   },      // 0xB3     LAX IZY     5*
    {nes_ldy,	nes_zpx,	4   },      // 0xB4     LDY ZPX     4
    {nes_lda,	nes_zpx,    4   },      // 0xB5     LDA ZPX     4
    {nes_ldx,	nes_zpy,    4   },      // 0xB6     LDX ZPY     4
    {nes_lax,	nes_zpy,    4   },      // 0xB7     LAX ZPY     4
    {nes_clv,	NULL,	    2   },      // 0xB8     CLV         2
    {nes_lda,	nes_aby,	4   },      // 0xB9     LDA ABY     4*
    {nes_tsx,	NULL,	    2   },      // 0xBA     TSX         2
    {nes_las,	nes_aby,	4   },      // 0xBB     LAS ABY     4*
    {nes_ldy,	nes_abx,	4   },      // 0xBC     LDY ABX     4*
    {nes_lda,	nes_abx,	4   },      // 0xBD     LDA ABX     4*
    {nes_ldx,	nes_aby,	4   },      // 0xBE     LDX ABY     4*
    {nes_lax,	nes_aby,	4   },      // 0xBF     LAX ABY     4*
    {nes_cpy,	nes_imm,   	2   },      // 0xC0     CPY IMM     2
    {nes_cmp,   nes_izx,    6   },      // 0xC1     CMP IZX     6
    {nes_nop,   nes_imm,	2   },      // 0xC2     NOP IMM     2
    {nes_dcp,	nes_izx,	8   },      // 0xC3     DCP IZX     8
    {nes_cpy,	nes_zp,  	3   },      // 0xC4     CPY ZP      3
    {nes_cmp,	nes_zp,     3   },      // 0xC5     CMP ZP      3
    {nes_dec,	nes_zp,     5   },      // 0xC6     DEC ZP      5
    {nes_dcp,	nes_zp,     5   },      // 0xC7     DCP ZP      5
    {nes_iny,	NULL,	    2   },      // 0xC8     INY         2
    {nes_cmp,	nes_imm,	2   },      // 0xC9     CMP IMM     2
    {nes_dex,	NULL,	    2   },      // 0xCA     DEX         2
    {nes_axs,	nes_imm,	2   },      // 0xCB     AXS IMM     2
    {nes_cpy,	nes_abs,	4   },      // 0xCC     CPY ABS     4
    {nes_cmp,	nes_abs,	4   },      // 0xCD     CMP ABS     4
    {nes_dec,	nes_abs,	6   },      // 0xCE     DEC ABS     6
    {nes_dcp,	nes_abs,	6   },      // 0xCF     DCP ABS     6
    {nes_bne,	nes_rel,   	2   },      // 0xD0     BNE REL     2*
    {nes_cmp,   nes_izy,    5   },      // 0xD1     CMP IZY     5*
    {NULL,      NULL,	    0   },      // 0xD2     KIL         0
    {nes_dcp,	nes_izy,	8   },      // 0xD3     DCP IZY     8
    {nes_nop,	nes_zpx,  	4   },      // 0xD4     NOP ZPX     4
    {nes_cmp,	nes_zpx,    4   },      // 0xD5     CMP ZPX     4
    {nes_dec,	nes_zpx,    6   },      // 0xD6     DEC ZPX     6
    {nes_dcp,	nes_zpx,    6   },      // 0xD7     DCP ZPX     6
    {nes_cld,	NULL,	    2   },      // 0xD8     CLD         2
    {nes_cmp,	nes_aby,	4   },      // 0xD9     CMP ABY     4*
    {nes_nop,	NULL,	    2   },      // 0xDA     NOP         2
    {nes_dcp,	nes_aby,	7   },      // 0xDB     DCP ABY     7
    {nes_nop,	nes_abx,	4   },      // 0xDC     NOP ABX     4*
    {nes_cmp,	nes_abx,	4   },      // 0xDD     CMP ABX     4*
    {nes_dec,	nes_abx,	7   },      // 0xDE     DEC ABX     7
    {nes_dcp,	nes_abx,	7   },      // 0xDF     DCP ABX     7
    {nes_cpx,	nes_imm,   	2   },      // 0xE0     CPX IMM     2
    {nes_sbc,   nes_izx,    6   },      // 0xE1     SBC IZX     6
    {nes_nop,   nes_imm,	2   },      // 0xE2     NOP IMM     2
    {nes_isc,	nes_izx,	8   },      // 0xE3     ISC IZX     8
    {nes_cpx,	nes_zp,  	3   },      // 0xE4     CPX ZP      3
    {nes_sbc,	nes_zp,     3   },      // 0xE5     SBC ZP      3
    {nes_inc,	nes_zp,     5   },      // 0xE6     INC ZP      5
    {nes_isc,	nes_zp,     5   },      // 0xE7     ISC ZP      5
    {nes_inx,	NULL,	    2   },      // 0xE8     INX         2
    {nes_sbc,	nes_imm,	2   },      // 0xE9     SBC IMM     2
    {nes_nop,	NULL,	    2   },      // 0xEA     NOP         2
    {nes_sbc,	nes_imm,	2   },      // 0xEB     SBC IMM     2
    {nes_cpx,	nes_abs,	4   },      // 0xEC     CPX ABS     4
    {nes_sbc,	nes_abs,	4   },      // 0xED     SBC ABS     4
    {nes_inc,	nes_abs,	6   },      // 0xEE     INC ABS     6
    {nes_isc,	nes_abs,	6   },      // 0xEF     ISC ABS     6
    {nes_beq,	nes_rel,   	2   },      // 0xF0     BEQ REL     2*
    {nes_sbc,   nes_izy,    5   },      // 0xF1     SBC IZY     5*
    {NULL,      NULL,	    0   },      // 0xF2     KIL         0
    {nes_isc,	nes_izy,	8   },      // 0xF3     ISC IZY     8
    {nes_nop,	nes_zpx,  	4   },      // 0xF4     NOP ZPX     4
    {nes_sbc,	nes_zpx,    4   },      // 0xF5     SBC ZPX     4
    {nes_inc,	nes_zpx,    6   },      // 0xF6     INC ZPX     6
    {nes_isc,	nes_zpx,    6   },      // 0xF7     ISC ZPX     6
    {nes_sed,	NULL,	    2   },      // 0xF8     SED         2
    {nes_sbc,	nes_aby,	4   },      // 0xF9     SBC ABY     4*
    {nes_nop,	NULL,	    2   },      // 0xFA     NOP         2
    {nes_isc,	nes_aby,	7   },      // 0xFB     ISC ABY     7
    {nes_nop,	nes_abx,	4   },      // 0xFC     NOP ABX     4*
    {nes_sbc,	nes_abx,	4   },      // 0xFD     SBC ABX     4*
    {nes_inc,	nes_abx,	7   },      // 0xFE     INC ABX     7
    {nes_isc,	nes_abx,	7   },      // 0xFF     ISC ABX     7
};

#ifdef __DEBUG__

static char* nes_opcode_name[256] = {
    "BRK    ","ORA IZX","KIL    ","SLO IZX","NOP ZP ","ORA ZP ","ASL ZP ","SLO ZP ","PHP","ORA IMM","ASL","ANC IMM","NOP ABS","ORA ABS","ASL ABS","SLO ABS",
    "BPL REL","ORA IZY","KIL    ","SLO IZY","NOP ZPX","ORA ZPX","ASL ZPX","SLO ZPX","CLC","ORA ABY","NOP","SLO ABY","NOP ABX","ORA ABX","ASL ABX","SLO ABX",
    "JSR ABS","AND IZX","KIL    ","RLA IZX","BIT ZP ","AND ZP ","ROL ZP ","RLA ZP ","PLP","AND IMM","ROL","ANC IMM","BIT ABS","AND ABS","ROL ABS","RLA ABS",
    "BMI REL","AND IZY","KIL    ","RLA IZY","NOP ZPX","AND ZPX","ROL ZPX","RLA ZPX","SEC","AND ABY","NOP","RLA ABY","NOP ABX","AND ABX","ROL ABX","RLA ABX",
    "RTI    ","EOR IZX","KIL    ","SRE IZX","NOP ZP ","EOR ZP ","LSR ZP ","SRE ZP ","PHA","EOR IMM","LSR","ALR IMM","JMP ABS","EOR ABS","LSR ABS","SRE ABS",
    "BVC REL","EOR IZY","KIL    ","SRE IZY","NOP ZPX","EOR ZPX","LSR ZPX","SRE ZPX","CLI","EOR ABY","NOP","SRE ABY","NOP ABX","EOR ABX","LSR ABX","SRE ABX",
    "RTS    ","ADC IZX","KIL    ","RRA IZX","NOP ZP ","ADC ZP ","ROR ZP ","RRA ZP ","PLA","ADC IMM","ROR","ARR IMM","JMP IND","ADC ABS","ROR ABS","RRA ABS",
    "BVS REL","ADC IZY","KIL    ","RRA IZY","NOP ZPX","ADC ZPX","ROR ZPX","RRA ZPX","SEI","ADC ABY","NOP","RRA ABY","NOP ABX","ADC ABX","ROR ABX","RRA ABX",
    "NOP IMM","STA IZX","NOP IMM","SAX IZX","STY ZP ","STA ZP ","STX ZP ","SAX ZP ","DEY","NOP IMM","TXA","XAA IMM","STY ABS","STA ABS","STX ABS","SAX ABS",
    "BCC REL","STA IZY","KIL    ","AHX IZY","STY ZPX","STA ZPX","STX ZPY","SAX ZPY","TYA","STA ABY","TXS","TAS ABY","SHY ABX","STA ABX","SHX ABY","AHX ABY",
    "LDY IMM","LDA IZX","LDX IMM","LAX IZX","LDY ZP ","LDA ZP ","LDX ZP ","LAX ZP ","TAY","LDA IMM","TAX","LAX IMM","LDY ABS","LDA ABS","LDX ABS","LAX ABS",
    "BCS REL","LDA IZY","KIL    ","LAX IZY","LDY ZPX","LDA ZPX","LDX ZPY","LAX ZPY","CLV","LDA ABY","TSX","LAS ABY","LDY ABX","LDA ABX","LDX ABY","LAX ABY",
    "CPY IMM","CMP IZX","NOP IMM","DCP IZX","CPY ZP ","CMP ZP ","DEC ZP ","DCP ZP ","INY","CMP IMM","DEX","AXS IMM","CPY ABS","CMP ABS","DEC ABS","DCP ABS",
    "BNE REL","CMP IZY","KIL    ","DCP IZY","NOP ZPX","CMP ZPX","DEC ZPX","DCP ZPX","CLD","CMP ABY","NOP","DCP ABY","NOP ABX","CMP ABX","DEC ABX","DCP ABX",
    "CPX IMM","SBC IZX","NOP IMM","ISC IZX","CPX ZP ","SBC ZP ","INC ZP ","ISC ZP ","INX","SBC IMM","NOP","SBC IMM","CPX ABS","SBC ABS","INC ABS","ISC ABS",
    "BEQ REL","SBC IZY","KIL    ","ISC IZY","NOP ZPX","SBC ZPX","INC ZPX","ISC ZPX","SED","SBC ABY","NOP","ISC ABY","NOP ABX","SBC ABX","INC ABX","ISC ABX",
};
static uint64_t cycles = 7;

extern FILE * debug_fp;
uint8_t cycles_old = 0;
#endif


void nes_opcode(nes_t* nes,uint16_t ticks){
    if (nes->nes_cpu.irq_nmi) {
        nes_nmi(nes);
        nes->nes_cpu.irq_nmi = 0;
    }
    while (ticks > nes->nes_cpu.cycles){
#ifdef __DEBUG__
        // fprintf(debug_fp,"A:0x%02X X:0x%02X Y:0x%02X SP:0x%02X \nP:0x%02X \nC:0x%02X Z:0x%02X I:0x%02X D:0x%02X B:0x%02X V:0x%02X N:0x%02X \n",
        //         nes->nes_cpu.A,nes->nes_cpu.X,nes->nes_cpu.Y,nes->nes_cpu.SP,
        //         nes->nes_cpu.P,nes->nes_cpu.C,nes->nes_cpu.Z,nes->nes_cpu.I,nes->nes_cpu.D,nes->nes_cpu.B,nes->nes_cpu.V,nes->nes_cpu.N);
        // fprintf(debug_fp,"PC: 0x%04X cycles:%lld \n", nes->nes_cpu.PC,cycles);
        // if (cycles == 56955){
        //     printf("cycles");
        //     printf("cycles");
        //     printf("cycles");
        // }
        // cycles_old = nes->nes_cpu.cycles;
#endif
        nes->nes_cpu.opcode = nes_read_cpu(nes,nes->nes_cpu.PC++);
        nes_opcode_table[nes->nes_cpu.opcode].instruction(nes);
        nes->nes_cpu.cycles += nes_opcode_table[nes->nes_cpu.opcode].ticks;
#ifdef __DEBUG__
        // cycles += nes->nes_cpu.cycles - cycles_old;
        // fprintf(debug_fp,"\nopcode: %s \n",nes_opcode_name[nes->nes_cpu.opcode]);
#endif
    }
    nes->nes_cpu.cycles -= ticks;
}


