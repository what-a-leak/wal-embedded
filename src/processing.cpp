#include "processing.h"
#include <arduinoFFT.h>

void performFFT(double* data, uint16_t samples, double samplingFrequency) {
    double vReal[samples];
    double vImag[samples];
    for (uint16_t i = 0; i < samples; i++) {
        vReal[i] = data[i];
        vImag[i] = 0.0;
    }

    ArduinoFFT<double> FFT(vReal, vImag, samples, samplingFrequency);

    // Remove DC Offset
    double mean = 0;
    for (uint16_t i = 0; i < samples; i++) {
        mean += vReal[i];
    }
    mean /= samples;
    for (uint16_t i = 0; i < samples; i++) {
        vReal[i] -= mean;
    }

    // Apply FFT Windowing
    FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);

    // Compute the FFT
    FFT.compute(FFTDirection::Forward);

    // Compute Magnitudes
    FFT.complexToMagnitude();

    // Output Results
    Serial.println("Frequency (Hz) : Magnitude");
    for (uint16_t i = 0; i < samples / 2; i++) {
        double frequency = i * (samplingFrequency / samples);
        Serial.print(frequency, 2);
        Serial.print(" Hz: ");
        Serial.println(vReal[i], 4);
    }

    // Find Peak Frequency
    double peakFrequency = FFT.majorPeak();
    Serial.print("Peak Frequency: ");
    Serial.print(peakFrequency, 2);
    Serial.println(" Hz");
}