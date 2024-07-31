/*
 * Copyright 2023-2024 Dozingfiretruck
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _NES_MAPPER_
#define _NES_MAPPER_

#ifdef __cplusplus
    extern "C" {
#endif

struct nes;
typedef struct nes nes_t;

/* https://www.nesdev.org/wiki/Mapper   */
typedef struct {
    /* Initialize Mapper */
    void (*mapper_init)(nes_t* nes);
    /* Write to Mapper */
    void (*mapper_write)(nes_t* nes, uint16_t write_addr, uint8_t data );
    /* Write to SRAM */
    void (*mapper_sram)(nes_t* nes, uint16_t write_addr, uint8_t data );
    /* Write to Apu */
    void (*mapper_apu)(nes_t* nes, uint16_t write_addr, uint8_t data );
    /* Read from Apu */
    uint8_t (*mapper_read_apu)(nes_t* nes, uint16_t write_addr );
    /* Callback at VSync */
    void (*mapper_vsync)(nes_t* nes);
    /* Callback at HSync */
    void (*mapper_hsync)(nes_t* nes);
    /* Callback at PPU read/write */
    void (*mapper_ppu)(nes_t* nes, uint16_t write_addr );
    /* Callback at Rendering Screen 1:BG, 0:Sprite */
    void (*mapper_render_screen)(nes_t* nes, uint8_t mode );
} nes_mapper_t;

/* mapper */
int nes_load_mapper(nes_t* nes);
/* prg rom */
void nes_load_prgrom_8k(nes_t* nes,uint8_t des, uint16_t src);
void nes_load_prgrom_16k(nes_t* nes,uint8_t des, uint16_t src);
void nes_load_prgrom_32k(nes_t* nes,uint8_t des, uint16_t src);
/* chr rom */
void nes_load_chrrom_1k(nes_t* nes,uint8_t des, uint8_t src);
void nes_load_chrrom_8k(nes_t* nes,uint8_t des, uint8_t src);



/* mapper */
/* iNES 1.0 mapper */
int nes_mapper0_init(nes_t* nes);
int nes_mapper1_init(nes_t* nes);
int nes_mapper2_init(nes_t* nes);
int nes_mapper3_init(nes_t* nes);
int nes_mapper4_init(nes_t* nes);
int nes_mapper5_init(nes_t* nes);
int nes_mapper6_init(nes_t* nes);
int nes_mapper7_init(nes_t* nes);
int nes_mapper8_init(nes_t* nes);
int nes_mapper9_init(nes_t* nes);
int nes_mapper10_init(nes_t* nes);
int nes_mapper11_init(nes_t* nes);
int nes_mapper12_init(nes_t* nes);
int nes_mapper13_init(nes_t* nes);
int nes_mapper14_init(nes_t* nes);
int nes_mapper15_init(nes_t* nes);
int nes_mapper16_init(nes_t* nes);
int nes_mapper17_init(nes_t* nes);
int nes_mapper18_init(nes_t* nes);
int nes_mapper19_init(nes_t* nes);
int nes_mapper20_init(nes_t* nes);
int nes_mapper21_init(nes_t* nes);
int nes_mapper22_init(nes_t* nes);
int nes_mapper23_init(nes_t* nes);
int nes_mapper24_init(nes_t* nes);
int nes_mapper25_init(nes_t* nes);
int nes_mapper26_init(nes_t* nes);
int nes_mapper27_init(nes_t* nes);
int nes_mapper28_init(nes_t* nes);
int nes_mapper29_init(nes_t* nes);
int nes_mapper30_init(nes_t* nes);
int nes_mapper31_init(nes_t* nes);
int nes_mapper32_init(nes_t* nes);
int nes_mapper33_init(nes_t* nes);
int nes_mapper34_init(nes_t* nes);
int nes_mapper35_init(nes_t* nes);
int nes_mapper36_init(nes_t* nes);
int nes_mapper37_init(nes_t* nes);
int nes_mapper38_init(nes_t* nes);
int nes_mapper39_init(nes_t* nes);
int nes_mapper40_init(nes_t* nes);
int nes_mapper41_init(nes_t* nes);
int nes_mapper42_init(nes_t* nes);
int nes_mapper43_init(nes_t* nes);
int nes_mapper44_init(nes_t* nes);
int nes_mapper45_init(nes_t* nes);
int nes_mapper46_init(nes_t* nes);
int nes_mapper47_init(nes_t* nes);
int nes_mapper48_init(nes_t* nes);
int nes_mapper49_init(nes_t* nes);
int nes_mapper50_init(nes_t* nes);
int nes_mapper51_init(nes_t* nes);
int nes_mapper52_init(nes_t* nes);
int nes_mapper53_init(nes_t* nes);
int nes_mapper54_init(nes_t* nes);
int nes_mapper55_init(nes_t* nes);
int nes_mapper56_init(nes_t* nes);
int nes_mapper57_init(nes_t* nes);
int nes_mapper58_init(nes_t* nes);
int nes_mapper59_init(nes_t* nes);
int nes_mapper60_init(nes_t* nes);
int nes_mapper61_init(nes_t* nes);
int nes_mapper62_init(nes_t* nes);
int nes_mapper63_init(nes_t* nes);
int nes_mapper64_init(nes_t* nes);
int nes_mapper65_init(nes_t* nes);
int nes_mapper66_init(nes_t* nes);
int nes_mapper67_init(nes_t* nes);
int nes_mapper68_init(nes_t* nes);
int nes_mapper69_init(nes_t* nes);
int nes_mapper70_init(nes_t* nes);
int nes_mapper71_init(nes_t* nes);
int nes_mapper72_init(nes_t* nes);
int nes_mapper73_init(nes_t* nes);
int nes_mapper74_init(nes_t* nes);
int nes_mapper75_init(nes_t* nes);
int nes_mapper76_init(nes_t* nes);
int nes_mapper77_init(nes_t* nes);
int nes_mapper78_init(nes_t* nes);
int nes_mapper79_init(nes_t* nes);
int nes_mapper80_init(nes_t* nes);
int nes_mapper81_init(nes_t* nes);
int nes_mapper82_init(nes_t* nes);
int nes_mapper83_init(nes_t* nes);
int nes_mapper84_init(nes_t* nes);
int nes_mapper85_init(nes_t* nes);
int nes_mapper86_init(nes_t* nes);
int nes_mapper87_init(nes_t* nes);
int nes_mapper88_init(nes_t* nes);
int nes_mapper89_init(nes_t* nes);
int nes_mapper90_init(nes_t* nes);
int nes_mapper91_init(nes_t* nes);
int nes_mapper92_init(nes_t* nes);
int nes_mapper93_init(nes_t* nes);
int nes_mapper94_init(nes_t* nes);
int nes_mapper95_init(nes_t* nes);
int nes_mapper96_init(nes_t* nes);
int nes_mapper97_init(nes_t* nes);
int nes_mapper98_init(nes_t* nes);
int nes_mapper99_init(nes_t* nes);
int nes_mapper100_init(nes_t* nes);
int nes_mapper101_init(nes_t* nes);
int nes_mapper102_init(nes_t* nes);
int nes_mapper103_init(nes_t* nes);
int nes_mapper104_init(nes_t* nes);
int nes_mapper105_init(nes_t* nes);
int nes_mapper106_init(nes_t* nes);
int nes_mapper107_init(nes_t* nes);
int nes_mapper108_init(nes_t* nes);
int nes_mapper109_init(nes_t* nes);
int nes_mapper110_init(nes_t* nes);
int nes_mapper111_init(nes_t* nes);
int nes_mapper112_init(nes_t* nes);
int nes_mapper113_init(nes_t* nes);
int nes_mapper114_init(nes_t* nes);
int nes_mapper115_init(nes_t* nes);
int nes_mapper116_init(nes_t* nes);
int nes_mapper117_init(nes_t* nes);
int nes_mapper118_init(nes_t* nes);
int nes_mapper119_init(nes_t* nes);
int nes_mapper120_init(nes_t* nes);
int nes_mapper121_init(nes_t* nes);
int nes_mapper122_init(nes_t* nes);
int nes_mapper123_init(nes_t* nes);
int nes_mapper124_init(nes_t* nes);
int nes_mapper125_init(nes_t* nes);
int nes_mapper126_init(nes_t* nes);
int nes_mapper127_init(nes_t* nes);
int nes_mapper128_init(nes_t* nes);
int nes_mapper129_init(nes_t* nes);
int nes_mapper130_init(nes_t* nes);
int nes_mapper131_init(nes_t* nes);
int nes_mapper132_init(nes_t* nes);
int nes_mapper133_init(nes_t* nes);
int nes_mapper134_init(nes_t* nes);
int nes_mapper135_init(nes_t* nes);
int nes_mapper136_init(nes_t* nes);
int nes_mapper137_init(nes_t* nes);
int nes_mapper138_init(nes_t* nes);
int nes_mapper139_init(nes_t* nes);
int nes_mapper140_init(nes_t* nes);
int nes_mapper141_init(nes_t* nes);
int nes_mapper142_init(nes_t* nes);
int nes_mapper143_init(nes_t* nes);
int nes_mapper144_init(nes_t* nes);
int nes_mapper145_init(nes_t* nes);
int nes_mapper146_init(nes_t* nes);
int nes_mapper147_init(nes_t* nes);
int nes_mapper148_init(nes_t* nes);
int nes_mapper149_init(nes_t* nes);
int nes_mapper150_init(nes_t* nes);
int nes_mapper151_init(nes_t* nes);
int nes_mapper152_init(nes_t* nes);
int nes_mapper153_init(nes_t* nes);
int nes_mapper154_init(nes_t* nes);
int nes_mapper155_init(nes_t* nes);
int nes_mapper156_init(nes_t* nes);
int nes_mapper157_init(nes_t* nes);
int nes_mapper158_init(nes_t* nes);
int nes_mapper159_init(nes_t* nes);
int nes_mapper160_init(nes_t* nes);
int nes_mapper161_init(nes_t* nes);
int nes_mapper162_init(nes_t* nes);
int nes_mapper163_init(nes_t* nes);
int nes_mapper164_init(nes_t* nes);
int nes_mapper165_init(nes_t* nes);
int nes_mapper166_init(nes_t* nes);
int nes_mapper167_init(nes_t* nes);
int nes_mapper168_init(nes_t* nes);
int nes_mapper169_init(nes_t* nes);
int nes_mapper170_init(nes_t* nes);
int nes_mapper171_init(nes_t* nes);
int nes_mapper172_init(nes_t* nes);
int nes_mapper173_init(nes_t* nes);
int nes_mapper174_init(nes_t* nes);
int nes_mapper175_init(nes_t* nes);
int nes_mapper176_init(nes_t* nes);
int nes_mapper177_init(nes_t* nes);
int nes_mapper178_init(nes_t* nes);
int nes_mapper179_init(nes_t* nes);
int nes_mapper180_init(nes_t* nes);
int nes_mapper181_init(nes_t* nes);
int nes_mapper182_init(nes_t* nes);
int nes_mapper183_init(nes_t* nes);
int nes_mapper184_init(nes_t* nes);
int nes_mapper185_init(nes_t* nes);
int nes_mapper186_init(nes_t* nes);
int nes_mapper187_init(nes_t* nes);
int nes_mapper188_init(nes_t* nes);
int nes_mapper189_init(nes_t* nes);
int nes_mapper190_init(nes_t* nes);
int nes_mapper191_init(nes_t* nes);
int nes_mapper192_init(nes_t* nes);
int nes_mapper193_init(nes_t* nes);
int nes_mapper194_init(nes_t* nes);
int nes_mapper195_init(nes_t* nes);
int nes_mapper196_init(nes_t* nes);
int nes_mapper197_init(nes_t* nes);
int nes_mapper198_init(nes_t* nes);
int nes_mapper199_init(nes_t* nes);
int nes_mapper200_init(nes_t* nes);
int nes_mapper201_init(nes_t* nes);
int nes_mapper202_init(nes_t* nes);
int nes_mapper203_init(nes_t* nes);
int nes_mapper204_init(nes_t* nes);
int nes_mapper205_init(nes_t* nes);
int nes_mapper206_init(nes_t* nes);
int nes_mapper207_init(nes_t* nes);
int nes_mapper208_init(nes_t* nes);
int nes_mapper209_init(nes_t* nes);
int nes_mapper210_init(nes_t* nes);
int nes_mapper211_init(nes_t* nes);
int nes_mapper212_init(nes_t* nes);
int nes_mapper213_init(nes_t* nes);
int nes_mapper214_init(nes_t* nes);
int nes_mapper215_init(nes_t* nes);
int nes_mapper216_init(nes_t* nes);
int nes_mapper217_init(nes_t* nes);
int nes_mapper218_init(nes_t* nes);
int nes_mapper219_init(nes_t* nes);
int nes_mapper220_init(nes_t* nes);
int nes_mapper221_init(nes_t* nes);
int nes_mapper222_init(nes_t* nes);
int nes_mapper223_init(nes_t* nes);
int nes_mapper224_init(nes_t* nes);
int nes_mapper225_init(nes_t* nes);
int nes_mapper226_init(nes_t* nes);
int nes_mapper227_init(nes_t* nes);
int nes_mapper228_init(nes_t* nes);
int nes_mapper229_init(nes_t* nes);
int nes_mapper230_init(nes_t* nes);
int nes_mapper231_init(nes_t* nes);
int nes_mapper232_init(nes_t* nes);
int nes_mapper233_init(nes_t* nes);
int nes_mapper234_init(nes_t* nes);
int nes_mapper235_init(nes_t* nes);
int nes_mapper236_init(nes_t* nes);
int nes_mapper237_init(nes_t* nes);
int nes_mapper238_init(nes_t* nes);
int nes_mapper239_init(nes_t* nes);
int nes_mapper240_init(nes_t* nes);
int nes_mapper241_init(nes_t* nes);
int nes_mapper242_init(nes_t* nes);
int nes_mapper243_init(nes_t* nes);
int nes_mapper244_init(nes_t* nes);
int nes_mapper245_init(nes_t* nes);
int nes_mapper246_init(nes_t* nes);
int nes_mapper247_init(nes_t* nes);
int nes_mapper248_init(nes_t* nes);
int nes_mapper249_init(nes_t* nes);
int nes_mapper250_init(nes_t* nes);
int nes_mapper251_init(nes_t* nes);
int nes_mapper252_init(nes_t* nes);
int nes_mapper253_init(nes_t* nes);
int nes_mapper254_init(nes_t* nes);
int nes_mapper255_init(nes_t* nes);
/* NES 2.0 mappers 256-511 */




#ifdef __cplusplus          
    }
#endif

#endif// _NES_MAPPER_
