#include <stdio.h>
#include "esp_err.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "i2c.h"
#include "bh1750.h"
#include "sht40.h"
//#include "../components/i2c/include/i2c.h"
//#include "../components/bh1750/include/bh1750.h"

uint16_t lux = 0;
float temp = 0.0, RH = 0.0;

void read_lux(){
	for(;;){
		lux = bh1750_read();
		printf("Lux: %d\n", lux);
		vTaskDelay(pdMS_TO_TICKS(2000));
		SHT41measurment(&temp, &RH);
		printf("Temperature: %f\nRH: %f\n", temp, RH);
		vTaskDelay(pdMS_TO_TICKS(2000));
	}
}

void read_temp(){
	for(;;){
		SHT41measurment(&temp, &RH);
		printf("Temperature: %f\nRH: %f\n", temp, RH);
		vTaskDelay(pdMS_TO_TICKS(2000));
	}
}

void app_main(void)
{	
	// inicjalizacja magistrali
	if (i2c_master_init() == ESP_OK)
		printf("I2C INIT OK\n");
	
	//uruchomienie BH1750
	err = bh1750_power_on();
	if (err != ESP_OK)
    	ESP_LOGE("BH1750", "I2C CMD ERROR: 0x%x", err);
    	
	vTaskDelay(200 / portTICK_PERIOD_MS);
	// cykliczny odczyt
	xTaskCreatePinnedToCore(read_lux, "lux", 4096, NULL, 1, NULL, 0);
	//xTaskCreatePinnedToCore(read_temp, "temp", 4096, NULL, 2, NULL, 0);
}