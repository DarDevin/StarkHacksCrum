#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* serverIP = "192.168.1.XXX"; // IP of the machine running Flask

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

// --- Setup ---
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected: " + WiFi.localIP().toString());
}

// --- Loop: call every 10 minutes ---
void loop() {
  float temp = 2.0;      // Replace with real sensor reading (e.g. DHT22)
  float humidity = 85.0; // Replace with real sensor reading

  float prob = getSnowProbability(temp, humidity);

  if (prob >= 0) {
    Serial.printf("Snow probability: %.1f%%\n", prob * 100);
    if (prob > 0.6) {
      Serial.println("High snow chance — shutting off sprayer.");
      // digitalWrite(SOLENOID_PIN, LOW);
    }
  } else {
    Serial.println("API call failed.");
  }

  delay(600000); // wait 10 minutes
}