#include "sensors.h"

const int micPin = 0; // Electret Microphone connected to GPIO0

void initializeSensors() {
    pinMode(micPin, INPUT);
}

double readMicrophone() {
    return analogRead(micPin);
}