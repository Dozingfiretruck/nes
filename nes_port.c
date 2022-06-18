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

#include <SDL.h>

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Surface *screen = NULL;
#define FRAMERATE 60

void event_loop() {
    while (1) {
        uint32_t begin = SDL_GetTicks();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_MOUSEMOTION:
                    SDL_Log("Mouse move(x:%d,y:%d)", event.motion.x, event.motion.y);
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    SDL_Log("Mouse down(x:%d,y:%d)", event.button.x, event.button.y);
                    break;
                case SDL_MOUSEBUTTONUP:
                    SDL_Log("Mouse up(x:%d,y:%d)", event.button.x, event.button.y);
                    break;
                case SDL_QUIT:
                    return;
            }
        }
        uint32_t current = SDL_GetTicks();
        uint32_t cost = current - begin;
        uint32_t frame = 1000 / FRAMERATE;

        if (frame > cost) {
            SDL_Delay(frame - cost);
        }
    }
}


static int NES_SDL(void *ptr){
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)) {
        SDL_Log("Can not init video, %s", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow(
            NES_NAME,
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            NES_WIDTH, NES_HEIGHT,
            SDL_WINDOW_SHOWN
    );
    if (window == NULL) {
        SDL_Log("Can not create window, %s", SDL_GetError());
        return 1;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (renderer == NULL) {
        SDL_Log("Can not create renderer, %s", SDL_GetError());
        return 1;
    }

    screen = SDL_GetWindowSurface(window);

    // SDL_RenderPresent(renderer);

    // uint32_t* color = nes_malloc(4*4*4);
    // for (size_t i = 0; i < 4*4; i++){
    //     color[i] = 0xffff0000;
    // }
    // nes_draw(0, 0, 3, 3, color);

    event_loop();

    nes_deinit();

    return 0;
}

int nes_init(void){
    SDL_Thread *thread;
    int         threadReturnValue;
    thread = SDL_CreateThread(NES_SDL, "NES_SDL", (void *)NULL);

    if (NULL == thread) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateThread failed: %s\n", SDL_GetError());
    } 
    return 0;
}

int nes_deinit(void){
    // SDL_DestroyTexture(framebuffer);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

int nes_draw(size_t x1, size_t y1, size_t x2, size_t y2, uint32_t* color_data){
    int i = 0;
    for (size_t y = 0; y < y2-y1+1; y++){
        for (size_t x = 0; x < x2-x1+1; x++){
            ((uint32_t *) (screen->pixels))[(y1+y) * NES_WIDTH + x1 + x] = color_data[i];
            i++;
        }
    }
    SDL_UpdateWindowSurface(window);
    return 0;
}
