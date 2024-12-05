#include "lsm6ds3.h"
#include "esp_log.h"

void init_lsm6ds3(i2c_port_t i2c_num) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LSM6DS3_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, CTRL1_XL, true);  // Address of CTRL1_XL register
    i2c_master_write_byte(cmd, 0xA0, true);      // 0x60 = 6.66kHz, ±2g, normal mode
    i2c_master_stop(cmd);
    ESP_ERROR_CHECK(i2c_master_cmd_begin(i2c_num, cmd, 10 / portTICK_PERIOD_MS));
    i2c_cmd_link_delete(cmd);
}

void read_acceleration(i2c_port_t i2c_num, float *acc_x, float *acc_y, float *acc_z) {
    uint8_t data[6];
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LSM6DS3_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, OUTX_L_XL, true);  // Address of the first register
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LSM6DS3_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, 6, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    ESP_ERROR_CHECK(i2c_master_cmd_begin(i2c_num, cmd, 10 / portTICK_PERIOD_MS));
    i2c_cmd_link_delete(cmd);

    int16_t raw_x = (int16_t)(data[1] << 8 | data[0]);
    int16_t raw_y = (int16_t)(data[3] << 8 | data[2]);
    int16_t raw_z = (int16_t)(data[5] << 8 | data[4]);

    const float scale = 2.0f / 32768.0f;
    *acc_x = raw_x * scale;
    *acc_y = raw_y * scale;
    *acc_z = raw_z * scale;
}


// Read only the Z axis
void read_acceleration_z(i2c_port_t i2c_num, float *acc_z) {
    uint8_t data[2];
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LSM6DS3_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, OUTX_L_XL + 4, true);  // Address of the Z register
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LSM6DS3_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, 2, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    ESP_ERROR_CHECK(i2c_master_cmd_begin(i2c_num, cmd, 10 / portTICK_PERIOD_MS));
    i2c_cmd_link_delete(cmd);

    int16_t raw_z = (int16_t)(data[1] << 8 | data[0]);

    const float scale = 2.0f / 32768.0f;
    *acc_z = raw_z * scale;
}

