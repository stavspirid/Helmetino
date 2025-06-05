#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Ultrasonic.h>
#include <SPI.h>
#include <RF22.h>
#include <RF22Router.h>

#define MY_ADDRESS 14 // define my unique address
#define DESTINATION_ADDRESS_1 13 // define who I can talk to

Adafruit_MPU6050 mpu;
const int btnLEDPIN = 13;
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
Ultrasonic rightUltrasonic(A2,A3);
Ultrasonic leftUltrasonic(A0,A1);

// Singleton instance of the radio
RF22Router rf22(MY_ADDRESS); // initiate the class to talk to my radio with MY_ADDRESS
int number_of_bytes=0; // will be needed to measure bytes of message

int counter = 0;
int crashFlag = 0;
int lightFlag = 0;
int soundFlag = 0;
int tempMeasurement = 27;

uint8_t rssi;
float Pr=-90;


void setup(void) {
  Serial.begin(9600);
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
  if(crashDetection()){
    crashFlag = true;
    while(1){
      sendData();
      delay(1000);
    }
  }
  
  Serial.println("");
  delay(500);


  buttonControl(btnLEDPIN);
  buttonControl(btnDISPIN);
  // getLight();
  lightControl();
  distanceControl(rightUltrasonic, rightLED, btnDISPIN);
  distanceControl(leftUltrasonic, leftLED, btnDISPIN);
  
  sendData();
  
  delay(500); // Total delay of 1 second per loop
}

void sendData() {
  uint8_t data_send[RF22_ROUTER_MAX_MESSAGE_LEN];
  char data_read[RF22_ROUTER_MAX_MESSAGE_LEN];

  // Clear buffers
  memset(data_read, '\0', RF22_ROUTER_MAX_MESSAGE_LEN);
  memset(data_send, '\0', RF22_ROUTER_MAX_MESSAGE_LEN);

  // Format the data string with all sensor information
  snprintf(data_read, RF22_ROUTER_MAX_MESSAGE_LEN, 
           "CRASH=%d, LIGHT=%d, SOUND=%d, TEMP=%d", 
           crashFlag, lightFlag, soundFlag, tempMeasurement);

  // Copy to data_send byte array for radio
  memcpy(data_send, data_read, strlen(data_read) + 1); // +1 for null terminator

  // Count actual used bytes
  number_of_bytes = strlen(data_read) + 1; // +1 for null terminator

  Serial.print("Data to send: ");
  Serial.println(data_read);
  Serial.print("Number of Bytes: ");
  Serial.println(number_of_bytes);

  // For debugging - print the actual bytes being sent
  Serial.print("Raw bytes: ");
  for (int i = 0; i < number_of_bytes; i++) {
    Serial.print((char)data_send[i]);
  }
  Serial.println();

  if (rf22.sendtoWait(data_send, number_of_bytes, DESTINATION_ADDRESS_1) != RF22_ROUTER_ERROR_NONE) {
    Serial.println("sendtoWait failed");
  }
  else {
    Serial.println("Message sent successfully");
  }
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
      tone(buzzerID, 1000);
      // digitalWrite(buzzerID, HIGH);
    } else {
      noTone(buzzerID);
      // digitalWrite(buzzerID, LOW);
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

  // printSensorMeasurements(sensorMeasurements);
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
