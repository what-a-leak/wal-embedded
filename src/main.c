#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_dsp.h"
#include "../lib/peripherals/mic/inmp441.h"

// Constants
#define SAMPLE_RATE 44100
#define N_SAMPLES 1024

// Buffers for DSP
__attribute__((aligned(16)))
float input_data[N_SAMPLES];
__attribute__((aligned(16)))
float wind[N_SAMPLES];
__attribute__((aligned(16)))
float y_cf[N_SAMPLES * 2];
float *y1_cf = &y_cf[0];

void process_fft(const int16_t *raw_data_buffer, size_t num_samples) {
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
    float max_magnitude = -INFINITY;
    int max_index = 0;
    for (int i = 0; i < num_samples / 2; i++) {
        y1_cf[i] = 10 * log10f((y_cf[i * 2 + 0] * y_cf[i * 2 + 0] + y_cf[i * 2 + 1] * y_cf[i * 2 + 1]) / num_samples);
        if (y1_cf[i] > max_magnitude) {
            max_magnitude = y1_cf[i];
            max_index = i;
        }
    }

    // Calculate the frequency of the peak
    float frequency_resolution = (float)SAMPLE_RATE / num_samples;
    float peak_frequency = max_index * frequency_resolution;

    // Display spectrum
    printf("FFT Power Spectrum:\n");
    // dsps_view(y1_cf, num_samples / 2, 64, 10, -60, 40, '|');

    // Print the peak frequency
    printf("Peak Frequency: %.2f Hz (Magnitude: %.2f dB)\n", peak_frequency, max_magnitude);
}

void simple_task() {
    int16_t raw_data_buffer[I2S_READ_LEN];

    // Initialize microphone and FFT
    inmp_init(GPIO_SCK, GPIO_SD, GPIO_WS, SAMPLE_RATE);
    dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE);
    dsps_wind_hann_f32(wind, N_SAMPLES);

    while (true) {
        size_t bytes = inmp_read_raw_data(raw_data_buffer, I2S_READ_LEN); // Read PCM data
        size_t num_samples = bytes / sizeof(int16_t);
        
        if (num_samples == N_SAMPLES) {
            process_fft(raw_data_buffer, num_samples); // Perform FFT on the data
        } else {
            printf("Insufficient data for FFT: %zu samples\n", num_samples);
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay to prevent continuous output
    }
}

void app_main() {
    xTaskCreate(simple_task, "simple_task", 12000, NULL, 5, NULL);
}
