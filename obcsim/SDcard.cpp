#include <string.h>

#include "Arduino.h"
#include "SDcard.hpp"
#include <SPI.h>
#include <SD.h>
#include "RTC.hpp"

static unsigned char conf[300];

void daq_init(void)
{
  char filename[13] = "none";
  
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


void daq_write_new_file(unsigned char *data, unsigned long len)
{
  /* Retrieve last stored file name... */
  File lastfile = SD.open("lastfile.txt", FILE_WRITE);
  char filename[13];
  if (lastfile) {
        Serial.print("1: pos = "); Serial.println(lastfile.position());
    lastfile.seek(0);
        Serial.print("2: pos = "); Serial.println(lastfile.position());
    lastfile.read(filename, 12);
  } else {
    Serial.println("daq_write_new_file(): Could not open lastfile.txt!");
    return;
  }

  /* ... add 1 to it, unless none exists, in which case start from 0... */
  char s[64];
  long int i = 0;
  if (strcmp(filename, "none") != 0) {
    strncpy(s, &filename[3], 5);
    i = 1 + strtol(s, NULL, 10);
  }
  sprintf(filename, "daq%05d.dat", i);
      Serial.print("3: pos = "); Serial.println(lastfile.position());
  lastfile.seek(0);
      Serial.print("4: pos = "); Serial.println(lastfile.position());
  lastfile.write(filename);
      Serial.print("5: pos = "); Serial.println(lastfile.position());
  lastfile.flush();
  lastfile.close();

  /* ... and write! */
  File dataFile;
  if (!SD.exists(filename)) {
    sprintf(s, "Writing data to new file %s", filename);
    Serial.println(s);
    dataFile = SD.open(filename, FILE_WRITE);
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
    } else {
      sprintf(s, "Could not open %s for writing!", filename);
      Serial.println(s);
    }
  } else {
    sprintf(s, "Could not open %s, file already exists!", filename);
    Serial.println(s);
  }
}


void daq_read_last_file(void)
{
  char filename[13];
  
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
