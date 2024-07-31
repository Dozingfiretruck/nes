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
