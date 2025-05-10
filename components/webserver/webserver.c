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

    #dutyValue {
      font-weight: bold;
      color: #2980b9;
    }
  </style>
</head>
<body>
  <div class="section">
    <h2>Sterowanie oświetleniem</h2>
    <label>
      <input type="radio" name="mode" value="manual" id="manualMode" checked onchange="switchMode('manual')">
      Manualny
    </label>
    <label>
      <input type="radio" name="mode" value="auto" id="autoMode" onchange="switchMode('auto')">
      Automatyczny
    </label>
    
    <div id="manualControls">
      <p>Wartość zadana: <span id="dutyValue">128</span></p>
      <input type="range" min="0" max="100" value="50" id="dutySlider" onchange="updateDutyCycle(this.value)">
    </div>

    <div id="autoControls" style="display: none;">
      <p>Wartość zadana: <span id="setLuxValue">200</span> lx</p>
      <input type="range" min="0" max="1000" value="200" id="setLuxSlider" onchange="updateSetLux(this.value)">
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
  
  	function switchMode(mode) {
      if (mode === 'manual') {
        document.getElementById('manualControls').style.display = 'block';
        document.getElementById('autoControls').style.display = 'none';
        fetch('/setMode?value=0');
      } else {
        document.getElementById('manualControls').style.display = 'none';
        document.getElementById('autoControls').style.display = 'block';
        fetch('/setMode?value=1');
      }
    }
    
    dutySlider.addEventListener('input', (e) => {
      document.getElementById('dutyValue').innerText = e.target.value;
    });
    setLuxSlider.addEventListener('input', (e) => {
      document.getElementById('setLuxValue').innerText = e.target.value;
    });
  
    function updateDutyCycle(value) {
      document.getElementById('dutyValue').innerText = value;
      fetch(`/setDuty?value=${value}`);
    }
    function updateSetLux(value) {
      document.getElementById('setLuxValue').innerText = value;
      fetch(`/setLux?value=${value}`);
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
        
      fetch('/getLux').then(response => response.text()).then(value => {
        document.getElementById('setLuxValue').innerText = value;
        document.getElementById('setLuxSlider').value = value;
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
    int duty = 50;
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
            ESP_LOGI("SET_DUTY", "Nowa wartosc: %d", duty);
        }
    }

    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t set_mode_handler(httpd_req_t *req) {
    char param[8];
    if (httpd_req_get_url_query_len(req) > 0) {
        char query[100];
        httpd_req_get_url_query_str(req, query, sizeof(query));
        if (httpd_query_key_value(query, "value", param, sizeof(param)) == ESP_OK) {
            int mode = atoi(param);
                ESP_LOGI("SET_MODE", "Nowy tryb: %d", mode);
        }
    }

    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t set_lux_handler(httpd_req_t *req) {
    char param[16];
    if (httpd_req_get_url_query_len(req) > 0) {
        char query[100];
        httpd_req_get_url_query_str(req, query, sizeof(query));
        if (httpd_query_key_value(query, "value", param, sizeof(param)) == ESP_OK) {
            float set_lux = atof(param);
            ESP_LOGI("SET_SET_LUX", "Nowa wartość zadana luksów: %.1f", set_lux);
        }
    }
    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t get_lux_handler(httpd_req_t *req) {
    char response[16];
    float lux = 1;
    snprintf(response, sizeof(response), "%.1f", lux);
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t get_current_handler(httpd_req_t *req) {
    char response[16];
    float current = 1;
    snprintf(response, sizeof(response), "%.1f", current);
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t get_power_handler(httpd_req_t *req) {
    char response[16];
    float power = 1;
    snprintf(response, sizeof(response), "%.1f", power);
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t get_temp_handler(httpd_req_t *req) {
    char response[16];
    float temperature = 1;
    snprintf(response, sizeof(response), "%.1f", temperature);
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t get_humidity_handler(httpd_req_t *req) {
    char response[16];
    float humidity = 1;
    snprintf(response, sizeof(response), "%.1f", humidity);
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t get_pir_handler(httpd_req_t *req) {
    char response[16];
    int pir = 1;
    snprintf(response, sizeof(response), "%d", pir);
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

httpd_uri_t get_duty_uri = {
    .uri = "/getDuty",
    .method = HTTP_GET,
    .handler = get_duty_handler,
    .user_ctx = NULL
};

httpd_uri_t set_duty_uri = {
    .uri = "/setDuty",
    .method = HTTP_GET,
    .handler = set_duty_handler,
    .user_ctx = NULL
};

httpd_uri_t set_mode_uri = {
    .uri = "/setMode",
    .method = HTTP_GET,
    .handler = set_mode_handler,
    .user_ctx = NULL
};

httpd_uri_t set_lux_uri = {
    .uri = "/setLux",
    .method = HTTP_GET,
    .handler = set_lux_handler,
    .user_ctx = NULL
};

httpd_uri_t get_lux_uri = {
    .uri = "/getLux",
    .method = HTTP_GET,
    .handler = get_lux_handler,
    .user_ctx = NULL
};

httpd_uri_t get_current_uri = {
    .uri = "/getCurrent",
    .method = HTTP_GET,
    .handler = get_current_handler,
    .user_ctx = NULL
};

httpd_uri_t get_power_uri = {
    .uri = "/getPower",
    .method = HTTP_GET,
    .handler = get_power_handler,
    .user_ctx = NULL
};

httpd_uri_t get_temp_uri = {
    .uri = "/getTemp",
    .method = HTTP_GET,
    .handler = get_temp_handler,
    .user_ctx = NULL
};

httpd_uri_t get_humidity_uri = {
    .uri = "/getHumidity",
    .method = HTTP_GET,
    .handler = get_humidity_handler,
    .user_ctx = NULL
};

httpd_uri_t get_pir_uri = {
    .uri = "/getPIR",
    .method = HTTP_GET,
    .handler = get_pir_handler,
    .user_ctx = NULL
};

httpd_uri_t uri_get = {
    .uri      = "/",
    .method   = HTTP_GET,
    .handler  = get_handler,
    .user_ctx = NULL
};

// Funkcja uruchamiająca serwer
httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 16;
    config.max_open_sockets = 7;
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &get_duty_uri);
        httpd_register_uri_handler(server, &set_duty_uri);
        httpd_register_uri_handler(server, &get_lux_uri);
        httpd_register_uri_handler(server, &get_current_uri);
        httpd_register_uri_handler(server, &get_power_uri);
        httpd_register_uri_handler(server, &get_temp_uri);
        httpd_register_uri_handler(server, &get_humidity_uri);
        httpd_register_uri_handler(server, &get_pir_uri);
        httpd_register_uri_handler(server, &set_mode_uri);
        httpd_register_uri_handler(server, &set_lux_uri);
        httpd_register_uri_handler(server, &uri_get);
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