#include <string.h>

#include "Arduino.h"
#include "SDcard.hpp"
#include <SPI.h>
#include <SD.h>
#include "RTC.hpp"

static unsigned char conf[300];

static char last_file[13] = "daq0000.dat";

/**
 * increment_file_number()
 * 
 * \brief Increment the NNNN part of "daqNNNN.dat", handling overflow (9999 -> 0).
 * 
 * \param name  File name to apply the number to.
 */
static void increment_file_number(char *name)
{
  char s[4];
  int n;

  strncpy(s, &name[3], 4);
  n = strtol(s, NULL, 10);
  if (n == 9999)
    n = 0;
  else
    ++n;

  sprintf(name, "daq%04d.dat", n);
}

/**
 * decrement_file_number()
 * 
 * \brief Decrement the NNNN part of "daqNNNN.dat", handling overflow (0 -> 9999).
 * 
 * \param name  File name to apply the number to.
 */
static void decrement_file_number(char *name)
{
  char s[4];
  int n;

  strncpy(s, &name[3], 4);
  n = strtol(s, NULL, 10);
  if (n == 0)
    n = 9999;
  else
    --n;

  sprintf(name, "daq%04d.dat", n);
}


void daq_init(void)
{
  char s[64];
  int found_files = 0;
  int num;

  Serial.println("SD card initializing...");
  SD.begin(10); // Pin 4 on ethernet shield, pin 10 on SD-prototype board

  Serial.println("Attempting to get the last written file...");

  while (SD.exists(last_file)) {
    ++found_files;
    increment_file_number(last_file);
  }

  /*
   * Last increment will have left us on a non-existing file name -- fix by
   * decrementing.
   * 
   * This also handles the case when no files exist on the SD card, by first
   * underflowing the file number to 9999, which should be overflowed again to 0
   * on the first file write.
   */
  decrement_file_number(last_file);

  if (found_files)
  {
    sprintf(s, "Last written file found is %s.", last_file);
    Serial.println(s);
  }
  else
  {
    sprintf(s, "No previous DAQ files found. Starting from %s.", last_file);
    Serial.println(s);
  }
}


void daq_write_new_file(unsigned char *data, unsigned long len)
{
  char s[64];
  bool should_decrem = false;

  /*
   * Prep the next file name and attempt writing it.
   */
  increment_file_number(last_file);

  if (!SD.exists(last_file))
  {
    sprintf(s, "Writing data to new file %s...", last_file);
    Serial.println(s);

    File dataFile = SD.open(last_file, FILE_WRITE);
    if (dataFile)
    {
      int written = dataFile.write(data, len);
      dataFile.close();
      Serial.print(F("SD-card write success to "));
      Serial.print(last_file);
      Serial.print(F(", \n  "));
      Serial.print(written);
      Serial.println(F(" bytes written"));
    }
    else
    {
      should_decrem = true;
      sprintf(s, "Could not open %s for writing!", last_file);
      Serial.println(s);
    }
  }
  else
  {
    should_decrem = true;
    sprintf(s, "Could not open %s, file already exists!", last_file);
    Serial.println(s);
  }

  /*
   * Something went wrong, couldn't write new file. Make sure file order is kept
   * by decrementing file number.
   */
  if (should_decrem)
    decrement_file_number(last_file);
}


void daq_read_last_file(char *buf, int *recv_len)
{
  if (SD.exists(last_file)) {
    Serial.print("Reading data from ");
    Serial.print(last_file);
    Serial.println();
    File readFile = SD.open(last_file, FILE_READ);
    if (readFile) {
      int i;
      for (i = 0; readFile.available(); i++) {
        buf[i] = readFile.read();
      }
      *recv_len = i;
      readFile.close();
    }
    else
      Serial.println(F("SD-card read failed"));
  }
  else
    Serial.println(F("SD-card read failed, file does not exist."));
}


void daq_read_file(char location[12], unsigned char *buf)
{
  File confFile = SD.open(location, FILE_READ);
  if (confFile) {
    for (int i = 0; confFile.available(); i++) {
      buf[i] = confFile.read();
    }
    Serial.println(F("SD-card Read"));
  }
  else
    Serial.println(F("SD-card read failed"));
  confFile.close();
}
