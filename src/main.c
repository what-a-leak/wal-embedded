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


#define CODING_RATE         8   // CR = 4/8
#define BANDWIDTH           6   // 62.5 kHz from the datasheet
#define SPREADING_FACTOR    12  // 4096 chirps: SF12
#define MAX_PACKET_SIZE     256

static const char* _packet = "Hello World!c";
static uint32_t _count = 0;

static uint8_t _rx_buff[MAX_PACKET_SIZE] = {0};


void send_task() {
    printf("Starting LoRa Sender Task\n");

    lora_init();
    lora_set_frequency(433e6);
    lora_enable_crc();
    lora_set_coding_rate(CODING_RATE);
    lora_set_bandwidth(BANDWIDTH);
    lora_set_spreading_factor(SPREADING_FACTOR);

    printf("LoRa Sender Setup Complete:\n");
    printf("Frequency: 433 MHz\n");
    printf("Coding Rate: %d\n", CODING_RATE);
    printf("Bandwidth: %d\n", BANDWIDTH);
    printf("Spreading Factor: %d\n", SPREADING_FACTOR);


    while (1) {
        esp_err_t err = lora_send_packet((uint8_t*)_packet, 13);

        if (err == ESP_OK) {
            printf("[%ld] LoRa: Packet Sent - %s\n", _count, _packet);
        } else {
            printf("[%ld] LoRa: Failed to Send Packet\n", _count);
        }
        _count++;
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // Delay for 1 second
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

    printf("LoRa Receiver Setup Complete:\n");
    printf("Frequency: 433 MHz\n");
    printf("Coding Rate: %d\n", CODING_RATE);
    printf("Bandwidth: %d\n", BANDWIDTH);
    printf("Spreading Factor: %d\n", SPREADING_FACTOR);

    while (1) {
        if (lora_received())
        {
            const int len = lora_receive_packet(_rx_buff, MAX_PACKET_SIZE);
            const int rssi = lora_packet_rssi();
            screen_log("LoRa:[%ld]rx %d", _count, len);
            screen_log("   %d dB", rssi);
            printf("[%ld] LoRa: received packet of size %d!\n", _count, len);
            printf("Payload: ");
            for(int i=0; i<len; i++)
                printf("0x%x ", _rx_buff[i]);
            printf("\n");
            _count++;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS); 
    }
}

void hello_world_task() {
    while (1) {
        printf("Hello, World!\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // Delay for 1 second
    }
}


void app_main() {
    // xTaskCreate(send_task, "send_task", 12000, NULL, 5, NULL);
    xTaskCreate(receive_task, "receive_task", 12000, NULL, 5, NULL);
}


/*
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
*/