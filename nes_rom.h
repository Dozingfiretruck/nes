#ifndef _NES_ROM_
#define _NES_ROM_

#include <stdint.h>

/* NES 2.0: https://wiki.nesdev.org/w/index.php/NES_2.0 */

typedef struct nes_header_info{
    uint8_t identification[4];      //0-3   Identification String. Must be "NES<EOF>".
    uint8_t prg_rom_size;           //4     PRG-ROM size LSB
    uint8_t chr_rom_size;           //5     PRG-ROM size LSB
    uint8_t flags1;                 //6     Flags 6
    //D0    Hard-wired nametable mirroring type        0: Horizontal or mapper-controlled 1: Vertical
    //D1    "Battery" and other non-volatile memory    0: Not present 1: Present
    //D2    512-byte Trainer                           0: Not present 1: Present between Header and PRG-ROM data
    //D3    Hard-wired four-screen mode                0: No 1: Yes
    //D4-7  Mapper Number D0..D3
    uint8_t flags2;                 //7     Flags 7
    //D0-1  Console type        0: Nintendo Entertainment System/Family Computer 1: Nintendo Vs. System 2: Nintendo Playchoice 10 3: Extended Console Type
    //D2-3  NES 2.0 identifier
    //D4-7  Mapper Number D4..D7
    uint8_t reserve[8];             //8-15  
} nes_header_info_t;

typedef struct nes_rom_info{
    uint8_t prg_rom_size;
    uint8_t chr_rom_size;
    uint8_t* prg_rom;
    uint8_t* chr_rom;
    uint8_t* sram;
    uint8_t  mapper_number;
    uint8_t  mirroring_type;    //0: Horizontal or mapper-controlled 1: Vertical
    uint8_t     four_screen;    //0: No 1: Yes
    uint8_t     save_ram;       //0: Not present 1: Present
    // uint8_t     reserved[4];
} nes_rom_info_t;

extern nes_rom_info_t* nes_rom;

int nes_load_mapper(uint8_t mapper_number);
int nes_load_rom(const char* file_path);
int nes_rom_free(void);

#endif// _NES_ROM_
