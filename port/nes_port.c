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

#include "nes.h"

#include <SDL.h>

/* memory */
void *nes_malloc(int num){
    return SDL_malloc(num);
}

void nes_free(void *address){
    SDL_free(address);
}

void *nes_memcpy(void *str1, const void *str2, size_t n){
    return SDL_memcpy(str1, str2, n);
}

void *nes_memset(void *str, int c, size_t n){
    return SDL_memset(str,c,n);
}

int nes_memcmp(const void *str1, const void *str2, size_t n){
    return SDL_memcmp(str1,str2,n);
}

#if (NES_USE_FS == 1)
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
#endif

/* wait */
void nes_wait(uint32_t ms){
    SDL_Delay(ms);
}

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *framebuffer = NULL;

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
        }
    }
}

// #define FREQ 44100
// #define SAMPLES 2048
// static const double SoundFreq = 261.63;
// static const double TimeLag = 1.0 / FREQ;
// static int g_callbackIndex = 0;

// static void AudioCallback(void* userdata, Uint8* stream, int len) {
//     int16_t* source = (int16_t*)stream;
//     int count = len / 2;
//     double r = 0.0;
//     int startIndex = (g_callbackIndex * count) % (int)(FREQ/SoundFreq*10);
//     for (int i = 0; i < count; ++i) {
//         r = M_PI * 2.0 * SoundFreq * TimeLag * (startIndex + i);
//         source[i] = INT16_MAX * sin(r);
//     }
//     g_callbackIndex++;
// }

static int NES_SDL(void *ptr){
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_JOYSTICK| SDL_INIT_TIMER)) {
        SDL_Log("Can not init video, %s", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow(
            NES_NAME,
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            NES_WIDTH * 2, NES_HEIGHT * 2,
            SDL_WINDOW_SHOWN|SDL_WINDOW_ALLOW_HIGHDPI
    );
    if (window == NULL) {
        SDL_Log("Can not create window, %s", SDL_GetError());
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    framebuffer = SDL_CreateTexture(renderer,
                                    SDL_PIXELFORMAT_ARGB8888,
                                    SDL_TEXTUREACCESS_STREAMING,
                                    NES_WIDTH,
                                    NES_HEIGHT);

    // SDL_AudioSpec desired;
    // SDL_AudioSpec obtained;

    // desired.freq = FREQ;
    // desired.format = AUDIO_S16SYS;
    // desired.channels = 1;
    // desired.silence = 0;
    // desired.samples = SAMPLES;
    // desired.callback = AudioCallback;
    // desired.userdata = NULL;

    // SDL_AudioDeviceID nes_audio_device = SDL_OpenAudioDevice(NULL, SDL_FALSE, &desired, &obtained, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
    // if (!nes_audio_device) {
    //     SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open audio: %s\n", SDL_GetError());
    // }
    // SDL_PauseAudioDevice(nes_audio_device, SDL_FALSE);
    
    // SDL_CloseAudioDevice(nes_audio_device);
    sdl_event_loop(ptr);
    return 0;
}

int nes_initex(nes_t *nes){
    SDL_Thread *thread;
    thread = SDL_CreateThread(NES_SDL, "NES_SDL", nes);

    if (NULL == thread) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateThread failed: %s\n", SDL_GetError());
        return -1;
    } 
    return 0;
}

int nes_deinitex(nes_t *nes){
    SDL_DestroyTexture(framebuffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

int nes_draw(size_t x1, size_t y1, size_t x2, size_t y2, nes_color_t* color_data){
    if (!framebuffer){
        return -1;
    }
    SDL_Rect rect;
    rect.x = x1;
    rect.y = y1;
    rect.w = x2 - x1 + 1;
    rect.h = y2 - y1 + 1;
    SDL_UpdateTexture(framebuffer, &rect, color_data, rect.w * 4);
    return 0;
}

void nes_frame(void){
    //渲染
    SDL_RenderCopy(renderer, framebuffer, NULL, NULL);
    SDL_RenderPresent(renderer);
    //此处可做帧同步
    nes_wait(10);
}

