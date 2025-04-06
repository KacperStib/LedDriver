#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoOTA.h>

#include "html.h"

// Konfiguracja PWM
extern const int ledPin;
extern const int pwmChannel;
extern const int pwmFreq;
extern const int pwmResolution;
extern int dutyCycle;

// Zmienne do czunika BH1750
extern float lux;
extern unsigned long previousMillis;
extern const long interval;  // Interwał 5 sekund

// Zmienne do czujnika INA219
extern float current_mA;
extern float power_mW;

// Zmienne do czunika SHT40
extern float temperature;
extern float humidity;

// Zmienne do czujnika PIR
extern uint8_t pirPin;
extern uint64_t lastTrigger;
extern uint32_t pirSeconds;

// Konfiguracja Wi-Fi
const char* ssid = "STIBDOM";       
const char* password = "kacper24";

// Statyczna konfiguracja sieci
IPAddress local_IP(192, 168, 0, 180);        // Statyczny adres IP
IPAddress gateway(192, 168, 0, 1);           // Brama
IPAddress subnet(255, 255, 255, 0);          // Maska podsieci
IPAddress primaryDNS(8, 8, 8, 8);            // Główny serwer DNS (Google)
IPAddress secondaryDNS(8, 8, 4, 4);          // Zapasowy serwer DNS (Google)

WebServer server(80);

// Aktualizacja poprzez WiFi
void setupOTA() {
  ArduinoOTA.onStart([]() {
    String type = (ArduinoOTA.getCommand() == U_FLASH) ? "flash" : "filesystem";
    Serial.println("Rozpoczęto aktualizację: " + type);
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nAktualizacja zakończona!");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Postęp: %u%%\r", (progress * 100) / total);
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Błąd [%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Błąd autoryzacji");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Błąd inicjalizacji");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Błąd połączenia");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Błąd odbioru");
    else if (error == OTA_END_ERROR) Serial.println("Błąd zakończenia");
  });

  ArduinoOTA.begin();
  Serial.println("OTA gotowe, czekam na aktualizacje...");
}

void handleSetDuty() {
  if (server.hasArg("value")) {
    dutyCycle = server.arg("value").toInt();
    ledcWrite(ledPin, dutyCycle);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Brak parametru 'value'");
  }
}

void wiFiConfigure(){
  // Konfiguracja statycznego IP
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Błąd konfiguracji statycznego IP");
  }

  // Połączenie z Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Łączenie z Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nPołączono!");
  Serial.print("Adres IP: ");
  Serial.println(WiFi.localIP());
}

void webServerConfigure(){
  // Konfiguracja serwera
  server.on("/", []() {
    server.send(200, "text/html", index_html);
  });
  // Ustawienie wartosci wysterowania LED
  server.on("/setDuty", handleSetDuty);
  // Wartosc luxow
  server.on("/getLux", []() {
    server.send(200, "text/plain", String(lux));
  });
  // Wartosc pradu
  server.on("/getCurrent", []() {
    server.send(200, "text/plain", String(current_mA));
  });
  // Wartosc mocy
  server.on("/getPower", []() {
    server.send(200, "text/plain", String(power_mW));
  });
  // Wartosc temperatury
  server.on("/getTemp", []() {
    server.send(200, "text/plain", String(temperature));
  });
  // Wartosc wilgotnosci
  server.on("/getHumidity", []() {
    server.send(200, "text/plain", String(humidity));
  });
  // Odczytanie wartosci wysterowania LED
  server.on("/getDuty", []() {
    server.send(200, "text/plain", String(dutyCycle));
  });
  // Odczyt sekund od ostatniego wykrycia ruchu
  server.on("/getPIR", []() {
    server.send(200, "text/plain", String(pirSeconds));
  });
  // Uruchomienie serwera
  server.begin();
  Serial.println("Serwer uruchomiony");
}