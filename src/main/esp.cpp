#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "esp.h"
#include "readValues.h"
#include "push_telemetry.h"

const char* ssid     = "StarkHacks5";
const char* password = "StarkHacks2026";
<<<<<<< Updated upstream
const char* backendServerIP = "10.10.8.55"; // Your PC's IP address
const int backendServerPort = 8000;            // FastAPI server port
const char* backendTelemetryPath = "/telemetry"; // FastAPI telemetry endpoint
=======
const char* backendServerIP = "10.10.8.55"; // Replace with your FastAPI server LAN IP
const int backendServerPort = 8000;            // Replace if your FastAPI server runs on a different port
const char* backendTelemetryPath = "/telemetry"; // FastAPI telemetry endpoint
const char* snowServerIP = "10.10.8.55"; // Replace with your snow prediction API server IP
const int snowServerPort = 5000;               // Replace if the snow prediction API runs on another port
>>>>>>> Stashed changes

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
  Serial.println("Waiting for turn to complete...");
  delay(2000);
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
<<<<<<< Updated upstream
=======
}
>>>>>>> Stashed changes

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


