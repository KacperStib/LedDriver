#include <stdio.h>
#include <string.h>
#include "webserver.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_http_server.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

// HTML strona do serwowania
const char index_html[] = R"rawliteral(
<!DOCTYPE html>
<html lang="pl">
<head>
  <meta charset="UTF-8">
  <title>Sterowanie Oświetleniem</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background: #f0f4f8;
      margin: 0;
      padding: 20px;
      color: #333;
    }

    h2 {
      color: #2c3e50;
      margin-top: 30px;
      border-left: 5px solid #3498db;
      padding-left: 10px;
    }

    .section {
      background: #ffffff;
      border-radius: 10px;
      padding: 20px;
      margin-bottom: 20px;
      box-shadow: 0 2px 5px rgba(0,0,0,0.1);
    }

    p {
      font-size: 16px;
      margin: 10px 0;
    }

    input[type=range] {
      width: 100%;
      margin-top: 10px;
      appearance: none;
      height: 10px;
      background: #ddd;
      border-radius: 5px;
      outline: none;
    }

    input[type=range]::-webkit-slider-thumb {
      appearance: none;
      width: 20px;
      height: 20px;
      background: #3498db;
      border-radius: 50%;
      cursor: pointer;
      border: none;
      margin-top: -5px;
    }

    input[type=range]::-moz-range-thumb {
      width: 20px;
      height: 20px;
      background: #3498db;
      border-radius: 50%;
      cursor: pointer;
      border: none;
    }

    #dutyValue {
      font-weight: bold;
      color: #2980b9;
    }
  </style>
</head>
<body>
  <div class="section">
    <h2>Sterowanie oświetleniem</h2>
    <p>Aktualna wartość: <span id="dutyValue">128</span></p>
    <input type="range" min="0" max="255" value="128" id="dutySlider" oninput="updateDutyCycle(this.value)">
  </div>

  <div class="section">
    <h2>Pomiar światła</h2>
    <p>Aktualna wartość luksów: <span id="luxValue">0.0</span> lx</p>
  </div>

  <div class="section">
    <h2>Pomiar prądu i mocy</h2>
    <p>Aktualny prąd: <span id="currentValue">0.0</span> mA</p>
    <p>Aktualna moc: <span id="powerValue">0.0</span> mW</p>
  </div>

  <div class="section">
    <h2>Pomiar temperatury i wilgotności</h2>
    <p>Aktualna temperatura: <span id="tempValue">0.0</span> °C</p>
    <p>Aktualna wilgotność: <span id="humidityValue">0.0</span> %</p>
  </div>

  <div class="section">
    <h2>Pomiar PIR</h2>
    <p>Czas od ostatniego wykrycia ruchu: <span id="pirValue">0</span> sekund</p>
  </div>

  <script>
    function updateDutyCycle(value) {
      document.getElementById('dutyValue').innerText = value;
      fetch(`/setDuty?value=${value}`);
    }

    function updateLux() {
      fetch('/getLux')
        .then(response => response.text())
        .then(data => {
          document.getElementById('luxValue').innerText = data;
        });
    }

    function updateCurrent() {
      fetch('/getCurrent')
        .then(response => response.text())
        .then(data => {
          document.getElementById('currentValue').innerText = data;
        });
    }

    function updatePower() {
      fetch('/getPower')
        .then(response => response.text())
        .then(data => {
          document.getElementById('powerValue').innerText = data;
        });
    }

    function updateTemp() {
      fetch('/getTemp')
        .then(response => response.text())
        .then(data => {
          document.getElementById('tempValue').innerText = data;
        });
    }

    function updateHumidity() {
      fetch('/getHumidity')
        .then(response => response.text())
        .then(data => {
          document.getElementById('humidityValue').innerText = data;
        });
    }

    function updatePIR() {
      fetch('/getPIR')
        .then(response => response.text())
        .then(data => {
          document.getElementById('pirValue').innerText = data;
        });
    }

    function initializeValues() {
      fetch('/getDuty')
        .then(response => response.text())
        .then(data => {
          document.getElementById('dutyValue').innerText = data;
          document.getElementById('dutySlider').value = data;
        });

      updateLux();
      updateCurrent();
      updatePower();
      updateTemp();
      updateHumidity();
      updatePIR();
    }

    setInterval(updateLux, 5000);
    setInterval(updateCurrent, 5000);
    setInterval(updatePower, 5000);
    setInterval(updateTemp, 5000);
    setInterval(updateHumidity, 5000);
    setInterval(updatePIR, 5000);

    window.onload = initializeValues;
  </script>
</body>
</html>

)rawliteral";

esp_err_t get_handler(httpd_req_t *req)
{
    httpd_resp_send(req, index_html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t get_duty_handler(httpd_req_t *req) {
    char response[8];
    int duty = 128;
    snprintf(response, sizeof(response), "%d", duty);
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t set_duty_handler(httpd_req_t *req) {
    char param[8];
    if (httpd_req_get_url_query_len(req) > 0) {
        char query[100];
        httpd_req_get_url_query_str(req, query, sizeof(query));
        if (httpd_query_key_value(query, "value", param, sizeof(param)) == ESP_OK) {
            int duty = atoi(param);
            //ESP_LOGI("SET_DUTY", "Nowa wartosc: %d", duty);
        }
    }

    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}


// Funkcja GET dla /getLux
esp_err_t get_lux_handler(httpd_req_t *req) {
    char response[16];
    float lux = 1;
    snprintf(response, sizeof(response), "%.1f", lux);
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Funkcja GET dla /getCurrent
esp_err_t get_current_handler(httpd_req_t *req) {
    char response[16];
    float current = 1;
    snprintf(response, sizeof(response), "%.1f", current);
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Funkcja GET dla /getPower
esp_err_t get_power_handler(httpd_req_t *req) {
    char response[16];
    float power = 1;
    snprintf(response, sizeof(response), "%.1f", power);
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Funkcja GET dla /getTemp
esp_err_t get_temp_handler(httpd_req_t *req) {
    char response[16];
    float temperature = 1;
    snprintf(response, sizeof(response), "%.1f", temperature);
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Funkcja GET dla /getHumidity
esp_err_t get_humidity_handler(httpd_req_t *req) {
    char response[16];
    float humidity = 1;
    snprintf(response, sizeof(response), "%.1f", humidity);
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Funkcja GET dla /getPIR
esp_err_t get_pir_handler(httpd_req_t *req) {
    char response[16];
    int pir = 1;
    snprintf(response, sizeof(response), "%d", pir);
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}


esp_err_t post_handler(httpd_req_t *req)
{
    /* Destination buffer for content of HTTP POST request.
     * httpd_req_recv() accepts char* only, but content could
     * as well be any binary data (needs type casting).
     * In case of string data, null termination will be absent, and
     * content length would give length of string */
    char content[100];

    /* Truncate if content length larger than the buffer */
    size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {  /* 0 return value indicates connection closed */
        /* Check if timeout occurred */
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            /* In case of timeout one can choose to retry calling
             * httpd_req_recv(), but to keep it simple, here we
             * respond with an HTTP 408 (Request Timeout) error */
            httpd_resp_send_408(req);
        }
        /* In case of error, returning ESP_FAIL will
         * ensure that the underlying socket is closed */
        return ESP_FAIL;
    }

    /* Send a simple response */
    const char resp[] = "URI POST Response";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* URI handler structure for GET /uri */
httpd_uri_t uri_get = {
    .uri      = "/uri",
    .method   = HTTP_GET,
    .handler  = get_handler,
    .user_ctx = NULL
};

// URI handler dla GET /getDuty
httpd_uri_t get_duty_uri = {
    .uri = "/getDuty",
    .method = HTTP_GET,
    .handler = get_duty_handler,
    .user_ctx = NULL
};

// URI handler dla GET /setDuty
httpd_uri_t set_duty_uri = {
    .uri = "/setDuty",
    .method = HTTP_GET,
    .handler = set_duty_handler,
    .user_ctx = NULL
};

// URI handler dla GET /getLux
httpd_uri_t get_lux_uri = {
    .uri = "/getLux",
    .method = HTTP_GET,
    .handler = get_lux_handler,
    .user_ctx = NULL
};

// URI handler dla GET /getCurrent
httpd_uri_t get_current_uri = {
    .uri = "/getCurrent",
    .method = HTTP_GET,
    .handler = get_current_handler,
    .user_ctx = NULL
};

// URI handler dla GET /getPower
httpd_uri_t get_power_uri = {
    .uri = "/getPower",
    .method = HTTP_GET,
    .handler = get_power_handler,
    .user_ctx = NULL
};

// URI handler dla GET /getTemp
httpd_uri_t get_temp_uri = {
    .uri = "/getTemp",
    .method = HTTP_GET,
    .handler = get_temp_handler,
    .user_ctx = NULL
};

// URI handler dla GET /getHumidity
httpd_uri_t get_humidity_uri = {
    .uri = "/getHumidity",
    .method = HTTP_GET,
    .handler = get_humidity_handler,
    .user_ctx = NULL
};

// URI handler dla GET /getPIR
httpd_uri_t get_pir_uri = {
    .uri = "/getPIR",
    .method = HTTP_GET,
    .handler = get_pir_handler,
    .user_ctx = NULL
};

// Funkcja uruchamiająca serwer
httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 16;
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &get_duty_uri);
        httpd_register_uri_handler(server, &set_duty_uri);
        httpd_register_uri_handler(server, &get_lux_uri);
        httpd_register_uri_handler(server, &get_current_uri);
        httpd_register_uri_handler(server, &get_power_uri);
        httpd_register_uri_handler(server, &get_temp_uri);
        httpd_register_uri_handler(server, &get_humidity_uri);
        httpd_register_uri_handler(server, &get_pir_uri);
   		ESP_LOGI("WEBSERVER", "All URI handlers registered");
	} else {
	    ESP_LOGE("WEBSERVER", "Failed to start server");
	}
    return server;
}

// Funkcja zatrzymująca serwer
void stop_webserver(httpd_handle_t server)
{
    if (server) {
        httpd_stop(server);
    }
}