#include <stdio.h>
#include "i2c.h"
// Pierwszy blok pamieci
#define M24C08_ADDR 0x50

void eeprom_write(uint8_t REG, uint8_t VAL);
uint8_t eeprom_read(uint8_t REG);