#include <stdio.h>
#include "esp_err.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "i2c.h"
#include "sht40.h"
#include "ledcDriver.h"
#include "pir.h"
#include "ina219.h"
#include "myNvs.h"

#include "webserver.h"


// Zmienne glownej petli programu
uint16_t lux = 0;
float temp = 0.0, RH = 0.0;
float current = 0.0, power = 0.0;
float powerUsed = 0.0;
uint16_t pirSeconds = 0;
uint8_t xPir = 0;
uint16_t pirTime = 15;
uint8_t ambient = 0;
uint8_t dt = 200;

// nvs flsah
 nvs_handle_t my_handle;
 
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
	if (i2c_master_init() != ESP_OK)
		ESP_LOGE("I2c", "I2C init: ERROR");
	
	// Config ledc
	led_config();
	
	// Przerwanie PIR
	pir_config();
	
	// Urchomienie i config ina219
	ina219_power_on(0.1, 3.2);
	if (err != ESP_OK)
    	ESP_LOGE("INA219", "I2C CMD ERROR: 0x%x", err);	
	vTaskDelay(200 / portTICK_PERIOD_MS);
	
	// NVS
	nvs_open("storage", NVS_READWRITE, &my_handle);
}

void measure_task(){
	for(;;){
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
		powerUsed += power * (1.0 / 3600.0);
		//printf("Current: %f\nPower: %f\n", current, power);
		vTaskDelay(pdMS_TO_TICKS(100));
		
		// Przerwa
		vTaskDelay(pdMS_TO_TICKS(1000));
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
 				led_write(ambient);
		}
		
		// Tryb manualny (bez czujki ruchu)
		else
			led_write(dutyCycle);
			
		vTaskDelay(pdMS_TO_TICKS(dt));
	}
}

void write_to_server(){
	for(;;){
		server_data.temp = temp;
		server_data.rh = RH;
		server_data.current = current;
		server_data.power = power;
		// reset zuzycia energii w pamieci
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
		// Zaleznie od wybranego trybu
		dutyCycle = server_data.duty * 2.56;
		ambient = server_data.lux_sp * 2.56;
			
		xPir = server_data.pir_on_off;
		pirTime = server_data.hold_up_time;
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

void eeprom_handler(){
	// Odczyt przy starcie	
	server_data.duty = read_duty() / 2.56;
	server_data.lux_sp = read_ambient() / 2.56;
 
 	server_data.pir_on_off = read_pir();
 	server_data.hold_up_time = read_pir_time();
 	
 	powerUsed = read_power_usage();
 	
 	uint16_t counter = 0;
	for(;;){
		if (newValue & (1 << 1)){
			write_duty(dutyCycle);
			newValue &= ~(1 << 1);
			printf("writtenD\n");
		}
		if (newValue & (1 << 2)){
			write_ambient(ambient);
	 		newValue &= ~(1 << 2);	
	 		printf("writtenA\n");
	 	}
	 	if (newValue & (1 << 3)){
	 		write_pir(xPir);
	 		newValue &= ~(1 << 3);	
	 		printf("writtenP\n");
	 	}
 		if (newValue & (1 << 4)){
	 		write_pir_time(pirTime);
	 		newValue &= ~(1 << 4);	
	 		printf("writtenT\n");
	 	}
	 	if(counter >= 1800){
 			write_power_usage((uint32_t)powerUsed);
 			counter = 0;
 			printf("writtenPOW\n");
 		}
 		
 		// Obliczanie zuzytej energii (mWH) Energia = P * t [h]
		powerUsed += power * (1.0 / 3600.0);
		counter ++;
		//printf("PowerUsed: %f mWh\n", powerUsed);
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

void app_main(void)
{	
	// Konfiguracja i Inicjalizacja
	config_init();
	
	// Cykliczne taski
	// Pomiary - 1 s
	xTaskCreatePinnedToCore(measure_task, "measure", 4096, NULL, 1, NULL, 0);
	// Sterowanie LED - dt (200ms)
	xTaskCreatePinnedToCore(led_task, "led", 4096, NULL, 2, NULL, 0);
	// Zapis wartosci na WebServer - 1s
	xTaskCreatePinnedToCore(write_to_server, "server_write", 4096, NULL, 3, NULL, 0);
	// Odczyt wartosci z WebServera - 1s
	xTaskCreatePinnedToCore(read_from_server, "server_read", 4096, NULL, 4, NULL, 0);
	// Zapis do eeprom - 1s
	xTaskCreatePinnedToCore(eeprom_handler, "eeprom", 4096, NULL, 5, NULL, 0);
}
