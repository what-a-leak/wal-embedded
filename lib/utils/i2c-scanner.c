#include "i2c-scanner.h"
#include "esp_log.h"

void i2c_scanner_init(i2c_port_t i2c_num, gpio_num_t sda_io, gpio_num_t scl_io, uint32_t clk_speed) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda_io,
        .scl_io_num = scl_io,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = clk_speed,
    };
    ESP_ERROR_CHECK(i2c_param_config(i2c_num, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(i2c_num, I2C_MODE_MASTER, 0, 0, 0));
}

void i2c_scan_devices(i2c_port_t i2c_num) {
    for (int i = 0; i < 128; i++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);

        esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 10 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

        if (ret == ESP_OK) {
            printf("Found device at address 0x%02x\n", i);
        }
    }
}
