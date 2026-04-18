#include <Arduino.h>
#include <WiFi.h>

const char* ap_ssid     = "ESP32-TestAP";
const char* ap_password = "Test1234";

void onClientConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.print("Client connected: ");
  for (int i = 0; i < 6; i++) {
    if (i) Serial.print(":");
    Serial.printf("%02X", info.sta_connected.mac[i]);
  }
  Serial.println();
  Serial.print("Current connected clients: ");
  Serial.println(WiFi.softAPgetStationNum());
}

void onClientDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.print("Client disconnected: ");
  for (int i = 0; i < 6; i++) {
    if (i) Serial.print(":");
    Serial.printf("%02X", info.sta_disconnected.mac[i]);
  }
  Serial.println();
  Serial.print("Current connected clients: ");
  Serial.println(WiFi.softAPgetStationNum());
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nStarting ESP32 WiFi AP test...");

  WiFi.mode(WIFI_MODE_AP);
  WiFi.onEvent(onClientConnected, ARDUINO_EVENT_WIFI_AP_STACONNECTED);
  WiFi.onEvent(onClientDisconnected, ARDUINO_EVENT_WIFI_AP_STADISCONNECTED);

  bool success = WiFi.softAP(ap_ssid, ap_password);
  IPAddress ip = WiFi.softAPIP();

  if (success) {
    Serial.println("Soft AP started successfully.");
  } else {
    Serial.println("Failed to start Soft AP.");
  }

  Serial.print("SSID: ");
  Serial.println(ap_ssid);
  Serial.print("Password: ");
  Serial.println(ap_password);
  Serial.print("AP IP address: ");
  Serial.println(ip);
  Serial.println("Connect your laptop to this WiFi network and watch the serial monitor.");
}

void loop() {
  static unsigned long lastReport = 0;
  unsigned long now = millis();

  if (now - lastReport >= 5000) {
    lastReport = now;
    Serial.print("Connected stations: ");
    Serial.println(WiFi.softAPgetStationNum());
  }

  delay(100);
}

