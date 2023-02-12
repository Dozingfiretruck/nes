#ifndef _NES_ROM_
#define _NES_ROM_

#ifdef __cplusplus
    extern "C" {
#endif

/* NES 2.0: https://wiki.nesdev.org/w/index.php/NES_2.0 */
#define TRAINER_SIZE            (0x200)
#define PRG_ROM_UNIT_SIZE       (0x4000)
#define CHR_ROM_UNIT_SIZE       (0x2000)
typedef struct {
    uint8_t identification[4];          /*  0-3   Identification String. Must be "NES<EOF>". */
    uint8_t prg_rom_size_l;             /*  4     PRG-ROM size LSB */
    uint8_t chr_rom_size_l;             /*  5     CHR-ROM size LSB */
    struct {
        uint8_t mirroring:1;            /*  D0    Hard-wired nametable mirroring type        0: Horizontal or mapper-controlled 1: Vertical */
        uint8_t save:1;                 /*  D1    "Battery" and other non-volatile memory    0: Not present 1: Present */
        uint8_t trainer:1;              /*  D2    512-byte Trainer                           0: Not present 1: Present between Header and PRG-ROM data */
        uint8_t four_screen:1;          /*  D3    Hard-wired four-screen mode                0: No 1: Yes */
        uint8_t mapper_number_l:4;      /*  D4-7  Mapper Number D0..D3 */
    };
    struct {
        uint8_t console_type:2;         /*  D0-1  Console type   0: Nintendo Entertainment System/Family Computer 1: Nintendo Vs. System 2: Nintendo Playchoice 10 3: Extended Console Type */
        uint8_t identifier2:2;          /*  D2-3  NES 2.0 identifier */
        uint8_t mapper_number_m:4;      /*  D4-7  Mapper Number D4..D7 */
    }; 
    struct {
        uint8_t mapper_number_h:4;      /*  D0-3  Mapper number D8..D11 */
        uint8_t submapper:4;            /*  D4-7  Submapper number */
    };                                  /*  Mapper MSB/Submapper */
    struct {
        uint8_t prg_rom_size_m:4;       /*  D0-3  PRG-ROM size MSB */
        uint8_t chr_rom_size_m:4;       /*  D4-7  CHR-ROM size MSB */
    };                                  /*  PRG-ROM/CHR-ROM size MSB */
    struct {
        uint8_t prg_ram_size_m:4;       /*  D0-3  PRG-RAM (volatile) shift count */
        uint8_t eeprom_size_m:4;        /*  D4-7  PRG-NVRAM/EEPROM (non-volatile) shift count */
    };                                  /*  PRG-RAM/EEPROM size
                                            If the shift count is zero, there is no PRG-(NV)RAM.
                                            If the shift count is non-zero, the actual size is
                                            "64 << shift count" bytes, i.e. 8192 bytes for a shift count of 7. */
    struct {
        uint8_t chr_ram_size_m:4;       /*  D0-3  CHR-RAM size (volatile) shift count */
        uint8_t chr_nvram_size_m:4;     /*  D4-7  CHR-NVRAM size (non-volatile) shift count */
    };                                  /*  CHR-RAM size
                                            If the shift count is zero, there is no CHR-(NV)RAM.
                                            If the shift count is non-zero, the actual size is
                                            "64 << shift count" bytes, i.e. 8192 bytes for a shift count of 7. */
    struct {
        uint8_t timing_mode :2;         /*  D0-3    CPU/PPU timing mode 
                                                    0: RP2C02 ("NTSC NES")
                                                    1: RP2C07 ("Licensed PAL NES")
                                                    2: Multiple-region
                                                    3: UMC 6527P ("Dendy") */
        uint8_t :5;
    };                                  /*  CPU/PPU Timing */
    struct {
        uint8_t ppu_type:4;             /*  D0-3  Vs. PPU Type */
        uint8_t hardware_type:4;        /*  D4-7  Vs. Hardware Type */
    };                                  /*  When Byte 7 AND 3 =1: Vs. System Type
                                            When Byte 7 AND 3 =3: Extended Console Type */
    struct {
        uint8_t miscellaneous_number:2; /*  D0-2  Number of miscellaneous ROMs present */
        uint8_t :5;
    };                                  /*  Miscellaneous ROMs */
    struct {
        uint8_t expansion_device:5;     /*  D0-5  Default Expansion Device */
        uint8_t :2;
    };                                  /*  Default Expansion Device */
} nes_header_info_t;

typedef struct {
    uint16_t prg_rom_size;
    uint16_t chr_rom_size;
    uint8_t* prg_rom;
    uint8_t* chr_rom;
    uint8_t* sram;
    uint16_t mapper_number;             /*  Mapper Number */
    uint8_t  mirroring_type;            /*  0: Horizontal or mapper-controlled 1: Vertical */
    uint8_t  four_screen;               /*  0: No 1: Yes */
    uint8_t  save_ram;                  /*  0: Not present 1: Present */
} nes_rom_info_t;

#ifdef __cplusplus          
    }
#endif

#endif// _NES_ROM_