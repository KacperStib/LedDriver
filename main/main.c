#include <stdio.h>
#include "esp_err.h"
#include "esp_log.h"
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

#include "webserver.h"
#include "nvs_flash.h"

// Zmienne glownej petli programu
uint16_t lux = 0;
float temp = 0.0, RH = 0.0;
float current = 0.0, power = 0.0;
float powerUsed = 0.0;
uint8_t xAuto = 0;
uint16_t pirSeconds = 0;
uint8_t xPir = 0;
uint16_t pirTime = 15;
uint16_t luxSetpoint = 0;

// Krok regulatora
float dt = 200;

void config_init(){
	// Inicjalizacja pamięci NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	// Uruchomienie połączenia WiFi
	wifi_init_sta();
	
	// Uruchomienie webservera
	httpd_handle_t server = start_webserver();
	if (server == NULL) {
	    ESP_LOGE("MAIN", "Webserver failed to start!");
	}
	
	// Inicjalizacja magistrali I2C
	if (i2c_master_init() == ESP_OK)
		ESP_LOGE("I2c", "I2C init: OK");
	
	//uruchomienie BH1750
	err = bh1750_power_on();
	if (err != ESP_OK)
    	ESP_LOGE("BH1750", "I2C CMD ERROR: 0x%x", err);	
	vTaskDelay(200 / portTICK_PERIOD_MS);
	
	// Config ledc
	led_config();
	
	// Przerwanie PIR
	pir_config();
	
	// Urchomienie i config ina219
	ina219_power_on(0.05, 10);
	if (err != ESP_OK)
    	ESP_LOGE("INA219", "I2C CMD ERROR: 0x%x", err);	
	vTaskDelay(200 / portTICK_PERIOD_MS);
	
	// Test eepromu
	//eeprom_write(1, 0x50);
	//printf("EEPROM REG 1: %d", eeprom_read(1));
	//write_duty(100);
	//printf("DUTYEEPROM: %d\n", read_duty());
}

void measure_task(){
	for(;;){
		// Odczyt lux - tylko jesli nie uzywamy PIDa
		if(!xAuto){
			lux = bh1750_read();
			vTaskDelay(pdMS_TO_TICKS(100));
		}
		
		// Odczyt temperatury i wilgotnosci
		SHT40measurment(&temp, &RH);
		//printf("Temperature: %f\nRH: %f\n", temp, RH);
		vTaskDelay(pdMS_TO_TICKS(100));
		
		// Uplyw czasu od wykrycia ruchu
		pirSeconds = ((esp_timer_get_time() / 1000) - pirMillis) / 1000;
		//printf("Last PIR movement: %d\n", pirSeconds);
		vTaskDelay(pdMS_TO_TICKS(100));
		
		// Odczyt pradu i mocy
		current = ina219_read_current() * 1000;
		power = ina219_read_power() * 1000;
		//printf("Current: %f\nPower: %f\n", current, power);
		vTaskDelay(pdMS_TO_TICKS(100));
		
		// Przerwa
		vTaskDelay(pdMS_TO_TICKS(200));
	}
}

void led_task(){
	for(;;){
		// Tryb czujnika ruchu
		if(xPir == 1){
			//printf("PIRTIME: %d\n", pirTime);
			if(pirSeconds < pirTime)
				led_write(dutyCycle);
			else
 				led_write(0);
		}
		
		// Tryb manualny
		else
			led_write(dutyCycle);
			
		vTaskDelay(pdMS_TO_TICKS(dt));
	}
}

void pid_task(){
	for(;;){
		if(xAuto){
			dutyCycle = pid_compute(luxSetpoint, lux, dt / 1000);
			// Lux
			lux = bh1750_read();
		}
		//printf("LUX SP: %d, DUTY: %d\n", luxSetpoint,dutyCycle);
		vTaskDelay(pdMS_TO_TICKS(dt));
	}
}

void write_to_server(){
	for(;;){
		server_data.lux = lux;
		server_data.temp = temp;
		server_data.rh = RH;
		server_data.current = current;
		server_data.power = power;
		if (server_data.pwr_usg_rst){
			powerUsed = 0;
			server_data.pwr_usg_rst = false;
		}
		server_data.power_usage = powerUsed;
		server_data.last_movement = pirSeconds;
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

void read_from_server(){
	for(;;){
		xAuto = server_data.mode;
		// Zaleznie od wybranego trybu
		if (xAuto == 0)
			dutyCycle = server_data.duty * 2.56;
		else if (xAuto == 1)
			luxSetpoint = server_data.lux_sp;
			
		xPir = server_data.pir_on_off;
		pirTime = server_data.hold_up_time;
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

void eeprom_handler(){
	// Odczyt przy starcie
	server_data.mode = read_mode();
	if (server_data.mode == 0)
		server_data.duty = read_duty() / 2.56;
	else
 		server_data.lux_sp = read_duty();
 	
 	server_data.pir_on_off = read_pir();
 	server_data.hold_up_time = read_pir_time();
 	
 	powerUsed = read_power_usage();
 	
	for(;;){
		write_mode(xAuto);
		if (xAuto == 0)
			write_duty(dutyCycle);
		else
 			write_duty(luxSetpoint);
 			
 		write_pir(xPir);
 		write_pir_time(pirTime);
 		
 		write_power_usage((uint32_t)powerUsed);
 		
		powerUsed += power * (1.0 / 3600.0);
		printf("PowerUsed: %f mWh\n", powerUsed);
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

void app_main(void)
{	
	// Konfiguracja i Inicjalizacja
	config_init();
	
	// Cykliczne taski
	// Pomiary
	xTaskCreatePinnedToCore(measure_task, "measure", 4096, NULL, 1, NULL, 0);
	// Sterowanie LED
	xTaskCreatePinnedToCore(led_task, "led", 4096, NULL, 2, NULL, 0);
	// Regulator PID - tryb auto
	xTaskCreatePinnedToCore(pid_task, "pid", 4096, NULL, 3, NULL, 0);
	// Zapis wartosci na WebServer
	xTaskCreatePinnedToCore(write_to_server, "server_write", 4096, NULL, 4, NULL, 0);
	// Odczyt wartosci z WebServera
	xTaskCreatePinnedToCore(read_from_server, "server_read", 4096, NULL, 5, NULL, 0);
	// Zapis do eeprom
	xTaskCreatePinnedToCore(eeprom_handler, "eeprom", 4096, NULL, 6, NULL, 0);
}
