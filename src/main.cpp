#include <Arduino.h>
#include "sensors.h"
#include "processing.h"

const uint16_t samples = 128;             // Power of 2
const double samplingFrequency = 1000.0;  // Sampling frequency in Hz
double data[samples];

void setup() {
    Serial.begin(115200);
    initializeSensors();
    Serial.println("Starting FFT processing...");
}

void loop() {
    // Sample the microphone signal
    for (uint16_t i = 0; i < samples; i++) {
        unsigned long startMicros = micros();

        data[i] = readMicrophone();

        // Wait for the next sample
        while (micros() - startMicros < (1000000 / samplingFrequency));
    }

    // Perform FFT on the sampled data
    performFFT(data, samples, samplingFrequency);

    delay(500); // Repeat every 500ms
}