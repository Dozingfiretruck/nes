#include "nes_port.h"
#include "nes.h"

int nes_mapper0_init(nes_t* nes){
    const int mirror = nes->nes_rom.prg_rom_size & 2;
    nes_load_prgrom_8k(nes,0, 0);
    nes_load_prgrom_8k(nes,1, 1);
    nes_load_prgrom_8k(nes,2, mirror+0);
    nes_load_prgrom_8k(nes,3, mirror+1);

    for (int i = 0; i < 8; i++){
        nes_load_chrrom_1k(nes,i,i);
    }
    return 0;
}

