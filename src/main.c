#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_task_wdt.h"
#include "esp_dsp.h"
#include "../lib/peripherals/mic/inmp441.h"
#include "../lib/processing/fft.h"
#include "../lib/peripherals/lora/sx1278.h"

// Constants
#define SAMPLE_RATE 88200 // Fixed aliasing issue
#define N_SAMPLES 1024

void simple_task() {
    esp_task_wdt_delete(NULL);

    int16_t raw_data_buffer[N_SAMPLES];

    // Initialize microphone and FFT
    inmp_init(GPIO_SCK, GPIO_SD, GPIO_WS, SAMPLE_RATE);
    fft_init(N_SAMPLES);

    while (true) {
        size_t bytes = inmp_read_raw_data(raw_data_buffer, N_SAMPLES * sizeof(int16_t));
        size_t num_samples = bytes / sizeof(int16_t);

        if (num_samples == N_SAMPLES) {
            fft_process(raw_data_buffer, num_samples); // Perform FFT on the data
            
        } else {
            printf("Insufficient data for FFT: %zu samples\n", num_samples);
        }

        vTaskDelay(pdMS_TO_TICKS(10)); // Delay to prevent continuous output
    }
}

void app_main() {
    xTaskCreate(simple_task, "simple_task", 12000, NULL, 5, NULL);
}