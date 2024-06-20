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

#if (NES_ENABLE_SOUND == 1)

// https://www.nesdev.org/wiki/APU_Length_Counter
static const uint8_t length_counter_table[32] = {
/*       |   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
---------+---------------------------------------------------------------- */
/*00-0F*/  0x0A,0xFE,0x14,0x02,0x28,0x04,0x50,0x06,0xA0,0x08,0x3C,0x0A,0x0E,0x0C,0x1A,0x0E,
/*10-1F*/  0x0C,0x10,0x18,0x12,0x30,0x14,0x60,0x16,0xC0,0x18,0x48,0x1A,0x10,0x1C,0x20,0x1E,
};

void nes_apu_init(nes_t *nes){
    nes->nes_apu.status = 0;
}

uint8_t nes_read_apu_register(nes_t *nes,uint16_t address){
    uint8_t data = 0;
    if(address==0x4015){
        data=nes->nes_apu.status;

        if (nes->nes_apu.pulse1.len_counter_load) data |= 1;
        if (nes->nes_apu.pulse2.len_counter_load) data |= (1 << 1);
        if (nes->nes_apu.triangle.len_counter_load) data |= (1 << 2);
        if (nes->nes_apu.noise.len_counter_load) data |= (1 << 3);
        if (nes->nes_apu.dmc.load_counter) data |= (1 << 4);
        if (nes->nes_apu.frame_interrupt) data |= (1 << 6);
        // dmc_interrupt
        nes->nes_apu.frame_interrupt = 0; //读取$4015时清除frame_interrupt
    }else{
        nes_printf("nes_read apu %04X %02X\n",address,data);
    }
    return data;
}

void nes_write_apu_register(nes_t* nes,uint16_t address,uint8_t data){
    nes_printf("nes_write apu %04X %02X\n",address,data);
    switch(address){
        // Pulse ($4000–$4007)
        // Pulse0 ($4000–$4003)
        case 0x4000:
            nes->nes_apu.pulse1.control0=data;
            break;
        case 0x4001:
            nes->nes_apu.pulse1.control1=data;
            break;
        case 0x4002:
            nes->nes_apu.pulse1.timer_low=data;
            break;
        case 0x4003:
            nes->nes_apu.pulse1.control3=data;
            if (nes->nes_apu.status_pulse1){
                nes->nes_apu.pulse1.length_counter = length_counter_table[nes->nes_apu.pulse1.len_counter_load];
            }
            break;
        // Pulse1 ($4004–$4007)
        case 0x4004:
            nes->nes_apu.pulse2.control0=data;
            break;
        case 0x4005:
            nes->nes_apu.pulse2.control1=data;
            break;
        case 0x4006:
            nes->nes_apu.pulse2.timer_low=data;
            break;
        case 0x4007:
            nes->nes_apu.pulse2.control3=data;
            if (nes->nes_apu.status_pulse2){
                nes->nes_apu.pulse2.length_counter = length_counter_table[nes->nes_apu.pulse2.len_counter_load];
            }
            break;
        // Triangle ($4008–$400B)
        case 0x4008:
            nes->nes_apu.triangle.control0=data;
            break;
        // case 0x4009:
        //     break;
        case 0x400A:
            nes->nes_apu.triangle.timer_low=data;
            break;
        case 0x400B:
            nes->nes_apu.triangle.control3=data;
            if (nes->nes_apu.status_triangle){
                nes->nes_apu.triangle.length_counter = length_counter_table[nes->nes_apu.triangle.len_counter_load];
            }
            break;
        // Noise ($400C–$400F)
        case 0x400C:
            nes->nes_apu.noise.control0=data;
            break;
        // case 0x400D:
        //     break;
        case 0x400E:
            nes->nes_apu.noise.control2=data;
            break;
        case 0x400F:
            nes->nes_apu.noise.control3=data;
            if (nes->nes_apu.status_noise){
                nes->nes_apu.noise.length_counter = length_counter_table[nes->nes_apu.noise.len_counter_load];
            }
            break;
        // DMC ($4010–$4013)
        case 0x4010:
            nes->nes_apu.dmc.control0=data;
            break;
        case 0x4011:
            nes->nes_apu.dmc.control1=data;
            break;
        case 0x4012:
            nes->nes_apu.dmc.sample_address=data;
            break;
        case 0x4013:
            nes->nes_apu.dmc.sample_length=data;
            break;
        // case 0x4014:
        //     break;
        // Status ($4015) https://www.nesdev.org/wiki/APU#Status_($4015)
        case 0x4015:
            nes->nes_apu.status=data;
            if (nes->nes_apu.status_pulse1==0){
                nes->nes_apu.pulse1.len_counter_load=0;
            }
            if (nes->nes_apu.status_pulse2==0){
                nes->nes_apu.pulse2.len_counter_load=0;
            }
            if (nes->nes_apu.status_triangle==0){
                nes->nes_apu.triangle.len_counter_load=0;
            }
            if (nes->nes_apu.status_noise==0){
                nes->nes_apu.noise.len_counter_load=0;
            }
            nes->nes_apu.dmc_interrupt = 0;
            break;
        case 0x4017:
            nes->nes_apu.frame_counter=data;
            if (nes->nes_apu.mode){
                /* code */
            }
            
            break;
        default:
            // nes_printf("nes_write apu %04X %02X\n",address,data);
            break;
    }
}

static inline void nes_apu_pulse_sweep(pulse_t* pulse){

}

void nes_apu_length_counter_and_sweep(nes_t* nes){
    // length_counter
    if (!nes->nes_apu.pulse1.len_counter_halt && nes->nes_apu.pulse1.length_counter){
        nes->nes_apu.pulse1.length_counter--;
    }
    if (!nes->nes_apu.pulse2.len_counter_halt && nes->nes_apu.pulse2.length_counter){
        nes->nes_apu.pulse2.length_counter--;
    }
    if (!nes->nes_apu.triangle.len_counter_halt && nes->nes_apu.triangle.length_counter){
        nes->nes_apu.triangle.length_counter--;
    }
    if (!nes->nes_apu.noise.len_counter_halt && nes->nes_apu.noise.length_counter){
        nes->nes_apu.noise.length_counter--;
    }
    // sweep
    nes_apu_pulse_sweep(&nes->nes_apu.pulse1);
    nes_apu_pulse_sweep(&nes->nes_apu.pulse2);
}

void nes_apu_envelopes_and_linear_counter(nes_t *nes){

}

void nes_apu_play(nes_t* nes){

}

extern void nes_cpu_irq(nes_t* nes);
static inline void nes_apu_frame_irq(nes_t *nes){
    if (nes->nes_apu.irq_inhibit_flag==0){
        nes_cpu_irq(nes);
    }
}

/*
https://www.nesdev.org/wiki/APU#Frame_Counter_($4017)
https://www.nesdev.org/wiki/APU_Frame_Counter

mode 0:    mode 1:       function
---------  -----------  -----------------------------
 - - - f    - - - - -    IRQ (if bit 6 is clear)
 - l - l    - l - - l    Length counter and sweep
 e e e e    e e e - e    Envelope and linear counter
*/
void nes_apu_frame(nes_t* nes){
    if(nes->nes_apu.mode){// 5 step mode
        switch(nes->nes_apu.clock_count %= 5){
            case 0:
                nes_apu_envelopes_and_linear_counter(nes);
                break;
            case 1:
                nes_apu_length_counter_and_sweep(nes);
                nes_apu_envelopes_and_linear_counter(nes);
                break;
            case 2:
                nes_apu_envelopes_and_linear_counter(nes);
                break;
            case 4:
                nes_apu_length_counter_and_sweep(nes);
                nes_apu_envelopes_and_linear_counter(nes);
                break;
        }
    }else{  // 4 step mode
        switch(nes->nes_apu.clock_count %= 4){
            case 0:
                nes_apu_envelopes_and_linear_counter(nes);
                break;
            case 1:
                nes_apu_length_counter_and_sweep(nes);
                nes_apu_envelopes_and_linear_counter(nes);
                break;
            case 2:
                nes_apu_envelopes_and_linear_counter(nes);
                break;
            case 3:
                nes_apu_frame_irq(nes);
                nes_apu_length_counter_and_sweep(nes);
                nes_apu_envelopes_and_linear_counter(nes);
                break;
            default:
                break;
        }
    }

    nes_apu_play(nes);
    nes->nes_apu.clock_count++;
}

#endif



