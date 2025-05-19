//Libraries
#include "Ultrasonic.h"

const int buzzerL = 8; //buzzer to arduino pin 8
const int buzzerR = 9; //buzzer to arduino pin 9
const int button = 6;  //button to arduino pin 6

//Define pins ultrasonic(trig,echo)
Ultrasonic ultrasonicL(A0,A1);
Ultrasonic ultrasonicR(A2,A3);

//Variables
int distanceL;
int distanceR;

void setup() {
  Serial.begin(9600);
  pinMode(buzzerL, OUTPUT); // Set buzzer - pin 9 as an output
  pinMode(buzzerR, OUTPUT); // Set buzzer - pin 9 as an output
  pinMode(button, INPUT);
}

void loop()
{
  distanceL = ultrasonicL.Ranging(CM); //Use 'CM' for centimeters or 'INC' for inches
  distanceR = ultrasonicR.Ranging(CM); //Use 'CM' for centimeters or 'INC' for inches

  //Alert for close obstacles
  if(distanceL < 15){
      tone(buzzerL, 1000); // Send 1KHz sound signal...
      delay(2000);         // ...for x sec
      noTone(buzzerL);     // Stop sound...
  }

  if((distanceL < 15) || (distanceR < 15)){
      tone(buzzerR, 1000); // Send 1KHz sound signal...
      delay(2000);         // ...for x sec
      noTone(buzzerR);     // Stop sound...
  }

  delay(1000);
}
