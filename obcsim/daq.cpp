#include "daq.hpp"

#include "Arduino.h"

#include <string.h>
#include <SPI.h>
#include <SD.h>

#include "RTC.hpp"

/* File name to store the last DAQ in... */
#define LAST_FILE_FILE  "_lastfil.txt"

/* ... and the local variable to refer to during operation */
static char last_file[13] = "daq0000.dat";

/* Status flag for... */
static bool new_file_available = false;

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

  Serial.println("SD card initializing...");
  SD.begin(10); // Pin 4 on ethernet shield, pin 10 on SD-prototype board

  Serial.println("Attempting to get the last written file...");

  if (SD.exists(LAST_FILE_FILE)) {
    File lf = SD.open(LAST_FILE_FILE, FILE_READ);
    lf.read(last_file, sizeof(last_file));
    lf.close();
  } else {
    Serial.print("  Note: "); Serial.print(LAST_FILE_FILE);
      Serial.println(" does not exist... ");
      Serial.println("  Attempting to find any files actually written to disk...");
  }

  /*
   * In case the "last file" file does not exist - or for some reason the DAQ
   * file stored in this txt file is wrong, try to find the last written DAQ
   * file. First increment the file number and see if the file exists. If it
   * does, repeat the process until the file does not exist. Finally, decrement
   * the file number [daq_write_new_file() increments it when it enters] and
   * write to the "last file" file.
   */
  increment_file_number(last_file);
  while (SD.exists(last_file)) {
    increment_file_number(last_file);
  }
  decrement_file_number(last_file);

  File lf = SD.open(LAST_FILE_FILE, O_WRITE | O_CREAT);
  if (lf) {
    lf.write(last_file);
    sprintf(s, "Last written file found is %s.", last_file);
    Serial.println(s);
  } else {
    Serial.print("Unable to open "); Serial.print(LAST_FILE_FILE);
      Serial.println(" for writing!");
  }
  lf.close();
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

      /* 
       * Write last file name to the dedicated file; dedicated file is created
       * if it does not exist.
       */
      File lf = SD.open(LAST_FILE_FILE, O_WRITE | O_CREAT);
      if (lf) {
        lf.write(last_file);
      } else {
        Serial.print("Unable to open "); Serial.print(LAST_FILE_FILE);
          Serial.println(" for writing!");
      }
      lf.close();
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
  else
    new_file_available = true;
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
      new_file_available = false;
    }
    else
      Serial.println(F("SD-card read failed"));
  }
  else
    Serial.println(F("SD-card read failed, file does not exist."));
}


bool daq_new_file_available()
{
  return new_file_available;
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


int daq_delete_all_files(void)
{
  int deleted_files = 0;
  while(SD.exists(last_file)){
    SD.remove(last_file);
    decrement_file_number(last_file);
    deleted_files++;
  }
  File root = SD.open("/");
  root.rewindDirectory();
  while(true){
    File remaining_file = root.openNextFile();
    if(!remaining_file)
      break;
    String next_file = remaining_file.name();
    remaining_file.close();
    SD.remove(next_file);
    deleted_files++;
  }
  Serial.println("All files removed from SD-card");
  Serial.print("Starting over from ");
  Serial.println(last_file);
  return deleted_files;
}
