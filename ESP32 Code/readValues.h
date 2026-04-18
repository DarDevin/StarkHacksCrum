#ifndef READVALUES_H
#define READVALUES_H

void initSensors();
float readTemp();
float readHumidity();
void readAccelerometer(float* ax, float* ay, float* az);
void moveSolenoid();
void moveMotor();
void readLatLong();

#endif // READVALUES_H
