#include <stdio.h>
#include "ledcDriver.h"

// Poczatkowa wartosc
uint8_t dutyCycle = 0;

// Mapowanie krzywej liniowej na logarytmiczna przy rozdzielczosci 8 bit - todo
float mapLogCurve[255] = {  0.10, 0.10, 0.10, 0.11, 0.11, 0.11, 0.11, 0.12, 0.12, 0.12, 0.13, 0.13, 0.14, 0.14, 0.14, 0.15,
                  0.15, 0.15 ,0.16 ,0.16 ,0.17 ,0.17, 0.18, 0.18 ,0.19 ,0.19 ,0.20 ,0.20 ,0.21 ,0.21 ,0.22 ,0.23,
                  0.23, 0.24, 0.25, 0.25, 0.26, 0.27, 0.27 ,0.28 ,0.29 ,0.30 ,0.31 ,0.31 ,0.32 ,0.33 ,0.34 ,0.35,
                  0.36, 0.37, 0.38, 0.39, 0.40, 0.41, 0.43, 0.44, 0.45, 0.46, 0.47, 0.49, 0.50, 0.51, 0.53, 0.54, 
                  0.56, 0.57, 0.59, 0.61, 0.62, 0.64, 0.66, 0.68, 0.69, 0.71, 0.73, 0.75, 0.78, 0.80, 0.82, 0.84, 
                  0.86, 0.89, 0.91, 0.94, 0.96, 0.99, 1.02, 1.05, 1.08, 1.11, 1.14, 1.17, 1.20, 1.23, 1.27, 1.30, 
                  1.34, 1.38, 1.41, 1.45, 1.49, 1.53, 1.58, 1.62, 1.66, 1.71, 1.76, 1.81, 1.86, 1.91, 1.96, 2.02, 
                  2.07, 2.13, 2.19, 2.25, 2.31, 2.37, 2.44, 2.51, 2.58, 2.65, 2.72, 2.80, 2.87, 2.95, 3.04, 3.12, 
                  3.21, 3.29, 3.39, 3.48, 3.58, 3.68, 3.78, 3.88, 3.99, 4.10, 4.21, 4.33, 4.45, 4.57, 4.70, 4.83, 
                  4.96, 5.10, 5.24, 5.39, 5.54, 5.69, 5.85, 6.01, 6.17, 6.35, 6.52, 6.70, 6.89, 7.08, 7.27, 7.47, 
                  7.68, 7.89, 8.11, 8.34, 8.57, 8.81, 9.05, 9.30, 9.56, 9.82, 10.09, 10.37, 10.66, 10.95, 11.26, 11.57, 
                  11.89, 12.22, 12.56, 12.90, 13.26, 13.63, 14.01, 14.39, 14.79, 15.20, 15.62, 16.06, 16.50, 16.96, 17.43, 17.91, 
                  18.40, 18.91, 19.44, 19.97, 20.53, 21.10, 21.68, 22.28, 22.90, 23.53, 24.18, 24.85, 25.54, 26.25, 26.97, 27.72, 
                  28.49, 29.28, 30.09, 30.92, 31.77, 32.65, 33.56, 34.49, 35.44, 36.42, 37.43, 38.47, 39.53, 40.63, 41.75, 42.91, 
                  44.09, 45.31, 46.57, 47.86, 49.18, 50.54, 51.94, 53.38, 54.86, 56.38, 57.94, 59.54, 61.19, 62.88, 64.62, 66.41, 
                  68.25, 70.14, 72.08, 74.08, 76.13, 78.23, 80.40, 82.62, 84.91, 87.26, 89.68, 92.16, 94.71, 97.33, 100.0
};

// Konfiguracja API
void led_config(){
	// Konfiguracja timera
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = LEDC_TIMER,
        .freq_hz          = LEDC_FREQUENCY,  
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Konfiguracja kanalu
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO,
        .duty           = 0,
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

// Sterowanie PWM
void led_write(uint8_t dutyCycle){
    // Ustawienie wartosci
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, dutyCycle);
    // Wyslanie wartosci
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}