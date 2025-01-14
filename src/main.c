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

static uint8_t _rx_buff[MAX_PACKET_SIZE] = {0};

// Payload structure: Header (8 bit), Node ID (16 bit), Timestamp (32 bit), Battery Level (8 bit), Temperature (8 bit), Reduced_FFT (184 bit)
typedef struct {
    uint8_t header;
    uint16_t node_id;
    uint32_t timestamp;
    uint8_t battery_level;
    uint8_t temperature;
    uint8_t reduced_fft[23]; // 184 bits / 8 bits per byte = 23 bytes
} payload_t;



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
    uint8_t decimated_output_data[32];

    // Initialize microphone and FFT
    inmp_init(GPIO_SCK, GPIO_SD, GPIO_WS, SAMPLE_RATE);
    fft_init(N_SAMPLES);

    while (true) {
        size_t bytes = inmp_read_raw_data(raw_data_buffer, N_SAMPLES * sizeof(int16_t));
        size_t num_samples = bytes / sizeof(int16_t);

        if (num_samples == N_SAMPLES) {
            fft_process(raw_data_buffer, num_samples, magnitude_data); // Perform FFT on the data and store the magnitude
            decimate_fft(magnitude_data, num_samples / 2, decimated_output_data, 32); // Decimate the FFT data to fit into 23 bytes
            send_decimated_fft_data(decimated_output_data, 32); // Send the decimated FFT data
            
            // send_fft_data(magnitude_data, num_samples / 2);
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

            if (len == sizeof(payload_t)) {
                payload_t* received_payload = (payload_t*)_rx_buff;
                printf("Header: 0x%02X\n", received_payload->header);
                printf("Node ID: 0x%04X\n", received_payload->node_id);
                printf("Timestamp: 0x%08lX\n", received_payload->timestamp);
                printf("Battery Level: 0x%02X\n", received_payload->battery_level);
                printf("Temperature: 0x%02X\n", received_payload->temperature);
                printf("Reduced FFT: ");
                for (int i = 0; i < sizeof(received_payload->reduced_fft); i++) {
                    printf("0x%02X ", received_payload->reduced_fft[i]);
                }
                printf("\n");
            } else {
                printf("Received packet size does not match payload size\n");
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS); 
    }
}

void app_main() {
    xTaskCreate(compute_fft_task, "compute_fft_task", 12000, NULL, 5, NULL);
    xTaskCreate(send_task, "send_task", 4000, NULL, 5, NULL); 
    // xTaskCreate(receive_task, "receive_task", 4000, NULL, 5, NULL);
}



