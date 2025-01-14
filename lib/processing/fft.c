#include "fft.h"
#include <stdio.h>
#include <math.h>
#include "esp_dsp.h"

// Buffers for DSP (global, for simplicity, align to cache line)
__attribute__((aligned(16)))
static float input_data[N_SAMPLES];
__attribute__((aligned(16)))
static float wind[N_SAMPLES];
__attribute__((aligned(16)))
static float y_cf[N_SAMPLES * 2];
static float y1_cf[N_SAMPLES / 2];

void fft_init(size_t num_samples) {
    // Initialize DSP library and Hann window
    dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE);
    dsps_wind_hann_f32(wind, num_samples);
}

void fft_process(const int16_t *raw_data_buffer, size_t num_samples, float *output_data) {
    // Convert raw PCM data to float and apply the Hann window
    for (int i = 0; i < num_samples; i++) {
        input_data[i] = (float)raw_data_buffer[i] * wind[i];
    }

    // Fill complex array with input data (real and imaginary parts)
    for (int i = 0; i < num_samples; i++) {
        y_cf[i * 2 + 0] = input_data[i]; // Real part
        y_cf[i * 2 + 1] = 0; // Imaginary part
    }

    // Perform FFT
    dsps_fft2r_fc32(y_cf, num_samples);
    dsps_bit_rev_fc32(y_cf, num_samples);
    dsps_cplx2reC_fc32(y_cf, num_samples);

    // Compute magnitude (power spectrum) for the first half of the FFT
    for (int i = 0; i < num_samples / 2; i++) {
        output_data[i] = 10 * log10f((y_cf[i * 2 + 0] * y_cf[i * 2 + 0] + y_cf[i * 2 + 1] * y_cf[i * 2 + 1]) / num_samples);
    }
}

void send_fft_data(const float *fft_data, size_t num_samples) {
    for (int i = 0; i < num_samples; i++) {
        printf("%.2f,", fft_data[i]);
    }
    printf("\n");
}


#include <stdint.h>
#include <math.h>
#include <string.h>

void compress_fft(const float *fft_data, size_t num_samples, uint8_t *compressed_data, size_t compressed_size) {
    // Vérifiez que compressed_size est suffisant pour encoder les données
    if (compressed_size < 2) {
        return; // Trop petit pour stocker quoi que ce soit
    }

    // Calcul des valeurs minimales et maximales pour normaliser
    float min_val = FLT_MAX;
    float max_val = -FLT_MAX;
    for (size_t i = 0; i < num_samples; i++) {
        if (fft_data[i] < min_val) min_val = fft_data[i];
        if (fft_data[i] > max_val) max_val = fft_data[i];
    }

    // Encode l'échelle (min et max) dans les deux premiers octets
    compressed_data[0] = (uint8_t)((min_val + 128) * 2); // Assure la conversion en plage [0-255]
    compressed_data[1] = (uint8_t)((max_val + 128) * 2);

    // Compression des données FFT restantes
    for (size_t i = 0; i < compressed_size - 2; i++) {
        size_t index = i * num_samples / (compressed_size - 2); // Rééchantillonnage
        float normalized_value = (fft_data[index] - min_val) / (max_val - min_val); // Normalisation [0-1]
        compressed_data[i + 2] = (uint8_t)(normalized_value * 255); // Compression dans [0-255]
    }
}


void send_compressed_fft_data(const uint8_t *compressed_data, size_t compressed_size) {
    // Convertir les données compressées en une chaîne de caractères hexadécimaux
    char buffer[compressed_size * 2 + 2];
    for (size_t i = 0; i < compressed_size; i++) {
        snprintf(&buffer[i * 2], 3, "%02X", compressed_data[i]);
    }

    // Ajouter une fin de ligne pour délimiter les messages
    strcat(buffer, "\n");

    // Envoyer les données sur la sortie série (exemple)
    printf("%s", buffer);
}