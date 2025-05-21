#include "SPI.h"
#include "RF22.h"
#include "RF22Router.h"
#include "Ultrasonic.h"

#define MY_ADDRESS 14 // define my unique address
#define DESTINATION_ADDRESS_1 13 // define who I can talk to
//#define DESTINATION_ADDRESS_2 3 // define who I can talk to

// Singleton instance of the radio
RF22Router rf22(MY_ADDRESS); // initiate the class to talk to my radio with MY_ADDRESS

// ----- OUR IMPLEMENTATION ----- //
const int buzzerL = 8;       //buzzer to arduino pin 8
const int buzzerR = 9;       //buzzer to arduino pin 9
const int button_led = 6;    //button to arduino pin 6
const int button_sound = 5;  //button to arduino pin 6
const int led = 4;           //led to arduino pin 4
const int light_sensor = 7;  //light sensor to arduino pin 7

bool sound_active = false;
bool led_state = false;
bool Lobject_detect = false;
bool Robject_detect = false;
bool Sbutton_prev = LOW;
bool Lbutton_prev = LOW;

//Define pins ultrasonic(trig,echo)
Ultrasonic ultrasonicL(A0,A1);
Ultrasonic ultrasonicR(A2,A3);

//Variables
int distanceL;
int distanceR;

// ------------------------------ //

int counter=0;
int initial_time=0;
int final_time=0;


uint8_t rssi;
float Pr=-90;


void setup() {
  Serial.begin(9600); // to be able to view the results in the computer's monitor

  // ----- OUR IMPLEMENTATION ----- //

  pinMode(buzzerL, OUTPUT);      // Set left buzzer - pin 8 as an output
  pinMode(buzzerR, OUTPUT);      // Set right buzzer - pin 9 as an output
  pinMode(button_led, INPUT_PULLUP);    // set led button - pin 6 as input
  pinMode(button_sound, INPUT);  // set sound button - pin 5 as input
  pinMode(led, OUTPUT);          // set led - pin 4 as output
  pinMode(light_sensor, INPUT);   // set light sensor - pin 7 as input
  digitalWrite(led, LOW);

  // ------------------------------ //

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

void loop() 
{
  counter=0;
  initial_time=millis();

  // ----- OUR IMPLEMENTATION ----- //

  Lobject_detect = false;
  Robject_detect = false;

  distanceL = ultrasonicL.Ranging(CM);  
  distanceR = ultrasonicR.Ranging(CM);  

  Serial.print("Distance L: ");
  Serial.println(distanceL);
  Serial.print("Distance R: ");
  Serial.println(distanceR);

  // Read the current button states
  bool Sbutton_curr = digitalRead(button_sound); 
  bool Lbutton_curr = digitalRead(button_led);

  // Check for rising edge on sound button
  if (Sbutton_prev == HIGH && Sbutton_curr == LOW) {
    sound_active = !sound_active;
    delay(200); // debounce
  }

  // Check for rising edge on LED button
  if (Lbutton_prev == HIGH && Lbutton_curr == LOW) {
    led_state = !led_state;
    digitalWrite(led, led_state);
    delay(200); // debounce
  }

  // Save current states for next loop
  Sbutton_prev = Sbutton_curr;
  Lbutton_prev = Lbutton_curr;

  Serial.print("Button's state:");
  Serial.print(Lbutton_curr);

  // If sound is active, respond to obstacles
  if (sound_active) {
    if (distanceL < 15) {
      Lobject_detect = !Lobject_detect;
      tone(buzzerL, 1000);
      delay(500);
      noTone(buzzerL);
    }

    if (distanceR < 15) {
      Robject_detect = !Robject_detect;
      tone(buzzerR, 1000);
      delay(500);
      noTone(buzzerR);
    }
  }

  // ------------------------------ //

// the following variables are used in order to transform my integer measured value into a uint8_t variable, which is proper for my radio
  char data_read[RF22_ROUTER_MAX_MESSAGE_LEN];
  uint8_t data_send[RF22_ROUTER_MAX_MESSAGE_LEN];
  memset(data_read, '\0', RF22_ROUTER_MAX_MESSAGE_LEN);
  memset(data_send, '\0', RF22_ROUTER_MAX_MESSAGE_LEN);  

  sprintf(data_read, "%d", distanceR); // I'm copying the measurement sensorVal into variable data_read
  data_read[RF22_ROUTER_MAX_MESSAGE_LEN - 1] = '\0'; 
  memcpy(data_send, data_read, RF22_ROUTER_MAX_MESSAGE_LEN); // now I'm copying data_read to data_send

  // just demonstrating that the string I will send, after those transformation from integer to char and back remains the same
  int sensorVal2=0;
  sensorVal2=atoi(data_read);
  Serial.print("The string I'm ready to send is= ");
  Serial.println(sensorVal2);

  if (rf22.sendtoWait(data_send, sizeof(data_send), DESTINATION_ADDRESS_1) != RF22_ROUTER_ERROR_NONE) // I'm sending the data in variable data_send to DESTINATION_ADDRESS_1... cross fingers
  {
    Serial.println("sendtoWait failed"); // for some reason I have failed
  }
  else
  {
    counter=counter+1;
    rssi = rf22.rssiRead();
    Pr=((float)rssi-230.0)/1.8;
    Serial.print("RSSI= ");
    Serial.print(Pr);
    Serial.println(" dBm");

    Serial.println("sendtoWait Successful"); // I have received an acknowledgement from DESTINATION_ADDRESS_1. Data have been delivered!
  }


  final_time=millis();
  Serial.print("Initial time= ");  
  Serial.print(initial_time);
  Serial.print("     Final time= ");  
  Serial.println(final_time);

}
