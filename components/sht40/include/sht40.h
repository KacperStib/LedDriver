#include "i2c.h"

#define SHT41_ADRESS 0x44

#define HIGH_PREC_MEAS 0xFD
#define MED_PREC_MEAS 0xF6
#define SERIAL_NUM 0x89

void SHT41measurment(float* T, float* RH);