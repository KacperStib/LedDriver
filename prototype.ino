#include "WebServices.h"
#include "Sensors.h"

#define INTERVAL 5000 // Interwal odczytu z czujnikow

// Konfiguracja PWM
const int ledPin = 10; // Protype
//const int ledPin = 0; // PCB
const int pwmChannel = 0;
const int pwmFreq = 5000;
const int pwmResolution = 8;
int dutyCycle = 128;

// Zmienne do czunika BH1750
float lux = 0.0;
unsigned long previousMillis = 0;

// Zmienne do czujnika INA219
float current_mA = 0;
float power_mW = 0;

// Zmienne do czunika SHT40
float temperature = 0;
float humidity = 0;

// Zmienne do czujnika PIR
uint8_t pirPin = 6; // Protype
//uint8_t pirPin = 1; // PCB
uint64_t lastTrigger = 0;
uint32_t pirSeconds = 0;

// Przerwanie dla czujnika PIR
void IRAM_ATTR detectsMovement() {
  lastTrigger = millis();
}

void setup() {
  Serial.begin(115200);

  // Konfiguracja pinu PWM
  ledcAttach(ledPin, pwmFreq, pwmResolution);
  ledcWrite(ledPin, dutyCycle);

  // Konfiguracja i polaczenie WiFi
  wiFiConfigure();

  // Konfiguracja WebServera
  webServerConfigure();

  // Konfiguracja OTA
  setupOTA();

  // Inicjalizacja sensorow
  initSensors();
  
  // Przerwanie dla wykrywania ruchu
  pinMode(pirPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(pirPin), detectsMovement, RISING);
}

void loop() {
  // Obsluga systemow webowych
  server.handleClient();
  ArduinoOTA.handle();

  // Odczyt pomiarow co 5 sekund
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= INTERVAL) {
    previousMillis = currentMillis;
    // Odczytanie danych z czujnikow
    measurement(&lux, &current_mA, &power_mW, &temperature, &humidity);
    // Wypisanie danych z czujnikow (test)
    Serial.print("Lux: "); Serial.println(lux);
    Serial.print("Current mA: "); Serial.println(current_mA);
    Serial.print("Power mW: "); Serial.println(power_mW);
    Serial.print("Temp: "); Serial.println(temperature);
    Serial.print("Humidity: "); Serial.println(humidity);
    // Obliczenie sekund od ostaniego wykrycia
    pirSeconds = (millis() - lastTrigger) / 1000;
    Serial.print("PIR seconds: "); Serial.println(pirSeconds);
  }

  delay(20);
}