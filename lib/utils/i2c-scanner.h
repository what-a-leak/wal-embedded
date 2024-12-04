#ifndef I2C_SCANNER_H
#define I2C_SCANNER_H

#include "driver/i2c.h"

void i2c_scanner_init(i2c_port_t i2c_num, gpio_num_t sda_io, gpio_num_t scl_io, uint32_t clk_speed);
void i2c_scan_devices(i2c_port_t i2c_num);

#endif // I2C_SCANNER_H
