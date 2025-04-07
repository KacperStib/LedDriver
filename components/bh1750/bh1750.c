#include "include/bh1750.h"

esp_err_t bh1750_power_on(){
	// opecode do uruchomienia czujnika
	err = i2c_write_reg(BH1750_ADDR, OPCODE_POWER_ON);
	// ustaweienie czasu pomiaru
	err = i2c_write_reg(BH1750_ADDR, OPCODE_MT_HI | (MEAS_TIME >> 5));
	err = i2c_write_reg(BH1750_ADDR, OPCODE_MT_LO | (MEAS_TIME & 0x1F));
	// rozpoczecie pomiaru ciaglego high
	err = i2c_write_reg(BH1750_ADDR, OPCODE_CONT | OPCODE_HIGH);
	return err;
}

uint16_t bh1750_read(){
	uint8_t buf[2];
	uint16_t lux;
	// odczytanie surowej wartosci
	err = i2c_read(BH1750_ADDR, buf, 2);
	if (err != ESP_OK)
    	ESP_LOGE("BH1750", "I2C CMD ERROR: 0x%x", err);
    	
	for(int i = 0; i < 2 ; i++)
    	printf(" %d ", buf[i]);
    printf("\n");	
	// konwersja na luksy
	lux = (buf[0] << 8) | buf[1];
    lux = (lux * 10) / 12;
    // zwrocenie luksow
    return lux;
}