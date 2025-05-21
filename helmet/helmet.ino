#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Ultrasonic.h>

Adafruit_MPU6050 mpu;
const int btnLEDPIN = 8;
const int btnDISPIN = 7;
const int leftLED   = 12;
const int rightLED  = 11;
const int LED       = 10;
const int daySensor = 4;


struct SensorMeasurements {
  float   accelX               =  0.;
  float   accelY               =  0.;
  float   accelZ               =  0.;
  float   gyroX                =  0.;
  float   gyroY                =  0.;
  float   gyroZ                =  0.;
} sensorMeasurements;

void getMeasurements();   // Gets Gyroscope Measurements
void printSensorMeasurements(const SensorMeasurements& s);  // Prints Gyroscope Measurements
int crashDetection();     // Detect if the rider has crashed
bool getLight();          // Detect Day or Night
void lightControl();      // Control LED light of the helmet
void distanceControl(Ultrasonic sensor, const int buzzerID, bool btnState);   // Measure the distance from nearby objects and alert with buzzers
void buttonControl(const int btnID);    // Control how the helmet's buttons work
int ax[2], ay[2], az[2];  // Accelerometer Measurements to define crashing
int distanceL;            // Distance from Left Sensor
int distanceR;            // Distance from Right Sensor

void getAccelX();
void getAccelY();
void getAccelZ();
void getGyroX();
void getGyroY();
void getGyroZ();

sensors_event_t a, g, tmp;

//Define pins ultrasonic(trig,echo)
Ultrasonic rightUltrasonic(A0,A1);
Ultrasonic leftUltrasonic(A2,A3);




void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10); // will pause until serial console opens
  Serial.println("");
  delay(100);

  mpu.begin();
  pinMode(btnLEDPIN, INPUT_PULLUP);
  pinMode(btnDISPIN, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  pinMode(leftLED, OUTPUT);
  pinMode(rightLED, OUTPUT);
  pinMode(daySensor, INPUT);


}

void loop() {

  // Get new Gyroscope sensor events with the readings
  getMeasurements();
  // printSensorMeasurements(sensorMeasurements);
  
  // When a crash is detected call a neverending loop
  if(crashDetection()) {
    while(true){
    }
  }
  
  Serial.println("");
  delay(1000);

  // if(digitalRead(btnLEDPIN)==0){
  //   Serial.println("LED Button is NOT pressed");
  //   digitalWrite(LED, LOW);
  // } else {
  //   Serial.println("LED Button is pressed");
  //   digitalWrite(LED, HIGH);
  // }
  // if(digitalRead(btnDISPIN)==0){
  //   Serial.println("Distance Button is NOT pressed");
  // } else{
  //   Serial.println("Distance Button is pressed");
  // }

  buttonControl(btnLEDPIN);
  buttonControl(btnDISPIN);
  // getLight();
  lightControl();
  distanceControl(rightUltrasonic, rightLED, btnDISPIN);
  distanceControl(leftUltrasonic, leftLED, btnDISPIN);
}

// Always on at night and can be on when button is pressed during the day
void lightControl(){
  if (digitalRead(btnLEDPIN)!=0 || !getLight()) {
    digitalWrite(LED, HIGH);
  } else {
    digitalWrite(LED, LOW);
  }
}

void buttonControl(const int btnID){
  switch (btnID) {
    case btnLEDPIN : {
      if(digitalRead(btnLEDPIN)==0){
        Serial.println("LED Button is NOT pressed");
        digitalWrite(LED, LOW);
      } else {
        Serial.println("LED Button is pressed");
        digitalWrite(LED, HIGH);
      }
      break;
    }

    case btnDISPIN : {
      if(digitalRead(btnDISPIN)==0){
        Serial.println("Distance Button is NOT pressed");
      } else {
        Serial.println("Distance Button is pressed");
      }
      break;
    }
  }
}

void distanceControl(Ultrasonic sensor, const int buzzerID, int btnID){   // TODO : Might change to btnState if we get state at the beginning of the loop
  if (digitalRead(btnDISPIN)!=0) {
    // Serial.println("Inside Distance Control");
    if (sensor.Ranging(CM) < 15) {
      // tone(buzzerID, 1000);
      digitalWrite(buzzerID, HIGH);
    } else {
      // noTone(buzzerID);
      digitalWrite(buzzerID, LOW);
    }
  }
}

bool getLight(){
  if(digitalRead(daySensor) == 0){
    Serial.println("DAY");
    return true;
  } else {
    Serial.println("NOT DAY");
    return false;
  }
}


void getMeasurements() {
  getAccelX();
  getAccelY();
  getAccelZ();
  getGyroX();
  getGyroY();
  getGyroZ();
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

  if (abs(ax[0]-ax[1])>3 || abs(ay[0]-ay[1])>3 || abs(az[0]-az[1])>3){
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