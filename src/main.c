#include <stdio.h>
#include <math.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "esp_task_wdt.h"
#include "esp_dsp.h"
#include "../lib/peripherals/mic/inmp441.h"
#include "../lib/processing/fft.h"
#include "../lib/peripherals/lora/sx127x.h"
#include "../lib/peripherals/lora/sx1278_lora.h"

// Configuration: Uncomment one of the following lines to select the operation scenario
// #define SCENARIO_LORA_SEND
// #define SCENARIO_LORA_RECEIVE
// #define SCENARIO_FFT_COMPUTE
// #define SCENARIO_IDLE
#define SCENARIO_DEEP_SLEEP

// LoRa Configuration
#define CODING_RATE         8   // CR = 4/8
#define BANDWIDTH           6   // 62.5 kHz from the datasheet
#define SPREADING_FACTOR    12  // 4096 chirps: SF12
#define MAX_PACKET_SIZE     256
#define LORA_FREQUENCY      433e6

static const char* _packet = "Hello World!";
static uint32_t _count = 0;
static uint8_t _rx_buff[MAX_PACKET_SIZE] = {0};

// Deep Sleep Task
#define DEEP_SLEEP_DURATION_SEC 60  // Sleep for 60 seconds

void deep_sleep_task() {
    printf("Entering Deep Sleep mode in 5 seconds...\n");
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    printf("Preparing for Deep Sleep...\n");
    esp_sleep_enable_timer_wakeup(DEEP_SLEEP_DURATION_SEC * 1000000ULL);
    printf("The system will now enter Deep Sleep for %d seconds.\n", DEEP_SLEEP_DURATION_SEC);

    esp_deep_sleep_start();
}

// LoRa Send Task
void send_task() {
    printf("Starting LoRa Sender Task\n");

    lora_init();
    lora_set_frequency(LORA_FREQUENCY);
    lora_enable_crc();
    lora_set_coding_rate(CODING_RATE);
    lora_set_bandwidth(BANDWIDTH);
    lora_set_spreading_factor(SPREADING_FACTOR);

    printf("LoRa Sender Setup Complete:\n");
    printf("Frequency: %0.1f MHz\n", LORA_FREQUENCY / 1e6);
    printf("Coding Rate: %d\n", CODING_RATE);
    printf("Bandwidth: %d\n", BANDWIDTH);
    printf("Spreading Factor: %d\n", SPREADING_FACTOR);

    while (1) {
        esp_err_t err = lora_send_packet((uint8_t*)_packet, strlen(_packet));

        if (err == ESP_OK) {
            printf("[%ld] LoRa: Packet Sent - %s\n", _count, _packet);
        } else {
            printf("[%ld] LoRa: Failed to Send Packet\n", _count);
        }
        _count++;
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // Delay for 1 second
    }
}

// LoRa Receive Task
void receive_task() {
    lora_init();
    lora_set_frequency(LORA_FREQUENCY);
    lora_enable_crc();
    lora_set_coding_rate(CODING_RATE);
    lora_set_bandwidth(BANDWIDTH);
    lora_set_spreading_factor(SPREADING_FACTOR);
    lora_receive();  // Set to receive mode

    printf("LoRa Receiver Setup Complete:\n");
    printf("Frequency: %0.1f MHz\n", LORA_FREQUENCY / 1e6);
    printf("Coding Rate: %d\n", CODING_RATE);
    printf("Bandwidth: %d\n", BANDWIDTH);
    printf("Spreading Factor: %d\n", SPREADING_FACTOR);

    while (1) {
        if (lora_received()) {
            const int len = lora_receive_packet(_rx_buff, MAX_PACKET_SIZE);
            const int rssi = lora_packet_rssi();
            printf("[%ld] LoRa: Received packet of size %d, RSSI: %d dB\n", _count, len, rssi);
            printf("Payload: ");
            for (int i = 0; i < len; i++) {
                printf("0x%x ", _rx_buff[i]);
            }
            printf("\n");
            _count++;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS); 
    }
}

// FFT Compute Task
#define SAMPLE_RATE 88200
#define N_SAMPLES 1024

void fft_task() {
    int16_t raw_data_buffer[N_SAMPLES];

    inmp_init(GPIO_SCK, GPIO_SD, GPIO_WS, SAMPLE_RATE);
    fft_init(N_SAMPLES);

    while (1) {
        size_t bytes = inmp_read_raw_data(raw_data_buffer, N_SAMPLES * sizeof(int16_t));
        size_t num_samples = bytes / sizeof(int16_t);

        if (num_samples == N_SAMPLES) {
            printf("Performing FFT computation...\n");
            fft_process(raw_data_buffer, num_samples);
        } else {
            printf("Insufficient data for FFT: %zu samples\n", num_samples);
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);  // 10 ms delay
    }
}

// Idle Task
void idle_task() {
    while (1) {
        printf("Idle Task: System is idle...\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // 1 second delay
    }
}

// Main Entry Point
void app_main() {
#ifdef SCENARIO_LORA_SEND
    xTaskCreate(send_task, "send_task", 12000, NULL, 5, NULL);
#elif defined(SCENARIO_LORA_RECEIVE)
    xTaskCreate(receive_task, "receive_task", 12000, NULL, 5, NULL);
#elif defined(SCENARIO_FFT_COMPUTE)
    xTaskCreate(fft_task, "fft_task", 12000, NULL, 5, NULL);
#elif defined(SCENARIO_IDLE)
    xTaskCreate(idle_task, "idle_task", 12000, NULL, 5, NULL);
#elif defined(SCENARIO_DEEP_SLEEP)
    xTaskCreate(deep_sleep_task, "deep_sleep_task", 12000, NULL, 5, NULL);
#else
    #error "No operation scenario defined. Please define one."
#endif
}
