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
static float *y1_cf = &y_cf[0];

void fft_init(size_t num_samples) {
    // Initialize DSP library and Hann window
    dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE);
    dsps_wind_hann_f32(wind, num_samples);
}

void fft_process(const int16_t *raw_data_buffer, size_t num_samples) {
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

    // Compute magnitude (power spectrum)
    for (int i = 0; i < num_samples / 2; i++) {
        y1_cf[i] = 10 * log10f((y_cf[i * 2 + 0] * y_cf[i * 2 + 0] + y_cf[i * 2 + 1] * y_cf[i * 2 + 1]) / num_samples);
    }

    // Find the frequency with the maximum power and print it
    float max_power = y1_cf[0];
    int max_power_index = 0;
    for (int i = 1; i < num_samples / 2; i++) {
        if (y1_cf[i] > max_power) {
            max_power = y1_cf[i];
            max_power_index = i;
        }
    }
    printf("Max power at frequency: %d Hz\n", max_power_index * 88200 / num_samples);



    // Send FFT data
    // send_fft_data(y1_cf, num_samples / 2);
    

}

void send_fft_data(const float *fft_data, size_t num_samples) {
    for (int i = 0; i < num_samples; i++) {
        printf("%.2f,", fft_data[i]);
    }
    printf("\n");
}