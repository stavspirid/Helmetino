#include <SPI.h>
#include <RF22.h>
#include <RF22Router.h>

#define MY_ADDRESS 14 // define my unique address
#define DESTINATION_ADDRESS_1 13 // define who I can talk to

// MY CODE BEGIN

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;
const int btnPIN = 8;


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

// MY CODE END


// Singleton instance of the radio
RF22Router rf22(MY_ADDRESS); // initiate the class to talk to my radio with MY_ADDRESS
int number_of_bytes=0; // will be needed to measure bytes of message

float throughput=0; // will be needed for measuring throughput
int flag_measurement=0;

int counter=0;
int initial_time=0;
int final_time=0;


uint8_t rssi;
float Pr=-90;




// named constant for the pin the sensor is connected to
const int sensorPin = A0; // will be needed to measure something from pin A0

void setup() {
  // MY CODE BEGIN
  // Serial.begin(115200);
  while (!Serial)
    delay(10); // will pause until serial console opens
  Serial.println("");
  delay(100);

  mpu.begin();
  pinMode(btnPIN, INPUT);
  // MY CODE END

  Serial.begin(9600);
  if (!rf22.init()) // initialize my radio
    Serial.println("RF22 init failed");
  // Defaults after init are 434.0MHz, 0.05MHz AFC pull-in, modulation FSK_Rb2_4Fd36
  if (!rf22.setFrequency(434.0)) // set the desired frequency
    Serial.println("setFrequency Fail");
  rf22.setTxPower(RF22_TXPOW_20DBM); // set the desired power for my transmitter in dBm
  //1,2,5,8,11,14,17,20 DBM
  rf22.setModemConfig(RF22::GFSK_Rb125Fd125  ); // set the desired modulation
  //modulation

  // Manually define the routes for this network
  rf22.addRouteTo(DESTINATION_ADDRESS_1, DESTINATION_ADDRESS_1); // tells my radio card that if I want to send data to DESTINATION_ADDRESS_1 then I will send them directly to DESTINATION_ADDRESS_1 and not to another radio who would act as a relay
  for(int pinNumber = 4; pinNumber<6; pinNumber++) // I can use pins 4 to 6 as digital outputs (in the example to turn on/off LEDs that show my status)
  {
    pinMode(pinNumber,OUTPUT);
    digitalWrite(pinNumber, LOW);
  }
  delay(1000); // delay for 1 s
}

// Replace the loop() function with this corrected version

void loop() 
{
  // MY CODE BEGIN
  /* Get new sensor events with the readings */
  // getMeasurements();
  
  if(crashDetection()) {
    while(true){
      // Consider adding actual emergency logic here instead of infinite loop
    }
  }
  
  Serial.println("");
  delay(500);

  // MY CODE END
  
  counter=0;
  initial_time=millis();
  int sensorVal = analogRead(sensorPin); // measure something
  Serial.print("My measurement is: ");
  Serial.println(sensorVal); // and show it on Serial

  // the following variables are used for radio transmission
  char label[] = "TEMP"; // Changed from "DATA" to "TEMP" as requested
  uint8_t data_send[RF22_ROUTER_MAX_MESSAGE_LEN];
  char data_read[RF22_ROUTER_MAX_MESSAGE_LEN];

  // Clear buffers
  memset(data_read, '\0', RF22_ROUTER_MAX_MESSAGE_LEN);
  memset(data_send, '\0', RF22_ROUTER_MAX_MESSAGE_LEN);

  // Format the string properly - this is the key fix!
  snprintf(data_read, RF22_ROUTER_MAX_MESSAGE_LEN, "%s: %d", label, sensorVal);
  
  // DON'T overwrite the formatted string like in the original code
  // This was causing the issue, as it was ignoring your label!
  
  // Now copy to data_send byte array for radio
  memcpy(data_send, data_read, strlen(data_read) + 1); // +1 for null terminator

  // Count actual used bytes (excluding unused buffer tail)
  number_of_bytes = strlen(data_read) + 1; // +1 for null terminator

  Serial.print("Data to send: ");
  Serial.println(data_read);
  Serial.print("Number of Bytes= ");
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

  delay(1000); // Send once per second
}


// MY CODE BEGIN
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
// MY CODE END
