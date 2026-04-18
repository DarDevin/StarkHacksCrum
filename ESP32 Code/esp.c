#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "readValues.h"

const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* serverIP = "192.168.1.XXX"; // IP of the machine running Flask

/**RUN THROUGH OF EACH FUNCTION:

getSnowProbability() -> gets the snowprobability from flask on a separate database

setup() -> CALL SETUP FIRST, sets up everything first

loop() -> General loop, calls getSnowProbability every 10 minutes, and prints the result to the serial monitor. In a real implementation, you would replace the hardcoded temp and humidity with actual sensor readings.

**/


// --- Main function you call anywhere in your code ---
float getSnowProbability(float tempC, float humidity) {
  if (WiFi.status() != WL_CONNECTED) return -1.0;

  HTTPClient http;
  String url = String("http://") + serverIP + ":5000/predict"
             + "?temp=" + String(tempC, 2)
             + "&humidity=" + String(humidity, 2);

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    StaticJsonDocument<128> doc;
    deserializeJson(doc, payload);
    float prob = doc["snow_probability"];
    http.end();
    return prob;  // 0.0 to 1.0
  }

  http.end();
  return -1.0; // error
}

float getSnowProbabilityFromSensors() {
  float temp = readTemp();
  float humidity = readHumidity();
  return getSnowProbability(temp, humidity);
}

// --- Setup ---
void setup() {
  Serial.begin(115200);
  initSensors();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected: " + WiFi.localIP().toString());
}

// --- Loop: call every 10 minutes ---
void loop() {
  roundCheck();
  delay(600000); // wait 10 minutes
}

void roundCheck() {
  float prob = getSnowProbabilityFromSensors();
  while true{
    if (prob >= 0) {
      Serial.printf("Snow probability: %.1f%%\n", prob * 100);
      if (prob > 0.6f) {
        Serial.println("High snow chance — sprayer active.");
        




        
      }
    } else {
      Serial.println("API call failed.");
    }

    delay(86400000); // wait 24 hours before checking again
  }
  // Read accelerometer
  float ax, ay, az;
  readAccelerometer(&ax, &ay, &az);
  Serial.printf("Accelerometer: X=%.2f, Y=%.2f, Z=%.2f g\n", ax, ay, az);
}


