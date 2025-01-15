#include "send.h"
#include "protocol.h"

#include <esp_timer.h>
#include <esp_task_wdt.h>
#include <esp_dsp.h>
#include <math.h>

#include "../lib/peripherals/mic/inmp441.h"
#include "../lib/processing/fft.h"

void send_task() {
    lora_init();
    lora_set_frequency(433e6);
    lora_enable_crc();
    lora_set_coding_rate(CODING_RATE);
    lora_set_bandwidth(BANDWIDTH);
    lora_set_spreading_factor(SPREADING_FACTOR);

    // Dummy payload
    payload_t payload = {
        .header = 0x01,
        .node_id = 0x0001,
        .timestamp = 0x00000001,
        .battery_level = 0x20,
        .temperature = 0x14,
        .reduced_fft = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16}
    };

    while (1) {
        esp_err_t err = lora_send_packet((uint8_t*)&payload, sizeof(payload));

        if (err == ESP_OK) {
            // printf("LoRa: Payload Sent\n");
        } else {
            // printf("LoRa: Failed to Send Payload\n");
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void compute_fft_task() {
    esp_task_wdt_delete(NULL);

    int16_t raw_data_buffer[N_SAMPLES];
    float magnitude_data[N_SAMPLES / 2];
    uint8_t decimated_output_data[20];

    // Initialize microphone and FFT
    inmp_init(GPIO_SCK, GPIO_SD, GPIO_WS, SAMPLE_RATE);
    fft_init(N_SAMPLES);

    while (true) {
        size_t bytes = inmp_read_raw_data(raw_data_buffer, N_SAMPLES * sizeof(int16_t));
        size_t num_samples = bytes / sizeof(int16_t);

        if (num_samples == N_SAMPLES) {
            fft_process(raw_data_buffer, num_samples, magnitude_data); // Perform FFT on the data and store the magnitude
            compress_fft(magnitude_data, num_samples / 2, decimated_output_data, 20); // Compress the magnitude data
            send_compressed_fft_data(decimated_output_data, 20); // Send the compressed data
            
            // send_fft_data(magnitude_data, num_samples / 2);
        } else {
            printf("Insufficient data for FFT: %zu samples\n", num_samples);
        }

        vTaskDelay(pdMS_TO_TICKS(10)); // Delay to prevent continuous output
    }
}