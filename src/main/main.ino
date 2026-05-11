#include <Arduino.h>
#include "esp.h"

// Forward declare functions from esp.cpp
extern void espSetup();
extern void espLoop();

void setup() {
  espSetup();
}

void loop() {
  espLoop();
}

