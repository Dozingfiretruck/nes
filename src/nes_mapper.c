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

#include "nes.h"

#define NES_CASE_LOAD_MAPPER(mapper_id) case mapper_id: return nes_mapper##mapper_id##_init(nes)

int nes_load_mapper(nes_t* nes){
    switch (nes->nes_rom.mapper_number){
        NES_CASE_LOAD_MAPPER(0);
        NES_CASE_LOAD_MAPPER(2);
        NES_CASE_LOAD_MAPPER(3);
        // NES_CASE_LOAD_MAPPER(4);
        NES_CASE_LOAD_MAPPER(7);
        NES_CASE_LOAD_MAPPER(94);
        NES_CASE_LOAD_MAPPER(117);
        NES_CASE_LOAD_MAPPER(180);
        default :
            nes_printf("mapper:%03d is unsupported\n",nes->nes_rom.mapper_number);
            return NES_ERROR;
    }
}
