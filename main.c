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


#include "nes.h"

int main(int argc, char *argv[]){
    if (argc == 2){
        const char *nes_file_path = argv[1];
        size_t nes_file_path_len = strlen(nes_file_path);
        if (memcmp(nes_file_path+nes_file_path_len-4,".nes",4)==0 || memcmp(nes_file_path+nes_file_path_len-4,".NES",4)==0){
            nes_printf("nes_file_path:%s\n",nes_file_path);
            nes_t* nes = nes_load_file(nes_file_path);
            if (!nes){
                nes_printf("nes load file fail\n");
                goto error;
            }
            nes_run(nes);
            nes_unload_file(nes);
            return 0;
        }else{
            nes_printf("Please enter xxx.nes\n");
            goto error;
        }
    }else{
        nes_printf("Please enter the nes file path\n");
        goto error;
    }
error:
    getchar();
    return -1;
}

