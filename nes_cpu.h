#ifndef _NES_CPU_
#define _NES_CPU_

#include "nes.h"
#include "nes_mapper/nes_mapper.h"
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

extern uint8_t CPU_RAM[];         // main memory
extern uint8_t prg_banks[];         // Bank

#define NES_VERCTOR_NMI      0xFFFA     // NMI vector (NMI=not maskable interupts)
#define NES_VERCTOR_RESET    0xFFFC     // Reset vector
#define NES_VERCTOR_IRQBRK   0xFFFE     // IRQ vector

/*
Bit No. 7  6  5  4  3  2  1  0
        N  V  U  B  D  I  Z  C 
*/
typedef struct nes_status_flag{
    uint8_t C:1;    //carry flag (1 on unsigned overflow)
    uint8_t Z:1;    //zero flag (1 when all bits of a result are 0)
    uint8_t I:1;    //IRQ flag (when 1, no interupts will occur (exceptions are IRQs forced by BRK and NMIs))
    uint8_t D:1;    //decimal flag (1 when CPU in BCD mode)
    uint8_t B:1;    //break flag (1 when interupt was caused by a BRK)
    uint8_t U:1;    //unused (always 1)
    uint8_t V:1;    //overflow flag (1 on signed overflow)
    uint8_t N:1;    //negative flag (1 when result is negative)
} nes_status_flag_t;

typedef struct nes_cpu6502{
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
} nes_cpu6502_t;

typedef struct nes_opcode{
    void (*instruction)(void);      //instructions 
    uint16_t (*addressing_mode)(void);  //addressing_mode
    uint8_t	ticks;
} nes_opcode_t;

static nes_opcode_t nes_opcode_table[256] ;

uint8_t nes_read_cpu(uint16_t address);
void nes_write_cpu(uint16_t address, uint8_t data);
uint16_t nes_read_cpu_word(uint16_t address);
void nes_write_cpu_word(uint16_t address, uint16_t data);

void nes_cpu_reset(void);

#endif// _NES_CPU_
