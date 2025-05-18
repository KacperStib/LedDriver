#include <stdio.h>
#include "i2c.h"
// Pierwszy blok pamieci
#define M24C08_ADDR 0x50

#define MODE 0x01
#define DUTY1 0x02
#define DUTY2 0x03

#define PIR 0x04
#define PIR_TIME 0x05

#define POWER_USAGE1 0x06
#define POWER_USAGE2 0x07

void eeprom_write(uint8_t REG, uint8_t VAL);
uint8_t eeprom_read(uint8_t REG);

extern uint16_t power_used;

// Odczyt danych
uint8_t read_mode();
uint16_t read_duty();
uint8_t read_pir();
uint8_t read_pir_time();
uint16_t read_power_uwage();

// Zapis danych
void write_mode(uint8_t m);
void write_duty(uint16_t d);
void write_pir(uint8_t p);
void write_pir_time(uint8_t pt);
void write_power_usage(uint16_t pu);

// Obliczanie zuzytej mocy
void calc_power(float pow, uint16_t dt);