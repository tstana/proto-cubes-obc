#include <Arduino.h>

#include <string.h>
#include <SPI.h>
#include <SD.h>

#include "daq.hpp"
#include "RTC.hpp"

#include "debug.hpp"

/* File name to store the last written DAQ file in... */
#define LAST_FILE_STOR  "_lastfil.txt"

/* ... and the local variable to refer to during operation */
static char last_file[13] = "daq0000.dat";

/*
 * Status flag to indicate whether a new file is availalbe for readout from
 * SD card
 */
static bool new_file_available = false;

/* Variables for DAQ time and status */
static uint8_t daq_dur_ardu = 0;
static uint32_t time_daq_start = 0;
static bool timed_daq_en = false;

static uint32_t time_sync = 0;
static const uint32_t SYNC_OFFSET = 30; // seconds

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

  DEBUG_PRINT("SD card initializing...");
  SD.begin(10); // Pin 4 on ethernet shield, pin 10 on SD-prototype board

  DEBUG_PRINT("Attempting to get the last written file...");

  if (SD.exists(LAST_FILE_STOR)) {
    File lf = SD.open(LAST_FILE_STOR, FILE_READ);
    lf.read(last_file, sizeof(last_file));
    lf.close();
  } else {
    sprintf(s, "Could not find a \"last DAQ\" file named %s", LAST_FILE_STOR);
    DEBUG_PRINT(s);
    DEBUG_PRINT("Attempting to find any files actually written to disk...");
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

  File lf = SD.open(LAST_FILE_STOR, O_WRITE | O_CREAT);
  if (lf) {
    lf.write(last_file);
    sprintf(s, "Found last written file %s", last_file);
    DEBUG_PRINT(s);
  } else {
    sprintf(s, "Unable to open %s for writing", LAST_FILE_STOR);
    DEBUG_PRINT(s);
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
    DEBUG_PRINT(s);

    File dataFile = SD.open(last_file, FILE_WRITE);
    if (dataFile)
    {
      int written = dataFile.write(data, len);
      dataFile.close();
      sprintf(s, "SD-card write success to %s, %d bytes written",
        last_file, written);
      DEBUG_PRINT(s);

      /* 
       * Write last file name to the dedicated file; dedicated file is created
       * if it does not exist.
       */
      File lf = SD.open(LAST_FILE_STOR, O_WRITE | O_CREAT);
      if (lf) {
        lf.write(last_file);
      } else {
        sprintf(s, "Unable to open %s for writing", LAST_FILE_STOR);
        DEBUG_PRINT(s);
      }
      lf.close();
    }
    else
    {
      should_decrem = true;
      sprintf(s, "Could not open %s for writing!", last_file);
      DEBUG_PRINT(s);
    }
  }
  else
  {
    should_decrem = true;
    sprintf(s, "Could not open %s, file already exists!", last_file);
    DEBUG_PRINT(s);
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
  char s[64];
  if (SD.exists(last_file)) {
    sprintf(s, "Reading data from %s", last_file);
    DEBUG_PRINT(s);
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
      DEBUG_PRINT("SD-card read failed");
  }
  else
    DEBUG_PRINT("SD-card read failed, file does not exist.");
}


bool daq_new_file_available()
{
  return new_file_available;
}


void daq_read_file(char location[12], unsigned char *buf)
{
  char s[32];
  File f = SD.open(location, FILE_READ);
  if (f) {
    for (int i = 0; f.available(); i++) {
      buf[i] = f.read();
    }
    sprintf(s, "SD-card read from %s", location);
    DEBUG_PRINT(s);
  }
  else
    DEBUG_PRINT("SD-card read failed");
  f.close();
}


int daq_delete_all_files(void)
{
  int deleted_files = 0;
  char s[32];

  while(SD.exists(last_file)){
    SD.remove(last_file);
    decrement_file_number(last_file);
    deleted_files++;
  }

  File root = SD.open("/");
  root.rewindDirectory();
  
  while(true) {
    File remaining_file = root.openNextFile();
    if(!remaining_file)
      break;
    String next_file = remaining_file.name();
    remaining_file.close();
    SD.remove(next_file);
    deleted_files++;
  }

  DEBUG_PRINT("All files removed from SD-card");
  sprintf(s, "Starting over from %s", last_file);
  DEBUG_PRINT(s);
  return deleted_files;
}


boolean daq_data_request_timeout(void)
{
  uint32_t now = rtc_get_time();
  uint32_t delta = now - time_daq_start;
  
  return (delta >= daq_dur_ardu) ? true : false;
}


boolean daq_sync_timeout(void)
{
  uint32_t now = rtc_get_time();
  uint32_t delta = time_sync - now;

  if (delta > SYNC_OFFSET) {
    time_sync = now + SYNC_OFFSET;
    return true;
  } else {
    return false;
  }
}


void daq_set_dur(uint8_t cubes_dur)
{
  daq_dur_ardu = cubes_dur + 1;
}


void daq_start_timers(void)
{
  /*
   * Set time_daq_start to the current time, so that data is requested from
   * the CUBES PCB after DAQ_DUR plus an offset to account for timing
   * mismatches. Also set the sync. time, so that periodic time synchronization
   * with CUBES is performed.
   * 
   * Then, set the "member" variable to indicate timed DAQ is running.
   */
  uint32_t now = rtc_get_time();

  time_daq_start = now;
  time_sync = now + SYNC_OFFSET;
  timed_daq_en = true;
}


void daq_stop_timers(void)
{
  /*
   * Set local times to 2^32-1 so that timed DAQ "never triggers"
   * while off. Also set "member" variable to indicate timed DAQ is
   * not running.
   */
  time_daq_start = UINT32_MAX;
  time_sync = UINT32_MAX;
  timed_daq_en = false;
}


bool daq_running(void)
{
  return timed_daq_en;
}
