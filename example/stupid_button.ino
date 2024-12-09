#include <Arduino.h>

int buttonState = 0;  // variable for reading the pushbutton status
int pin = 1;

void setup() {
  Serial.begin(9600);
  pinMode(pin, INPUT);
}

void loop() {
  buttonState = digitalRead(pin);

  if (buttonState == HIGH)
  {
    Serial.println("Bouton ON");
    delay(200); // Debounce
  }
}
