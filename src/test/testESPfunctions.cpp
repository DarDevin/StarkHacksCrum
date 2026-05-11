#include "readValues.h"
#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing sensors...");
  initSensors();
  Serial.println("Sensors initialized.");
}

void loop() {
  // Test temperature and humidity
  Serial.println("--- Testing DHT Sensor ---");
  float temp = readTemp();
  float hum = readHumidity();
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println(" °C");
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.println(" %");

  // Test accelerometer
  Serial.println("--- Testing MPU6050 ---");
  float ax, ay, az;
  readAccelerometer(&ax, &ay, &az);
  Serial.print("Acceleration X: ");
  Serial.print(ax);
  Serial.print(" g, Y: ");
  Serial.print(ay);
  Serial.print(" g, Z: ");
  Serial.print(az);
  Serial.println(" g");

  // Test if robot is moving
  bool moving = isRobotMoving(0.1f); // threshold of 0.1g
  Serial.print("Robot moving: ");
  Serial.println(moving ? "Yes" : "No");

  // Test GPS
  Serial.println("--- Testing GPS ---");
  float lat, lon;
  bool gpsValid = readLatLong(&lat, &lon, 1000); // 1 second timeout
  if (gpsValid) {
    Serial.print("Latitude: ");
    Serial.print(lat, 6);
    Serial.print(", Longitude: ");
    Serial.println(lon, 6);
  } else {
    Serial.println("GPS: No valid location");
  }

  // Test solenoid (short spray at low intensity)
  Serial.println("--- Testing Solenoid ---");
  Serial.println("Spraying solenoid for 500ms at intensity 128...");
  spraySolenoid(500, 128);
  Serial.println("Solenoid test complete.");

  // Test motors (brief turn on then off)
  Serial.println("--- Testing Motors ---");
  Serial.println("Turning on motors at speed 100...");
  turnOnMotors(100);
  delay(1000); // Run for 1 second
  turnOffMotors();
  Serial.println("Motors turned off.");

  // Wait before next test cycle
  Serial.println("--- Waiting 5 seconds ---");
  delay(5000);
}

