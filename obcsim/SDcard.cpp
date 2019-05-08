#include "Arduino.h"
#include "SDcard.hpp"
#include <SPI.h>
#include <SD.h>
#include "RTC.hpp"

static unsigned char conf[300];
static char filename[] = "log00000.txt";
void SD_init(void) {
  SD.begin(10); //4 on ethernet shield, 10 on SD-prototype board
}
void SD_read(unsigned char* target, char location[12]) {
  File confFile = SD.open(location, FILE_READ);
  if (confFile) {
    for (int i = 0; confFile.available(); i++) {
      target[i] = confFile.read();
    }
    Serial.println(F("SD-card Read"));
  }
  else
    Serial.println(F("SD-card read failed"));
  confFile.close();
}
void SD_send(unsigned char *data, unsigned long len) {
  File dataFile;
  uint32_t j = 0;
  for (uint32_t i = 0; i < 100000; i++) { /* creates a new file every time function is called,  */
    j = i;
    filename[3] = j / 10000;
    filename[3] += 0x30;
    j %= 10000;
    filename[4] = j / 1000;
    filename[4] += 0x30;
    j %= 1000;
    filename[5] = j / 100;
    filename[5] += 0x30;
    j %= 100;
    filename[6] = j / 10;
    filename[6] += 0x30;
    filename[7] = j % 10;
    filename[7] += 0x30;
    if (!SD.exists(filename)) {
      dataFile = SD.open(filename, FILE_WRITE);
      break;
    }
  }
  if (dataFile) {
    int written = dataFile.print("Unix time: ");
    written += dataFile.println(RTC_get_seconds());
    written += dataFile.write(data, len);
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
