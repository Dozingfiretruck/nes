#ifndef _NES_CPU_
#define _NES_CPU_

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
    extern "C" {
#endif

#define NES_CPU_RAM_SIZE        0x800       //2KB
#define NES_PPU_VRAM_SIZE       0x1000      //4KB

#define NES_VERCTOR_NMI         0xFFFA      // NMI vector (NMI=not maskable interupts)
#define NES_VERCTOR_RESET       0xFFFC      // Reset vector
#define NES_VERCTOR_IRQBRK      0xFFFE      // IRQ vector

/* NES 2.0: https://wiki.nesdev.org/w/index.php/NES_2.0 */
typedef struct {
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

typedef struct {
    uint8_t prg_rom_size;
    uint8_t chr_rom_size;
    uint8_t* prg_rom;
    uint8_t* chr_rom;
    uint8_t* sram;
    uint8_t  mapper_number;
    uint8_t  mirroring_type;    //0: Horizontal or mapper-controlled 1: Vertical
    uint8_t  four_screen;       //0: No 1: Yes
    uint8_t  save_ram;          //0: Not present 1: Present
    // uint8_t     reserved[4];
} nes_rom_info_t;

/* 
CPU 地址空间
+---------+-------+-------+-----------------------+
| 地址    | 大小  | 标记  |         描述          |
+---------+-------+-------+-----------------------+
| $0000   | $800  |       | RAM                   |
| $0800   | $800  | M     | RAM                   |
| $1000   | $800  | M     | RAM                   |
| $1800   | $800  | M     | RAM                   |
| $2000   | 8     |       | Registers             |
| $2008   | $1FF8 | R     | Registers             |
| $4000   | $20   |       | Registers             |
| $4020   | $1FDF |       | Expansion ROM         |
| $6000   | $2000 |       | SRAM                  |
| $8000   | $4000 |       | PRG-ROM               |
| $C000   | $4000 |       | PRG-ROM               |
+---------+-------+-------+-----------------------+
标记图例: M = $0000的镜像
            R = $2000-2008 每 8 bytes 的镜像
        (e.g. $2008=$2000, $2018=$2000, etc.)
*/

/*
Bit No. 7  6  5  4  3  2  1  0
        N  V  U  B  D  I  Z  C 
*/
typedef struct {
    uint8_t C:1;    //carry flag (1 on unsigned overflow)
    uint8_t Z:1;    //zero flag (1 when all bits of a result are 0)
    uint8_t I:1;    //IRQ flag (when 1, no interupts will occur (exceptions are IRQs forced by BRK and NMIs))
    uint8_t D:1;    //decimal flag (1 when CPU in BCD mode)
    uint8_t B:1;    //break flag (1 when interupt was caused by a BRK)
    uint8_t U:1;    //unused (always 1)
    uint8_t V:1;    //overflow flag (1 on signed overflow)
    uint8_t N:1;    //negative flag (1 when result is negative)
} nes_status_flag_t;

/*
Bit No. 15      14      13      12      11      10      9       8
        A1      B1      Select1 Start1  Up1     Down1   Left1   Right1 
Bit No. 7       6       5       4       3       2       1       0
        A2      B2      Select2 Start2  Up2     Down2   Left2   Right2 
*/
typedef struct {
    uint8_t R2:1;   
    uint8_t L2:1;    
    uint8_t D2:1;    
    uint8_t U2:1;  
    uint8_t ST2:1; 
    uint8_t SE2:1;
    uint8_t B2:1;    
    uint8_t A2:1;
    uint8_t R1:1;   
    uint8_t L1:1;    
    uint8_t D1:1;    
    uint8_t U1:1;  
    uint8_t ST1:1; 
    uint8_t SE1:1;
    uint8_t B1:1;    
    uint8_t A1:1;  
} nes_joypad_flag_t;

typedef struct {
    uint8_t offset1;
    uint8_t offset2;
    uint8_t mask;
    union {
        nes_joypad_flag_t J;
        uint16_t joypad;
    };
} nes_joypad_t;

typedef struct {
    uint32_t cycles;  
    uint8_t opcode;     
    uint8_t A;          //Accumulator
    uint8_t X;          //Indexes X
    uint8_t Y;          //Indexes Y
    union {
        nes_status_flag_t P;//Status Register
        uint8_t UP;
    };
    uint8_t SP;         //Stack Pointer
    uint16_t PC;        //Program Counter
    uint8_t cpu_ram[NES_CPU_RAM_SIZE];
    uint8_t* prg_banks[4];//4 bank ( 8Kb * 4 ) = 32KB 
    nes_joypad_t joypad;
} nes_cpu_t;

typedef struct {
    void (*instruction)(void);      //instructions 
    uint16_t (*addressing_mode)(void);  //addressing_mode
    uint8_t	ticks;
} nes_opcode_t;

#ifdef __cplusplus          
    }
#endif

#endif// _NES_
