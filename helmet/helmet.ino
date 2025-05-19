#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

void getMeasurements();

void getAccelX();
void getAccelY();
void getAccelZ();
void getGyroX();
void getGyroY();
void getGyroZ();

struct SensorMeasurements {
  float   accelX               =  0.;
  float   accelY               =  0.;
  float   accelZ               =  0.;
  float   gyroX                =  0.;
  float   gyroY                =  0.;
  float   gyroZ                =  0.;
} sensorMeasurements;

sensors_event_t a, g, tmp;

void printSensorMeasurements(const SensorMeasurements& s);

void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10); // will pause until serial console opens
  Serial.println("");
  delay(100);

  mpu.begin();
}

void loop() {

  /* Get new sensor events with the readings */
  getMeasurements();

  Serial.println("");
  delay(500);
}


void getMeasurements() {
  getAccelX();
  getAccelY();
  getAccelZ();
  getGyroX();
  getGyroY();
  getGyroZ();

  printSensorMeasurements(sensorMeasurements);
}

void printSensorMeasurements(const SensorMeasurements& s) {
  Serial.print("Accelerometer X: "); Serial.println(s.accelX);
  Serial.print("Accelerometer Y: "); Serial.println(s.accelY);
  Serial.print("Accelerometer Z: "); Serial.println(s.accelZ);
  Serial.print("Gyroscope X:     "); Serial.println(s.gyroX);
  Serial.print("Gyroscope Y:     "); Serial.println(s.gyroY);
  Serial.print("Gyroscope Z:     "); Serial.println(s.gyroZ);
}


void getAccelX() {
  // sensors_event_t a, g, tmp;
  mpu.getEvent(&a, &g, &tmp);
  sensorMeasurements.accelX = a.acceleration.x;
}

void getAccelY() {
  // sensors_event_t a, g, tmp;
  mpu.getEvent(&a, &g, &tmp);
  sensorMeasurements.accelY = a.acceleration.y;
}

void getAccelZ() {
  // sensors_event_t a, g, tmp;
  mpu.getEvent(&a, &g, &tmp);
  sensorMeasurements.accelZ = a.acceleration.z;
}

void getGyroX() {
  // sensors_event_t a, g, tmp;
  mpu.getEvent(&a, &g, &tmp);
  sensorMeasurements.gyroX = g.gyro.x;
}

void getGyroY() {
  // sensors_event_t a, g, tmp;
  mpu.getEvent(&a, &g, &tmp);
  sensorMeasurements.gyroY = g.gyro.y;
}

void getGyroZ() {
  // sensors_event_t a, g, tmp;
  mpu.getEvent(&a, &g, &tmp);
  sensorMeasurements.gyroZ = g.gyro.z;
}