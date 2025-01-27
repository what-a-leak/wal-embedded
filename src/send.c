#include "send.h"
#include "protocol.h"

#include <esp_timer.h>
#include <esp_task_wdt.h>
#include <esp_dsp.h>
#include <math.h>
#include <string.h>

#include "../lib/peripherals/mic/inmp441.h"
#include "../lib/processing/fft.h"
#include "../lib/utils/encrypt.h"
#ifndef USE_SCREEN
#define screen_log(...)
#else
#include "../lib/peripherals/screen/logger.h"
#endif

// Dummy payload
static const payload_t _payload = {
    .header = 0x01,
    .node_id = 0x0001,
    .timestamp = 0x00000001,
    .battery_level = 0x20,
    .temperature = 0x14,
    .reduced_fft = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16}
};
static int32_t _count = 0;

void send_task() {
    lora_init();
    lora_set_frequency(433e6);
    lora_enable_crc();
    lora_set_coding_rate(CODING_RATE);
    lora_set_bandwidth(BANDWIDTH);
    lora_set_spreading_factor(SPREADING_FACTOR);

    while (1) {
        esp_err_t err = lora_send_packet((uint8_t*)(&_payload), sizeof(payload_t));

        if (err == ESP_OK) {
            printf("LoRa: Payload Sent\n");
            screen_log("[%d]LoRa: send", _count);
        } else {
            printf("LoRa: Failed to Send Payload\n");
            screen_log("LoRa: FAILED");
        }
        // ToA = 0.7 seconds -> 10% duty cycle for 433 MHz
        vTaskDelay(7000 / portTICK_PERIOD_MS);
        _count++;
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

#ifndef DISABLE_SECURITY
void test_encryption_task() {
    const char *test_message = "Hello World";
    uint8_t encrypted_data[MAX_PAYLOAD_SIZE];
    size_t encrypted_len;

    char decrypted_data[MAX_PAYLOAD_SIZE];
    size_t decrypted_len;

    while (1) {
        printf("\n[Encryption test] Original Message: %s\n", test_message);

        if (encrypt_message(test_message, strlen(test_message), encrypted_data, &encrypted_len) == 0) {
            printf("[Encryption test] Encrypted Data:\n");
            for (size_t i = 0; i < encrypted_len; i++) {
                printf("%02x ", encrypted_data[i]);
            }
            printf("\n");
        } else {
            printf("[Encryption test] Encryption failed\n");
            break;
        }

        if (decrypt_message(encrypted_data, encrypted_len, decrypted_data, &decrypted_len) == 0) {
            decrypted_data[decrypted_len] = '\0'; // \0 end the message
            printf("[Encryption test] Decrypted Message: %s\n", decrypted_data);
        } else {
            printf("[Encryption test] Decryption failed\n");
            break;
        }

        vTaskDelay(4200 / portTICK_PERIOD_MS);
    }
}
#endif
