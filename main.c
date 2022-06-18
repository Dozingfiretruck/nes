// #include "nes.h"
#include "nes_rom.h"
#include "nes_cpu.h"


int main(void){
    nes_init();

    nes_load_rom("nestest.nes");
    //nes_load_rom("super_mario.nes");

    extern void nes_test(void);
    nes_test();

    int getc = getchar();
    nes_rom_free();
    return 0;
}

