#ifndef FFT_H
#define FFT_H

#include <stddef.h>
#include <stdint.h>

#define N_SAMPLES 1024

// Initialize FFT library with configuration
void fft_init(size_t num_samples);

// Process FFT on a raw data buffer
void fft_process(const int16_t *raw_data_buffer, size_t num_samples);

// Send FFT data to output for visualization
void send_fft_data(const float *fft_data, size_t num_samples);

#endif // FFT_H
