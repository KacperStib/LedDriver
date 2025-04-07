#include "../../i2c/include/i2c.h"

#define BH1750_ADDR 0x23

#define OPCODE_HIGH  0x0
#define OPCODE_HIGH2 0x1
#define OPCODE_LOW   0x3

#define OPCODE_CONT 0x10
#define OPCODE_OT   0x20

#define OPCODE_POWER_DOWN 0x00
#define OPCODE_POWER_ON   0x01
#define OPCODE_MT_HI      0x40
#define OPCODE_MT_LO      0x60

#define MEAS_TIME 120

// uruchomienie czujnika
esp_err_t bh1750_power_on();

// odczyt natezenia swiatla
uint16_t bh1750_read();