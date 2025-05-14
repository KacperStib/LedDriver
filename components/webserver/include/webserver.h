#include "esp_err.h"
#include "esp_http_server.h"

// Deklaracje funkcji obsługujących zapytania GET i POST
esp_err_t get_handler(httpd_req_t *req);
esp_err_t get_duty_handler(httpd_req_t *req);
esp_err_t set_duty_handler(httpd_req_t *req);
esp_err_t get_lux_handler(httpd_req_t *req);
esp_err_t get_current_handler(httpd_req_t *req);
esp_err_t get_power_handler(httpd_req_t *req);
esp_err_t get_temp_handler(httpd_req_t *req);
esp_err_t get_humidity_handler(httpd_req_t *req);
esp_err_t get_pir_handler(httpd_req_t *req);

// Deklaracje URI handlerów
extern httpd_uri_t uri_get;
extern httpd_uri_t get_duty_uri;
extern httpd_uri_t set_duty_uri;
extern httpd_uri_t get_lux_uri;
extern httpd_uri_t get_current_uri;
extern httpd_uri_t get_power_uri;
extern httpd_uri_t get_temp_uri;
extern httpd_uri_t get_humidity_uri;
extern httpd_uri_t get_pir_uri;

typedef struct {
	uint8_t mode;
	uint16_t duty;
	uint16_t lux;
	float temp;
	float rh;
	float current;
	float power;
	uint16_t power_usage;
	uint16_t last_movement;
	uint8_t pir_on_off;
	uint8_t hold_up_time;
} web_server_data ;

extern web_server_data server_data;

// Deklaracje funkcji uruchamiających i zatrzymujących serwer WWW
httpd_handle_t start_webserver(void);
void stop_webserver(httpd_handle_t server);