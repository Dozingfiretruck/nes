#include "nes.h"

static uint8_t nes_runing = 0;

int nes_init(nes_t *nes){
    nes_initex(nes);
    nes_cpu_init(nes);
    nes_ppu_init(nes);
    return 0;
}

int nes_deinit(nes_t *nes){
    nes_runing = 0;
    nes_deinitex(nes);
    return 0;
}

extern nes_color_t nes_palette[];

static inline void nes_palette_generate(nes_t* nes){
    for (size_t i = 0; i < 32; i++) {
        nes->nes_ppu.palette[i] = nes_palette[nes->nes_ppu.palette_indexes[i]];
    }
    for (size_t i = 1; i < 8; i++){
        nes->nes_ppu.palette[4 * i] = nes->nes_ppu.palette[0];
    }
}

static inline const uint16_t nes_sprite_overflow_line(nes_t* nes){
    if (nes->nes_ppu.MASK_b | nes->nes_ppu.MASK_s){
        uint8_t buffer[256 + 16];
        memset(buffer, 0, 256);
        const int sprite_size = nes->nes_ppu.CTRL_H ? 16 : 8; //Sprite size
        for (int i = 0; i != 64; ++i) {
            const uint8_t y = nes->nes_ppu.sprite_info[i].y;
            for (int i = 0; i != sprite_size; ++i) buffer[y + i]++;
        }
        uint16_t line ;
        for (line = 0; line != NES_HEIGHT; ++line) if (buffer[line] > 8) break;
        return line;
    }else
        return NES_HEIGHT;
}

uint8_t sp0_hittest_buffer[NES_WIDTH] = {0};

static void nes_render_background_line(nes_t* nes,uint16_t scanline,nes_color_t* draw_data){
    uint8_t p = 0;
    int8_t m = 7 - nes->nes_ppu.x;
    const uint8_t dx = (const uint8_t)nes->nes_ppu.v.coarse_x;
    const uint8_t dy = scanline & 0x7;
    const uint8_t tile_y = scanline >> 3;
    uint8_t nametable_id = (uint8_t)nes->nes_ppu.v.nametable;
    for (uint8_t tile_x = dx; tile_x < 32; tile_x++){
        uint32_t pattern_id = nes->nes_ppu.name_table[nametable_id][tile_x + (tile_y << 5)];
        const uint8_t* bit0_p = nes->nes_ppu.pattern_table[nes->nes_ppu.CTRL_B ? 4 : 0] + pattern_id * 16;
        const uint8_t* bit1_p = bit0_p + 8;
        const uint8_t bit0 = bit0_p[dy];
        const uint8_t bit1 = bit1_p[dy];

        uint8_t attribute = nes->nes_ppu.name_table[nametable_id][960 + ((tile_y >> 2) << 3) + (tile_x >> 2)];
        // 1:D4-D5/D6-D7 0:D0-D1/D2-D3
        // 1:D2-D3/D6-D7 0:D0-D1/D4-D5
        const uint8_t high_bit = ((attribute >> (((tile_y & 2) << 1) | (tile_x & 2))) & 3) << 2;
        for (; m >= 0; m--){
            uint8_t low_bit = ((bit0 >> m) & 0x01) | ((bit1 >> m)<<1 & 0x02);
            uint8_t palette_index = (high_bit & 0x0c) | low_bit;
            draw_data[p++] = nes->nes_ppu.palette[palette_index];
        }
        m = 7;
    }
    nametable_id ^= nes->nes_rom.mirroring_type ? 1:2;
    for (uint8_t tile_x = 0; tile_x <= dx; tile_x++){
        uint32_t pattern_id = nes->nes_ppu.name_table[nametable_id][tile_x + (tile_y << 5)];
        const uint8_t* bit0_p = nes->nes_ppu.pattern_table[nes->nes_ppu.CTRL_B ? 4 : 0] + pattern_id * 16;
        const uint8_t* bit1_p = bit0_p + 8;
        const uint8_t bit0 = bit0_p[dy];
        const uint8_t bit1 = bit1_p[dy];

        uint8_t attribute = nes->nes_ppu.name_table[nametable_id][960 + ((tile_y >> 2) << 3) + (tile_x >> 2)];
        // 1:D4-D5/D6-D7 0:D0-D1/D2-D3
        // 1:D2-D3/D6-D7 0:D0-D1/D4-D5
        const uint8_t high_bit = ((attribute >> (((tile_y & 2) << 1) | (tile_x & 2))) & 3) << 2;
        if (tile_x == dx){
            if (nes->nes_ppu.x){
                m = nes->nes_ppu.x;
            }else
                break;
        }
        for (; m >= 0; m--){
            uint8_t low_bit = ((bit0 >> m) & 0x01) | ((bit1 >> m)<<1 & 0x02);
            uint8_t palette_index = (high_bit & 0x0c) | low_bit;
            draw_data[p++] = nes->nes_ppu.palette[palette_index];
        }
        m = 7;
    }
}

static void nes_render_sprite_line(nes_t* nes,uint16_t scanline,nes_color_t* draw_data){
    uint8_t sprite_number = 0;
    uint8_t sprite_size = nes->nes_ppu.CTRL_H?16:8;//
    
    for (uint8_t i = 63; i > 0 ; i--){
        if (nes->nes_ppu.sprite_info[i].y >= 0xEF)
            continue;
        uint8_t sprite_y = (uint8_t)(nes->nes_ppu.sprite_info[i].y + 1);
        if (scanline < sprite_y || scanline >= sprite_y + sprite_size)
            continue;
        sprite_number ++;
        if(sprite_number > 8 ){
            nes->nes_ppu.STATUS_O = 1;
            break;
        }
        if (!nes->nes_ppu.sprite_info[i].priority)
            continue;
        //开始渲染
        uint8_t	tile_index_number = nes->nes_ppu.CTRL_H?nes->nes_ppu.sprite_info[i].pattern_8x16:nes->nes_ppu.CTRL_S;

    }

    // 后续渲染
    if (nes->nes_ppu.sprite_info[0].y >= 0xEF)
        return;
    uint8_t sprite_y = (uint8_t)(nes->nes_ppu.sprite_info[0].y + 1);
    if (scanline < sprite_y || scanline >= sprite_y + sprite_size)
        return;
    sprite_number ++;
    if(sprite_number > 8 ){
        nes->nes_ppu.STATUS_O = 1;
        return;
    }
    if (!nes->nes_ppu.sprite_info[0].priority)
        return;

    const uint8_t tile_index_number = nes->nes_ppu.sprite_info[0].tile_index_number;
    const uint8_t* sprite_bit0_p = nes->nes_ppu.pattern_table[nes->nes_ppu.CTRL_H?((tile_index_number&1)?4:0):(nes->nes_ppu.CTRL_S?4:0)]+tile_index_number * 16;
    const uint8_t* sprite_bit1_p = sprite_bit0_p + 8;

    uint8_t dy = scanline - sprite_y;

    // uint8_t nametable_id = (uint8_t)nes->nes_ppu.v.nametable;
    // uint32_t pattern_id = nes->nes_ppu.name_table[nametable_id][tile_x + (tile_y << 5)];//
    // const uint8_t* bit0_p = nes->nes_ppu.pattern_table[nes->nes_ppu.CTRL_B ? 4 : 0] + pattern_id * 16;
    // const uint8_t* bit1_p = bit0_p + 8;
    // const uint8_t bit0 = bit0_p[dy];
    // const uint8_t bit1 = bit1_p[dy];

    if (nes->nes_ppu.sprite_info[0].flip_h){
        dy = sprite_size - dy;
    }
    
    const uint8_t sprite_bit0 = sprite_bit0_p[dy];
    const uint8_t sprite_bit1 = sprite_bit1_p[dy];

    const uint8_t sprite_date = sprite_bit0 | sprite_bit1;



    // uint8_t low_bit = bit0_p[dy] | bit1_p[dy]<<1;

    // for (size_t i = 0; i < count; i++)
    // {
    //     /* code */
    // }

    // for (; m >= 0; m--){
    //     uint8_t low_bit = ((bit0 >> m) & 0x01) | ((bit1 >> m)<<1 & 0x02);
    //     uint8_t palette_index = (high_bit & 0x0c) | low_bit;
    //     draw_data[p++] = nes->nes_ppu.palette[palette_index];
    // }

    if (nes->nes_ppu.MASK_b){
        if (nes->nes_ppu.STATUS_S == 0 && scanline == 30){
            nes->nes_ppu.STATUS_S = 1;
        }
    }

}


void nes_run(nes_t* nes){
    // nes_printf("mapper:%03d\n",nes->nes_rom.mapper_number);
    // nes_printf("prg_rom_size:%d*16kb\n",nes->nes_rom.prg_rom_size);
    // nes_printf("chr_rom_size:%d*8kb\n",nes->nes_rom.chr_rom_size);
    nes_cpu_reset(nes);
    // nes->nes_cpu.PC = 0xC000;
    // printf("nes->nes_cpu.PC %02X",nes->nes_cpu.PC);
    uint64_t frame_cnt = 0;
    uint16_t scanline = 0;
    nes_runing = 1;
    while (nes_runing){
        frame_cnt++;
        nes_palette_generate(nes);
        const uint16_t sprite_overflow_line = nes_sprite_overflow_line(nes);

        if (nes->nes_ppu.MASK_b == 0){
            for (size_t i = 0; i < NES_HEIGHT * NES_WIDTH; i++){
                nes->nes_draw_data[i] = nes->nes_ppu.palette[0];
            }
        }
        
        for(scanline = 0; scanline < 240; scanline++) { // 0-239 Visible frame
            nes_opcode(nes,NES_PPU_CPU_CLOCKS);
            if (nes->nes_ppu.MASK_b){
#if (NES_RAM_LACK == 1)
                nes_render_background_line(nes,scanline,nes->nes_draw_data);
#else
                nes_render_background_line(nes,scanline,nes->nes_draw_data + scanline * NES_WIDTH);
#endif
            }

            if (nes->nes_ppu.MASK_s){
#if (NES_RAM_LACK == 1)
                nes_render_sprite_line(nes,scanline,nes->nes_draw_data);
#else
                nes_render_sprite_line(nes,scanline,nes->nes_draw_data + scanline * NES_WIDTH);
#endif
            }

            if (nes->nes_ppu.MASK_b){
                if ((nes->nes_ppu.v.fine_y) < 7) {
                    nes->nes_ppu.v.fine_y++;
                }else {
                    nes->nes_ppu.v.fine_y = 0;
                    uint8_t y = nes->nes_ppu.v.coarse_y;
                    if (y == 29) {
                        y = 0;
                        nes->nes_ppu.v_reg ^= 0x0800;
                    }else if (y == 31) {
                        y = 0;
                    }else {
                        y++;
                    }
                    nes->nes_ppu.v.coarse_y = y;
                }
                // v: ....A.. ...BCDEF <- t: ....A.. ...BCDEF
                nes->nes_ppu.v_reg = (nes->nes_ppu.v_reg & (uint16_t)0xFBE0) | (nes->nes_ppu.t_reg & (uint16_t)0x041F);
            }
            
#if (NES_RAM_LACK == 1)
            if((frame_cnt % (NES_FRAME_SKIP+1))==0){
                nes_draw(0, scanline, NES_WIDTH-1, scanline, nes->nes_draw_data);
            }
#endif
        }
#if (NES_RAM_LACK == 0)
        if((frame_cnt % (NES_FRAME_SKIP+1))==0){
            nes_draw(0, 0, NES_WIDTH-1, NES_HEIGHT-1, nes->nes_draw_data);
        }
#endif
        nes_opcode(nes,NES_PPU_CPU_CLOCKS); //240 Post-render line
        nes_vblank_start(nes);
        for(scanline = 241; scanline < 261; scanline++){ // 241-260行 垂直空白行 x20
            nes_opcode(nes,NES_PPU_CPU_CLOCKS);
        }
        nes->nes_ppu.ppu_status = 0;
        if (nes->nes_ppu.CTRL_V) {
            nes_nmi(nes);
        }
        // nes_vblank_end(nes);
        nes->nes_ppu.ppu_status &= 0x10;

        nes_opcode(nes,NES_PPU_CPU_CLOCKS); //261 Pre-render line
        if (nes->nes_ppu.MASK_b){
            // v: GHIA.BC DEF..... <- t: GHIA.BC DEF.....
            nes->nes_ppu.v_reg = (nes->nes_ppu.v_reg & (uint16_t)0x841F) | (nes->nes_ppu.t_reg & (uint16_t)0x7BE0);
        }
        nes_wait(10);
    }
}

