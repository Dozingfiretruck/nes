#include "nes.h"
#include "nes_port.h"

int main(int argc, char *argv[]){

    // nes_t* nes = nes_load_rom("nestest.nes");
    nes_t* nes = nes_load_rom("super_mario.nes");
    // nes_t* nes = nes_load_rom("color_test.nes");
    // nes_t* nes = nes_load_rom("cpu_interrupts.nes");
    
    
    if (!nes){
        return -1;
    }
    nes_wait(2000);     //wait sdl2 init
    nes_run(nes);

    // nes_rom_free(nes);
    return 0;
}

