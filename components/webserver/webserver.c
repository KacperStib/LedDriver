#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "webserver.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_http_server.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

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

// Struktura danych
web_server_data server_data;

// HTML strona do serwowania
const char index_html[] = R"rawliteral(
<!DOCTYPE html>
<html lang="pl">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Sterowanie Oświetleniem</title>
  <style>
    :root {
      --primary: #3498db;
      --primary-dark: #2980b9;
      --secondary: #2ecc71;
      --danger: #e74c3c;
      --light: #ecf0f1;
      --dark: #2c3e50;
      --gray: #95a5a6;
    }
    
    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
    }
    
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background: #f5f7fa;
      color: var(--dark);
      line-height: 1.6;
      padding: 20px;
    }
    
    .container {
      max-width: 1400px;
      margin: 0 auto;
      display: grid;
      grid-template-columns: repeat(4, 1fr);
      gap: 20px;
    }
    
    @media (max-width: 1200px) {
      .container {
        grid-template-columns: repeat(2, 1fr);
      }
    }
    
    @media (max-width: 768px) {
      .container {
        grid-template-columns: 1fr;
      }
    }
    
    .card {
      background: white;
      border-radius: 10px;
      box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
      padding: 20px;
      transition: transform 0.3s ease, box-shadow 0.3s ease;
      min-height: 200px;
    }
    
    .card:hover {
      transform: translateY(-5px);
      box-shadow: 0 10px 20px rgba(0, 0, 0, 0.1);
    }
    
    .card-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 15px;
      padding-bottom: 10px;
      border-bottom: 1px solid #eee;
    }
    
    .card-title {
      font-size: 1.2rem;
      font-weight: 600;
      color: var(--primary);
    }
    
    .card-body {
      margin-bottom: 15px;
    }
    
    .metric {
      margin-bottom: 15px;
    }
    
    .metric-title {
      font-size: 0.9rem;
      color: var(--gray);
      margin-bottom: 5px;
    }
    
    .value {
      font-size: 1.8rem;
      font-weight: 700;
      color: var(--dark);
    }
    
    .unit {
      font-size: 1rem;
      color: var(--gray);
    }
    
    .btn {
      background: var(--primary);
      color: white;
      border: none;
      padding: 8px 16px;
      border-radius: 5px;
      cursor: pointer;
      font-size: 0.9rem;
      transition: background 0.3s;
    }
    
    .btn:hover {
      background: var(--primary-dark);
    }
    
    .btn-danger {
      background: var(--danger);
    }
    
    .btn-danger:hover {
      background: #c0392b;
    }
    
    .btn-sm {
      padding: 5px 10px;
      font-size: 0.8rem;
    }
    
    .control-panel {
      grid-column: 1 / -1;
    }
    
    .form-group {
      margin-bottom: 15px;
    }
    
    label {
      display: block;
      margin-bottom: 5px;
      font-weight: 500;
    }
    
    input[type="range"] {
      width: 100%;
      height: 8px;
      -webkit-appearance: none;
      background: #ddd;
      border-radius: 5px;
      outline: none;
    }
    
    input[type="range"]::-webkit-slider-thumb {
      -webkit-appearance: none;
      width: 18px;
      height: 18px;
      background: var(--primary);
      border-radius: 50%;
      cursor: pointer;
    }
    
    input[type="number"] {
      width: 60px;
      padding: 5px;
      border: 1px solid #ddd;
      border-radius: 4px;
    }
    
    .radio-group {
      display: flex;
      gap: 15px;
      margin-bottom: 15px;
    }
    
    .radio-option {
      display: flex;
      align-items: center;
      gap: 5px;
    }
    
    .status-indicator {
      display: inline-block;
      width: 10px;
      height: 10px;
      border-radius: 50%;
      background: var(--gray);
      margin-right: 5px;
    }
    
    .status-active {
      background: var(--secondary);
    }
  </style>
</head>
<body>
  <div class="container">
    <!-- Control Panel -->
    <div class="card control-panel">
      <div class="card-header">
        <h2 class="card-title">Sterowanie oświetleniem</h2>
        <span id="clock" style="font-size:0.9rem; color: var(--gray); font-family: monospace;">--:--:--</span>
      </div>
      <div class="card-body">
        <div class="radio-group">
          <label class="radio-option">
            <input type="radio" name="mode" value="manual" id="manualMode" checked onchange="switchMode('manual')">
            Manualny
          </label>
          <label class="radio-option">
            <input type="radio" name="mode" value="auto" id="autoMode" onchange="switchMode('auto')">
            Automatyczny
          </label>
        </div>
        
        <div id="manualControls">
          <div class="form-group">
            <label for="dutySlider">Wartość zadana: <span id="dutyValue">50</span>%</label>
            <input type="range" min="0" max="100" value="50" id="dutySlider" onchange="updateDutyCycle(this.value)">
          </div>
        </div>

        <div id="autoControls" style="display: none;">
          <div class="form-group">
            <label for="setLuxSlider">Wartość zadana: <span id="setLuxValue">200</span> <span class="unit">lx</span></label>
            <input type="range" min="0" max="1000" value="200" id="setLuxSlider" onchange="updateSetLux(this.value)">
          </div>
        </div>
      </div>
    </div>

    <!-- Light Sensor -->
    <div class="card">
      <div class="card-header">
        <h2 class="card-title">Pomiar światła</h2>
      </div>
      <div class="card-body">
        <div class="metric">
          <div class="value"><span id="luxValue">0.0</span> <span class="unit">lx</span></div>
        </div>
      </div>
    </div>

    <!-- PIR Sensor -->
    <div class="card">
      <div class="card-header">
        <h2 class="card-title">Detekcja ruchu</h2>
      </div>
      <div class="card-body">
        <div class="form-group">
          <label class="radio-option">
            <input type="checkbox" name="enablePir" id="pirCheckbox">
            <span class="status-indicator"></span>
            Aktywna detekcja
          </label>
        </div>
        <div class="form-group">
          <label for="pirTime">Czas podtrzymania (s):</label>
          <div style="display: flex; gap: 10px;">
            <input type="number" class="tab" min="1" max="240" id="pirTime" style="flex-grow: 1;">
            <button id="submit" class="btn btn-sm">Zapisz</button>
          </div>
        </div>
        <div class="metric">
          <div class="value"><span id="pirValue">0</span> <span class="unit">sekund</span></div>
          <div class="metric-title">Od ostatniego ruchu</div>
        </div>
      </div>
    </div>

    <!-- Current/Power -->
    <div class="card">
      <div class="card-header">
        <h2 class="card-title">Energia</h2>
      </div>
      <div class="card-body">
        <div class="metric">
          <div class="metric-title">Prąd</div>
          <div class="value"><span id="currentValue">0.0</span> <span class="unit">mA</span></div>
        </div>
        <div class="metric">
          <div class="metric-title">Moc</div>
          <div class="value"><span id="powerValue">0.0</span> <span class="unit">mW</span></div>
        </div>
        <div class="metric">
          <div class="metric-title">Zużycie</div>
          <div class="value"><span id="powerUsageValue">0</span> <span class="unit">mWh</span></div>
          <button onclick="resetPowerUsage()" class="btn btn-danger btn-sm" style="margin-top: 5px;">Reset</button>
        </div>
      </div>
    </div>

    <!-- Environment -->
    <div class="card">
      <div class="card-header">
        <h2 class="card-title">Środowisko</h2>
      </div>
      <div class="card-body">
        <div class="metric">
          <div class="metric-title">Temperatura</div>
          <div class="value"><span id="tempValue">0.0</span> <span class="unit">°C</span></div>
        </div>
        <div class="metric">
          <div class="metric-title">Wilgotność</div>
          <div class="value"><span id="humidityValue">0.0</span> <span class="unit">%</span></div>
        </div>
      </div>
    </div>
  </div>

  <script>
    // Funkcje JavaScript pozostają bez zmian
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
    
    document.getElementById('submit').addEventListener('click', () => {
      const pirEnabled = document.getElementById('pirCheckbox').checked ? 1 : 0;
      const pirTime = document.getElementById('pirTime').value;
      fetch(`/setPIR?value=${pirEnabled}&time=${pirTime}`);
    });

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
          document.getElementById('luxTime').innerText = new Date().toLocaleTimeString();
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
    
    function updatePowerUsage() {
      fetch('/getPowerUsage')
        .then(response => response.text())
        .then(data => {
          document.getElementById('powerUsageValue').innerText = data;
        });
    }
    
    function resetPowerUsage() {
      fetch('/resetPowerUsage')
        .then(response => response.text())
        .then(data => {
          if (data === "OK") {
            updatePowerUsage();
          }
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
          // Update status indicator
          const indicator = document.querySelector('.status-indicator');
          if (data > 0) {
            indicator.classList.add('status-active');
          } else {
            indicator.classList.remove('status-active');
          }
        });
    }
    
	function updateClock() {
	  const now = new Date();
	  const timeString = now.toLocaleTimeString();
	  document.getElementById('clock').innerText = timeString;
	}
	
	setInterval(updateClock, 1000);
	window.onload = () => {
	  initializeValues();
	  updateClock();
	};
	
    function initializeValues() {
      fetch('/getMode')
        .then(response => response.text())
        .then(mode => {
          if (mode === '1') { // AUTO
            document.getElementById('autoMode').checked = true;
            document.getElementById('manualControls').style.display = 'none';
            document.getElementById('autoControls').style.display = 'block';
          } else { // MANUAL
            document.getElementById('manualMode').checked = true;
            document.getElementById('manualControls').style.display = 'block';
            document.getElementById('autoControls').style.display = 'none';
          }
        });
      
      fetch('/getDuty')
        .then(response => response.text())
        .then(data => {
          document.getElementById('dutyValue').innerText = data;
          document.getElementById('dutySlider').value = data;
        });

      fetch('/getLuxSP')
        .then(response => response.text())
        .then(data => {
          document.getElementById('setLuxValue').innerText = data;
          document.getElementById('setLuxSlider').value = data;
        });

      fetch('/getLux')
        .then(response => response.text())
        .then(value => {
          document.getElementById('setLuxValue').innerText = value;
          document.getElementById('setLuxSlider').value = value;
        });
      
      fetch('/getPIRSettings')
        .then(response => response.text())
        .then(data => {
          const [enabled, time] = data.split(',');
          document.getElementById('pirCheckbox').checked = enabled === '1';
          document.getElementById('pirTime').value = time;
        });
      
      updateLux();
      updateCurrent();
      updatePower();
      updatePowerUsage();
      updateTemp();
      updateHumidity();
      updatePIR();
    }

    setInterval(updateLux, 1000);
    setInterval(updateCurrent, 1000);
    setInterval(updatePower, 1000);
    setInterval(updatePowerUsage, 1000);
    setInterval(updateTemp, 1000);
    setInterval(updateHumidity, 1000);
    setInterval(updatePIR, 1000);

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
    //int duty = 50;
    snprintf(response, sizeof(response), "%d", server_data.duty);
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t set_duty_handler(httpd_req_t *req) {
    char param[8];
    if (httpd_req_get_url_query_len(req) > 0) {
        char query[100];
        httpd_req_get_url_query_str(req, query, sizeof(query));
        if (httpd_query_key_value(query, "value", param, sizeof(param)) == ESP_OK) {
            server_data.duty = atoi(param);
            //ESP_LOGI("SET_DUTY", "Nowa wartosc: %d", duty);
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
            server_data.mode = atoi(param);
                //ESP_LOGI("SET_MODE", "Nowy tryb: %d", mode);
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
            server_data.lux_sp = atoi(param);
            //ESP_LOGI("SET_SET_LUX", "Nowa wartość zadana luksów: %.1f", set_lux);
        }
    }
    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t set_pir_handler(httpd_req_t *req) {
    char query[100];
    char value_param[8], time_param[8];

    if (httpd_req_get_url_query_len(req) > 0) {
        httpd_req_get_url_query_str(req, query, sizeof(query));

        if (httpd_query_key_value(query, "value", value_param, sizeof(value_param)) == ESP_OK) {
            server_data.pir_on_off = atoi(value_param) != 0;
            ESP_LOGI("SET_PIR", "PIR ON/OFF: %d", server_data.pir_on_off);
        }

        if (httpd_query_key_value(query, "time", time_param, sizeof(time_param)) == ESP_OK) {
            server_data.hold_up_time = atoi(time_param);
            ESP_LOGI("SET_PIR", "PIR hold_up_time: %d", server_data.hold_up_time);
        }
    }

    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t get_lux_handler(httpd_req_t *req) {
    char response[16];
    //float lux = 1;
    snprintf(response, sizeof(response), "%d", server_data.lux);
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t get_current_handler(httpd_req_t *req) {
    char response[16];
    //float current = 1;
    snprintf(response, sizeof(response), "%.1f", server_data.current);
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t get_power_handler(httpd_req_t *req) {
    char response[16];
    //float power = 1;
    snprintf(response, sizeof(response), "%.1f", server_data.power);
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t get_power_usage_handler(httpd_req_t *req) {
    char response[16];
    snprintf(response, sizeof(response), "%lu", server_data.power_usage);
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t reset_power_usage_handler(httpd_req_t *req) {
    server_data.pwr_usg_rst = true; // Resetuj wartość
    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    ESP_LOGI("RESET_PWR", "POWER USAGE RESET!");
    return ESP_OK;
}

esp_err_t get_temp_handler(httpd_req_t *req) {
    char response[16];
    //float temperature = 1;
    snprintf(response, sizeof(response), "%.1f", server_data.temp);
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t get_humidity_handler(httpd_req_t *req) {
    char response[16];
    //float humidity = 1;
    snprintf(response, sizeof(response), "%.1f", server_data.rh);
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t get_pir_handler(httpd_req_t *req) {
    char response[16];
    //int pir = 1;
    snprintf(response, sizeof(response), "%d", server_data.last_movement);
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t get_pir_settings_handler(httpd_req_t *req) {
    char response[32];
    snprintf(response, sizeof(response), "%d,%d", server_data.pir_on_off, server_data.hold_up_time);
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t get_mode_handler(httpd_req_t *req) {
    char response[8];
    snprintf(response, sizeof(response), "%d", server_data.mode);  // 0 = MANUAL, 1 = AUTO
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t get_lux_sp_handler(httpd_req_t *req) {
    char response[16];
    snprintf(response, sizeof(response), "%d", server_data.lux_sp);
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

httpd_uri_t set_pir_uri = {
    .uri = "/setPIR",
    .method = HTTP_GET,
    .handler = set_pir_handler,
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

httpd_uri_t get_power_usage_uri = {
    .uri = "/getPowerUsage",
    .method = HTTP_GET,
    .handler = get_power_usage_handler,
    .user_ctx = NULL
};

httpd_uri_t reset_power_usage_uri = {
    .uri = "/resetPowerUsage",
    .method = HTTP_GET,
    .handler = reset_power_usage_handler,
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

httpd_uri_t get_pir_settings_uri = {
    .uri = "/getPIRSettings",
    .method = HTTP_GET,
    .handler = get_pir_settings_handler,
    .user_ctx = NULL
};

httpd_uri_t get_mode_uri = {
    .uri = "/getMode",
    .method = HTTP_GET,
    .handler = get_mode_handler,
    .user_ctx = NULL
};

httpd_uri_t get_lux_sp_uri = {
    .uri = "/getLuxSP",
    .method = HTTP_GET,
    .handler = get_lux_sp_handler,
    .user_ctx = NULL
};

// Funkcja uruchamiająca serwer
httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 20;
    config.max_open_sockets = 7;
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &get_duty_uri);
        httpd_register_uri_handler(server, &set_duty_uri);
        httpd_register_uri_handler(server, &get_lux_uri);
        httpd_register_uri_handler(server, &get_current_uri);
        httpd_register_uri_handler(server, &get_power_uri);
        httpd_register_uri_handler(server, &get_power_usage_uri);
        httpd_register_uri_handler(server, &reset_power_usage_uri);
        httpd_register_uri_handler(server, &get_temp_uri);
        httpd_register_uri_handler(server, &get_humidity_uri);
        httpd_register_uri_handler(server, &get_pir_uri);
        httpd_register_uri_handler(server, &set_mode_uri);
        httpd_register_uri_handler(server, &set_lux_uri);
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &set_pir_uri);
        httpd_register_uri_handler(server, &get_pir_settings_uri);
        httpd_register_uri_handler(server, &get_mode_uri);
        httpd_register_uri_handler(server, &get_lux_sp_uri);
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