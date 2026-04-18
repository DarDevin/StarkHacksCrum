#include "esp.h"
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <DHT.h>
#include <Wire.h>
#include <MPU6050.h>
#include <math.h>

// ── CONFIG ────────────────────────────────────────────────────────
const char* SSID       = "StarkHacks-5";
const char* PASSWORD   = "StarkHacks2026";
const char* SERVER_URL = "http://10.10.8.42:8000/telemetry";

#define SALT_PIN      10   // solenoid
#define MOTOR_PWM_PIN 11
#define DHTPIN        5
#define DHTTYPE       DHT22
#define GPS_RX_PIN    16
#define GPS_TX_PIN    17

// ── GLOBALS ───────────────────────────────────────────────────────
DHT dht(DHTPIN, DHTTYPE);
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);
MPU6050 mpu;

bool isFirstPing = true;
bool isSalting   = false;

// ── WIFI ──────────────────────────────────────────────────────────
void connectWiFi() {
    Serial.print("Connecting to WiFi");
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected: " + WiFi.localIP().toString());
}

// ── SALTER ────────────────────────────────────────────────────────
void startSalting() {
    analogWrite(SALT_PIN, 200);
}

void stopSalting() {
    analogWrite(SALT_PIN, 0);
}

// ── MOTORS ────────────────────────────────────────────────────────
void turnOnMotors(uint8_t speed) {
    analogWrite(MOTOR_PWM_PIN, speed);
}

void turnOffMotors() {
    analogWrite(MOTOR_PWM_PIN, 0);
}

// ── SENSORS ───────────────────────────────────────────────────────
float readTemp() {
    return dht.readTemperature();
}

bool isRobotMoving(float threshold) {
    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);
    float mag = sqrt(pow(ax/16384.0f,2) + pow(ay/16384.0f,2) + pow(az/16384.0f,2));
    return fabs(mag - 1.0f) > threshold;
}

bool readLatLong(float* lat, float* lng, unsigned long timeoutMs) {
    unsigned long start = millis();
    while (millis() - start < timeoutMs) {
        while (gpsSerial.available()) gps.encode(gpsSerial.read());
        if (gps.location.isUpdated()) {
            *lat = gps.location.lat();
            *lng = gps.location.lng();
            return true;
        }
    }
    return false;
}

// ── SERVER ────────────────────────────────────────────────────────
bool sendTelemetry(float lat, float lng, bool salting, bool firstPing) {
    if (WiFi.status() != WL_CONNECTED) connectWiFi();

    HTTPClient http;
    http.begin(SERVER_URL);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<128> doc;
    doc["latitude"]      = lat;
    doc["longitude"]     = lng;
    doc["is_salting"]    = salting;
    doc["is_first_ping"] = firstPing;

    String body;
    serializeJson(doc, body);
    Serial.println("Sending: " + body);

    int responseCode = http.POST(body);
    if (responseCode == 200) {
        String response = http.getString();
        Serial.println("Response: " + response);

        StaticJsonDocument<256> resDoc;
        DeserializationError err = deserializeJson(resDoc, response);
        if (err) {
            http.end();
            return salting;
        }

        bool result = resDoc["decision"]["start_salting"];
        http.end();
        return result;
    }

    Serial.println("HTTP error: " + String(responseCode));
    http.end();
    return salting;
}

// ── SETUP / LOOP ──────────────────────────────────────────────────
void espSetup() {
    Serial.begin(115200);
    dht.begin();
    Wire.begin(21, 22);
    mpu.initialize();
    gpsSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    pinMode(SALT_PIN, OUTPUT);
    pinMode(MOTOR_PWM_PIN, OUTPUT);
    analogWrite(SALT_PIN, 0);
    analogWrite(MOTOR_PWM_PIN, 0);
    connectWiFi();
    turnOnMotors(180);
}

void espLoop() {
    float lat = 0, lng = 0;
    if (!readLatLong(&lat, &lng, 2000)) {
        Serial.println("Waiting for GPS fix...");
        return;
    }

    isSalting = sendTelemetry(lat, lng, isSalting, isFirstPing);
    isFirstPing = false;

    if (isSalting) startSalting();
    else           stopSalting();

    delay(2000);
}
