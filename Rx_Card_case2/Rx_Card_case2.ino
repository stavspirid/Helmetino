#include <SPI.h>
#include <RF22.h>
#include <RF22Router.h>

#define MY_ADDRESS 13 // define my unique address
#define DESTINATION_ADDRESS_1 14 // define who I can talk to

// Singleton instance of the radio
RF22Router rf22(MY_ADDRESS); // initiate the class to talk to my radio with MY_ADDRESS
int received_value=0;
uint8_t rssi;
float Pr=-90;


void setup() {
  Serial.begin(9600);
  if (!rf22.init())
    Serial.println("RF22 init failed");
  // Defaults after init are 434.0MHz, 0.05MHz AFC pull-in, modulation FSK_Rb2_4Fd36
  if (!rf22.setFrequency(434.0)) // The frequency should be the same as that of the transmitter. Otherwise no communication will take place
    Serial.println("setFrequency Fail");
  rf22.setTxPower(RF22_TXPOW_20DBM);
  //1,2,5,8,11,14,17,20 DBM
  rf22.setModemConfig(RF22::GFSK_Rb125Fd125  );// The modulation should be the same as that of the transmitter. Otherwise no communication will take place
  //modulation

  // Manually define the routes for this network
  rf22.addRouteTo(DESTINATION_ADDRESS_1, DESTINATION_ADDRESS_1); // tells my radio card that if I want to send data to DESTINATION_ADDRESS_1 then I will send them directly to DESTINATION_ADDRESS_1 and not to another radio who would act as a relay 
  for(int pinNumber = 4; pinNumber<6; pinNumber++)// I can use pins 4 to 6 as digital outputs (in the example to turn on/off LEDs that show my status)
  {
    pinMode(pinNumber,OUTPUT);
    digitalWrite(pinNumber, LOW);
  }
  delay(100); 
}

void loop() 
{
  uint8_t buf[RF22_ROUTER_MAX_MESSAGE_LEN];
  char incoming[RF22_ROUTER_MAX_MESSAGE_LEN];
  memset(buf,      '\0', RF22_ROUTER_MAX_MESSAGE_LEN);
  memset(incoming,'\0', RF22_ROUTER_MAX_MESSAGE_LEN);
  uint8_t len = sizeof(buf);
  uint8_t from;

  if (rf22.recvfromAck(buf, &len, &from)) 
  {
    buf[RF22_ROUTER_MAX_MESSAGE_LEN - 1] = '\0';
    memcpy(incoming, buf, RF22_ROUTER_MAX_MESSAGE_LEN);

    // Παίρνουμε ΜΟΝΟ τον ακέραιο μετά το "TEMP:"
    int tempVal = 0;
    if (sscanf(incoming, "TEMP:%d", &tempVal) == 1) {
      Serial.print("TEMP: ");
      Serial.println(tempVal);
    } else {
      Serial.println("Failed to parse TEMP value");
    }
  }
}

