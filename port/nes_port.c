/*
 * MIT License
 *
 * Copyright (c) 2022 Dozingfiretruck
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

#include <stdarg.h>
#include <SDL.h>
#include <string.h>

#include "nes_port.h"
#include "nes.h"

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

/* wait */
void nes_wait(uint32_t ms){
    SDL_Delay(ms);
}

static SDL_Window* window = NULL;
static SDL_Surface *screen = NULL;

static void sdl_event_loop(nes_t *nes) {
    SDL_Event event;
    while (!nes->nes_quit) {
        if (SDL_PollEvent(&event)){
            switch (event.type) {
                case SDL_KEYDOWN:
                    switch (event.key.keysym.scancode){
                        case 26://W
                            nes->nes_cpu.joypad.U1 = 1;
                            break;
                        case 22://S
                            nes->nes_cpu.joypad.D1 = 1;
                            break;
                        case 4://A
                            nes->nes_cpu.joypad.L1 = 1;
                            break;
                        case 7://D
                            nes->nes_cpu.joypad.R1 = 1;
                            break;
                        case 13://J
                            nes->nes_cpu.joypad.A1 = 1;
                            break;
                        case 14://K
                            nes->nes_cpu.joypad.B1 = 1;
                            break;
                        case 25://V
                            nes->nes_cpu.joypad.SE1 = 1;
                            break;
                        case 5://B
                            nes->nes_cpu.joypad.ST1 = 1;
                            break;
                        case 82://↑
                            nes->nes_cpu.joypad.U2 = 1;
                            break;
                        case 81://↓
                            nes->nes_cpu.joypad.D2 = 1;
                            break;
                        case 80://←
                            nes->nes_cpu.joypad.L2 = 1;
                            break;
                        case 79://→
                            nes->nes_cpu.joypad.R2 = 1;
                            break;
                        case 93://5
                            nes->nes_cpu.joypad.A2 = 1;
                            break;
                        case 94://6
                            nes->nes_cpu.joypad.B2 = 1;
                            break;
                        case 89://1
                            nes->nes_cpu.joypad.SE2 = 1;
                            break;
                        case 90://2
                            nes->nes_cpu.joypad.ST2 = 1;
                            break;
                        default:
                            break;
                        }
                    break;
                case SDL_KEYUP:
                    switch (event.key.keysym.scancode){
                        case 26://W
                            nes->nes_cpu.joypad.U1 = 0;
                            break;
                        case 22://S
                            nes->nes_cpu.joypad.D1 = 0;
                            break;
                        case 4://A
                            nes->nes_cpu.joypad.L1 = 0;
                            break;
                        case 7://D
                            nes->nes_cpu.joypad.R1 = 0;
                            break;
                        case 13://J
                            nes->nes_cpu.joypad.A1 = 0;
                            break;
                        case 14://K
                            nes->nes_cpu.joypad.B1 = 0;
                            break;
                        case 25://V
                            nes->nes_cpu.joypad.SE1 = 0;
                            break;
                        case 5://B
                            nes->nes_cpu.joypad.ST1 = 0;
                            break;
                        case 82://↑
                            nes->nes_cpu.joypad.U2 = 0;
                            break;
                        case 81://↓
                            nes->nes_cpu.joypad.D2 = 0;
                            break;
                        case 80://←
                            nes->nes_cpu.joypad.L2 = 0;
                            break;
                        case 79://→
                            nes->nes_cpu.joypad.R2 = 0;
                            break;
                        case 93://5
                            nes->nes_cpu.joypad.A2 = 0;
                            break;
                        case 94://6
                            nes->nes_cpu.joypad.B2 = 0;
                            break;
                        case 89://1
                            nes->nes_cpu.joypad.SE2 = 0;
                            break;
                        case 90://2
                            nes->nes_cpu.joypad.ST2 = 0;
                            break;
                        default:
                            break;
                        }
                    break;
                case SDL_QUIT:
                    nes_deinit(nes);
                    return;
            }
        }else{
            //渲染
        }
    }
}

static int NES_SDL(void *ptr){
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_JOYSTICK)) {
        SDL_Log("Can not init video, %s", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow(
            NES_NAME,
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            NES_WIDTH, NES_HEIGHT,
            SDL_WINDOW_SHOWN|SDL_WINDOW_ALLOW_HIGHDPI
    );
    if (window == NULL) {
        SDL_Log("Can not create window, %s", SDL_GetError());
        return 1;
    }
    screen = SDL_GetWindowSurface(window);
    sdl_event_loop(ptr);
    return 0;
}

int nes_initex(nes_t *nes){
    SDL_Thread *thread;
    thread = SDL_CreateThread(NES_SDL, "NES_SDL", nes);

    if (NULL == thread) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateThread failed: %s\n", SDL_GetError());
    } 
    return 0;
}

int nes_deinitex(nes_t *nes){
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

int nes_draw(size_t x1, size_t y1, size_t x2, size_t y2, nes_color_t* color_data){
    if (screen == NULL){
        return -1;
    }
    SDL_LockSurface(screen);
    int i = 0;
    for (size_t y = 0; y < y2-y1+1; y++){
        for (size_t x = 0; x < x2-x1+1; x++){
            ((uint32_t *) (screen->pixels))[(y1+y) * NES_WIDTH + x1 + x] = color_data[i++];
        }
    }
    SDL_UnlockSurface(screen);	
    return SDL_UpdateWindowSurface(window);
}
