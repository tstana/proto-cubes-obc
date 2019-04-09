#include "Arduino.h"

void RS_init(void){
  Serial2.begin(115200, SERIAL_8E1); //Set 115200 Baud rate, 8bit 1stop even parity
  Serial2.println(F("RS232 Serial started")); //Remove later, just for testing first message.
  
}

void RS_read(void){
  
}

