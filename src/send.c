#include "send.h"
#include "protocol.h"

#include <esp_timer.h>
#include <esp_task_wdt.h>
#include <esp_dsp.h>
#include <esp_random.h>
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

#define COMPRESSED_SIZE 22

static uint8_t battery = 255;

// === Structure du payload contenant les données FFT compressées ===
static payload_t _payload = {
    .header = 0x01,
    .node_id = 0x0001,
    .timestamp = 0x00000001,
    .battery_level = 0x20,
    .temperature = 0x14,
    .reduced_fft = {0}  // Initialisé à 0
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
        _payload.timestamp = esp_random();  // Génération d'un timestamp unique
        _payload.battery_level = (uint8_t)battery;

        // Envoyer les données compressées
        esp_err_t err = lora_send_packet((uint8_t*)(&_payload), sizeof(payload_t));

        if (err == ESP_OK) {
            // printf("LoRa: Payload Sent\n");
            screen_log("[%d]LoRa: send", _count);
        } else {
            printf("LoRa: Failed to Send Payload\n");
            screen_log("LoRa: FAILED");
        }

        // ToA = 0.7 secondes -> 10% duty cycle pour 433 MHz
        vTaskDelay(7000 / portTICK_PERIOD_MS);
        _count++;
        battery = battery==0 ? 255 : battery-1;
    }
}

void compute_fft_task() {
    esp_task_wdt_delete(NULL);

    int16_t raw_data_buffer[N_SAMPLES];
    float magnitude_data[N_SAMPLES / 2];
    uint8_t compressed_data[COMPRESSED_SIZE];

    // Initialisation du microphone et FFT
    inmp_init(GPIO_SCK, GPIO_SD, GPIO_WS, SAMPLE_RATE);
    fft_init(N_SAMPLES);

    while (true) {
        size_t bytes = inmp_read_raw_data(raw_data_buffer, N_SAMPLES * sizeof(int16_t));
        size_t num_samples = bytes / sizeof(int16_t);

        if (num_samples == N_SAMPLES) {
            // Calcul de la FFT et stockage du spectre de magnitude
            fft_process(raw_data_buffer, num_samples, magnitude_data);

            // Compression FFT (22 octets)
            compress_fft(magnitude_data, num_samples / 2, compressed_data, COMPRESSED_SIZE);

            send_compressed_fft_data(compressed_data, COMPRESSED_SIZE);

            // Mise à jour du payload LoRa avec la FFT compressée
            memcpy(_payload.reduced_fft, compressed_data, COMPRESSED_SIZE);
        } else {
            printf("Insufficient data for FFT: %zu samples\n", num_samples);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
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
            decrypted_data[decrypted_len] = '\0'; // Terminaison de chaîne
            printf("[Encryption test] Decrypted Message: %s\n", decrypted_data);
        } else {
            printf("[Encryption test] Decryption failed\n");
            break;
        }

        vTaskDelay(4200 / portTICK_PERIOD_MS);
    }
}
#endif
