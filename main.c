#include "nes.h"
#include "nes_port.h"

int main(int argc, char *argv[]){

    // nes_t* nes = nes_load_rom("nestest.nes");
    nes_t* nes = nes_load_rom("super_mario.nes");

    // nes_t* nes = nes_load_rom("color_test.nes");

    // nes_t* nes = nes_load_rom("cpu_interrupts.nes");//mapper 1 
    // nes_t* nes = nes_load_rom("cpu_dummy_reads.nes");//mapper 3 

    // nes_t* nes = nes_load_rom("cpu_dummy_writes_oam.nes"); // #7
    // nes_t* nes = nes_load_rom("cpu_dummy_writes_ppumem.nes"); // #11

    // nes_t* nes = nes_load_rom("test_cpu_exec_space_apu.nes"); // #2
    // nes_t* nes = nes_load_rom("test_cpu_exec_space_ppuio.nes"); // #3

    // nes_t* nes = nes_load_rom("ram_after_reset.nes");
    // nes_t* nes = nes_load_rom("registers.nes");

    // nes_t* nes = nes_load_rom("cpu_timing_test.nes");// OP $18
    
    if (!nes){
        return -1;
    }
    nes_wait(2000);     //wait sdl2 init
    nes_run(nes);

    // nes_rom_free(nes);
    return 0;
}

