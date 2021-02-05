/*
   obcsim.ino

   Main file for the CUBES OBC simulator.
*/

#include <Arduino.h>

#include "msp_obc.h"
#include "obcsim_transactions.hpp"
#include "obcsim_configuration.hpp"

#include "debug.hpp"
#include "daq.hpp"
#include "RS232.hpp"
#include "rtc.hpp"


#define I2C_SPEED (400L*1000L)
#define I2C_TIMEOUT (100L*1000L)

#define CUBES_RESET_PIN   13


static msp_link_t exp_link;
static unsigned char exp_buf[EXP_MTU + 5];
static unsigned char recv_buf[REQUEST_BUFFER_SIZE];
static unsigned long recv_len = 0;

extern bool msp_i2c_error;


/* Send Unix time readout to CUBES */
static void cubes_send_rtc_time()
{
  unsigned char current_time[4];

  DEBUG_PRINT("Syncing CUBES UTC time");
  DEBUG_PRINT("Invoking MSP_OP_SEND_TIME");
  to_bigendian32(current_time, rtc_get_time());
  invoke_send(&exp_link, MSP_OP_SEND_TIME, current_time, 4, BYTES);
}


/* Reset CUBES board */
static void cubes_reset()
{
  const int dly = 100;

  char s[32];

  sprintf(s, "Asserting CUBES reset for %d ms", dly);
  DEBUG_PRINT(s);
  digitalWrite(CUBES_RESET_PIN, LOW);
  delay(dly);
  digitalWrite(CUBES_RESET_PIN, HIGH);

  DEBUG_PRINT("Initiating CUBES start-up delay, 1 second");
  delay(1000);

  DEBUG_PRINT("Invoking ACTIVE (MSP)");
  invoke_syscommand(&exp_link, MSP_OP_ACTIVE);

  cubes_send_rtc_time();
}


/* Arduino Setup */
void setup()
{
  /* Start up debug connection on programming USB port */
  debug_init();

  DEBUG_PRINT("Proto-CUBES OBC started");

  /* Create link to the experiment & start I2C */
  exp_link = msp_create_link(EXP_ADDR, msp_seqflags_init(), exp_buf, EXP_MTU);
  msp_i2c_start(I2C_SPEED, I2C_TIMEOUT);

  /* Init RS-232 connection, RTC and Proto-CUBES DAQ */
  RS_init();
  rtc_init();
  daq_init();

  /* Init CUBES reset pin and assert CUBES reset */
  pinMode(CUBES_RESET_PIN, OUTPUT);
  cubes_reset();
}


/* Arduino Loop */
void loop()
{
  static bool initiated = false;

  if (!initiated) {
    sequence_init(&exp_link);
    initiated = true;
  }

  /* Re-init the I2C communication on '-9' error. */
  if (msp_i2c_error) {
    msp_i2c_error = false;
    
    DEBUG_PRINT("MSP I2C error, re-starting Arduino I2C comms");
    msp_i2c_stop();
    msp_i2c_start(I2C_SPEED, I2C_TIMEOUT);
  }

  /* Process any incoming commands */
  if (Serial1.available()) {
    RS_read(&exp_link);
  }

  /* Finally, request payload on DAQ timeout (if DAQ is running) */
  if (daq_running()) {
    if (daq_data_request_timeout()) {
      daq_stop_timers();
      
      /* Request new histogram data and write it to SD card */
      DEBUG_PRINT("Invoking MSP_OP_REQ_PAYLOAD");
      invoke_request(&exp_link, MSP_OP_REQ_PAYLOAD, recv_buf, &recv_len, NONE);

      daq_write_new_file(recv_buf, recv_len);
      
      /* Re-start DAQ */
      DEBUG_PRINT("Invoking MSP_OP_CUBES_DAQ_START");
      invoke_syscommand(&exp_link, MSP_OP_CUBES_DAQ_START);

      daq_start_timers();
    } else if (daq_sync_timeout()) {
      cubes_send_rtc_time();
    }
  }
}
