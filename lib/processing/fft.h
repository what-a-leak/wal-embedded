#ifndef FFT_H
#define FFT_H

#include <stddef.h>
#include <stdint.h>
#include <float.h>

#define N_SAMPLES 1024

// Initialize FFT library with configuration
void fft_init(size_t num_samples);

// Process FFT on a raw data buffer
void fft_process(const int16_t *raw_data_buffer, size_t num_samples, float *output_data);

// Compress function to fit the magnitude data into a specified number of bytes
void compress_fft(const float *fft_data, size_t num_samples, uint8_t *compressed_data, size_t compressed_size);

// Send FFT data to serial for visualization
void send_fft_data(const float *fft_data, size_t num_samples);

// Send compressed FFT data to serial for visualization
void send_compressed_fft_data(const uint8_t *compressed_data, size_t compressed_size);

#endif // FFT_H
