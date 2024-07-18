/*
 * MIT License
 *
 * Copyright (c) 2023 Dozingfiretruck
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


#include "nes.h"

static inline uint8_t nes_read_ppu_memory(nes_t* nes){
    const uint16_t address = nes->nes_ppu.v_reg & (uint16_t)0x3FFF;
    const uint16_t index = address >> 10;
    const uint16_t offset = address & (uint16_t)0x3FF;
    if (address < (uint16_t)0x3F00) {
        uint8_t data = nes->nes_ppu.buffer;
        nes->nes_ppu.buffer = nes->nes_ppu.chr_banks[index][offset];
        return data;
    } else {
        nes->nes_ppu.buffer = nes->nes_ppu.chr_banks[index][offset];
        return nes->nes_ppu.palette_indexes[address & (uint16_t)0x1f];
    }
}

static inline void nes_write_ppu_memory(nes_t* nes,uint8_t data){
    const uint16_t address = nes->nes_ppu.v_reg & (uint16_t)0x3FFF;
    if (address < (uint16_t)0x3F00) {
        nes->nes_ppu.chr_banks[(uint8_t)(address >> 10)][(uint16_t)(address & (uint16_t)0x3FF)] = data;
    } else {
        if (address & (uint16_t)0x03) {
            nes->nes_ppu.palette_indexes[address & (uint16_t)0x1f] = data & 0x3F;
        } else {
            const uint16_t offset = address & (uint16_t)0x0f;
            nes->nes_ppu.palette_indexes[offset] = data & 0x3F;
            nes->nes_ppu.palette_indexes[offset | 0x10] = data & 0x3F;
        }
    }
}

// https://www.nesdev.org/wiki/PPU_registers
uint8_t nes_read_ppu_register(nes_t* nes,uint16_t address){
    uint8_t data = 0;
    switch (address & (uint16_t)0x07){
        case 2://Status ($2002) < read
            // w:                  <- 0
            data = nes->nes_ppu.ppu_status;
            nes->nes_ppu.STATUS_V = 0;
            nes->nes_ppu.w = 0;
            break;
        case 4://OAM data ($2004) <> read/write
            data = nes->nes_ppu.oam_data[nes->nes_ppu.oam_addr];
            break;
        case 7://Data ($2007) <> read/write
            data = nes_read_ppu_memory(nes);
            nes->nes_ppu.v_reg += (uint16_t)((nes->nes_ppu.CTRL_I) ? 32 : 1);
            break;
        default :
            nes_printf("nes_read_ppu_register error %04X\n",address);
            break;
    }
    // nes_printf("nes_read_ppu_register %04X %02X\n",address,data);
    return data;
}

void nes_write_ppu_register(nes_t* nes,uint16_t address, uint8_t data){
    // nes_printf("nes_write_ppu_register %04X %02X\n",address,data);
    switch (address & (uint16_t)0x07){
        case 0://Controller ($2000) > write
            // t: ....GH.. ........ <- d: ......GH
            //     <used elsewhere> <- d: ABCDEF..
            nes->nes_ppu.ppu_ctrl = data;
            nes->nes_ppu.t.nametable = nes->nes_ppu.CTRL_N;
            break;
        case 1://Mask ($2001) > write
            nes->nes_ppu.ppu_mask = data;
            break;
        case 3://OAM address ($2003) > write
            nes->nes_ppu.oam_addr = data;
            break;
        case 4://OAM data ($2004) <> read/write
            nes->nes_ppu.oam_data[nes->nes_ppu.oam_addr++] = data;
            break;
        case 5://Scroll ($2005) >> write x2
            if (nes->nes_ppu.w) {   // w is 1
                // t: FGH..AB CDE..... <- d: ABCDEFGH
                // w:                  <- 0
                nes->nes_ppu.t.fine_y = (data & 0x07);
                nes->nes_ppu.t.coarse_y = (data & 0xF8)>>3;
                nes->nes_ppu.w = 0;
            } else {                // w is 0
                // t: ....... ...ABCDE <- d: ABCDE...
                // x:              FGH <- d: .....FGH
                // w:                  <- 1
                nes->nes_ppu.t.coarse_x = (data & 0xF8)>>3;
                nes->nes_ppu.x = (data & 0x07);
                nes->nes_ppu.w = 1;
            }
            break;
        case 6://Address ($2006) >> write x2
            if (nes->nes_ppu.w) {   // w is 1
                // t: ....... ABCDEFGH <- d: ABCDEFGH
                // v: <...all bits...> <- t: <...all bits...>
                // w:                  <- 0
                nes->nes_ppu.t_reg = (nes->nes_ppu.t_reg & (uint16_t)0xFF00) | (uint16_t)data;
                nes->nes_ppu.v_reg = nes->nes_ppu.t_reg;
                nes->nes_ppu.w = 0;
            } else {                // w is 0
                // t: ..CDEFGH ........ <- d: ..CDEFGH
                //         <unused>     <- d: AB......
                // t: .Z...... ........ <- 0 (bit Z is cleared)
                // w:                   <- 1
                nes->nes_ppu.t_reg = (nes->nes_ppu.t_reg & (uint16_t)0xFF) | (((uint16_t)data & 0x3F) << 8);
                nes->nes_ppu.w = 1;
            }
            break;
        case 7://Data ($2007) <> read/write
            nes_write_ppu_memory(nes,data);
            nes->nes_ppu.v_reg += (uint16_t)((nes->nes_ppu.CTRL_I) ? 32 : 1);
            break;
        default :
            nes_printf("nes_write_ppu_register error %04X %02X\n",address,data);
            break;
    }
}

void nes_ppu_init(nes_t *nes){
    // four_screen
    if (nes->nes_rom.four_screen) { 
        nes->nes_ppu.name_table[0] = nes->nes_ppu.ppu_vram0;
        nes->nes_ppu.name_table[1] = nes->nes_ppu.ppu_vram1;
        nes->nes_ppu.name_table[2] = nes->nes_ppu.ppu_vram2;
        nes->nes_ppu.name_table[3] = nes->nes_ppu.ppu_vram3;
    }
    // Vertical
    else if (nes->nes_rom.mirroring_type) {
        nes->nes_ppu.name_table[0] = nes->nes_ppu.ppu_vram0;
        nes->nes_ppu.name_table[1] = nes->nes_ppu.ppu_vram1;
        nes->nes_ppu.name_table[2] = nes->nes_ppu.ppu_vram0;
        nes->nes_ppu.name_table[3] = nes->nes_ppu.ppu_vram1;
    }
    // Horizontal or mapper-controlled
    else {
        nes->nes_ppu.name_table[0] = nes->nes_ppu.ppu_vram0;
        nes->nes_ppu.name_table[1] = nes->nes_ppu.ppu_vram0;
        nes->nes_ppu.name_table[2] = nes->nes_ppu.ppu_vram1;
        nes->nes_ppu.name_table[3] = nes->nes_ppu.ppu_vram1;
    }
    // mirrors
    nes->nes_ppu.chr_banks[12] = nes->nes_ppu.name_table[0];
    nes->nes_ppu.chr_banks[13] = nes->nes_ppu.name_table[1];
    nes->nes_ppu.chr_banks[14] = nes->nes_ppu.name_table[2];
    nes->nes_ppu.chr_banks[15] = nes->nes_ppu.name_table[3];
}

