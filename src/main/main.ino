#include <Arduino.h>
#include "esp.h"

#define DHTPIN 6
#define RELAYPIN 18
#define BUCKPIN 9
#define GPSPIN TX RX


// Forward declare functions from esp.cpp
extern void espSetup();
extern void espLoop();

void setup() {
  espSetup();
}

void loop() {
  espLoop();
}

void test() {

}
