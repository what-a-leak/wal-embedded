#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_dsp.h"
#include "../lib/peripherals/mic/inmp441.h"
#include "../lib/processing/fft.h"
#include "../lib/peripherals/lora/sx1278.h"

// Constants
#define SAMPLE_RATE 44100
#define N_SAMPLES 1024

void simple_task() {
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

        vTaskDelay(pdMS_TO_TICKS(100)); // Delay to prevent continuous output
    }
}


void sender_task(void *pvParameter) {
    if (sx1278_init() != ESP_OK) {
        printf("Failed to initialize SX1278.\n");
        vTaskDelete(NULL);
        return;
    }

    uint8_t tx_data[] = {0x01, 0x02, 0x03, 0x04}; // Example data to send
    while (1) {
        printf("Sending data...\n");
        if (sx1278_send(tx_data, sizeof(tx_data)) == ESP_OK) {
            printf("Data sent: ");
            for (int i = 0; i < sizeof(tx_data); i++) {
                printf("0x%02X ", tx_data[i]);
            }
            printf("\n");
        } else {
            printf("Failed to send data.\n");
        }

        // Wait for a second before sending again
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    sx1278_cleanup();
    vTaskDelete(NULL);
}


void app_main() {
    // xTaskCreate(simple_task, "simple_task", 12000, NULL, 5, NULL);

    xTaskCreate(sender_task, "sender_task", 2048, NULL, 5, NULL);
}