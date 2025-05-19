#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

struct SensorMeasurements {
  float   accelX               =  0.;
  float   accelY               =  0.;
  float   accelZ               =  0.;
  float   gyroX                =  0.;
  float   gyroY                =  0.;
  float   gyroZ                =  0.;
} sensorMeasurements;

void getMeasurements();
void printSensorMeasurements(const SensorMeasurements& s);
int crashDetection();
int ax[2], ay[2], az[2];

void getAccelX();
void getAccelY();
void getAccelZ();
void getGyroX();
void getGyroY();
void getGyroZ();

sensors_event_t a, g, tmp;




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
  
  if(crashDetection()) {
    while(true){
      
    }
  }
  
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

int crashDetection() {
  mpu.getEvent(&a, &g, &tmp);
  ax[0]=(a.acceleration.x);
  ay[0]=(a.acceleration.y);
  az[0]=(a.acceleration.z);
  delay(50);
  mpu.getEvent(&a, &g, &tmp);
  ax[1]=(a.acceleration.x);
  ay[1]=(a.acceleration.y);
  az[1]=(a.acceleration.z);

  if (abs(ax[0]-ax[1])>1 || abs(ay[0]-ay[1])>1 || abs(az[0]-az[1])>1){
    Serial.println("CRASH DETECTED");
    return 1;
  }
  return 0;
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