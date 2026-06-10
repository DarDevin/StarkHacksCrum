#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

WiFiMulti wifiMulti;

// --- CONFIG ---
const char* SSID     = "StarkHacks-2";
const char* PASSWORD = "StarkHacks2026";
const char* SERVER_IP = "10.10.8.55";  // your Flask server IP
const int   SERVER_PORT = 5000;

// Dummy sensor values — replace with real sensor reads
float getTemperature() { return -2.5; }
float getHumidity()    { return 88.0; }

void setup() {
  Serial.begin(115200);
  delay(1000);

  wifiMulti.addAP(SSID, PASSWORD);

  Serial.print("Connecting to WiFi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
}

void loop() {
  if (wifiMulti.run() == WL_CONNECTED) {
    HTTPClient http;

    float temp     = getTemperature();
    float humidity = getHumidity();

    // Build URL with query params
    String url = String("http://") + SERVER_IP + ":" + SERVER_PORT
               + "/predict?temp=" + String(temp, 2)
               + "&humidity="     + String(humidity, 2);

    Serial.println("Requesting: " + url);
    http.begin(url);

    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println("Response: " + payload);

      // Optional: parse snow_probability out of the JSON manually
      // payload looks like: {"snow_probability":0.8732,"inputs":{...}}
      int idx = payload.indexOf("snow_probability\":");
      if (idx != -1) {
        float prob = payload.substring(idx + 18).toFloat();
        Serial.printf("Snow probability: %.2f%%\n", prob * 100);
      }

    } else {
      Serial.printf("HTTP error: %d — %s\n", httpCode, http.errorToString(httpCode).c_str());
    }

    http.end();
  } else {
    Serial.println("WiFi disconnected, reconnecting...");
  }

  delay(60000); // poll every 60 seconds
}