#include <stdio.h>
#include "myNvs.h"

uint8_t read_duty(){
	uint8_t duty = 0;
	size_t size = sizeof(duty);
	nvs_get_blob(my_handle, "duty", &duty, &size);
	return duty;
}

uint8_t read_ambient(){
	uint8_t ambient = 0;
	size_t size = sizeof(ambient);
	nvs_get_blob(my_handle, "ambient", &ambient, &size);
	return ambient;
}
 	
uint8_t read_pir(){
	uint8_t pir = 0;
	size_t size = sizeof(pir);
	nvs_get_blob(my_handle, "pir", &pir, &size);
	return pir;
}

uint16_t read_pir_time(){
	uint16_t pirTime = 0;
	size_t size = sizeof(pirTime);
	nvs_get_blob(my_handle, "pirTime", &pirTime, &size);
	return pirTime;
}
 	
uint32_t read_power_usage(){
	uint32_t powerUsed = 0;
	size_t size = sizeof(powerUsed);
	nvs_get_blob(my_handle, "powerUsed", &powerUsed, &size);
	return powerUsed;
}

void write_duty(uint8_t duty){
	nvs_set_blob(my_handle, "duty", &duty, sizeof(duty));
	nvs_commit(my_handle);
}

void write_ambient(uint8_t ambient){
	nvs_set_blob(my_handle, "ambient", &ambient, sizeof(ambient));
	nvs_commit(my_handle);
}
 			
void write_pir(uint8_t pir){
	nvs_set_blob(my_handle, "pir", &pir, sizeof(pir));
	nvs_commit(my_handle);
}

void write_pir_time(uint16_t pirTime){
	nvs_set_blob(my_handle, "pirTime", &pirTime, sizeof(pirTime));
	nvs_commit(my_handle);
}
 		
void write_power_usage(uint32_t powerUsed){
	nvs_set_blob(my_handle, "powerUsed", &powerUsed, sizeof(powerUsed));
	nvs_commit(my_handle);
}