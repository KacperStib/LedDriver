#include "m24c08.h"

void eeprom_write(uint8_t REG, uint8_t VAL){
	i2c_write_val(M24C08_ADDR, REG, VAL);
	vTaskDelay(10 / portTICK_PERIOD_MS);
}

uint8_t eeprom_read(uint8_t REG){
	uint8_t buf = 0;
	i2c_write_reg(M24C08_ADDR, REG);
	vTaskDelay(10 / portTICK_PERIOD_MS);
	i2c_read(M24C08_ADDR, &buf, 1);
	return buf;
}
