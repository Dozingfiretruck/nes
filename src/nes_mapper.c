

#include "nes.h"

void nes_load_prgrom_8k(nes_t* nes,int des, int src) {
    nes->nes_cpu.prg_banks[des] = nes->nes_rom.prg_rom + 8 * 1024 * src;
}

void nes_load_chrrom_1k(nes_t* nes,int des, int src) {
    nes->nes_ppu.pattern_table[des] = nes->nes_rom.chr_rom + 1024 * src;
}

int nes_load_mapper(nes_t* nes){
    switch (nes->nes_rom.mapper_number){
        case 0 :
            return nes_mapper0_init(nes);
        default :
            nes_printf("mapper:%03d is unsupported\n",nes->nes_rom.mapper_number);
            return NES_ERROR;
    }
}
