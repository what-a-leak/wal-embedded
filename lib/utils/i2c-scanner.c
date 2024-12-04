#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "i2c-scanner.h"

void init_i2c() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = 8,
        .scl_io_num = 9,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    i2c_param_config(I2C_NUM_0, &conf);
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));
}

void scan_i2c() {
    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        for (int i = 0; i < 128; i++) {
            i2c_cmd_handle_t cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE, 1);
            i2c_master_stop(cmd);
            esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
            i2c_cmd_link_delete(cmd);

            if (ret == ESP_OK) {
                printf("I2C device found at address 0x%02x\n", i);
            }
        }
    }
}