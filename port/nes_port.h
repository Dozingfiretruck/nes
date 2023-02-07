#ifndef _NES_PORT_
#define _NES_PORT_

#include <stdint.h>
#include <stdio.h>
#include "nes_conf.h"

#ifdef __cplusplus
    extern "C" {
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


#ifdef __cplusplus          
    }
#endif

#endif// _NES_PORT_
