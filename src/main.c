#include <stdio.h>
#include <math.h>
#include "driver/i2c.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_dsp.h"
#include "../lib/peripherals/imu/lsm6ds3.h"
#include "../lib/peripherals/mic/inmp441.h"

void microphone_task(void *param)
{
    uint64_t start_time = 0;
    uint64_t end_time = 0;

    while (1) {
        // Measure the time it takes to read the sound level using esp_timer_get_time()
        // start_time = esp_timer_get_time();
        // inmp_read_sound_level();
        // end_time = esp_timer_get_time();
        // printf("Time: %lld us\n", end_time - start_time);
        

        printf("Sound level: %.2f\n", inmp_read_sound_level());

        // vTaskDelay(pdMS_TO_TICKS(1000)); // Short delay between readings
    }
}

void app_main()
{
    // Initialize the INMP441 microphone
    inmp_init(GPIO_SCK, GPIO_SD, GPIO_WS, SAMPLE_RATE);

    // Create the microphone task
    xTaskCreate(microphone_task, "microphone_task", 4096, NULL, 5, NULL);
}