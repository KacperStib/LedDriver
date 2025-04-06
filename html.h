const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Swiatlo</title>
</head>
<body>
  <h2>Sterowanie oswietleniem</h2>
  <p>Aktualna wartosc: <span id="dutyValue">128</span></p>
  <input type="range" min="0" max="255" value="128" id="dutySlider" oninput="updateDutyCycle(this.value)">
  
  <h2>Pomiar swiatla</h2>
  <p>Aktualna wartosc luksow: <span id="luxValue">0.0</span> lx</p>

  <h2>Pomiar pradu i mocy</h2>
  <p>Aktualny prad: <span id="currentValue">0.0</span> mA</p>
  <p>Aktualna moc: <span id="powerValue">0.0</span> mW</p>

  <h2>Pomiar temperatury i wilgotnosci</h2>
  <p>Aktualna temperatura: <span id="tempValue">0.0</span> °C</p>
  <p>Aktualna wilgotnosc: <span id="humidityValue">0.0</span> %</p>

  <h2>Pomiar PIR</h2>
  <p>Czas od ostatniego wykrycia ruchu: <span id="pirValue">0</span> sekund</p>

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
              console.log(data);
              document.getElementById('pirValue').innerText = data;
            });
    }

    function initializeValues() {
      // Pobranie aktualnej wartości dutyCycle z serwera
      fetch('/getDuty')
        .then(response => response.text())
        .then(data => {
          document.getElementById('dutyValue').innerText = data;
          document.getElementById('dutySlider').value = data;
        });
  
      // Zainicjowanie pozostałych wartości
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

    // Wywołanie funkcji inicjalizującej po załadowaniu strony
    window.onload = initializeValues;
  </script>
</body>
</html>
)rawliteral";