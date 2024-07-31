/*
 * Copyright 2023-2024 Dozingfiretruck
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _NES_CONF_
#define _NES_CONF_

#ifdef __cplusplus
    extern "C" {
#endif

#define NES_ENABLE_SOUND        (0)       /* enable sound */
#define NES_USE_SRAM            (0)       /* use SRAM */

#define NES_FRAME_SKIP          (0)       /* skip frames */
#define NES_COLOR_DEPTH         (32)      /* color depth */
#define NES_COLOR_SWAP          (0)       /* swap color channels */
#define NES_RAM_LACK            (0)       /* lack of RAM */

#define NES_USE_FS              (1)       /* use file system */



#ifndef NES_ENABLE_SOUND
#define NES_ENABLE_SOUND        (1)
#endif

#ifndef NES_USE_FS
#define NES_USE_FS              (0)
#endif

#ifndef NES_FRAME_SKIP
#define NES_FRAME_SKIP          (0)
#endif

#ifndef NES_RAM_LACK
#define NES_RAM_LACK            (0)
#endif

#if (NES_RAM_LACK == 1)
#define NES_DRAW_SIZE           (NES_WIDTH * NES_HEIGHT / 2) 
#else
#define NES_DRAW_SIZE           (NES_WIDTH * NES_HEIGHT)
#endif

#ifndef NES_COLOR_SWAP
#define NES_COLOR_SWAP          (0)
#endif

/* Color depth:
 * - 16: RGB565
 * - 32: ARGB8888
 */
#ifndef NES_COLOR_DEPTH
#define NES_COLOR_DEPTH         (32)
#endif

#if (NES_COLOR_DEPTH == 32)
#define nes_color_t uint32_t
#elif (NES_COLOR_DEPTH == 16)
#define nes_color_t uint16_t
#else
#error "no supprt color depth"
#endif

#ifdef __cplusplus          
    }
#endif

#endif// _NES_CONF_
