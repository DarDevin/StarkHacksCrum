#ifndef READVALUES_H
#define READVALUES_H

void initSensors();
float readTemp();
float readHumidity();
void readAccelerometer(float* ax, float* ay, float* az);
bool isRobotMoving(float threshold);
void spraySolenoid(unsigned long durationMs, uint8_t intensity);
void turnOnMotor1(uint8_t speed);
void turnOffMotor1();
void turnOnMotors(uint8_t speed);
void turnOffMotors();
bool readLatLong(float* latitude, float* longitude, unsigned long timeoutMs);

#endif // READVALUES_H
