/*
 * MIT License
 *
 * Copyright (c) 2022 Dozingfiretruck
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
#include "nes_port.h"

int main(int argc, char *argv[]){

    // nes_t* nes = nes_load_rom("nestest.nes");
    // nes_t* nes = nes_load_rom("super_mario.nes");
    nes_t* nes = nes_load_rom("contra.nes");


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

