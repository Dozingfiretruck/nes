#ifndef _NES_
#define _NES_

#include <stdint.h>

#include "nes_rom.h"
#include "nes_cpu.h"
#include "nes_ppu.h"

#ifdef __cplusplus
    extern "C" {
#endif

#define NES_NAME                "NES"
#define NES_WIDTH               256
#define NES_HEIGHT              240

#define NES_OK                  0 
#define NES_ERROR               -1


typedef struct {
    nes_color_t nes_draw_data[NES_DRAW_SIZE];
    nes_rom_info_t nes_rom;
    nes_cpu_t nes_cpu;
    nes_ppu_t nes_ppu;
} nes_t;

void nes_nmi(nes_t* nes);
void nes_opcode(nes_t* nes,uint16_t ticks);

int nes_init(nes_t *nes);
int nes_deinit(nes_t *nes);

int nes_initex(nes_t* nes);
int nes_deinitex(nes_t* nes);

int nes_rom_free(nes_t* nes);

void nes_cpu_reset(nes_t* nes);

int nes_load_mapper(nes_t* nes);
void nes_load_prgrom_8k(nes_t* nes,int des, int src);
void nes_load_chrrom_1k(nes_t* nes,int des, int src);
int nes_mapper0_init(nes_t* nes);


nes_t* nes_load_rom(const char* file_path);
void nes_ppu_init(nes_t *nes);
void nes_cpu_init(nes_t *nes);

uint8_t nes_read_ppu_register(nes_t *nes,uint16_t address);
void nes_write_ppu_register(nes_t *nes,uint16_t address, uint8_t data);

uint8_t nes_read_apu_register(nes_t *nes,uint16_t address);
void nes_write_apu_register(nes_t* nes,uint16_t address,uint8_t data);

void nes_run(nes_t* nes);

#ifdef __cplusplus          
    }
#endif

#endif// _NES_
