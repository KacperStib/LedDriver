#include <stdio.h>
#include "pir.h"

volatile uint64_t pirMillis = 0;

void IRAM_ATTR pirHandle(void *args){
	pirMillis = esp_timer_get_time() / 1000;
}

void pir_config()
{
	gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << PIR_PIN);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    
    gpio_config(&io_conf);
    
    gpio_install_isr_service(0);  // Instalujemy ISR (raz na program)
    gpio_isr_handler_add(PIR_PIN, pirHandle, NULL);
}
