#ifndef _NES_MAPPER_
#define _NES_MAPPER_

#ifdef __cplusplus
    extern "C" {
#endif

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

#ifdef __cplusplus          
    }
#endif

#endif// _NES_MAPPER_
