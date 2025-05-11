#include <stdio.h>
#include "esp_err.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "i2c.h"
#include "bh1750.h"
#include "sht40.h"
#include "ledcDriver.h"
#include "pir.h"
#include "pid.h"
#include "ina219.h"
#include "m24c08.h"
//#include "../components/i2c/include/i2c.h"
//#include "../components/bh1750/include/bh1750.h"

uint16_t lux = 0;

float temp = 0.0, RH = 0.0;

uint16_t pirSeconds = 0;

float current = 0.0, power = 0.0;

bool xAuto = false;
uint16_t luxSetpoint = 0;
uint8_t dt = 0.1;

void measure_task(){
	for(;;){
		// Lux
		lux = bh1750_read();
		printf("Lux: %d\n", lux);
		vTaskDelay(pdMS_TO_TICKS(100));
		// Temp and RH
		SHT41measurment(&temp, &RH);
		printf("Temperature: %f\nRH: %f\n", temp, RH);
		vTaskDelay(pdMS_TO_TICKS(100));
		// Movement
		pirSeconds = ((esp_timer_get_time() / 1000) - pirMillis) / 1000;
		printf("Last PIR movement: %d\n", pirSeconds);
		vTaskDelay(pdMS_TO_TICKS(100));
		// Power
		current = ina219_read_current();
		power = ina219_read_power();
		printf("Current: %f\nPower: %f\n", current, power);
		vTaskDelay(pdMS_TO_TICKS(100));
		// Wait
		vTaskDelay(pdMS_TO_TICKS(2000));
	}
}

void led_task(){
	for(;;){
		led_write(dutyCycle);
		vTaskDelay(pdMS_TO_TICKS(2000));
		dutyCycle += 20;
		if(dutyCycle > 250)
			dutyCycle = 0;
	}
}

void pid_task(){
	for(;;){
		if(xAuto){
			dutyCycle = pid_compute(&pid, luxSetpoint, lux, dt);
		}
		vTaskDelay(pdMS_TO_TICKS(dt));
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
	
	// config led
	led_config();
	
	// Przerwanie PIR
	pir_config();
	
	// urchomienie i config ina219
	ina219_power_on(0.05, 10);
	if (err != ESP_OK)
    	ESP_LOGE("INA219", "I2C CMD ERROR: 0x%x", err);	
	vTaskDelay(200 / portTICK_PERIOD_MS);
	
	// Test eepromu
	eeprom_write(1, 0x50);
	printf("EEPROM REG 1: %d", eeprom_read(1));
	
	// cykliczny odczyt
	xTaskCreatePinnedToCore(measure_task, "measure", 4096, NULL, 1, NULL, 0);
	xTaskCreatePinnedToCore(led_task, "led", 4096, NULL, 2, NULL, 0);
	//xTaskCreatePinnedToCore(pid_task, "pid", 4096, NULL, 3, NULL, 0);
}
