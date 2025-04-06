#include <BH1750.h>
#include <Adafruit_INA219.h>
#include "Adafruit_SHT4x.h"
#include <Wire.h>

// Instancje czujnikow
BH1750 lightMeter;
Adafruit_INA219 ina219;
Adafruit_SHT4x sht4 = Adafruit_SHT4x();

void initSensors(){
  // Magistrala I2C
  Wire.begin();

  // Luksomierz
  lightMeter.begin();

  // Pomiar mocy i pradu
  ina219.begin();

  // Temperatura i wilgotnosc
  sht4.begin();
  sht4.setPrecision(SHT4X_HIGH_PRECISION);
}

void measurement(float *lux, float *current_mA, float *power_mW, float *temperature, float *humidity){
  // Zmienne do cząstkowych pomiarów
  float c_mA = 0;
  float p_mW = 0;

  // Luksy
  *lux = lightMeter.readLightLevel();

  // Odczyt usrednionej wartosci pradu i mocy
  for(int i = 0; i < 10; i++){
    c_mA += ina219.getCurrent_mA();
    p_mW += ina219.getPower_mW();
    delay(50);
  }
  
  // Zapis usrednionej wartosci pradu i mocy
  *current_mA = c_mA / 10.0;
  if(*current_mA < 0) *current_mA = 0;
  *power_mW = p_mW / 10.0;
  if(*power_mW < 0) *power_mW = 0;

  // Temperatura i wilgotność
  sensors_event_t humidity_event, temp_event;
  sht4.getEvent(&humidity_event, &temp_event);
  *temperature = temp_event.temperature;
  *humidity = humidity_event.relative_humidity;
}