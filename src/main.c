#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_task_wdt.h"
#include "esp_dsp.h"
#include "../lib/peripherals/mic/inmp441.h"
#include "../lib/processing/fft.h"
#include "../lib/peripherals/lora/sx127x.h"
#include "../lib/peripherals/lora/sx1278_lora.h"


// Constants for FFT
#define SAMPLE_RATE 88200 // Fixed aliasing issue
#define N_SAMPLES 1024

// Constants for LoRa Communication
#define CODING_RATE         8   // CR = 4/8
#define BANDWIDTH           6   // 62.5 kHz from the datasheet
#define SPREADING_FACTOR    12  // 4096 chirps: SF12
#define MAX_PACKET_SIZE     256

static const char* _packet = "Hello World!";
static uint8_t _rx_buff[MAX_PACKET_SIZE] = {0};

void send_task() {
    lora_init();
    lora_set_frequency(433e6);
    lora_enable_crc();
    lora_set_coding_rate(CODING_RATE);
    lora_set_bandwidth(BANDWIDTH);
    lora_set_spreading_factor(SPREADING_FACTOR);

    while (1) {
        esp_err_t err = lora_send_packet((uint8_t*)_packet, 13);

        if (err == ESP_OK) {
            printf("LoRa: Packet Sent - %s\n", _packet);
        } else {
            printf("LoRa: Failed to Send Packet\n");
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void compute_fft_task() {
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

void receive_task() {
    lora_init();
    lora_set_frequency(433e6);
    lora_enable_crc();
    lora_set_coding_rate(CODING_RATE);
    lora_set_bandwidth(BANDWIDTH);
    lora_set_spreading_factor(SPREADING_FACTOR);
    lora_receive();  // Set to receive mode

    while (1) {
        if (lora_received()) {
            const int len = lora_receive_packet(_rx_buff, MAX_PACKET_SIZE);
            const int rssi = lora_packet_rssi();
            printf("LoRa: Received packet of size %d\n", len);
            printf("RSSI: %d dB\n", rssi);
            printf("Payload: ");
            for (int i = 0; i < len; i++) {
                printf("%c", _rx_buff[i]);
            }
            printf("\n");
        }
        vTaskDelay(100 / portTICK_PERIOD_MS); 
    }
}

void app_main() {
    xTaskCreate(compute_fft_task, "compute_fft_task", 12000, NULL, 5, NULL);
    xTaskCreate(send_task, "send_task", 4000, NULL, 5, NULL); 
    // xTaskCreate(receive_task, "receive_task", 4000, NULL, 5, NULL);
}



