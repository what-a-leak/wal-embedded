#include <stdio.h>
#include <math.h>
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_dsp.h"
#include "../lib/peripherals/imu/lsm6ds3.h"
#include "../lib/peripherals/mic/inmp441.h"

void microphone_task(void *param)
{
    while (1) {
        // Read sound level
        float sound_level = inmp_read_sound_level();
        printf("Sound Level (RMS): %.2f\n", sound_level);

        vTaskDelay(pdMS_TO_TICKS(50)); // Short delay between readings
    }
}

void app_main()
{
    // Initialize the INMP441 microphone
    inmp_init(GPIO_SCK, GPIO_SD, GPIO_WS, SAMPLE_RATE);

    // Create the microphone task
    xTaskCreate(microphone_task, "microphone_task", 4096, NULL, 5, NULL);
}