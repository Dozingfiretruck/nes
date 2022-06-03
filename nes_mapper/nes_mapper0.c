#include "nes_mapper.h"
#include "../nes_rom.h"
#include "../nes_cpu.h"

int nes_mapper0_init(void){
    if (nes_rom->prg_rom_size == 1){
        nes_memcpy(prg_banks,nes_rom->prg_rom,0x4000);
        nes_memcpy(prg_banks+0x4000,nes_rom->prg_rom,0x4000);
    }else if(nes_rom->prg_rom_size == 2){
        nes_memcpy(prg_banks,nes_rom->prg_rom,0x8000);
    }
    return 0;
}

