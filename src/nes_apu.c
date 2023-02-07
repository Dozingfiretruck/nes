#include "nes_port.h"
#include "nes.h"

uint8_t nes_read_apu_register(nes_t *nes,uint16_t address){
    uint8_t data = 0;
    nes_printf("nes_read apu %04X %02X\n",address,data);
    return data;
}

void nes_write_apu_register(nes_t* nes,uint16_t address,uint8_t data){
    nes_printf("nes_write apu %04X %02X\n",address,data);
}
