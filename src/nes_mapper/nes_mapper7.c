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

/* https://www.nesdev.org/wiki/AxROM */
static void nes_mapper_init(nes_t* nes){
    // CPU $8000-$FFFF: 32 KB switchable PRG ROM bank
    nes_load_prgrom_32k(nes, 0, 0);
    // CHR capacity: 8 KiB ROM.
    nes_load_chrrom_8k(nes, 0, 0);
}

/*
    Bank select ($8000-$FFFF)
    7  bit  0
    ---- ----
    xxxM xPPP
       |  |||
       |  +++- Select 32 KB PRG ROM bank for CPU $8000-$FFFF
       +------ Select 1 KB VRAM page for all 4 nametables
*/
typedef struct {
    uint8_t P:3;
    uint8_t :1;
    uint8_t M:1;
    uint8_t :3;
}bank_select_t;

static void nes_mapper_write(nes_t* nes, uint16_t address, uint8_t date) {
    const bank_select_t* bank_select = (bank_select_t*)&date;
    nes_load_prgrom_32k(nes, 0, bank_select->P);
    nes_ppu_screen_mirrors(nes, bank_select->M?NES_MIRROR_ONE_SCREEN1:NES_MIRROR_ONE_SCREEN0);
}

int nes_mapper7_init(nes_t* nes){
    nes->nes_mapper.mapper_init = nes_mapper_init;
    nes->nes_mapper.mapper_write = nes_mapper_write;
    return 0;
}

