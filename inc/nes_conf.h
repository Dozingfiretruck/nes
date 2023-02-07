#ifndef _NES_CONF_
#define _NES_CONF_

#ifdef __cplusplus
    extern "C" {
#endif

#define NES_FRAME_SKIP          0

#define NES_USE_SRAM            0
#define NES_COLOR_DEPTH         32 
#define NES_RAM_LACK            0

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

#ifdef __cplusplus          
    }
#endif

#endif// _NES_CONF_
