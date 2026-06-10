#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "esp.h"
#include "readValues.h"
#include "push_telemetry.h"

const char* ssid     = "StarkHacks5";
const char* password = "StarkHacks2026";
const char* backendServerIP = "10.10.8.55";
const int backendServerPort = 8000;
const char* backendTelemetryPath = "/telemetry";

String getTelemetryUrl() {
  return String("http://") + backendServerIP + ":" + String(backendServerPort) + backendTelemetryPath;
}

TelemetryResponse sendTelemetry(
    float latitude,
    float longitude,
    bool isSalting,
    bool isFirstPing
) {
  // Wait for the robot to finish its turn before sampling telemetry
  Serial.println("Waiting for turn to complete...");
  delay(2000);

  // Wait until the robot has fully stopped before sending
  while (isRobotMoving(0.1f)) {
    Serial.println("Robot still moving, waiting...");
    delay(100);
  }

  String url = getTelemetryUrl();
  Serial.println("Telemetry URL: " + url);
  TelemetryResponse response = pushTelemetryToFastAPI(
      url.c_str(),
      latitude,
      longitude,
      isSalting,
      isFirstPing
  );

  // Use the backend's salting decision
  if (response.success) {
    if (response.startSalting) {
      Serial.println("Activating solenoid based on backend decision");
      spraySolenoid(2000, 255);
    } else {
      Serial.println("Backend says don't salt - solenoid remains off");
    }
  } else {
    Serial.println("Failed to get backend decision - keeping current salting state");
  }

  return response;
}

void espSetup() {
  Serial.begin(115200);
  initSensors();

  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());

  // Send first ping to register with the server
  float lat = 0.0f, lng = 0.0f;
  readLatLong(&lat, &lng, 2000);
  sendTelemetry(lat, lng, false, true);
}

void espLoop() {
  static bool isSalting = false;

  // Only send telemetry when the robot is stopped
  if (isRobotMoving(0.1f)) {
    return;
  }

  float lat = 0.0f, lng = 0.0f;
  bool gotFix = readLatLong(&lat, &lng, 2000);
  if (!gotFix) {
    Serial.println("No GPS fix, skipping telemetry");
    return;
  }

  TelemetryResponse response = sendTelemetry(lat, lng, isSalting, false);
  if (response.success) {
    isSalting = response.startSalting;
  }

  delay(500);
}