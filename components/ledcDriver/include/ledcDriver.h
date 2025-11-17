#include "driver/ledc.h"

// Konfiguracja PWM
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
//#define LEDC_OUTPUT_IO          (0) // FINAL
#define LEDC_OUTPUT_IO          (10) //PROTOTYPE
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_8_BIT // Set duty resolution to 13 bits
#define LEDC_FREQUENCY          (4000) // Frequency in Hertz. Set frequency at 4 kHz

extern uint8_t dutyCycle;

extern float mapLogCurve[255];

void led_config();
void led_write(uint8_t dutyCycle);
