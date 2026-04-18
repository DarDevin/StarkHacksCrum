#include "readValues.h"
#include <DHT.h>
#include <Wire.h>
#include <MPU6050.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

// DHT Sensor
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Solenoid
#define SOLENOIDPIN 8

// Motor (L298N)
// MOTORPIN is the PWM speed pin; also define direction pins
#define MOTORPIN     9   // ENA (PWM)
#define MOTOR_IN1   10
#define MOTOR_IN2   11

// GPS (NEO-6M) on UART2
#define GPS_RX_PIN  16
#define GPS_TX_PIN  17
#define GPS_BAUD    9600

TinyGPSPlus gps;
HardwareSerial gpsSerial(2); // UART2

// MPU6050
MPU6050 mpu;

// ─── Init ────────────────────────────────────────────────────────────────────

void initSensors() {
  // DHT
  dht.begin();

  // MPU6050 via I2C
  Wire.begin(21, 22); // SDA=21, SCL=22
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed");
  } else {
    Serial.println("MPU6050 connected");
  }

  // Solenoid pin
  pinMode(SOLENOIDPIN, OUTPUT);
  digitalWrite(SOLENOIDPIN, LOW);

  // Motor pins
  pinMode(MOTORPIN, OUTPUT);
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);

  // GPS serial
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.println("GPS serial started");
}

// ─── DHT ─────────────────────────────────────────────────────────────────────

float readTemp() {
  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return 0.0f;
  }
  return t;
}

float readHumidity() {
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return 0.0f;
  }
  return h;
}

// ─── MPU6050 ─────────────────────────────────────────────────────────────────

void readAccelerometer(float* ax, float* ay, float* az) {
  int16_t ax_raw, ay_raw, az_raw;
  mpu.getAcceleration(&ax_raw, &ay_raw, &az_raw);
  *ax = ax_raw / 16384.0f; // Convert to g (2g range)
  *ay = ay_raw / 16384.0f;
  *az = az_raw / 16384.0f;
}

// ─── Solenoid ────────────────────────────────────────────────────────────────

// Turns the solenoid on for `durationMs` milliseconds, then off.
void moveSolenoid(unsigned long durationMs) {
  digitalWrite(SOLENOIDPIN, HIGH);
  delay(durationMs);
  digitalWrite(SOLENOIDPIN, LOW);
}

// ─── Motor (L298N) ───────────────────────────────────────────────────────────

// speed: 0–255 PWM value
// forward: true = forward, false = reverse
void moveMotor(uint8_t speed, bool forward) {
  if (forward) {
    digitalWrite(MOTOR_IN1, HIGH);
    digitalWrite(MOTOR_IN2, LOW);
  } else {
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, HIGH);
  }
  analogWrite(MOTORPIN, speed);
}

void stopMotor() {
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  analogWrite(MOTORPIN, 0);
}

// ─── GPS ─────────────────────────────────────────────────────────────────────

// Feeds GPS serial data for up to `timeoutMs` ms, then returns the latest fix.
// Returns true if a valid location was obtained, false otherwise.
// latitude and longitude are written to the provided pointers.
bool readLatLong(float* latitude, float* longitude, unsigned long timeoutMs) {
  unsigned long start = millis();
  while (millis() - start < timeoutMs) {
    while (gpsSerial.available() > 0) {
      gps.encode(gpsSerial.read());
    }
    if (gps.location.isUpdated()) {
      *latitude  = (float)gps.location.lat();
      *longitude = (float)gps.location.lng();
      return true;
    }
  }
  Serial.println("GPS: no fix within timeout");
  return false;
}