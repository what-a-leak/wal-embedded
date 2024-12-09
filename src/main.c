#include <stdio.h>
#include <math.h>
#include <complex.h>
#include "driver/i2c.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_dsp.h"
#include "../lib/peripherals/imu/lsm6ds3.h"
#include "../lib/peripherals/mic/inmp441.h"

void simple_task() {

    int16_t raw_data_buffer[I2S_READ_LEN];

    inmp_init(GPIO_SCK, GPIO_SD, GPIO_WS, SAMPLE_RATE);

    while (true) {
        size_t bytes = inmp_read_raw_data(raw_data_buffer, I2S_READ_LEN); // Read direct PCM data from microphone, I2S_READ_LEN = 1024 samples

        // Print samples
        for (size_t i = 0; i < bytes / sizeof(int16_t); i++) {
            printf("%d\n", raw_data_buffer[i]);
        }
        
        // 1 second delay
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

void app_main() {
    xTaskCreate(simple_task, "simple_task", 4096, NULL, 5, NULL);
}
