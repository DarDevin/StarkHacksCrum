#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "esp.h"
#include "readValues.h"
#include "push_telemetry.h"

const char* ssid     = "StarkHacks5";
const char* password = "StarkHacks2026";
const char* backendServerIP = "192.168.1.XXX"; // Replace with your FastAPI server LAN IP
const int backendServerPort = 8000;            // Replace if your FastAPI server runs on a different port
const char* backendTelemetryPath = "/telemetry"; // FastAPI telemetry endpoint
const char* snowServerIP = "192.168.1.XXX"; // Replace with your snow prediction API server IP
const int snowServerPort = 5000;               // Replace if the snow prediction API runs on another port

String getTelemetryUrl() {
  return String("http://") + backendServerIP + ":" + String(backendServerPort) + backendTelemetryPath;
}

TelemetryResponse sendTelemetry(
    float latitude,
    float longitude,
    float angleDelta,
    bool isSalting,
    bool isFirstPing
) {
  String url = getTelemetryUrl();
  Serial.println("Telemetry URL: " + url);
  TelemetryResponse response = pushTelemetryToFastAPI(
      url.c_str(),
      latitude,
      longitude,
      angleDelta,
      isSalting,
      isFirstPing
  );

  // Use the backend's salting decision
  if (response.success) {
    if (response.startSalting) {
      Serial.println("Activating solenoid based on backend decision");
      spraySolenoid(2000, 255); // Spray for 2 seconds at full intensity
    } else {
      Serial.println("Backend says don't salt - solenoid remains off");
    }
  } else {
    Serial.println("Failed to get backend decision - keeping current salting state");
  }

  return response;
}

/**RUN THROUGH OF EACH FUNCTION:

getSnowProbability() -> gets the snowprobability from flask on a separate database

espSetup() -> CALL SETUP FIRST, sets up everything first

espLoop() -> Main loop logic from esp.c

**/

float getSnowProbability(float tempC, float humidity) {
  if (WiFi.status() != WL_CONNECTED) return -1.0;

  HTTPClient http;
  String url = String("http://") + snowServerIP + ":" + String(snowServerPort) + "/predict"
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


