#ifndef LSM6DS3_H
#define LSM6DS3_H

#include "driver/i2c.h"

#define LSM6DS3_ADDR 0x6A  // Default I2C address of LSM6DS3
#define WHO_AM_I_REG 0x0F  // Identification register
#define CTRL1_XL 0x10      // Accelerometer configuration register
#define OUTX_L_XL 0x28     // First register of acceleration data (X, Y, Z)

void init_lsm6ds3(i2c_port_t i2c_num);
void read_acceleration(i2c_port_t i2c_num, float *acc_x, float *acc_y, float *acc_z);

#endif // LSM6DS3_H
