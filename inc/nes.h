#ifndef _NES_
#define _NES_

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
    extern "C" {
#endif


#define NES_FRAME_SKIP          0

#define NES_USE_SRAM            0
#define NES_COLOR_DEPTH         32 
#define NES_RAM_LACK            0

#define NES_NAME                "NES"
#define NES_WIDTH               256
#define NES_HEIGHT              240

#define NES_PPU_CPU_CLOCKS		113

#define NES_OK                  0 
#define NES_ERROR               -1

#ifndef NES_FRAME_SKIP
#define NES_FRAME_SKIP         0
#endif

#if (NES_RAM_LACK == 1)
#define NES_DRAW_SIZE         (NES_WIDTH) 
#else
#define NES_DRAW_SIZE         (NES_WIDTH * NES_HEIGHT)
#endif

/* Color depth:
 * - 16: RGB565
 * - 32: ARGB8888
 */
#ifndef NES_COLOR_DEPTH
#define NES_COLOR_DEPTH         32
#endif

#if (NES_COLOR_DEPTH == 32)
#define nes_color_t uint32_t
#elif (NES_COLOR_DEPTH == 16)
#define nes_color_t uint16_t
#error "no supprt color depth"
#endif

/* log */
#define nes_printf(...)  printf(__VA_ARGS__)

/* memory */
void *nes_malloc(int num);
void nes_free(void *address);
void *nes_memcpy(void *str1, const void *str2, size_t n);
void *nes_memset(void *str, int c, size_t n);
int nes_memcmp(const void *str1, const void *str2, size_t n);

/* io */
FILE *nes_fopen( const char * filename, const char * mode );
size_t nes_fread(void *ptr, size_t size_of_elements, size_t number_of_elements, FILE *a_file);
int nes_fseek(FILE *stream, long int offset, int whence);
int nes_fclose( FILE *fp );

void nes_wait(uint32_t ms);

int nes_draw(size_t x1, size_t y1, size_t x2, size_t y2, nes_color_t* color_data);

#if (NES_USE_SRAM == 1)
    #define SRAM_SIZE           (0x2000)
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


// https://www.nesdev.org/wiki/Controller_reading
#define NES_CPU_RAM_SIZE        0x800   /*  2KB */
#define NES_PPU_VRAM_SIZE       0x1000  /*  4KB */

#define NES_VERCTOR_NMI         0xFFFA  /*  NMI vector (NMI=not maskable interupts) */
#define NES_VERCTOR_RESET       0xFFFC  /*  Reset vector */
#define NES_VERCTOR_IRQBRK      0xFFFE  /*  IRQ vector */

/*
Bit No. 15      14      13      12      11      10      9       8
        A1      B1      Select1 Start1  Up1     Down1   Left1   Right1 
Bit No. 7       6       5       4       3       2       1       0
        A2      B2      Select2 Start2  Up2     Down2   Left2   Right2 
*/
typedef struct {
    uint8_t offset1;
    uint8_t offset2;
    uint8_t mask;
    union {
        struct {
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
        };
        uint16_t joypad;
    };
} nes_joypad_t;

// https://www.nesdev.org/wiki/CPU_registers
typedef struct {
    /*  CPU registers */
    uint8_t A;                          /*  Accumulator */
    uint8_t X;                          /*  Indexes X */
    uint8_t Y;                          /*  Indexes Y */
    uint16_t PC;                        /*  Program Counter */
    uint8_t SP;                         /*  Stack Pointer */
    union {
        struct {
            uint8_t C:1;                /*  carry flag (1 on unsigned overflow) */
            uint8_t Z:1;                /*  zero flag (1 when all bits of a result are 0) */
            uint8_t I:1;                /*  IRQ flag (when 1, no interupts will occur (exceptions are IRQs forced by BRK and NMIs)) */
            uint8_t D:1;                /*  decimal flag (1 when CPU in BCD mode) */
            uint8_t B:1;                /*  break flag (1 when interupt was caused by a BRK) */
            uint8_t U:1;                /*  unused (always 1) */
            uint8_t V:1;                /*  overflow flag (1 on signed overflow) */
            uint8_t N:1;                /*  negative flag (1 when result is negative) */
        };
        uint8_t P;                      /*  Status Register */
    };
    uint32_t cycles;  
    uint8_t opcode;     
    uint8_t cpu_ram[NES_CPU_RAM_SIZE];
    uint8_t* prg_banks[4];              /*  4 bank ( 8Kb * 4 ) = 32KB  */
    nes_joypad_t joypad;
} nes_cpu_t;

// https://www.nesdev.org/wiki/PPU_OAM
typedef struct{
    uint8_t	y;		                    /*  Y position of top of sprite */
    union {
        struct {
            uint8_t pattern_8x16:1;     /*  Bank ($0000 or $1000) of tiles */
            uint8_t tile_index_8x16 :7; /*  Tile number of top of sprite (0 to 254; bottom half gets the next tile) */
        };
        uint8_t	tile_index_number;	    /*  Tile index number */
    };
    union {
        struct {
            uint8_t sprite_palette:2;   /*  Palette (4 to 7) of sprite */
            uint8_t :3;                 /*  nimplemented (read 0) */
            uint8_t priority :1;        /*  Priority (0: in front of background; 1: behind background) */
            uint8_t flip_h :1;          /*  Flip sprite horizontally */
            uint8_t flip_v :1;          /*  Flip sprite vertically */
        };
        uint8_t	attributes;	            /*  Attributes */
    };
    uint8_t	x;		                    /*  X position of left side of sprite. */
} sprite_info_t;

// https://www.nesdev.org/wiki/PPU_registers
typedef struct {
    union {
        struct {
            uint8_t ppu_vram0[NES_PPU_VRAM_SIZE / 4];
            uint8_t ppu_vram1[NES_PPU_VRAM_SIZE / 4];
            uint8_t ppu_vram2[NES_PPU_VRAM_SIZE / 4];
            uint8_t ppu_vram3[NES_PPU_VRAM_SIZE / 4];
        };
        uint8_t ppu_vram[NES_PPU_VRAM_SIZE];
    };
    union {
        struct {
            uint8_t CTRL_N:2;           /*  Base nametable address (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00) */
            uint8_t CTRL_I:1;           /*  VRAM address increment per CPU read/write of PPUDATA (0: add 1, going across; 1: add 32, going down) */
            uint8_t CTRL_S:1;           /*  Sprite pattern table address for 8x8 sprites (0: $0000; 1: $1000; ignored in 8x16 mode) */
            uint8_t CTRL_B:1;           /*  Background pattern table address (0: $0000; 1: $1000) */
            uint8_t CTRL_H:1;           /*  Sprite size (0: 8x8 pixels; 1: 8x16 pixels – see PPU OAM#Byte 1) */
            uint8_t CTRL_P:1;           /*  (0: read backdrop from EXT pins; 1: output color on EXT pins) */
            uint8_t CTRL_V:1;           /*  Generate an NMI at the start of the vertical blanking interval (0: off; 1: on) */
        };
        uint8_t ppu_ctrl;
    };
    union {
        struct {
            uint8_t MASK_Gr:1;          /*  Greyscale (0: normal color, 1: produce a greyscale display) */
            uint8_t MASK_m:1;           /*  1: Show background in leftmost 8 pixels of screen, 0: Hide */
            uint8_t MASK_M:1;           /*  1: Show sprites in leftmost 8 pixels of screen, 0: Hide */
            uint8_t MASK_b:1;           /*  1: Show background */
            uint8_t MASK_s:1;           /*  1: Show sprites */
            uint8_t MASK_R:1;           /*  Emphasize red (green on PAL/Dendy) */
            uint8_t MASK_G:1;           /*  Emphasize green (red on PAL/Dendy) */
            uint8_t MASK_B:1;           /*  Emphasize blue */
        };
        uint8_t ppu_mask;
    };
    union {
        struct {
            uint8_t  :4;    
            uint8_t STATUS_F:1;         /*  VRAM write flag: 0 = write valid, 1 = write ignored */
            uint8_t STATUS_O:1;         /*  Sprite overflow. The intent was for this flag to be set
                                            whenever more than eight sprites appear on a scanline, but a
                                            hardware bug causes the actual behavior to be more complicated
                                            and generate false positives as well as false negatives; see
                                            PPU sprite evaluation. This flag is set during sprite
                                            evaluation and cleared at dot 1 (the second dot) of the
                                            pre-render line. */
            uint8_t STATUS_S:1;         /*  Sprite 0 Hit.  Set when a nonzero pixel of sprite 0 overlaps
                                            a nonzero background pixel; cleared at dot 1 of the pre-render
                                            line.  Used for raster timing. */
            uint8_t STATUS_V:1;         /*  Vertical blank has started (0: not in vblank; 1: in vblank).
                                            Set at dot 1 of line 241 (the line *after* the post-render
                                            line); cleared after reading $2002 and at dot 1 of the
                                            pre-render line. */
        };
        uint8_t ppu_status;
    };
    union {
        struct {
            uint8_t* pattern_table[8];
            uint8_t* name_table[4];
        };
        uint8_t* chr_banks[16];         /*  16k chr_banks,without background_palette and sprite_palette
                                            0 - 3 pattern_table_0 4k
                                            4 - 7 pattern_table_1 4k
                                            8     name_table_0    1k
                                            9     name_table_1    1k
                                            10    name_table_2    1k
                                            11    name_table_3    1k
                                            12-15 mirrors */
    };
    union {
		struct{ // Scroll
			uint16_t coarse_x  : 5;     
			uint16_t coarse_y  : 5;     
			uint16_t nametable : 2;     
			uint16_t fine_y    : 3;     
			uint16_t           : 1;     
		}v;
        uint16_t v_reg;                 /*  Current VRAM address (15 bits) */
    };
    union {
		struct{ // Scroll
			uint16_t coarse_x  : 5;
			uint16_t coarse_y  : 5;
			uint16_t nametable : 2;
			uint16_t fine_y    : 3;
			uint16_t           : 1;
		}t;
        uint16_t t_reg;                 /*  Temporary VRAM address (15 bits); can also be thought of as the address of the top left onscreen tile. */
    };
    struct {
        uint8_t x:3;                    /*  Fine X scroll (3 bits) */
        uint8_t w:1;                    /*  First or second write toggle (1 bit) */
        uint8_t :4;// 可利用做xxx标志位
    };
    uint8_t oam_addr;                   /*  OAM read/write address */
    union {
        sprite_info_t sprite_info[0x100 / 4];
        uint8_t oam_data[0x100];        /*  OAM data read/write 
                                            The OAM (Object Attribute Memory) is internal memory inside the PPU that contains a display list of up to 64 sprites, 
                                            where each sprite's information occupies 4 bytes.*/
    };
    uint8_t buffer;                     /*  PPU internal buffer */
    uint8_t palette_indexes[0x20];      /*  $3F00-$3F1F Palette RAM indexes */
    nes_color_t palette[0x20];
} nes_ppu_t;

typedef struct {
    nes_color_t nes_draw_data[NES_DRAW_SIZE];
    nes_rom_info_t nes_rom;
    nes_cpu_t nes_cpu;
    nes_ppu_t nes_ppu;
} nes_t;

typedef struct {
    /* Initialize Mapper */
    void (*mapper_init)(void);
    /* Write to Mapper */
    void (*mapper_write)( uint16_t write_addr, uint8_t data );
    /* Write to SRAM */
    void (*mapper_sram)( uint16_t write_addr, uint8_t data );
    /* Write to Apu */
    void (*mapper_apu)( uint16_t write_addr, uint8_t data );
    /* Read from Apu */
    uint8_t (*mapper_read_apu)( uint16_t write_addr );
    /* Callback at VSync */
    void (*mapper_vsync)(void);
    /* Callback at HSync */
    void (*mapper_hsync)(void);
    /* Callback at PPU read/write */
    void (*mapper_ppu)( uint16_t write_addr );
    /* Callback at Rendering Screen 1:BG, 0:Sprite */
    void (*mapper_render_screen)( uint8_t mode );
} nes_mapper_t;

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

void nes_vblank_start(nes_t* nes);
void nes_vblank_end(nes_t* nes);

void nes_run(nes_t* nes);

#ifdef __cplusplus          
    }
#endif

#endif// _NES_
