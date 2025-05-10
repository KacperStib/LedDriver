#include <stdio.h>
#include "esp_err.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "i2c.h"
#include "bh1750.h"
#include "sht40.h"
#include "webserver.h"
//#include "../components/i2c/include/i2c.h"
//#include "../components/bh1750/include/bh1750.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

int lux = 0;
float temp = 0.0, RH = 0.0;

#define WIFI_SSID      "Slosarczyk_Dom"
#define WIFI_PASS      "MaciekKazek2"

static const char *TAG = "wifi station";

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
	/*if (i2c_master_init() == ESP_OK)
		printf("I2C INIT OK\n");
	//uruchomienie BH1750
	err = bh1750_power_on();
	if (err != ESP_OK)
    	ESP_LOGE("BH1750", "I2C CMD ERROR: 0x%x", err);*/

	// Inicjalizacja pamięci NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	// Uruchomienie połączenia WiFi
	wifi_init_sta();

	httpd_handle_t server = start_webserver();
	if (server == NULL) {
	    ESP_LOGE("MAIN", "Webserver failed to start!");
	}
	// cykliczny odczyt
	//xTaskCreatePinnedToCore(read_lux, "lux", 4096, NULL, 1, NULL, 0);
	//xTaskCreatePinnedToCore(read_temp, "temp", 4096, NULL, 2, NULL, 0);
}
