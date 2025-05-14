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
#include "webserver.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#define WIFI_SSID      "KacperRouter"
#define WIFI_PASS      "dupa1231"
static const char *TAG = "wifi station";

uint16_t lux = 0;

float temp = 0.0, RH = 0.0;

float current = 0.0, power = 0.0;

uint8_t xAuto = 0;

uint16_t pirSeconds = 0;
uint8_t xPir = 0;
uint16_t pirTime = 0;

uint16_t luxSetpoint = 0;
uint8_t dt = 0.1;

// Inicjalizacja polaczenia do wifi
void wifi_init_sta(void) {
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();

    esp_wifi_set_ps(WIFI_PS_NONE);
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_connect();
    ESP_LOGI(TAG, "Connecting to WiFi...");
}

void measure_task(){
	for(;;){
		// Lux
		lux = bh1750_read();
		printf("Lux: %d\n", lux);
		vTaskDelay(pdMS_TO_TICKS(100));
		// Temp and RH
		SHT40measurment(&temp, &RH);
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

void write_to_server(){
	for(;;){
		server_data.lux = lux;
		server_data.temp = temp;
		server_data.rh = RH;
		server_data.current = current;
		server_data.power = power;
		server_data.power_usage = 0;
		server_data.last_movement = pirSeconds;
	}
}

void read_from_server(){
	for(;;){
		xAuto = server_data.mode;
		// Zaleznie od wybranego trybu
		if (xAuto == 0)
			dutyCycle = server_data.duty;
		else if (xAuto == 1)
			luxSetpoint = server_data.duty;
			
		xPir = server_data.pir_on_off;
		pirTime = server_data.hold_up_time;
	}
}

void app_main(void)
{	
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
