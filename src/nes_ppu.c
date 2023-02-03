#include "nes.h"


//https://www.nesdev.org/pal.txt
#if (NES_COLOR_DEPTH == 32)
// ARGB8888
nes_color_t nes_palette[]={
    0xFF757575, 0xFF271B8F, 0xFF0000AB, 0xFF47009F, 0xFF8F0077, 0xFFB0013, 0xFFA70000, 0xFF7F0B00,0xFF432F00, 0xFF004700, 0xFF005100, 0xFF003F17, 0xFF1B3F5F, 0xFF000000, 0xFF000000, 0xFF000000,
    0xFFBCBCBC, 0xFF0073EF, 0xFF233BEF, 0xFF8300F3, 0xFFBF00BF, 0xFF7005B, 0xFFDB2B00, 0xFFCB4F0F,0xFF8B7300, 0xFF009700, 0xFF00AB00, 0xFF00933B, 0xFF00838B, 0xFF000000, 0xFF000000, 0xFF000000,
    0xFFFFFFFF, 0xFF3FBFFF, 0xFF5F97FF, 0xFFA78BFD, 0xFFF77BFF, 0xFFF77B7, 0xFFFF7763, 0xFFFF9B3B,0xFFF3BF3F, 0xFF83D313, 0xFF4FDF4B, 0xFF58F898, 0xFF00EBDB, 0xFF000000, 0xFF000000, 0xFF000000,
    0xFFFFFFFF, 0xFFABE7FF, 0xFFC7D7FF, 0xFFD7CBFF, 0xFFFFC7FF, 0xFFFC7DB, 0xFFFFBFB3, 0xFFFFDBAB,0xFFFFE7A3, 0xFFE3FFA3, 0xFFABF3BF, 0xFFB3FFCF, 0xFF9FFFF3, 0xFF000000, 0xFF000000, 0xFF000000,
};
#elif (NES_COLOR_DEPTH == 16)
// RGB565
nes_color_t nes_palette[]={
    0x757575, 0x271B8F, 0x0000AB, 0x47009F, 0x8F0077, 0xB0013, 0xA70000, 0x7F0B00,0x432F00, 0x004700, 0x005100, 0x003F17, 0x1B3F5F, 0x000000, 0x000000, 0x000000,
    0xBCBCBC, 0x0073EF, 0x233BEF, 0x8300F3, 0xBF00BF, 0x7005B, 0xDB2B00, 0xCB4F0F,0x8B7300, 0x009700, 0x00AB00, 0x00933B, 0x00838B, 0x000000, 0x000000, 0x000000,
    0xFFFFFF, 0x3FBFFF, 0x5F97FF, 0xA78BFD, 0xF77BFF, 0xF77B7, 0xFF7763, 0xFF9B3B,0xF3BF3F, 0x83D313, 0x4FDF4B, 0x58F898, 0x00EBDB, 0x000000, 0x000000, 0x000000,
    0xFFFFFF, 0xABE7FF, 0xC7D7FF, 0xD7CBFF, 0xFFC7FF, 0xFC7DB, 0xFFBFB3, 0xFFDBAB,0xFFE7A3, 0xE3FFA3, 0xABF3BF, 0xB3FFCF, 0x9FFFF3, 0x000000, 0x000000, 0x000000,
};
#endif

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
        const uint16_t index = address >> 10;
        const uint16_t offset = address & (uint16_t)0x3FF;
        nes->nes_ppu.chr_banks[index][offset] = data;
    } else {
        if (address & (uint16_t)0x03) {
            nes->nes_ppu.palette_indexes[address & (uint16_t)0x1f] = data;
        } else {
            const uint16_t offset = address & (uint16_t)0x0f;
            nes->nes_ppu.palette_indexes[offset] = data;
            nes->nes_ppu.palette_indexes[offset | (uint16_t)0x10] = data;
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
            return -1;
    }
    // nes_printf("nes_read_ppu_register %04X %02X\n",address,data);
    return data;
}

void nes_write_ppu_register(nes_t* nes,uint16_t address, uint8_t data){
    // nes_printf("nes_write_ppu_register %04X %02X\n",address,data);
    switch (address & (uint16_t)0x07){
        case 0://Controller ($2000) > write
            // t: ...GH.. ........ <- d: ......GH
            //    <used elsewhere> <- d: ABCDEF..
            nes->nes_ppu.ppu_ctrl = data;
            nes->nes_ppu.t.nametable = (data & 0x03);
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
                // t: .CDEFGH ........ <- d: ..CDEFGH
                //        <unused>     <- d: AB......
                // t: Z...... ........ <- 0 (bit Z is cleared)
                // w:                  <- 1
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
            return;
    }
}

void nes_vblank_start(nes_t* nes){
    nes->nes_ppu.STATUS_V = 1;
}

void nes_vblank_end(nes_t* nes){
    nes->nes_ppu.STATUS_V = 0;
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

