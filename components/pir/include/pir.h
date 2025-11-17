#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include <esp_timer.h>

//#define PIR_PIN 1 //FINAL
#define PIR_PIN 6   
//PROTOTYPE
extern volatile uint64_t pirMillis;
extern gpio_num_t pin;

void pir_config();