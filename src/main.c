#include <stdio.h>
#include <math.h>
#include <complex.h>
#include "driver/i2c.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_dsp.h"
#include "../lib/peripherals/imu/lsm6ds3.h"
#include "../lib/peripherals/mic/inmp441.h"

#define SAMPLE_RATE 16000 // Sampling rate in Hz
#define FFT_SIZE 128     // FFT size (must be a power of 2)

void fft(double complex *X, int N) {
    if (N <= 1) return;

    // Split into even and odd indices
    double complex X_even[N/2];
    double complex X_odd[N/2];
    for (int i = 0; i < N / 2; i++) {
        X_even[i] = X[i * 2];
        X_odd[i] = X[i * 2 + 1];
    }

    // Recursive FFT on both halves
    fft(X_even, N / 2);
    fft(X_odd, N / 2);

    // Combine results
    for (int k = 0; k < N / 2; k++) {
        double complex t = cexp(-2.0 * I * M_PI * k / N) * X_odd[k];
        X[k] = X_even[k] + t;
        X[k + N / 2] = X_even[k] - t;
    }
}

void fft_task() {
    inmp_init(GPIO_SCK, GPIO_SD, GPIO_WS, SAMPLE_RATE);

    double samples[FFT_SIZE];
    double complex X[FFT_SIZE];

    while (1) {
        for (int i = 0; i < FFT_SIZE; i++) {
            samples[i] = inmp_read_sound_level();
        }

        for (int i = 0; i < FFT_SIZE; i++) {
            X[i] = samples[i] + 0.0 * I;
        }

        fft(X, FFT_SIZE);

        // Display amplitude graphically with '*' characters Frequency : ****
        // printf("------------------------------------------------------------------\n");
        // for (int k = 0; k < FFT_SIZE / 2; k++) {
        //     double frequency = (double)k * SAMPLE_RATE / FFT_SIZE;
        //     if (frequency >= 50 && frequency <= 2000) {
        //         double amplitude = cabs(X[k]);
        //         printf("%6.1f Hz       | ", frequency);
        //         for (int i = 0; i < amplitude; i += 100) {
        //             printf("*");
        //         }
        //         printf("\n");
        //     }
        // }
        // printf("------------------------------------------------------------------\n");    

        double max_amplitude = 0;
        double peak_frequency = 0;
        for (int i = 0; i < FFT_SIZE / 2; i++) {
            double frequency = (double)i * SAMPLE_RATE / FFT_SIZE;
            if (frequency >= 50 && frequency <= 2000) {
                double amplitude = cabs(X[i]);
                if (amplitude > max_amplitude) {
                    max_amplitude = amplitude;
                    peak_frequency = frequency;
                }
            }
        }
        printf("Peak frequency : %.02fHz \r\n", peak_frequency);
    }
}


void app_main() {
    xTaskCreate(fft_task, "fft_task", 100000, NULL, 5, NULL);
}
