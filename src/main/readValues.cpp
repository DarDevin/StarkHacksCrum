#include "readValues.h"
#include <DHT.h>
#include <Wire.h>
#include <MPU6050.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <math.h>

// DHT Sensor
#define DHTPIN 5
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Solenoid
#define SOLENOIDPIN 10

// Motor power control
// Pin 11 now drives the buck converter PWM input for both motors.
#define MOTOR_PWM_PIN 11

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

  // Motor pin for buck converter PWM control
  pinMode(MOTOR_PWM_PIN, OUTPUT);
  analogWrite(MOTOR_PWM_PIN, 0);

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
  Serial.println(t);
  return t;
}

float readHumidity() {
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return 0.0f;
  }
  Serial.println(h);
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

bool isRobotMoving(float threshold) {
  float ax, ay, az;
  readAccelerometer(&ax, &ay, &az);
  float magnitude = sqrt(ax*ax + ay*ay + az*az);
  // Assuming at rest, magnitude is around 1g (gravity)
  // If moving, it will deviate
  return fabs(magnitude - 1.0f) > threshold;
}

// ─── Solenoid ────────────────────────────────────────────────────────────────

// Sprays the solenoid for `durationMs` milliseconds at `intensity` (0-255 PWM).
void spraySolenoid(unsigned long durationMs, uint8_t intensity) {
  analogWrite(SOLENOIDPIN, intensity);
  delay(durationMs);
  analogWrite(SOLENOIDPIN, 0);
}

// ─── Motor control via buck converter PWM ───────────────────────────────────

// speed: 0–255 PWM value
void turnOnMotor1(uint8_t speed) {
  analogWrite(MOTOR_PWM_PIN, speed);
}

void turnOffMotor1() {
  analogWrite(MOTOR_PWM_PIN, 0);
}

void turnOnMotor2(uint8_t speed) {
  // Both motors share the same buck converter control pin.
  analogWrite(MOTOR_PWM_PIN, speed);
}

void turnOffMotor2() {
  analogWrite(MOTOR_PWM_PIN, 0);
}

// ─── Combined Motors ────────────────────────────────────────────────────────

void turnOnMotors(uint8_t speed) {
  turnOnMotor1(speed);
  turnOnMotor2(speed);
}

void turnOffMotors() {
  turnOffMotor1();
  turnOffMotor2();
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
  Serial.printf("GPS: %.6f, %.6f\n", *latitude, *longitude);
  return false;
}