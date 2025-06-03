#include "m24c08.h"

/*
x * 256 == x << 8
x / 256 == x >> 8

x * 16777216 == x << 32
x / 16777216 == x >> 32

& 0xFF do zabezpieczenia zeby wartosc byla 1 bajtowa
*/

uint16_t power_used = 0;

// Zapis jednego bajtu do eeprom
void eeprom_write(uint8_t REG, uint8_t VAL){
	// Zapis do rejestru
	i2c_write_val(M24C08_ADDR, REG, VAL);
	// Odczekanie po zapisie
	vTaskDelay(10 / portTICK_PERIOD_MS);
}

// Odczyt jednego bajtu z eeprom
uint8_t eeprom_read(uint8_t REG){
	uint8_t buf = 0;
	// Odpytanie rejestru
	i2c_write_reg(M24C08_ADDR, REG);
	// Odczekanie
	vTaskDelay(10 / portTICK_PERIOD_MS);
	// Odczyt wartosci
	i2c_read(M24C08_ADDR, &buf, 1);
	return buf;
}

// -----Odczyt danych-----
// tryb pracy
uint8_t read_mode(){
	return eeprom_read(MODE);
}

// wypełnienie PWM
uint16_t read_duty(){
	uint8_t buf[2];
	buf[0] = eeprom_read(DUTY1);
	buf[1] = eeprom_read(DUTY2);
	return (uint16_t)buf[0] * 256 + (uint16_t)buf[1];
}

// tryb czunjika ruchu
uint8_t read_pir(){
	return eeprom_read(PIR);
}

// czas podtrzymania
uint8_t read_pir_time(){
	return eeprom_read(PIR_TIME);
}

// zuzycie energii
uint32_t read_power_usage(){
	uint8_t buf[4];
	buf[0] = eeprom_read(POWER_USAGE1);
	buf[1] = eeprom_read(POWER_USAGE2);
	buf[2] = eeprom_read(POWER_USAGE3);
	buf[3] = eeprom_read(POWER_USAGE4);
	return (uint32_t)buf[0] * 16777216 +  // 2^24
           (uint32_t)buf[1] * 65536 +     // 2^16
           (uint32_t)buf[2] * 256 +       // 2^8
           (uint32_t)buf[3];
}

// -----Zapis danych-----
// tryb pracy
void write_mode(uint8_t m){
	eeprom_write(MODE, m);
}

// wypełnienie PWM
void write_duty(uint16_t d){
	eeprom_write(DUTY1, (uint8_t)(d >> 8));  // wyższy bajt
	eeprom_write(DUTY2, (uint8_t)(d & 0xFF)); // niższy bajt
}

// tryb czujnika ruchu
void write_pir(uint8_t p){
	eeprom_write(PIR, p);
}

// czas podtrzymania
void write_pir_time(uint8_t pt){
	eeprom_write(PIR_TIME, pt);
}

// zuzycie energii
void write_power_usage(uint32_t pu){
    eeprom_write(POWER_USAGE1, (uint8_t)(pu >> 24));  // Najstarszy bajt (MSB)
    eeprom_write(POWER_USAGE2, (uint8_t)(pu >> 16));   // Drugi bajt
    eeprom_write(POWER_USAGE3, (uint8_t)(pu >> 8));    // Trzeci bajt
    eeprom_write(POWER_USAGE4, (uint8_t)(pu));         // Najmłodszy bajt (LSB)
}