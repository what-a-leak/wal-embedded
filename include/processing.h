#ifndef PROCESSING_H
#define PROCESSING_H

#include <Arduino.h>

void performFFT(double* data, uint16_t samples, double samplingFrequency);

#endif // PROCESSING_H