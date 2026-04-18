#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "esp.h"
#include "readValues.h"

const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* serverIP = "192.168.1.XXX"; // IP of the machine running Flask

/**RUN THROUGH OF EACH FUNCTION:

getSnowProbability() -> gets the snowprobability from flask on a separate database

espSetup() -> CALL SETUP FIRST, sets up everything first

espLoop() -> Main loop logic from esp.c

**/

float getSnowProbability(float tempC, float humidity) {
  if (WiFi.status() != WL_CONNECTED) return -1.0;

  HTTPClient http;
  String url = String("http://") + serverIP + ":5000/predict"
             + "?temp=" + String(tempC, 2)
             + "&humidity=" + String(humidity, 2);

  http.begin(url);
  int httpCode = http.GET();
  float result = -1.0;

  if (httpCode == 200) {
    String payload = http.getString();
    StaticJsonDocument<128> doc;
    deserializeJson(doc, payload);
    result = doc["snow_probability"];
  }

  http.end();
  return result;
}

float getSnowProbabilityFromSensors() {
  float temp = readTemp();
  float humidity = readHumidity();
  return getSnowProbability(temp, humidity);
}

void roundCheck() {
  float prob = getSnowProbabilityFromSensors();
  if (prob >= 0) {
    Serial.printf("Snow probability: %.1f%%\n", prob * 100);
    if (prob > 0.6f) {
      Serial.println("High snow chance — sprayer active.");
    }
  } else {
    Serial.println("API call failed.");
  }

  float ax, ay, az;
  readAccelerometer(&ax, &ay, &az);
  Serial.printf("Accelerometer: X=%.2f, Y=%.2f, Z=%.2f g\n", ax, ay, az);
}

void espSetup() {
  Serial.begin(115200);
  initSensors();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected: " + WiFi.localIP().toString());
}

void espLoop() {
  while (true) {
    roundCheck();
    delay(86400000); // wait 24 hours before checking again
  }
}


