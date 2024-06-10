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

void nes_apu_init(nes_t *nes){

}

uint8_t nes_read_apu_register(nes_t *nes,uint16_t address){
    uint8_t data = 0;
    nes_printf("nes_read apu %04X %02X\n",address,data);
    switch(address){
        case 0x4015:

            break;
        // case 0x4001:
        //     break;
        // case 0x4002:
        //     break;
        // case 0x4003:
        //     break;
        // case 0x4004:
        //     break;
        // case 0x4005:
        //     break;
        // case 0x4006:
        //     break;
        // case 0x4007:
        //     break;
        default:
            break;
    }
    return data;
}

void nes_write_apu_register(nes_t* nes,uint16_t address,uint8_t data){
    nes_printf("nes_write apu %04X %02X\n",address,data);
    switch(address){
        // Pulse ($4000–$4007)
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
            break;
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
        case 0x4015:
            nes->nes_apu.status=data;
            break;
        case 0x4017:
            nes->nes_apu.frame_counter=data;
            break;
        default:
            // nes_printf("nes_write apu %04X %02X\n",address,data);
            break;
    }
}

// 上下
// 4015 03 // 开启方波1+方波2
// 4000 87 // 方波1 50%占空比 不循环 音量非常数 音量7-0
// 4001 89 // 方波1 启动滑音 分割器的周期为P+1个半帧=0+1 周期减(频率高) 移位1
// 4002 F0 // 方波1 240
// 4003 00 // 方波1 长度计数器 0

// 选择
// 4015 02 // 开启方波2
// 4004 3F // 方波2 12.5%占空比 循环 音量常数 音量15
// 4005 9A // 方波2 启动滑音 分割器的周期为P+1个半帧=0+1 周期减(频率高) 移位2
// 4006 FF // 方波2 255
// 4007 00 // 方波2 长度计数器 0 



