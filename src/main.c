#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "../lib/peripherals/imu/lsm6ds3.h"

void app_main() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = 8,
        .scl_io_num = 9,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));

    init_lsm6ds3(I2C_NUM_0);

    while (1) {
        float acc_x, acc_y, acc_z;
        read_acceleration(I2C_NUM_0, &acc_x, &acc_y, &acc_z);

        printf("%.2f, %.2f, %.2f\n", acc_x, acc_y, acc_z);

        vTaskDelay(10 / portTICK_PERIOD_MS);  
    }
}
