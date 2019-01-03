

// include the necessary libraries
#include <SoftwareSerial.h>
#include <Simcom.h>


/* assuming the TX and RX pins on the camera are attached to pins 4 and 5 of
 * the arduino. */

Sim900 gsmShield(2, 3);



int rCode;



void goOnline(void)
{
  if (gsmShield.getIP()) {
    rCode = gsmShield.gprsOn(F("internet.t-mobile"),
                             F("t-mobile"),
                             F("tm"));
    if (rCode) {
      Serial.print(F("Gprs failed. Error code "));
      Serial.println(rCode);
    } else {
      Serial.println(F("Online!"));
    }
  } else {
    rCode = gsmShield.gprsOff();
    if (rCode) {
      Serial.print(F("Can't go off. Error code "));
      Serial.println(rCode);
    } else {
      Serial.println(F("Offline."));
    }
  }

}
/******************************************/

void setup() {
  
  //delay(3000);
  
  Serial.begin(9600);
  while (!Serial);
  Serial.print(F("Starting..."));


  Serial.println(F("Start shield."));
  
  gsmShield.begin(57600);
  while (!gsmShield);
  
  Serial.print(F("Test shield: "));
  if (gsmShield.isOn()) {
    Serial.println(F("Shield is on, resetting."));
    gsmShield.end();
    gsmShield.powerReset();
  } else {
    Serial.println(F("Shield is off, turning on."));
    gsmShield.end();
    gsmShield.powerToggle();
  }
  Serial.println(F("Start shield again."));
  gsmShield.begin(57600);
  while (!gsmShield);  
  if (gsmShield.isOn()) {
    Serial.println(F("Shield is on and clear."));
  } else {
    Serial.println(F("Shield does not respond."));
    return;
  }
  

  gsmShield.setTimeout(2000);

  goOnline();


}


void loop() {

  delay(500);
  

}
