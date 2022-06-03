#include "nes_cpu.h"
#include "nes_rom.h"

uint8_t CPU_RAM[0x800];
uint8_t prg_banks[0x8000];

// uint32_t cycles = 7;
nes_cpu6502_t nes_cpu = {
    .cycles = 7,
    .P={
        .U = 1,
    }
};

#define NES_PUSH(a)   nes_write_cpu( 0x100 + nes_cpu.SP--,(a))
#define NES_PUSHW(a)  NES_PUSH( (a) >> 8 ); NES_PUSH( (a) & 0xff )
#define NES_POP(a)    a = nes_read_cpu( 0x100 + ++nes_cpu.SP )
#define NES_POPW(a)   NES_POP(a); a |= ( nes_read_cpu( 0x100 + ++nes_cpu.SP ) << 8 )

/* Adressing modes */

/* Implied */
static inline uint16_t nes_implied(void){
    return 0;
}

//#Immediate
static inline uint16_t nes_imm(void){
    return nes_cpu.PC++;
}

static inline uint16_t nes_rel(void){
    uint16_t address = nes_read_cpu(nes_cpu.PC++);
    if(address & 0x80)address-=0x100;
    if((address>>8)!=(nes_cpu.PC>>8))nes_cpu.cycles++;
    return address;
}

//ABS
static inline uint16_t nes_abs(void){
    const uint16_t address = nes_read_cpu_word(nes_cpu.PC);
    nes_cpu.PC += 2;
    return address;
}

//ABX
static inline uint16_t nes_abx(void){
    uint16_t address = nes_read_cpu_word(nes_cpu.PC);
    nes_cpu.PC += 2;
    //改
    if ((address>>8) != ((address+nes_cpu.X)>>8))nes_cpu.cycles++;
    address += nes_cpu.X;
    return address;
}

//ABY
static inline uint16_t nes_aby(void){
    uint16_t address = nes_read_cpu_word(nes_cpu.PC);
    nes_cpu.PC += 2;
    //改
    if ((address>>8) != ((address+nes_cpu.Y)>>8))nes_cpu.cycles++;
    return address + nes_cpu.Y;
}

static inline uint16_t nes_zp(void){
    const uint16_t address = nes_read_cpu(nes_cpu.PC++);
    return address;
}

static inline uint16_t nes_zpx(void){
    uint16_t address = nes_read_cpu(nes_cpu.PC++);
    address += nes_cpu.X;
    address &= 0x00FF;
    return address;
}

static inline uint16_t nes_zpy(void){
    uint16_t address = nes_read_cpu(nes_cpu.PC++);
    address += nes_cpu.Y;
    address &= 0x00FF;
    return address;
}

static inline uint16_t nes_izx(void){
    uint8_t address = nes_read_cpu(nes_cpu.PC++);
    address += nes_cpu.X;
    return nes_read_cpu(address)|(uint16_t)nes_read_cpu((uint8_t)(address + 1)) << 8;
}

static inline uint16_t nes_izy(void){
    uint8_t value = nes_read_cpu(nes_cpu.PC++);
    uint16_t address = nes_read_cpu(value)|(uint16_t)nes_read_cpu((uint8_t)(value + 1)) << 8;
    if ((address>>8) != ((address+nes_cpu.Y)>>8))nes_cpu.cycles++;
    return address + nes_cpu.Y;
}

static inline uint16_t nes_ind(void){
    uint16_t value1 = nes_read_cpu(nes_cpu.PC)|(uint16_t)nes_read_cpu((nes_cpu.PC + 1)) << 8;
    // 6502 BUG
    const uint16_t value2 = (value1 & (uint16_t)0xFF00)| ((value1 + 1) & (uint16_t)0x00FF);

    uint16_t address = nes_read_cpu(value1)|(uint16_t)nes_read_cpu(value2) << 8;
    nes_cpu.PC+=2;
    return address;
}


/* Logical and arithmetic commands: */

// A :=A or {adr}
static inline void nes_ora(uint16_t address){
    nes_cpu.A |= nes_read_cpu(address);
    if (nes_cpu.A) nes_cpu.P.Z = 0;
    else nes_cpu.P.Z = 1;
    if (nes_cpu.A & 0x80) nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
}

// A:=A&{adr}
static inline void nes_and(uint16_t address){
    nes_cpu.A &= nes_read_cpu(address);
    if (nes_cpu.A) nes_cpu.P.Z = 0;
    else nes_cpu.P.Z = 1;
    if (nes_cpu.A & 0x80) nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
}

// A:=A exor {adr}
static inline void nes_eor(uint16_t address){
    nes_cpu.A ^= nes_read_cpu(address);
    if (nes_cpu.A) nes_cpu.P.Z = 0;
    else nes_cpu.P.Z = 1;
    if (nes_cpu.A & 0x80) nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
}

// A:=A+{adr}
static inline void nes_adc(uint16_t address){
    const uint8_t src = nes_read_cpu(address);
    const uint16_t result16 = nes_cpu.A + src + nes_cpu.P.C;
    if (result16 >> 8)nes_cpu.P.C = 1;
    else nes_cpu.P.C = 0;
    const uint8_t result8 = (uint8_t)result16;
    if (!((nes_cpu.A ^ src) & 0x80) && ((nes_cpu.A ^ result8) & 0x80)) nes_cpu.P.V = 1;
    else nes_cpu.P.V = 0;
    nes_cpu.A = result8;
    nes_cpu.P.Z = nes_cpu.A==0;
    nes_cpu.P.N = (bool)(nes_cpu.A & 0x80);
}

// A:=A-{adr}
static inline void nes_sbc(uint16_t address){
    const uint8_t src = nes_read_cpu(address);
    const uint16_t result16 = nes_cpu.A - src - !nes_cpu.P.C;
    if (!(result16 >> 8))nes_cpu.P.C = 1;
    else nes_cpu.P.C = 0;
    const uint8_t result8 = (uint8_t)result16;
    if (((nes_cpu.A ^ src) & 0x80) && ((nes_cpu.A ^ result8) & 0x80)) nes_cpu.P.V = 1;
    else nes_cpu.P.V = 0;
    nes_cpu.A = result8;
    nes_cpu.P.Z = nes_cpu.A==0;
    nes_cpu.P.N = (bool)(nes_cpu.A & 0x80);
}

// A-{adr}
static inline void nes_cmp(uint16_t address){
    const uint16_t value = (uint16_t)nes_cpu.A - (uint16_t)nes_read_cpu(address);
    if (!(value & 0x8000))nes_cpu.P.C = 1;
    else nes_cpu.P.C = 0;
    if ((uint8_t)value & 0x80)nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
    if (value ==0)nes_cpu.P.Z = 1;
    else nes_cpu.P.Z = 0;
}

// X-{adr}
static inline void nes_cpx(uint16_t address){
    const uint16_t value = (uint16_t)nes_cpu.X - (uint16_t)nes_read_cpu(address);
    if (!(value & 0x8000))nes_cpu.P.C = 1;
    else nes_cpu.P.C = 0;
    if ((uint8_t)value & 0x80)nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
    if (value ==0)nes_cpu.P.Z = 1;
    else nes_cpu.P.Z = 0;
}

// Y-{adr}
static inline void nes_cpy(uint16_t address){
    const uint16_t value = (uint16_t)nes_cpu.Y - (uint16_t)nes_read_cpu(address);
    if (!(value & 0x8000))nes_cpu.P.C = 1;
    else nes_cpu.P.C = 0;
    if ((uint8_t)value & 0x80)nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
    if (value ==0)nes_cpu.P.Z = 1;
    else nes_cpu.P.Z = 0;
}

// {adr}:={adr}-1
static inline void nes_dec(uint16_t address){
    uint8_t data = nes_read_cpu(address);
    data--;
    nes_write_cpu(address, data);
    if (data) nes_cpu.P.Z = 0;
    else nes_cpu.P.Z = 1;
    if (data & 0x80) nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
}

// X:=X-1
static inline void nes_dex(void){
    if (--nes_cpu.X) nes_cpu.P.Z = 0;
    else nes_cpu.P.Z = 1;
    if (nes_cpu.X & 0x80) nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
}

// Y:=Y-1
static inline void nes_dey(void){
    if (--nes_cpu.Y) nes_cpu.P.Z = 0;
    else nes_cpu.P.Z = 1;
    if (nes_cpu.Y & 0x80) nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
}

// {adr}:={adr}+1
static inline void nes_inc(uint16_t address){
    uint8_t data = nes_read_cpu(address);
    data++;
    nes_write_cpu(address,data);
    // uint8_t data = nes_read_cpu(address);
    if (data) nes_cpu.P.Z = 0;
    else nes_cpu.P.Z = 1;
    if (data & 0x80) nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
}

// X:=X+1
static inline void nes_inx(void){
    if (++nes_cpu.X) nes_cpu.P.Z = 0;
    else nes_cpu.P.Z = 1;
    if (nes_cpu.X & 0x80) nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
}

// Y:=Y+1
static inline void nes_iny(void){
    if (++nes_cpu.Y) nes_cpu.P.Z = 0;
    else nes_cpu.P.Z = 1;
    if (nes_cpu.Y & 0x80) nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
}

// {adr}:={adr}*2
static inline void nes_asl(uint16_t address){
    uint8_t data = nes_read_cpu(address);
    if (data & 0x80)nes_cpu.P.C = 1;
    else nes_cpu.P.C = 0;
    data <<= 1;
    nes_write_cpu(address,data);
    if (data & 0x80)nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
    if (data ==0)nes_cpu.P.Z = 1;
    else nes_cpu.P.Z = 0;
}

static inline void nes_asla(void){
    if (nes_cpu.A&0x80)nes_cpu.P.C = 1;
    else nes_cpu.P.C = 0;
    nes_cpu.A <<= 1;
    if (nes_cpu.A & 0x80)nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
    if (nes_cpu.A ==0)nes_cpu.P.Z = 1;
    else nes_cpu.P.Z = 0;
}

// {adr}:={adr}*2+C
static inline void nes_rol(uint16_t address){
    uint8_t saveflags=nes_cpu.P.C;
    uint8_t value = nes_read_cpu(address);
    nes_cpu.UP= (nes_cpu.UP & 0xfe) | ((value>>7) & 0x01);
    value <<= 1;
    value |= saveflags;
    nes_write_cpu(address,value);
    if (value) nes_cpu.P.Z = 0;
    else nes_cpu.P.Z = 1;
    if (value & 0x80) nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
}

static inline void nes_rola(void){
    uint8_t saveflags=nes_cpu.P.C;
    nes_cpu.UP= (nes_cpu.UP & 0xfe) | ((nes_cpu.A>>7) & 0x01);
    nes_cpu.A <<= 1;
    nes_cpu.A |= saveflags;
    if (nes_cpu.A) nes_cpu.P.Z = 0;
    else nes_cpu.P.Z = 1;
    if (nes_cpu.A & 0x80) nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
}

// {adr}:={adr}/2
static inline void nes_lsr(uint16_t address){
    uint8_t data = nes_read_cpu(address);
    if (data & 1)nes_cpu.P.C = 1;
    else nes_cpu.P.C = 0;
    data >>= 1;
    nes_write_cpu(address,data);
    if (data) nes_cpu.P.Z = 0;
    else nes_cpu.P.Z = 1;
    if (data & 0x80) nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
}

static inline void nes_lsra(void){
    if (nes_cpu.A & 1)nes_cpu.P.C = 1;
    else nes_cpu.P.C = 0;
    nes_cpu.A >>= 1;
    if (nes_cpu.A) nes_cpu.P.Z = 0;
    else nes_cpu.P.Z = 1;
    if (nes_cpu.A & 0x80) nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
}

// {adr}:={adr}/2+C*128
static inline void nes_ror(uint16_t address){
    uint8_t saveflags=nes_cpu.P.C;
    uint8_t value = nes_read_cpu(address);
    nes_cpu.UP= (nes_cpu.UP & 0xfe) | (value & 0x01);
    value >>= 1;
    if (saveflags) value |= 0x80;
    nes_write_cpu(address,value);
    if (value) nes_cpu.P.Z = 0;
    else nes_cpu.P.Z = 1;
    if (value & 0x80) nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
}

static inline void nes_rora(void){
    uint8_t saveflags=nes_cpu.P.C;
    nes_cpu.UP= (nes_cpu.UP & 0xfe) | (nes_cpu.A & 0x01);
    nes_cpu.A >>= 1;
    if (saveflags) nes_cpu.A |= 0x80;
    if (nes_cpu.A) nes_cpu.P.Z = 0;
    else nes_cpu.P.Z = 1;
    if (nes_cpu.A & 0x80) nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
}


/* Move commands: */

// A:={adr}
static inline void nes_lda(uint16_t address){
    nes_cpu.A = nes_read_cpu(address);
    if (nes_cpu.A) nes_cpu.P.Z = 0;
    else nes_cpu.P.Z = 1;
    if (nes_cpu.A & 0x80) nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
}

// {adr}:=A
static inline void nes_sta(uint16_t address){
    nes_write_cpu(address,nes_cpu.A);
}

// X:={adr}
static inline void nes_ldx(uint16_t address){
    nes_cpu.X = nes_read_cpu(address);
    nes_cpu.P.Z = nes_cpu.X==0;
    nes_cpu.P.N = (bool)(nes_cpu.X & 0x80);
}

// {adr}:=X
static inline void nes_stx(uint16_t address){
    nes_write_cpu(address,nes_cpu.X);
}

// Y:={adr}
static inline void nes_ldy(uint16_t address){
    nes_cpu.Y = nes_read_cpu(address);
    nes_cpu.P.Z = nes_cpu.Y==0;
    nes_cpu.P.N = (bool)(nes_cpu.Y & 0x80);
}

// {adr}:=Y
static inline void nes_sty(uint16_t address){
    nes_write_cpu(address,nes_cpu.Y);
}

// X:=A
static inline void nes_tax(void){
    nes_cpu.X=nes_cpu.A;
    if (nes_cpu.X) nes_cpu.P.Z = 0;
    else nes_cpu.P.Z = 1;
    if (nes_cpu.X & 0x80) nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
}

// A:=X
static inline void nes_txa(void){
    nes_cpu.A=nes_cpu.X;
    if (nes_cpu.A) nes_cpu.P.Z = 0;
    else nes_cpu.P.Z = 1;
    if (nes_cpu.A & 0x80) nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
}

// Y:=A
static inline void nes_tay(void){
    nes_cpu.Y=nes_cpu.A;
    if (nes_cpu.Y) nes_cpu.P.Z = 0;
    else nes_cpu.P.Z = 1;
    if (nes_cpu.Y & 0x80) nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
}

// A:=Y
static inline void nes_tya(void){
    nes_cpu.A=nes_cpu.Y;
    if (nes_cpu.A) nes_cpu.P.Z = 0;
    else nes_cpu.P.Z = 1;
    if (nes_cpu.A & 0x80) nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
}

// X:=S
static inline void nes_tsx(void){
    nes_cpu.X=nes_cpu.SP;
    if (nes_cpu.X) nes_cpu.P.Z = 0;
    else nes_cpu.P.Z = 1;
    if (nes_cpu.X & 0x80) nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
}

// S:=X
static inline void nes_txs(void){
    nes_cpu.SP=nes_cpu.X;
}

// A:=+(S)
static inline void nes_pla(void){
    NES_POP(nes_cpu.A);
    if (nes_cpu.A) nes_cpu.P.Z = 0;
    else nes_cpu.P.Z = 1;
    if (nes_cpu.A & 0x80) nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
}

// (S)-:=A
static inline void nes_pha(void){
    NES_PUSH(nes_cpu.A);
}

// P:=+(S)
static inline void nes_plp(void){
    NES_POP(nes_cpu.UP);
    nes_cpu.P.U=1;
    nes_cpu.P.B=0;
}

// (S)-:=P
static inline void nes_php(void){
    NES_PUSH(nes_cpu.UP);
}

// Jump/Flag commands:

// branch on N=0
static inline void nes_bpl(void){
    if (nes_cpu.P.N==0)nes_cpu.PC += nes_rel();
    else nes_cpu.PC ++;
}

// branch on N=1
static inline void nes_bmi(void){
    if (nes_cpu.P.N)nes_cpu.PC += nes_rel();
    else nes_cpu.PC ++;
}

// branch on V=0
static inline void nes_bvc(void){
    if (nes_cpu.P.V==0)nes_cpu.PC += nes_rel();
    else nes_cpu.PC ++;
}

// branch on V=1
static inline void nes_bvs(void){
    if (nes_cpu.P.V)nes_cpu.PC += nes_rel();
    else nes_cpu.PC ++;
}

// branch on C=0
static inline void nes_bcc(void){
    if (nes_cpu.P.C==0)nes_cpu.PC += nes_rel();
    else nes_cpu.PC ++;
}

// branch on C=1
static inline void nes_bcs(void){
    if (nes_cpu.P.C)nes_cpu.PC += nes_rel();
    else nes_cpu.PC ++;
}

// branch on Z=0
static inline void nes_bne(void){
    if (nes_cpu.P.Z==0)nes_cpu.PC += nes_rel();
    else nes_cpu.PC ++;
}

// branch on Z=1
static inline void nes_beq(void){
    if (nes_cpu.P.Z)nes_cpu.PC += nes_rel();
    else nes_cpu.PC ++;
}

// (S)-:=PC,P PC:=($FFFE)
// BRK

// P,PC:=+(S)
static inline void nes_rti(void){
    nes_cpu.UP = NES_POP(nes_cpu.PC);
    nes_cpu.P.U = 1;
    nes_cpu.P.B = 0;
    NES_POPW(nes_cpu.PC);
}

// (S)-:=PC PC:={adr}
static inline void nes_jsr(uint16_t address){
    NES_PUSHW(nes_cpu.PC-1);
    nes_cpu.PC = address;
}

// PC:=+(S)
static inline void nes_rts(void){
    NES_POPW(nes_cpu.PC);
    ++nes_cpu.PC;
}

// PC:={adr}
static inline void nes_jmp(uint16_t address){
    nes_cpu.PC=address;
}

// N:=b7 V:=b6 Z:=A&{adr}
static inline void nes_bit(uint16_t address){
    const uint8_t value = nes_read_cpu(address);
    if (value & nes_cpu.A) nes_cpu.P.Z = 0;
    else nes_cpu.P.Z = 1;
    if (value & (uint8_t)(1 << 6)) nes_cpu.P.V = 1;
    else nes_cpu.P.V = 0;
    if (value & (uint8_t)(1 << 7)) nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
}

// C:=0
static inline void nes_clc(void){
    nes_cpu.P.C=0;
}

// C:=1
static inline void nes_sec(void){
    nes_cpu.P.C=1;
}

// D:=0
static inline void nes_cld(void){
    nes_cpu.P.D=0;
}

// D:=1
static inline void nes_sed(void){
    nes_cpu.P.D=1;
}

// I:=0
static inline void nes_cli(void){
    nes_cpu.P.I=0;
}

// I:=1
static inline void nes_sei(void){
    nes_cpu.P.I=1;
}

// V:=0
static inline void nes_clv(void){
    nes_cpu.P.V=0;
}

// nop
static inline void nes_nop(void){
}


// Illegal opcodes:

// {adr}:={adr}*2 A:=A or {adr}	
static inline void nes_slo(uint16_t address){
    uint8_t data = nes_read_cpu(address);
    if (data & 0x80)nes_cpu.P.C = 1;
    else nes_cpu.P.C = 0;
    data <<= 1;
    nes_write_cpu(address,data);

    nes_cpu.A |= data;
    if (nes_cpu.A) nes_cpu.P.Z = 0;
    else nes_cpu.P.Z = 1;
    if (nes_cpu.A & 0x80) nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
}

// {adr}:={adr}rol A:=A and {adr}
static inline void nes_rla(uint16_t address){
    uint8_t saveflags=nes_cpu.P.C;
    uint8_t value = nes_read_cpu(address);
    nes_cpu.UP= (nes_cpu.UP & 0xfe) | ((value>>7) & 0x01);
    value <<= 1;
    value |= saveflags;
    nes_write_cpu(address,value);

    nes_cpu.A &= value;
    if (nes_cpu.A) nes_cpu.P.Z = 0;
    else nes_cpu.P.Z = 1;
    if (nes_cpu.A & 0x80) nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
}

// {adr}:={adr}/2 A:=A exor {adr}
static inline void nes_sre(uint16_t address){
    uint8_t data = nes_read_cpu(address);
    if (data & 1)nes_cpu.P.C = 1;
    else nes_cpu.P.C = 0;
    data >>= 1;
    nes_write_cpu(address,data);

    nes_cpu.A ^= data;
    if (nes_cpu.A) nes_cpu.P.Z = 0;
    else nes_cpu.P.Z = 1;
    if (nes_cpu.A & 0x80) nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
}

// {adr}:={adr}ror A:=A adc {adr}
static inline void nes_rra(uint16_t address){
    uint8_t saveflags=nes_cpu.P.C;
    uint8_t value = nes_read_cpu(address);
    nes_cpu.UP= (nes_cpu.UP & 0xfe) | (value & 0x01);
    value >>= 1;
    if (saveflags) value |= 0x80;
    nes_write_cpu(address,value);

    const uint16_t result16 = nes_cpu.A + value + nes_cpu.P.C;
    if (result16 >> 8)nes_cpu.P.C = 1;
    else nes_cpu.P.C = 0;
    const uint8_t result8 = (uint8_t)result16;
    if (!((nes_cpu.A ^ value) & 0x80) && ((nes_cpu.A ^ result8) & 0x80)) nes_cpu.P.V = 1;
    else nes_cpu.P.V = 0;
    nes_cpu.A = result8;
    nes_cpu.P.Z = nes_cpu.A==0;
    nes_cpu.P.N = (bool)(nes_cpu.A & 0x80);
}

// {adr}:=A&X
static inline void nes_sax(uint16_t address){
    nes_write_cpu(address,nes_cpu.A & nes_cpu.X);
}

// A,X:={adr}
static inline void nes_lax(uint16_t address){
    nes_cpu.A = nes_read_cpu(address);
    nes_cpu.X = nes_cpu.A;
    if (nes_cpu.X) nes_cpu.P.Z = 0;
    else nes_cpu.P.Z = 1;
    if (nes_cpu.X & 0x80) nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
}

// {adr}:={adr}-1 A-{adr}
static inline void nes_dcp(uint16_t address){
    uint8_t data = nes_read_cpu(address);
    data--;
    nes_write_cpu(address,data);
    const uint16_t result16 = (uint16_t)nes_cpu.A - (uint16_t)data;

    if (!(result16 & (uint16_t)0x8000))nes_cpu.P.C = 1;
    else nes_cpu.P.C = 0;

    if ((uint8_t)result16) nes_cpu.P.Z = 0;
    else nes_cpu.P.Z = 1;
    if ((uint8_t)result16 & 0x80) nes_cpu.P.N = 1;
    else nes_cpu.P.N = 0;
}

// {adr}:={adr}+1 A:=A-{adr}
static inline void nes_isc(uint16_t address){
    uint8_t data = nes_read_cpu(address);
    data++;
    nes_write_cpu(address,data);

    const uint16_t result16 = nes_cpu.A - data - !nes_cpu.P.C;
    if (!(result16 >> 8))nes_cpu.P.C = 1;
    else nes_cpu.P.C = 0;
    const uint8_t result8 = (uint8_t)result16;
    if (((nes_cpu.A ^ data) & 0x80) && ((nes_cpu.A ^ result8) & 0x80)) nes_cpu.P.V = 1;
    else nes_cpu.P.V = 0;
    nes_cpu.A = result8;
    nes_cpu.P.Z = nes_cpu.A==0;
    nes_cpu.P.N = (bool)(nes_cpu.A & 0x80);
}

// A:=A&#{imm}
// ANC


// A:=(A&#{imm})/2
// ALR


// A:=(A&#{imm})/2
// ARR


// A:=X&#{imm}
// XAA²

// A,X:=#{imm}
// LAX²

// X:=A&X-#{imm}
// AXS

// A:=A-#{imm}
// SBC

// {adr}:=A&X&H
// AHX¹

// {adr}:=Y&H
// SHY¹

// {adr}:=X&H
// SHX¹

// S:=A&X {adr}:=S&H
// TAS¹

// {adr}:=X&H
// A,X,S:={adr}&S

uint8_t nes_read_cpu(uint16_t address){
    switch (address >> 13){
        case 0:
            // 高三位为0: [$0000, $2000): 系统主内存, 4次镜像
            return CPU_RAM[address & (uint16_t)0x07ff];
        case 1:
            // 高三位为1, [$2000, $4000): PPU寄存器, 8字节步进镜像
            nes_printf("NOT IMPL\n");
            return 0;
        case 2:
            // 高三位为2, [$4000, $6000): pAPU寄存器 扩展ROM区
            nes_printf("NOT IMPL\n");
            return 0;
        case 3:
            // 高三位为3, [$6000, $8000): 存档 SRAM区
            return nes_rom->sram[address & (uint16_t)0x1fff];
        case 4: case 5: case 6: case 7:
            // 高一位为1, [$8000, $10000) 程序PRG-ROM区
            // return famicom->prg_banks[address >> 13][address & (uint16_t)0x1fff];
            return prg_banks[address & (uint16_t)0x7fff];
    }
    nes_printf("invalid address\n");
    return 0;
}

void nes_write_cpu(uint16_t address, uint8_t data){
    switch (address >> 13){
        case 0:
            // 高三位为0: [$0000, $2000): 系统主内存, 4次镜像
            CPU_RAM[address & (uint16_t)0x07ff] = data;
            return;
        case 1:
            // 高三位为1, [$2000, $4000): PPU寄存器, 8字节步进镜像
            nes_printf("NOT IMPL\n");
            return;
        case 2:
            // 高三位为2, [$4000, $6000): pAPU寄存器 扩展ROM区
            nes_printf("NOT IMPL\n");
            return;
        case 3:
            // 高三位为3, [$6000, $8000): 存档 SRAM区
            nes_rom->sram[address & (uint16_t)0x1fff] = data;
            return;
        case 4: case 5: case 6: case 7:
            // 高一位为1, [$8000, $10000) 程序PRG-ROM区
            nes_printf("WARNING: PRG-ROM\n");
            // prg_banks[address >> 13][address & (uint16_t)0x1fff] = data;
            prg_banks[address & (uint16_t)0x7fff] = data;
            return;
    }
    nes_printf("invalid address\n");
}

uint16_t nes_read_cpu_word(uint16_t address){
    return nes_read_cpu(address)|(uint16_t)nes_read_cpu(address + 1) << 8;
}

void nes_write_cpu_word(uint16_t address, uint16_t data){
    nes_write_cpu( address, data & 0xff ); 
    nes_write_cpu( address + 1, data >> 8 );
}

// void nes_cpu_irq(uint8_t ){

// }

void nes_cpu_init(void){
    nes_cpu.A = 0;
    nes_cpu.X = 0;
    nes_cpu.Y = 0;
    nes_cpu.P.I = 1;
    nes_cpu.SP = 0xFD;
    nes_cpu.PC = nes_read_cpu_word(NES_VERCTOR_RESET);
}

void nes_cpu_reset(void){
    nes_cpu.A = 0;
    nes_cpu.X = 0;
    nes_cpu.Y = 0;
    // nes_cpu.P.B = 1;
    // nes_cpu.P = 0;
    nes_cpu.SP = 0xFD;
    nes_cpu.PC = nes_read_cpu_word(NES_VERCTOR_RESET);
}

nes_opcode_t nes_opcode_table[] = {
    {NULL,	NULL,	NULL},
};

int line = 1;
void nes_opcode_test(uint16_t opcode){
    nes_printf("%04d PC: $%04X opcode:%02X    ;   ",line++,nes_cpu.PC-1,opcode);
    nes_printf("A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%d\n",
                nes_cpu.A,nes_cpu.X,nes_cpu.Y,nes_cpu.P,nes_cpu.SP, nes_cpu.cycles);
    if (line == 4767)
        printf("%d\r\n",line);
    switch (opcode){
    // case 0x00://BRK 7
    //     // nes_brk();nes_cpu.cycles+=7;
    //     break;
    case 0x01://ORA izx 6
        nes_ora(nes_izx());nes_cpu.cycles+=6;
        break;
    // case 0x02://KIL
    //     break;
    case 0x03://SLO izx 8
        nes_slo(nes_izx());nes_cpu.cycles+=8;
        break;
    case 0x04://NOP zp 3
        nes_zp();nes_cpu.cycles+=3;
        break;
    case 0x05://ORA zp 3
        nes_ora(nes_zp());nes_cpu.cycles+=3;
        break;
    case 0x06://ASL zp 5
        nes_asl(nes_zp());nes_cpu.cycles+=5;
        break;
    case 0x07://SLO zp 5
        nes_slo(nes_zp());nes_cpu.cycles+=5;
        break;
    case 0x08://PHP 3
        nes_php();nes_cpu.cycles+=3;
        break;
    case 0x09://ORA imm 2
        nes_ora(nes_imm());nes_cpu.cycles+=2;
        break;
    case 0x0A://ASL 2
        nes_asla();nes_cpu.cycles+=2;
        break;
    // case 0x0B://ANC imm 2
    //     nes_anc(nes_imm());nes_cpu.cycles+=2;
    //     break;
    case 0x0C://NOP abs 4
        nes_abs();nes_cpu.cycles+=4;
        break;
    case 0x0D://ORA abs 4
        nes_ora(nes_abs());nes_cpu.cycles+=4;
        break;
    case 0x0E://ASL abs 6
        nes_asl(nes_abs());nes_cpu.cycles+=6;
        break;
    case 0x0F://SLO abs 6
        nes_slo(nes_abs());nes_cpu.cycles+=6;
        break;
    case 0x10://BPL rel 2*
        nes_bpl();nes_cpu.cycles+=2;
        break;
    case 0x11://ORA izy 5*
        nes_ora(nes_izy());nes_cpu.cycles+=5;
        break;
    // case 0x12://KIL
    //     nes_kil();
    //     break;
    case 0x13://SLO izy 8
        nes_slo(nes_izy());nes_cpu.cycles+=8;
        break;
    case 0x14://NOP zpx 4
        nes_zpx();nes_cpu.cycles+=4;
        break;
    case 0x15://ORA zpx 4
        nes_ora(nes_zpx());nes_cpu.cycles+=4;
        break;
    case 0x16://ASL zpx 6
        nes_asl(nes_zpx());nes_cpu.cycles+=6;
        break;
    case 0x17://SLO zpx 6
        nes_slo(nes_zpx());nes_cpu.cycles+=6;
        break;
    case 0x18://CLC 2
        nes_clc();nes_cpu.cycles+=2;
        break;
    case 0x19://ORA aby 4*
        nes_ora(nes_aby());nes_cpu.cycles+=4;
        break;
    case 0x1A://NOP 2
        nes_cpu.cycles+=2;
        break;
    case 0x1B://SLO aby 7
        nes_slo(nes_aby());nes_cpu.cycles+=7;
        break;
    case 0x1C://NOP abx 4*
        nes_abx();nes_cpu.cycles+=4;
        break;
    case 0x1D://ORA abx 4*
        nes_ora(nes_abx());nes_cpu.cycles+=4;
        break;
    case 0x1E://ASL abx 7
        nes_asl(nes_abx());nes_cpu.cycles+=7;
        break;
    case 0x1F://SLO abx 7
        nes_slo(nes_abx());nes_cpu.cycles+=7;
        break;
    case 0x20://JSR abs 6
        nes_jsr(nes_abs());nes_cpu.cycles+=6;
        break;
    case 0x21://AND izx 6
        nes_and(nes_izx());nes_cpu.cycles+=6;
        break;
    // case 0x22://KIL
    //     nes_kil();
    //     break;
    case 0x23://RLA izx 8
        nes_rla(nes_izx());nes_cpu.cycles+=8;
        break;
    case 0x24://BIT zp 3
        nes_bit(nes_zp());nes_cpu.cycles+=3;
        break;
    case 0x25://AND zp 3
        nes_and(nes_zp());nes_cpu.cycles+=3;
        break;
    case 0x26://ROL zp 5
        nes_rol(nes_zp());nes_cpu.cycles+=5;
        break;
    case 0x27://RLA zp 5
        nes_rla(nes_zp());nes_cpu.cycles+=5;
        break;
    case 0x28://PLP 4
        nes_plp();nes_cpu.cycles+=4;
        break;
    case 0x29://AND imm 2
        nes_and(nes_imm());nes_cpu.cycles+=2;
        break;
    case 0x2A://ROL 2
        nes_rola();nes_cpu.cycles+=2;
        break;
    // case 0x2B://ANC imm 2
    //     nes_anc(nes_imm());nes_cpu.cycles+=2;
    //     break;
    case 0x2C://BIT abs 4
        nes_bit(nes_abs());nes_cpu.cycles+=4;
        break;
    case 0x2D://AND abs 4
        nes_and(nes_abs());nes_cpu.cycles+=4;
        break;
    case 0x2E://ROL abs 6
        nes_rol(nes_abs());nes_cpu.cycles+=6;
        break;
    case 0x2F://RLA abs 6
        nes_rla(nes_abs());nes_cpu.cycles+=6;
        break;
    case 0x30://BMI rel 2*
        nes_bmi();nes_cpu.cycles+=2;
        break;
    case 0x31://AND izy 5*
        nes_and(nes_izy());nes_cpu.cycles+=5;
        break;
    // case 0x32://KIL
    //     nes_kil();
    //     break;
    case 0x33://RLA izy 8
        nes_rla(nes_izy());nes_cpu.cycles+=8;
        break;
    case 0x34://NOP zpx 4
        nes_zpx();nes_cpu.cycles+=4;
        break;
    case 0x35://AND zpx 4
        nes_and(nes_zpx());nes_cpu.cycles+=4;
        break;
    case 0x36://ROL zpx 6
        nes_rol(nes_zpx());nes_cpu.cycles+=6;
        break;
    case 0x37://RLA zpx 6
        nes_rla(nes_zpx());nes_cpu.cycles+=6;
        break;
    case 0x38://sec 2
        nes_sec();nes_cpu.cycles+=2;
        break;
    case 0x39://AND aby 4*
        nes_and(nes_aby());nes_cpu.cycles+=4;
        break;
    case 0x3A://NOP 2
        nes_cpu.cycles+=2;
        break;
    case 0x3B://RLA aby 7
        nes_rla(nes_aby());nes_cpu.cycles+=7;
        break;
    case 0x3C://NOP abx 4*
        nes_abx();nes_cpu.cycles+=4;
        break;
    case 0x3D://AND abx 4*
        nes_and(nes_abx());nes_cpu.cycles+=4;
        break;
    case 0x3E://ROL abx 7
        nes_rol(nes_abx());nes_cpu.cycles+=7;
        break;
    case 0x3F://RLA abx 7
        nes_rla(nes_abx());nes_cpu.cycles+=7;
        break;
    case 0x40://RTI 6
        nes_rti();nes_cpu.cycles+=6;
        break;
    case 0x41://EOR izx 6
        nes_eor(nes_izx());nes_cpu.cycles+=6;
        break;
    // case 0x42://KIL
    //     nes_kil();
    //     break;
    case 0x43://SRE izx 8
        nes_sre(nes_izx());nes_cpu.cycles+=8;
        break;
    case 0x44://NOP zp 3
        nes_zp();nes_cpu.cycles+=3;
        break;
    case 0x45://EOR zp 3
        nes_eor(nes_zp());nes_cpu.cycles+=3;
        break;
    case 0x46://LSR zp 5
        nes_lsr(nes_zp());nes_cpu.cycles+=5;
        break;
    case 0x47://SRE zp 5
        nes_sre(nes_zp());nes_cpu.cycles+=5;
        break;
    case 0x48://PHA 3
        nes_pha();nes_cpu.cycles+=3;
        break;
    case 0x49://EOR imm 2
        nes_eor(nes_imm());nes_cpu.cycles+=2;
        break;
    case 0x4A://LSR 2
        nes_lsra();nes_cpu.cycles+=2;
        break;
    // case 0x4B://ALR imm 2
    //     nes_alr(nes_imm());nes_cpu.cycles+=2;
    //     break;
    case 0x4C://JMP abs 3
        nes_jmp(nes_abs());nes_cpu.cycles+=3;
        break;
    case 0x4D://EOR abs 4
        nes_eor(nes_abs());nes_cpu.cycles+=4;
        break;
    case 0x4E://LSR abs 6
        nes_lsr(nes_abs());nes_cpu.cycles+=6;
        break;
    case 0x4F://SRE abs 6
        nes_sre(nes_abs());nes_cpu.cycles+=6;
        break;
    case 0x50://BVC rel 2*
        nes_bvc();nes_cpu.cycles+=2;
        break;
    case 0x51://EOR izy 5*
        nes_eor(nes_izy());nes_cpu.cycles+=5;
        break;
    // case 0x52://KIL
    //     nes_eor(nes_izy());nes_cpu.cycles+=5;
    //     break;
    case 0x53://SRE izy 8
        nes_sre(nes_izy());nes_cpu.cycles+=8;
        break;
    case 0x54://NOP zpx 4
        nes_zpx();nes_cpu.cycles+=4;
        break;
    case 0x55://EOR zpx 4
        nes_eor(nes_zpx());nes_cpu.cycles+=4;
        break;
    case 0x56://LSR zpx 6
        nes_lsr(nes_zpx());nes_cpu.cycles+=6;
        break;
    case 0x57://SRE zpx 6
        nes_sre(nes_zpx());nes_cpu.cycles+=6;
        break;
    case 0x58://CLI 2
        nes_cli();nes_cpu.cycles+=2;
        break;
    case 0x59://EOR aby 4*
        nes_eor(nes_aby());nes_cpu.cycles+=4;
        break;
    case 0x5A://NOP 2
        nes_cpu.cycles+=2;
        break;
    case 0x5B://SRE aby 7
        nes_sre(nes_aby());nes_cpu.cycles+=7;
        break;
    case 0x5C://NOP abx 4*
        nes_abx();nes_cpu.cycles+=4;
        break;
    case 0x5D://EOR abx 4*
        nes_eor(nes_abx());nes_cpu.cycles+=4;
        break;
    case 0x5E://LSR abx 7
        nes_lsr(nes_abx());nes_cpu.cycles+=7;
        break;
    case 0x5F://SRE abx 7
        nes_sre(nes_abx());nes_cpu.cycles+=7;
        break;
    case 0x60://RTS 6
        nes_rts();nes_cpu.cycles+=6;
        break;
    case 0x61://ADC izx 6
        nes_adc(nes_izx());nes_cpu.cycles+=6;
        break;
    // case 0x62://KIL
    //     nes_kil();
    //     break;
    case 0x63://RRA izx 8
        nes_rra(nes_izx());nes_cpu.cycles+=8;
        break;
    case 0x64://NOP zp 3
        nes_zp();nes_cpu.cycles+=3;
        break;
    case 0x65://ADC zp 3
        nes_adc(nes_zp());nes_cpu.cycles+=3;
        break;
    case 0x66://ROR zp 5
        nes_ror(nes_zp());nes_cpu.cycles+=5;
        break;
    case 0x67://RRA zp 5
        nes_rra(nes_zp());nes_cpu.cycles+=5;
        break;
    case 0x68://PLA 4
        nes_pla();nes_cpu.cycles+=4;
        break;
    case 0x69://ADC imm 2
        nes_adc(nes_imm());nes_cpu.cycles+=2;
        break;
    case 0x6A://ROR 2
        nes_rora();nes_cpu.cycles+=2;
        break;
    // case 0x6B://ARR imm 2
    //     nes_arr(nes_imm());nes_cpu.cycles+=2;
    //     break;
    case 0x6C://JMP ind 5
        nes_jmp(nes_ind());nes_cpu.cycles+=5;
        break;
    case 0x6D://ADC abs 4
        nes_adc(nes_abs());nes_cpu.cycles+=4;
        break;
    case 0x6E://ROR abs 6
        nes_ror(nes_abs());nes_cpu.cycles+=6;
        break;
    case 0x6F://RRA abs 6
        nes_rra(nes_abs());nes_cpu.cycles+=6;
        break;
    case 0x70://BVS rel 2*
        nes_bvs();nes_cpu.cycles+=2;
        break;
    case 0x71://ADC izy 5*
        nes_adc(nes_izy());nes_cpu.cycles+=5;
        break;
    // case 0x72://KIL
    //     nes_kil();
    //     break;
    case 0x73://RRA izy 8
        nes_rra(nes_izy());nes_cpu.cycles+=8;
        break;
    case 0x74://NOP zpx 4
        nes_zpx();nes_cpu.cycles+=4;
        break;
    case 0x75://ADC zpx 4
        nes_adc(nes_zpx());nes_cpu.cycles+=4;
        break;
    case 0x76://ROR zpx 6
        nes_ror(nes_zpx());nes_cpu.cycles+=6;
        break;
    case 0x77://RRA zpx 6
        nes_rra(nes_zpx());nes_cpu.cycles+=6;
        break;
    case 0x78://SEI 2
        nes_sei();nes_cpu.cycles+=2;
        break;
    case 0x79://ADC aby 4*
        nes_adc(nes_aby());nes_cpu.cycles+=4;
        break;
    case 0x7A://NOP 2
        nes_cpu.cycles+=2;
        break;
    case 0x7B://RRA aby 7
        nes_rra(nes_aby());nes_cpu.cycles+=7;
        break;
    case 0x7C://NOP abx 4*
        nes_abx();nes_cpu.cycles+=4;
        break;
    case 0x7D://ADC abx 4*
        nes_adc(nes_abx());nes_cpu.cycles+=4;
        break;
    case 0x7E://ROR abx 7
        nes_ror(nes_abx());nes_cpu.cycles+=7;
        break;
    case 0x7F://RRA abx 7
        nes_rra(nes_abx());nes_cpu.cycles+=7;
        break;
    case 0x80://NOP imm 2
        nes_imm();nes_cpu.cycles+=2;
        break;
    case 0x81://STA izx 6
        nes_sta(nes_izx());nes_cpu.cycles+=6;
        break;
    case 0x82://NOP imm 2
        nes_imm();nes_cpu.cycles+=2;
        break;
    case 0x83://SAX izx 6
        nes_sax(nes_izx());nes_cpu.cycles+=6;
        break;
    case 0x84://STY zp 3
        nes_sty(nes_zp());nes_cpu.cycles+=3;
        break;
    case 0x85://STA zp 3
        nes_sta(nes_zp());nes_cpu.cycles+=3;
        break;
    case 0x86://STX zp 3
        nes_stx(nes_zp());nes_cpu.cycles+=3;
        break;
    case 0x87://SAX zp 3
        nes_sax(nes_zp());nes_cpu.cycles+=3;
        break;
    case 0x88://DEY 2
        nes_dey();nes_cpu.cycles+=2;
        break;
    case 0x89://NOP imm 2
        nes_imm();nes_cpu.cycles+=2;
        break;
    case 0x8A://TXA 2
        nes_txa();nes_cpu.cycles+=2;
        break;
    // case 0x8B://XAA imm 2
    //     nes_xaa(nes_imm());nes_cpu.cycles+=2;
    //     break;
    case 0x8C://STY abs 4
        nes_sty(nes_abs());nes_cpu.cycles+=4;
        break;
    case 0x8D://STA abs 4
        nes_sta(nes_abs());nes_cpu.cycles+=4;
        break;
    case 0x8E://STX abs 4
        nes_stx(nes_abs());nes_cpu.cycles+=4;
        break;
    case 0x8F://SAX abs 4
        nes_sax(nes_abs());nes_cpu.cycles+=4;
        break;
    case 0x90://BCC rel 2*
        nes_bcc();nes_cpu.cycles+=2;
        break;
    case 0x91://STA izy 6
        nes_sta(nes_izy());nes_cpu.cycles+=6;
        break;
    // case 0x92://KIL
    //     nes_kil();
    //     break;
    // case 0x93://AHX izy 6
    //     nes_axh(nes_izy());nes_cpu.cycles+=6;
    //     break;
    case 0x94://STY zpx 4
        nes_sty(nes_zpx());nes_cpu.cycles+=4;
        break;
    case 0x95://STA zpx 4
        nes_sta(nes_zpx());nes_cpu.cycles+=4;
        break;
    case 0x96://STX zpy 4
        nes_stx(nes_zpy());nes_cpu.cycles+=4;
        break;
    case 0x97://SAX zpy 4
        nes_sax(nes_zpy());nes_cpu.cycles+=4;
        break;
    case 0x98://TYA 2
        nes_tya();nes_cpu.cycles+=2;
        break;
    case 0x99://STA aby 5
        nes_sta(nes_aby());nes_cpu.cycles+=5;
        break;
    case 0x9A://TXS 2
        nes_txs();nes_cpu.cycles+=2;
        break;
    // case 0x9B://TAS aby 5
    //     nes_tax(nes_aby());nes_cpu.cycles+=5;
    //     break;
    // case 0x9C://SHY abx 5
    //     nes_shy(nes_abx());nes_cpu.cycles+=5;
    //     break;
    case 0x9D://STA abx 5
        nes_sta(nes_abx());nes_cpu.cycles+=5;
        break;
    // case 0x9E://SHX aby 5
    //     nes_shx(nes_aby());nes_cpu.cycles+=5;
    //     break;
    // case 0x9F://AHX aby 5
    //     nes_ahx(nes_aby());nes_cpu.cycles+=5;
    //     break;
    case 0xA0://LDY imm 2
        nes_ldy(nes_imm());nes_cpu.cycles+=2;
        break;
    case 0xA1://LDA izx 6
        nes_lda(nes_izx());nes_cpu.cycles+=6;
        break;
    case 0xA2://LDX imm 2
        nes_ldx(nes_imm());nes_cpu.cycles+=2;
        break;
    case 0xA3://LAX izx 6
        nes_lax(nes_izx());nes_cpu.cycles+=6;
        break;
    case 0xA4://LDY zp 3
        nes_ldy(nes_zp());nes_cpu.cycles+=3;
        break;
    case 0xA5://LDA zp 3
        nes_lda(nes_zp());nes_cpu.cycles+=3;
        break;
    case 0xA6://LDX zp 3
        nes_ldx(nes_zp());nes_cpu.cycles+=3;
        break;
    case 0xA7://LAX zp 3
        nes_lax(nes_zp());nes_cpu.cycles+=3;
        break;
    case 0xA8://TAY 2
        nes_tay();nes_cpu.cycles+=2;
        break;
    case 0xA9://LDA imm 2
        nes_lda(nes_imm());nes_cpu.cycles+=2;
        break;
    case 0xAA://TAX 2
        nes_tax();nes_cpu.cycles+=2;
        break;
    // case 0xAB://LAX imm 2
    //     nes_lax(nes_imm());nes_cpu.cycles+=2;
    //     break;
    case 0xAC://LDY abs 4
        nes_ldy(nes_abs());nes_cpu.cycles+=4;
        break;
    case 0xAD://LDA abs 4
        nes_lda(nes_abs());nes_cpu.cycles+=4;
        break;
    case 0xAE://LDX abs 4
        nes_ldx(nes_abs());nes_cpu.cycles+=4;
        break;
    case 0xAF://LAX abs 4
        nes_lax(nes_abs());nes_cpu.cycles+=4;
        break;
    case 0xB0://BCS rel 2*
        nes_bcs();nes_cpu.cycles+=2;
        break;
    case 0xB1://LDA izy 5*
        nes_lda(nes_izy());nes_cpu.cycles+=5;
        break;
    // case 0xB2://KIL
    //     nes_kil();
    //     break;
    case 0xB3://LAX izy 5*
        nes_lax(nes_izy());nes_cpu.cycles+=5;
        break;
    case 0xB4://LDY zpx 4
        nes_ldy(nes_zpx());nes_cpu.cycles+=4;
        break;
    case 0xB5://LDA zpx 4
        nes_lda(nes_zpx());nes_cpu.cycles+=4;
        break;
    case 0xB6://LDX zpy 4
        nes_ldx(nes_zpy());nes_cpu.cycles+=4;
        break;
    case 0xB7://LAX zpy 4
        nes_lax(nes_zpy());nes_cpu.cycles+=4;
        break;
    case 0xB8://CLV 2
        nes_clv();nes_cpu.cycles+=2;
        break;
    case 0xB9://LDA aby 4*
        nes_lda(nes_aby());nes_cpu.cycles+=4;
        break;
    case 0xBA://TSX 2
        nes_tsx();nes_cpu.cycles+=2;
        break;
    // case 0xBB://LAS aby 4*
    //     nes_las(nes_aby());nes_cpu.cycles+=4;
    //     break;
    case 0xBC://LDY abx 4*
        nes_ldy(nes_abx());nes_cpu.cycles+=4;
        break;
    case 0xBD://LDA abx 4*
        nes_lda(nes_abx());nes_cpu.cycles+=4;
        break;
    case 0xBE://LDX aby 4*
        nes_ldx(nes_aby());nes_cpu.cycles+=4;
        break;
    case 0xBF://LAX aby 4*
        nes_lax(nes_aby());nes_cpu.cycles+=4;
        break;
    case 0xC0://CPY imm 2
        nes_cpy(nes_imm());nes_cpu.cycles+=2;
        break;
    case 0xC1://CMP izx 6
        nes_cmp(nes_izx());nes_cpu.cycles+=6;
        break;
    // case 0xC2://KIL
    //     nes_kil();
    //     break;
    case 0xC3://DCP izx 8
        nes_dcp(nes_izx());nes_cpu.cycles+=8;
        break;
    case 0xC4://CPY zp 3
        nes_cpy(nes_zp());nes_cpu.cycles+=3;
        break;
    case 0xC5://CMP zp 3
        nes_cmp(nes_zp());nes_cpu.cycles+=3;
        break;
    case 0xC6://DEC zp 5
        nes_dec(nes_zp());nes_cpu.cycles+=5;
        break;
    case 0xC7://DCP zp 5
        nes_dcp(nes_zp());nes_cpu.cycles+=5;
        break;
    case 0xC8://INY 2
        nes_iny();nes_cpu.cycles+=2;
        break;
    case 0xC9://CMP imm 2
        nes_cmp(nes_imm());nes_cpu.cycles+=2;
        break;
    case 0xCA://DEX 2
        nes_dex();nes_cpu.cycles+=2;
        break;
    // case 0xCB://AXS imm 2
    //     nes_axs(nes_imm());nes_cpu.cycles+=2;
    //     break;
    case 0xCC://CPY abs 4
        nes_cpy(nes_abs());nes_cpu.cycles+=4;
        break;
    case 0xCD://CMP abs 4
        nes_cmp(nes_abs());nes_cpu.cycles+=4;
        break;
    case 0xCE://DEC abs 6
        nes_dec(nes_abs());nes_cpu.cycles+=6;
        break;
    case 0xCF://DCP abs 6
        nes_dcp(nes_abs());nes_cpu.cycles+=6;
        break;
    case 0xD0://BNE rel 2*
        nes_bne();nes_cpu.cycles+=2;
        break;
    case 0xD1://CMP izy 5*
        nes_cmp(nes_izy());nes_cpu.cycles+=5;
        break;
    // case 0xD2://KIL
    //     nes_kil();
    //     break;
    case 0xD3://DCP izy 8
        nes_dcp(nes_izy());nes_cpu.cycles+=8;
        break;
    case 0xD4://NOP zpx 4
        nes_zpx();nes_cpu.cycles+=4;
        break;
    case 0xD5://CMP zpx 4
        nes_cmp(nes_zpx());nes_cpu.cycles+=4;
        break;
    case 0xD6://DEC zpx 6
        nes_dec(nes_zpx());nes_cpu.cycles+=6;
        break;
    case 0xD7://DCP zpx 6
        nes_dcp(nes_zpx());nes_cpu.cycles+=6;
        break;
    case 0xD8://CLD 2
        nes_cld();nes_cpu.cycles+=2;
        break;
    case 0xD9://CMP aby 4*
        nes_cmp(nes_aby());nes_cpu.cycles+=4;
        break;
    case 0xDA://NOP 2
        nes_cpu.cycles+=2;
        break;
    case 0xDB://DCP aby 7
        nes_dcp(nes_aby());nes_cpu.cycles+=7;
        break;
    case 0xDC://NOP abx 4*
        nes_abx();nes_cpu.cycles+=4;
        break;
    case 0xDD://CMP abx 4*
        nes_cmp(nes_abx());nes_cpu.cycles+=4;
        break;
    case 0xDE://DEC abx 7
        nes_dec(nes_abx());nes_cpu.cycles+=7;
        break;
    case 0xDF://DCP abx 7
        nes_dcp(nes_abx());nes_cpu.cycles+=7;
        break;
    case 0xE0://CPX imm 2
        nes_cpx(nes_imm());nes_cpu.cycles+=2;
        break;
    case 0xE1://SBC izx 6
        nes_sbc(nes_izx());nes_cpu.cycles+=6;
        break;
    // case 0xE2://NOP imm 2
    //     nes_imm();nes_cpu.cycles+=2;
    //     break;
    case 0xE3://ISC izx 8
        nes_isc(nes_izx());nes_cpu.cycles+=8;
        break;
    case 0xE4://CPX zp 3
        nes_cpx(nes_zp());nes_cpu.cycles+=3;
        break;
    case 0xE5://SBC zp 3
        nes_sbc(nes_zp());nes_cpu.cycles+=3;
        break;
    case 0xE6://INC zp 5
        nes_inc(nes_zp());nes_cpu.cycles+=5;
        break;
    case 0xE7://ISC zp 5
        nes_isc(nes_zp());nes_cpu.cycles+=5;
        break;
    case 0xE8://INX 2
        nes_inx();nes_cpu.cycles+=2;
        break;
    case 0xE9://SBC imm 2
        nes_sbc(nes_imm());nes_cpu.cycles+=2;
        break;
    case 0xEA://NOP 2
        nes_cpu.cycles+=2;
        break;
    case 0xEB://SBC imm 2
        nes_sbc(nes_imm());nes_cpu.cycles+=2;
        break;
    case 0xEC://CPX abs 4
        nes_cpx(nes_abs());nes_cpu.cycles+=4;
        break;
    case 0xED://SBC abs 4
        nes_sbc(nes_abs());nes_cpu.cycles+=4;
        break;
    case 0xEE://INC abs 6
        nes_inc(nes_abs());nes_cpu.cycles+=6;
        break;
    case 0xEF://ISC abs 6
        nes_isc(nes_abs());nes_cpu.cycles+=6;
        break;
    case 0xF0://BEQ rel 2*
        nes_beq();nes_cpu.cycles+=2;
        break;
    case 0xF1://SBC izy 5*
        nes_sbc(nes_izy());nes_cpu.cycles+=5;
        break;
    // case 0xF2://KIL
    //     nes_kil();
    //     break;
    case 0xF3://ISC izy 8
        nes_isc(nes_izy());nes_cpu.cycles+=8;
        break;
    case 0xF4://NOP zpx 4
        nes_zpx();nes_cpu.cycles+=4;
        break;
    case 0xF5://SBC zpx 4
        nes_sbc(nes_zpx());nes_cpu.cycles+=4;
        break;
    case 0xF6://INC zpx 6
        nes_inc(nes_zpx());nes_cpu.cycles+=6;
        break;
    case 0xF7://ISC zpx 6
        nes_isc(nes_zpx());nes_cpu.cycles+=6;
        break;
    case 0xF8://SED 2
        nes_sed();nes_cpu.cycles+=2;
        break;
    case 0xF9://SBC aby 4*
        nes_sbc(nes_aby());nes_cpu.cycles+=4;
        break;
    case 0xFA://NOP 2
        nes_cpu.cycles+=2;
        break;
    case 0xFB://ISC aby 7
        nes_isc(nes_aby());nes_cpu.cycles+=7;
        break;
    case 0xFC://NOP abx 4*
        nes_abx();nes_cpu.cycles+=4;
        break;
    case 0xFD://SBC abx 4*
        nes_sbc(nes_abx());nes_cpu.cycles+=4;
        break;
    case 0xFE://INC abx 7
        nes_inc(nes_abx());nes_cpu.cycles+=7;
        break;
    case 0xFF://ISC abx 7
        nes_isc(nes_abx());nes_cpu.cycles+=7;
        break;
    default:
        nes_printf("opcode:%02X not support\n",opcode);
        return;
        break;
    }
    nes_cpu.opcode = nes_read_cpu(nes_cpu.PC++);
    nes_opcode_test(nes_cpu.opcode);
}

void nes_test(void){
    nes_printf("mapper:%03d\n",nes_rom->mapper_number);
    nes_printf("prg_rom_size:%d*16kb\n",nes_rom->prg_rom_size);
    nes_printf("chr_rom_size:%d*8kb\n",nes_rom->chr_rom_size);
    nes_cpu_init();
    nes_cpu_reset();
    nes_cpu.PC = 0xC000;
    nes_cpu.opcode = nes_read_cpu(nes_cpu.PC++);
    nes_opcode_test(nes_cpu.opcode);
}
