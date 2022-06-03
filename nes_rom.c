#include "nes_rom.h"
#include "nes_config.h"
#include "nes_port.h"
#include "nes_mapper/nes_mapper.h"
#include <stdio.h>

nes_rom_info_t* nes_rom = NULL;

int nes_load_rom(const char* file_path ){
    nes_header_info_t nes_header_info = {0};

    FILE* nes_file = nes_fopen(file_path, "rb");
    if (!nes_file){
        goto err;
    } 
    nes_rom=(nes_rom_info_t *)nes_malloc(sizeof(nes_rom_info_t));
    if (nes_rom == NULL) {
        goto err;
    }
    memset(nes_rom, 0, sizeof(nes_rom_info_t));
#if (NES_USE_SRAM == 1)
    nes_rom->sram = (uint8_t*)nes_malloc(SRAM_SIZE);
#endif
    if (nes_fread(&nes_header_info, sizeof(nes_header_info), 1, nes_file)) {
        if ( nes_memcmp( nes_header_info.identification, "NES\x1a", 4 )){// check nes header
            goto err;
        }
        if (nes_header_info.flags1 & 0x04){// Trainer
#if (NES_USE_SRAM == 1)
            if (nes_fread(nes_rom->sram, 512, 1, nes_file)==0){
                goto err;
            }
#else
            nes_fseek(nes_file, 512, SEEK_CUR);
#endif
        }
        nes_rom->prg_rom = (uint8_t*)nes_malloc(0x4000 * nes_header_info.prg_rom_size);
        if (nes_rom->prg_rom == NULL) {
            goto err;
        }
        if (nes_fread(nes_rom->prg_rom, 0x4000, nes_header_info.prg_rom_size, nes_file)==0){
            goto err;
        }
        nes_rom->chr_rom = (uint8_t*)nes_malloc(0x2000 * nes_header_info.chr_rom_size);
        if (nes_rom->chr_rom == NULL) {
            goto err;
        }
        if (nes_fread(nes_rom->chr_rom, 0x2000, nes_header_info.chr_rom_size, nes_file)==0){
            goto err;
        }
        
        nes_rom->prg_rom_size = nes_header_info.prg_rom_size;
        nes_rom->chr_rom_size = nes_header_info.chr_rom_size;
        nes_rom->mapper_number = (nes_header_info.flags1 >> 4) | (nes_header_info.flags2 & 0xF0);
        nes_rom->mirroring_type = (nes_header_info.flags1 & 0x01);
        nes_rom->four_screen = (nes_header_info.flags1 & 0x08);
        nes_rom->save_ram = (nes_header_info.flags1 & 0x02);
    }else{
        goto err;
    }
    nes_fclose(nes_file);
    nes_load_mapper(nes_rom->mapper_number);
    return NES_OK;
err:
    if (nes_file){
        nes_fclose(nes_file);
    }
    nes_rom_free();
    return NES_ERROR;
}

int nes_rom_free(void){
    if (nes_rom->prg_rom){
        nes_free(nes_rom->prg_rom);
    }
    if (nes_rom->chr_rom){
        nes_free(nes_rom->chr_rom);
    }
    if (nes_rom->sram){
        nes_free(nes_rom->sram);
    }
    if (nes_rom){
        nes_free(nes_rom);
    }
    return NES_OK;
}

int nes_load_mapper(uint8_t mapper_number){
    switch (mapper_number){
        case 0 :
            return nes_mapper0_init();
        default :
            nes_printf("mapper:%03d is unsupported\n",mapper_number);
            return NES_ERROR;
    }
}
