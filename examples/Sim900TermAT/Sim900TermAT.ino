

#include <SoftwareSerial.h>


#define GSM_POWER           8 // connect GSM Module POWER to pin 8 
#define GSM_RESET           9 // connect GSM Module RESET to pin 9

#define AT_TIME_WAIT        50
#define AT_TIMEOUT          10000


SoftwareSerial simSerial(2, 3);



void power(void) {
  Serial.print(F("Power toggle..."));
  
  // generate turn on/off pulse
  digitalWrite(GSM_POWER, HIGH);
  delay(1100);
  digitalWrite(GSM_POWER, LOW);
  delay(2300);

  Serial.println(F("\tDone."));
}




void setup() 
{
  Serial.begin(9600);
  while (!Serial) {
  }
  Serial.println(F("Started.\nUse \"CR only\" option in console.\nType \'h\' for help.")); 
  
  // set some GSM pins as inputs, some as outputs
  pinMode(GSM_POWER, OUTPUT);            // sets pin 5 as output
  //pinMode(GSM_RESET, OUTPUT);            // sets pin 4 as output

  simSerial.begin(57600);
}


void loop() { 
  char inSerial[64];
  int cnum=0;

  //Serial.println();
  //Serial.print("> ");
  //delay(1000);
      Serial.setTimeout(10000);
  if (Serial.available() != 0) {
     cnum = Serial.readBytesUntil('\r', inSerial, 64 );
     inSerial[cnum] = '\r';
     inSerial[cnum+1] = '\0';
     Serial.print("> ");
     Serial.println(inSerial);
     switch (cnum) {
      case 0: break;
      case 1:
        switch (inSerial[0]) {
          case 'h':
            Serial.println("This is terminal for commutication with GSM module through AT commands.\n h \t help \n p \t power toggle");
            break;
          case 'p':
            power();
            break;
        } break;
        default :
          //Serial.print(F("Forwarding data..."));
          simSerial.print(inSerial);
          //Serial.println(F(" Done."));
     }
  }

        
  while (simSerial.available() > 0) {
    Serial.print((char)simSerial.read()); //read data     
  }



}  


 


 

  
