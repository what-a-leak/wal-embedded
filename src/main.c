#include <stdio.h>
#include <math.h>
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_dsp.h"
#include "../lib/peripherals/imu/lsm6ds3.h"

#define N_SAMPLES 1024 // Number of FFT samples
#define SAMPLING_RATE 100 // Sampling rate in Hz (e.g., 100 Hz -> 10 ms interval)
#define I2C_PORT I2C_NUM_0

// Buffers
float z_data[N_SAMPLES];
float fft_data[N_SAMPLES * 2]; // Complex array for FFT
float window[N_SAMPLES];

void process_fft(float *data, int length) {
    dsps_fft2r_fc32(data, length);
    dsps_bit_rev_fc32(data, length);
    dsps_cplx2reC_fc32(data, length);

    // Calculate magnitude in dB
    for (int i = 0; i < length / 2; i++) {
        data[i] = 10 * log10f((data[i * 2] * data[i * 2] + data[i * 2 + 1] * data[i * 2 + 1]) / length);
    }

    // Print FFT results as csv
    printf("FFT_START\n");
    for (int i = 0; i < length / 2; i++) {
        float freq = (float)i * SAMPLING_RATE / length;
        if (freq > 50) break; // Only print up to 50 Hz
        printf("%.2f,%.2f\n", freq, data[i]); // Frequency, Amplitude
    }
    printf("FFT_END\n");
}

void app_main() {
    // Initialize I2C and accelerometer
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = 8,
        .scl_io_num = 9,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    i2c_param_config(I2C_PORT, &conf);
    i2c_driver_install(I2C_PORT, I2C_MODE_MASTER, 0, 0, 0);

    init_lsm6ds3(I2C_PORT);

    // Initialize DSP FFT library
    dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE);

    // Create Hann window
    dsps_wind_hann_f32(window, N_SAMPLES);

    while (1) {
        // Collect Z-axis accelerometer data
        for (int i = 0; i < N_SAMPLES; i++) {
            read_acceleration_z(I2C_PORT, &z_data[i]);
            vTaskDelay((1000 / SAMPLING_RATE) / portTICK_PERIOD_MS); // Delay based on sampling rate
        }

        // Apply window function and prepare FFT input
        for (int i = 0; i < N_SAMPLES; i++) {
            fft_data[i * 2] = z_data[i] * window[i]; // Real part
            fft_data[i * 2 + 1] = 0;                // Imaginary part
        }

        // Process and analyze FFT
        process_fft(fft_data, N_SAMPLES);
    }
}
