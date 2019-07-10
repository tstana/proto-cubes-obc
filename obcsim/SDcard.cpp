#include <string.h>

#include "Arduino.h"
#include "SDcard.hpp"
#include <SPI.h>
#include <SD.h>
#include "RTC.hpp"

static unsigned char conf[300];
static char filename[] = "daq00000.dat";

void daq_init(void)
{
  Serial.println("SD card initializing...");
  SD.begin(10); // Pin 4 on ethernet shield, pin 10 on SD-prototype board

  Serial.println("Attempting to get the last written file...");
  if (SD.exists("lastfile.txt")) {
    Serial.println("lastfile.txt found.");
    File lastfile = SD.open("lastfile.txt", FILE_READ);
    if (lastfile) {
      lastfile.read(filename, 12);
      lastfile.close();
      char s[64];
      sprintf(s, "Last written file name (%s) read successfully!", filename);
      Serial.println(s);
    } else {
      Serial.println("Could not open lastfile.txt for reading, even though it exists!");
    }
  } else {
    Serial.println("lastfile.txt not found, creating...");
    File lastfile = SD.open("lastfile.txt", FILE_WRITE);
    if (lastfile) {
      lastfile.write(filename);
      lastfile.write('\n');
      lastfile.close();
    } else {
      Serial.println("Could not open lastfile.txt for writing!");
    }
   }
}


void daq_write_new_file(unsigned char *data, unsigned long len) {
  File dataFile;
  uint32_t j = 0;
  /*
   * Create a new file every time function is called. The file name is
   * "daqNNNNN.dat", where NNNNN is from the interval [0, 99999]. The
   * code below parses this interval until a file that does not exist is
   * found, creates it, then breaks.
   * 
   * Adding the '0' character makes ASCII strings out of number calculations.
   */
  for (uint32_t i = 0; i < 100000; i++) {
    j = i;
    filename[3] = j / 10000;
    filename[3] += '0';
    j %= 10000;
    filename[4] = j / 1000;
    filename[4] += '0';
    j %= 1000;
    filename[5] = j / 100;
    filename[5] += '0';
    j %= 100;
    filename[6] = j / 10;
    filename[6] += '0';
    filename[7] = j % 10;
    filename[7] += '0';
    if (!SD.exists(filename)) {
      Serial.print("Writing data to file ");
      Serial.print(filename);
      Serial.println();
      dataFile = SD.open(filename, FILE_WRITE);
      break;
    }
  }
  if (dataFile) {
    int written = dataFile.print("Unix time: ");
    written += dataFile.println(RTC_get_seconds());
    written += dataFile.write(data, len);
    dataFile.close();
    Serial.print(F("SD-card write success to "));
    Serial.print(filename);
    Serial.print(F(", \n  "));
    Serial.print(written);
    Serial.println(F(" bytes written"));
  }
  else
    Serial.println(F("SD-card write failed"));
}


void daq_read_last_file(void) {
  static int filecounter = 0;
  int j = filecounter;
  filename[3] = j / 10000;
  filename[3] += '0';
  j %= 10000;
  filename[4] = j / 1000;
  filename[4] += '0';
  j %= 1000;
  filename[5] = j / 100;
  filename[5] += '0';
  j %= 100;
  filename[6] = j / 10;
  filename[6] += '0';
  filename[7] = j % 10;
  filename[7] += '0';
  if (SD.exists(filename)) {
    Serial.print("Reading data from ");
    Serial.print(filename);
    Serial.println();
    File readFile = SD.open(filename, FILE_READ);
    if (readFile) {
      for (int i = 0; readFile.available(); i++) {
        Serial2.write(readFile.read());
      }
      Serial2.println("");
      readFile.close();
      filecounter++;
    }
    else
      Serial.println(F("SD-card read failed"));
  }
  else
    Serial.println(F("SD-card read failed, file does not exist."));
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
