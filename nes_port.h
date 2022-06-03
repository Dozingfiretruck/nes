#ifndef _NES_PORT_
#define _NES_PORT_

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#ifdef __cplusplus
    extern "C" {
#endif

/* log */
void nes_printf(const char *format, ...);

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

#ifdef __cplusplus          
    }
#endif

#endif// _NES_PORT_
