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
#include <stdio.h>
#include <string.h>

#define COMPRESSED_SIZE 22

void compress_fft(const float *fft_data, size_t num_samples, uint8_t *compressed_data, size_t compressed_size) {
    if (compressed_size != COMPRESSED_SIZE) {
        return;  // Assurez-vous que la taille est correcte
    }

    // Trouver la valeur maximale pour la normalisation
    float max_val = -INFINITY;
    for (size_t i = 0; i < num_samples / 2; i++) {
        if (fft_data[i] > max_val) {
            max_val = fft_data[i];
        }
    }
    
    // Éviter la division par zéro
    if (max_val < 1e-6) {
        max_val = 1e-6;
    }

    // Compression avec quantification logarithmique sur 8 bits
    for (size_t i = 0; i < COMPRESSED_SIZE; i++) {
        size_t index = (i * (num_samples / 2)) / COMPRESSED_SIZE;  // Sélectionner des points représentatifs
        float norm_value = fft_data[index] / max_val;  // Normaliser entre 0 et 1
        norm_value = log1p(norm_value) / log1p(1.0);   // Appliquer une échelle logarithmique
        compressed_data[i] = (uint8_t)(norm_value * 255.0);  // Quantifier sur 8 bits
    }
}

void send_compressed_fft_data(const uint8_t *compressed_data, size_t compressed_size) {
    for (size_t i = 0; i < compressed_size; i++) {
        printf("%u,", compressed_data[i]);
    }
    printf("\n");
}
