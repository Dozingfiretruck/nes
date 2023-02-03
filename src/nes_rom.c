
#include "nes.h"

nes_t* nes_load_rom(const char* file_path ){
    nes_header_info_t nes_header_info = {0};
    nes_t* nes = NULL;

    FILE* nes_file = nes_fopen(file_path, "rb");
    if (!nes_file){
        goto error;
    } 
    nes = (nes_t *)nes_malloc(sizeof(nes_t));
    if (nes == NULL) {
        goto error;
    }
    memset(nes, 0, sizeof(nes_t));

#if (NES_USE_SRAM == 1)
    nes->nes_rom.sram = (uint8_t*)nes_malloc(SRAM_SIZE);
#endif
    if (nes_fread(&nes_header_info, sizeof(nes_header_info), 1, nes_file)) {
        if ( nes_memcmp( nes_header_info.identification, "NES\x1a", 4 )){
            goto error;
        }
        if (nes_header_info.trainer){
#if (NES_USE_SRAM == 1)
            if (nes_fread(nes->nes_rom.sram, TRAINER_SIZE, 1, nes_file)==0){
                goto error;
            }
#else
            nes_fseek(nes_file, TRAINER_SIZE, SEEK_CUR);
#endif
        }
        nes->nes_rom.prg_rom_size = ((nes_header_info.prg_rom_size_m << 8) & 0xF00) | nes_header_info.prg_rom_size_l;
        nes->nes_rom.chr_rom_size = ((nes_header_info.prg_rom_size_m << 8) & 0xF00) | nes_header_info.chr_rom_size_l;
        nes->nes_rom.mapper_number = ((nes_header_info.mapper_number_h << 8) & 0xF00) | ((nes_header_info.mapper_number_m << 4) & 0xF0) | (nes_header_info.mapper_number_l & 0x0F);
        nes->nes_rom.mirroring_type = (nes_header_info.mirroring);
        nes->nes_rom.four_screen = (nes_header_info.four_screen);
        nes->nes_rom.save_ram = (nes_header_info.save);

        nes->nes_rom.prg_rom = (uint8_t*)nes_malloc(PRG_ROM_UNIT_SIZE * nes->nes_rom.prg_rom_size);
        if (nes->nes_rom.prg_rom == NULL) {
            goto error;
        }
        if (nes_fread(nes->nes_rom.prg_rom, PRG_ROM_UNIT_SIZE, nes->nes_rom.prg_rom_size, nes_file)==0){
            goto error;
        }
        nes->nes_rom.chr_rom = (uint8_t*)nes_malloc(CHR_ROM_UNIT_SIZE * (nes->nes_rom.chr_rom_size ? nes->nes_rom.chr_rom_size : 1));
        if (nes->nes_rom.chr_rom == NULL) {
            goto error;
        }
        if (nes_fread(nes->nes_rom.chr_rom, CHR_ROM_UNIT_SIZE, (nes->nes_rom.chr_rom_size ? nes->nes_rom.chr_rom_size : 1), nes_file)==0){
            goto error;
        }
        
    }else{
        goto error;
    }
    nes_fclose(nes_file);
    nes_init(nes);
    nes_load_mapper(nes);
    return nes;
error:
    if (nes_file){
        nes_fclose(nes_file);
    }
    if (nes){
        nes_rom_free(nes);
    }
    return NULL;
}
// release
int nes_rom_free(nes_t* nes){
    if (nes->nes_rom.prg_rom){
        nes_free(nes->nes_rom.prg_rom);
    }
    if (nes->nes_rom.chr_rom){
        nes_free(nes->nes_rom.chr_rom);
    }
    if (nes->nes_rom.sram){
        nes_free(nes->nes_rom.sram);
    }
    if (nes){
        nes_free(nes);
    }
    return NES_OK;
}

