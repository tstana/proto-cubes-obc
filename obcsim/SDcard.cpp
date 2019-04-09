#include "Arduino.h"
#include "SDcard.hpp"
#include <SPI.h>
#include <SD.h>
static unsigned char conf[300];

void initSD(void){
    SD.begin(10); //4 on ethernet shield, 10 on SD-prototype board
    SD.remove("Datafil.txt");
}
void readSD(unsigned char* target, char location[12]){
  File confFile = SD.open(location, FILE_READ);
  if(confFile){
    for(int i=0; confFile.available(); i++){
      target[i] = confFile.read();
    }
    Serial.println(F("SD-card Read"));
  }
  else
    Serial.println(F("SD-card read failed")); 
  confFile.close();
}
void sendSD(unsigned char *data, unsigned long len){
    File dataFile = SD.open("Datafil.txt", FILE_WRITE);
    if(dataFile){
      int written = dataFile.write(data, len);
      dataFile.close();
      Serial.print(F("SD-card write success, "));
      Serial.print(written);
      Serial.println(F(" bytes written"));

    }
    else
      Serial.println(F("SD-card write failed"));
}

/* Code for converting the binary files to binary values. Saves data transfer to convert to binary before sending to satellite.
 int cols = 8;
int j = 0;
char output[144];
for(i=0; i<len(input) && input[i] != "\n"; i++){
 if(i%cols == 0 && i != 0){
    j++;
  }
  else {
    if(input[i] == '0'){
      input[i] = input[i]<<1;
    }
    else if(input[i] == '1'){
      input[i]=(input[i]<<1)+1;
    }
  }
}
 */
