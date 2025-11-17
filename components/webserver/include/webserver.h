#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_netif_ip_addr.h"

#define WIFI_SSID      "STIBDOM"
#define WIFI_PASS      "kacper24"
static const char *TAG = "wifi station";
// Inicjalizacja wifi
void wifi_init_sta(void);

// Deklaracje funkcji obsługujących zapytania GET i POST
esp_err_t get_handler(httpd_req_t *req);
esp_err_t get_duty_handler(httpd_req_t *req);
esp_err_t set_duty_handler(httpd_req_t *req);
esp_err_t get_lux_handler(httpd_req_t *req);
esp_err_t get_current_handler(httpd_req_t *req);
esp_err_t get_power_handler(httpd_req_t *req);
esp_err_t get_power_usage_handler(httpd_req_t *req);
esp_err_t reset_power_usage_handler(httpd_req_t *req);
esp_err_t get_temp_handler(httpd_req_t *req);
esp_err_t get_humidity_handler(httpd_req_t *req);
esp_err_t get_pir_handler(httpd_req_t *req);
esp_err_t get_pir_settings_handler(httpd_req_t *req);
esp_err_t get_mode_handler(httpd_req_t *req);
esp_err_t get_lux_sp_handler(httpd_req_t *req);

// Deklaracje URI handlerów
extern httpd_uri_t uri_get;
extern httpd_uri_t get_duty_uri;
extern httpd_uri_t set_duty_uri;
extern httpd_uri_t get_lux_uri;
extern httpd_uri_t get_current_uri;
extern httpd_uri_t get_power_uri;
extern httpd_uri_t get_power_usage_uri;
extern httpd_uri_t reset_power_usage_uri;
extern httpd_uri_t get_temp_uri;
extern httpd_uri_t get_humidity_uri;
extern httpd_uri_t get_pir_uri;
extern httpd_uri_t get_pir_settings_uri;
extern httpd_uri_t get_mode_uri;
extern httpd_uri_t get_lux_sp_uri;

// Struktura danych webservera
typedef struct {
	uint8_t mode;
	uint16_t duty;
	uint16_t lux_sp;
	uint16_t lux;
	float temp;
	float rh;
	float current;
	float power;
	uint32_t power_usage;
	bool pwr_usg_rst;
	uint16_t last_movement;
	uint8_t pir_on_off;
	uint8_t hold_up_time;
} web_server_data ;

// Nowe wartosci
extern uint8_t newValue;

extern web_server_data server_data;

// Deklaracje funkcji uruchamiających i zatrzymujących serwer WWW
httpd_handle_t start_webserver(void);
void stop_webserver(httpd_handle_t server);