#include <Arduino.h>

// Relay module control pin for the solenoid
#define RELAY_PIN 10

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);

  // Start with relay off
  digitalWrite(RELAY_PIN, LOW);
  Serial.println("Relay test initialized. Pin 10 is set to LOW.");
}

void loop() {
  // Turn the relay on for 2 seconds
  digitalWrite(RELAY_PIN, HIGH);
  Serial.println("Relay ON");
  delay(2000);

  // Turn the relay off for 2 seconds
  digitalWrite(RELAY_PIN, LOW);
  Serial.println("Relay OFF");
  delay(2000);
}
