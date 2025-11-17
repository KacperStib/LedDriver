#include "nvs_flash.h"
#include <stdio.h>

// nvs flsah
 extern nvs_handle_t my_handle;

uint8_t read_duty();
uint8_t read_ambient();
 	
uint8_t read_pir();
uint16_t read_pir_time();
 	
uint32_t read_power_usage();

void write_duty(uint8_t duty);
void write_ambient(uint8_t ambient);
 			
void write_pir(uint8_t pir);
void write_pir_time(uint16_t pirTime);
 		
void write_power_usage(uint32_t powerUsed);