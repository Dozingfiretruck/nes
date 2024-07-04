/*
 * MIT License
 *
 * Copyright (c) 2023 Dozingfiretruck
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _NES_PORT_
#define _NES_PORT_

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "nes_conf.h"

#ifdef __cplusplus
    extern "C" {
#endif

/* log */
#ifdef __DEBUG__
#define nes_printf(format,...)  printf("%s " format,__func__,##__VA_ARGS__)
#else
#define nes_printf(format,...)
#endif

/* memory */
void *nes_malloc(int num);
void nes_free(void *address);
void *nes_memcpy(void *str1, const void *str2, size_t n);
void *nes_memset(void *str, int c, size_t n);
int nes_memcmp(const void *str1, const void *str2, size_t n);

#if (NES_USE_FS == 1)

/* io */
FILE *nes_fopen(const char * filename, const char * mode );
size_t nes_fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t nes_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
int nes_fseek(FILE *stream, long int offset, int whence);
int nes_fclose(FILE *stream );

#endif

int nes_draw(int x1, int y1, int x2, int y2, nes_color_t* color_data);
int nes_sound_output(uint8_t *buffer, size_t len);

#ifdef __cplusplus          
    }
#endif

#endif// _NES_PORT_
