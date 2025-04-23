#include "i2c.h"

#define INA219_REG_CONFIG (0x00)
#define INA219_REG_SHUNTVOLTAGE (0x01)
#define INA219_REG_BUSVOLTAGE (0x02)
#define INA219_REG_POWER (0x03)
#define INA219_REG_CURRENT (0x04)
#define INA219_REG_CALIBRATION (0x05)

esp_err_t ina219_power_on();
esp_err_t ina219_read_voltage();
esp_err_t ina219_read_current();
esp_err_t ina219_read_power();void func(void);
