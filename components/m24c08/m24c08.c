#include "m24c08.h"

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

// Odczyt danych
uint8_t read_mode(){
	return eeprom_read(MODE);
}
uint16_t read_duty(){
	uint8_t buf[2];
	buf[0] = eeprom_read(DUTY1);
	buf[1] = eeprom_read(DUTY2);
	return (uint16_t)buf[0] * 256 + (uint16_t)buf[1];
}
uint8_t read_pir(){
	return eeprom_read(PIR);
}
uint8_t read_pir_time(){
	return eeprom_read(PIR_TIME);
}
uint16_t read_power_uwage(){
	uint8_t buf[2];
	buf[0] = eeprom_read(POWER_USAGE1);
	buf[1] = eeprom_read(POWER_USAGE2);
	return (uint16_t)buf[0] * 256 + (uint16_t)buf[1];
}
// Zapis danych
void write_mode(uint8_t m){
	eeprom_write(MODE, m);
}
void write_duty(uint16_t d){
	eeprom_write(DUTY1, (uint8_t)(d >> 8));  // wyższy bajt
eeprom_write(DUTY2, (uint8_t)(d & 0xFF)); // niższy bajt
}
void write_pir(uint8_t p){
	eeprom_write(PIR, p);
}
void write_pir_time(uint8_t pt){
	eeprom_write(PIR_TIME, pt);
}
void write_power_usage(uint16_t pu){
	eeprom_write(POWER_USAGE1, (uint8_t)(pu >> 8));  // wyższy bajt
	eeprom_write(POWER_USAGE2, (uint8_t)(pu & 0xFF)); // niższy bajt
}

void calc_power(float pow, uint16_t dt){
	power_used += pow * dt;
}