#include "readValues.h"
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <math.h>

// DHT Sensor
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Solenoid
#define SOLENOIDPIN 8

// Motor 1 (L298N)
#define MOTOR1_PIN     9   // ENA (PWM)
#define MOTOR1_IN1    10
#define MOTOR1_IN2    11

// Motor 2 (L298N)
#define MOTOR2_PIN     12  // ENB (PWM)
#define MOTOR2_IN1    13
#define MOTOR2_IN2    14

// GPS (NEO-6M) on UART2
#define GPS_RX_PIN  16
#define GPS_TX_PIN  17
#define GPS_BAUD    9600

TinyGPSPlus gps;
HardwareSerial gpsSerial(2); // UART2

// MPU6050
Adafruit_MPU6050 mpu;

// ─── Init ────────────────────────────────────────────────────────────────────

void initSensors() {
  // DHT
  dht.begin();

  // MPU6050 via I2C
  Wire.begin(21, 22); // SDA=21, SCL=22
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
  } else {
    Serial.println("MPU6050 connected");
  }

  // Solenoid pin
  pinMode(SOLENOIDPIN, OUTPUT);
  digitalWrite(SOLENOIDPIN, LOW);

  // Motor pins
  pinMode(MOTOR1_PIN, OUTPUT);
  pinMode(MOTOR1_IN1, OUTPUT);
  pinMode(MOTOR1_IN2, OUTPUT);
  digitalWrite(MOTOR1_IN1, LOW);
  digitalWrite(MOTOR1_IN2, LOW);

  pinMode(MOTOR2_PIN, OUTPUT);
  pinMode(MOTOR2_IN1, OUTPUT);
  pinMode(MOTOR2_IN2, OUTPUT);
  digitalWrite(MOTOR2_IN1, LOW);
  digitalWrite(MOTOR2_IN2, LOW);

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
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  *ax = a.acceleration.x;
  *ay = a.acceleration.y;
  *az = a.acceleration.z;
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

// ─── Motor 1 (L298N) ─────────────────────────────────────────────────────────

// speed: 0–255 PWM value
void turnOnMotor1(uint8_t speed) {
  digitalWrite(MOTOR1_IN1, HIGH);
  digitalWrite(MOTOR1_IN2, LOW);  // Forward direction
  analogWrite(MOTOR1_PIN, speed);
}

void turnOffMotor1() {
  digitalWrite(MOTOR1_IN1, LOW);
  digitalWrite(MOTOR1_IN2, LOW);
  analogWrite(MOTOR1_PIN, 0);
}

// ─── Motor 2 (L298N) ─────────────────────────────────────────────────────────

// speed: 0–255 PWM value
void turnOnMotor2(uint8_t speed) {
  digitalWrite(MOTOR2_IN1, HIGH);
  digitalWrite(MOTOR2_IN2, LOW);  // Forward direction
  analogWrite(MOTOR2_PIN, speed);
}

void turnOffMotor2() {
  digitalWrite(MOTOR2_IN1, LOW);
  digitalWrite(MOTOR2_IN2, LOW);
  analogWrite(MOTOR2_PIN, 0);
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
  Serial.println("GPS:" + *latitude + ", " + *longitude);
  return false;
}