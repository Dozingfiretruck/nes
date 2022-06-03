#include "nes_port.h"
#include <stdarg.h>

#define NES_LOG_BUF_SIZE 512
static char nes_log_buf[NES_LOG_BUF_SIZE];

/* log */
void nes_printf(const char *format, ...){
    va_list args;
    va_start(args, format);
    vsnprintf(nes_log_buf, sizeof(nes_log_buf), format, args);
    printf("%s", nes_log_buf);
    va_end(args);
}

/* memory */
void *nes_malloc(int num){
    return malloc(num);
}

void nes_free(void *address){
    free(address);
}

void *nes_memcpy(void *str1, const void *str2, size_t n){
    return memcpy(str1, str2, n);
}

void *nes_memset(void *str, int c, size_t n){
    return memset(str,c,n);
}

int nes_memcmp(const void *str1, const void *str2, size_t n){
    return memcmp(str1,str2,n);
}

/* io */
FILE *nes_fopen( const char * filename, const char * mode ){
    return fopen(filename,mode);
}

size_t nes_fread(void *ptr, size_t size_of_elements, size_t number_of_elements, FILE *a_file){
    return fread(ptr, size_of_elements, number_of_elements,a_file);
}

int nes_fseek(FILE *stream, long int offset, int whence){
    return fseek(stream,offset,whence);
}

int nes_fclose( FILE *fp ){
    return fclose(fp);
}


