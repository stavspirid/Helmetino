#include <Adafruit_MPU6050.h>
#include <stdio.h>

Adafruit_MPU6050 mpu;

void setup() {
  

}

void loop() {
  // put your main code here, to run repeatedly:

}

void getLightResistor(){
  transferFloatData(
    sensorBoardMeasurements.lightResistorValue = digitalRead(3)
    );
}

void getAccelX() {
  sensors_event_t a, g, tmp;
  mpu.getEvent(&a, &g, &tmp);
  transferFloatData(
    sensorBoardMeasurements.accelX = a.acceleration.x
    );
}

void getAccelY() {
  sensors_event_t a, g, tmp;
  mpu.getEvent(&a, &g, &tmp);
  transferFloatData(
    sensorBoardMeasurements.accelY = a.acceleration.y
    );
}

void getAccelZ() {
  sensors_event_t a, g, tmp;
  mpu.getEvent(&a, &g, &tmp);
  transferFloatData(
    sensorBoardMeasurements.accelZ = a.acceleration.z
    );
}

void getGyroX() {
  sensors_event_t a, g, tmp;
  mpu.getEvent(&a, &g, &tmp);
  transferFloatData(
    sensorBoardMeasurements.gyroX = g.gyro.x
    );
}

void getGyroY() {
  sensors_event_t a, g, tmp;
  mpu.getEvent(&a, &g, &tmp);
  transferFloatData(
    sensorBoardMeasurements.gyroY = g.gyro.y
    );
}

void getGyroZ() {
  sensors_event_t a, g, tmp;
  mpu.getEvent(&a, &g, &tmp);
  transferFloatData(
    sensorBoardMeasurements.gyroZ = g.gyro.z
    );
}