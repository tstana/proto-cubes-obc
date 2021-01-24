/*
   obcsim.ino

   Main file for the OBC simulator.
*/

#include <Arduino.h>
#include <arduino-timer.h>

#include "msp_obc.h"
#include "debug.hpp"
#include "daq.hpp"
#include "obcsim_configuration.hpp"
#include "obcsim_transactions.hpp"
#include "RS232.hpp"
#include "rtc.hpp"
#include "obcsim_transactions.hpp"
#include "obcsim_configuration.hpp"


#define I2C_SPEED (400L*1000L)
#define I2C_TIMEOUT (100L*1000L)

#define CUBES_RESET_PIN   13


static msp_link_t exp_link;
static unsigned char exp_buf[EXP_MTU + 5];
static unsigned char recv_buf[REQUEST_BUFFER_SIZE];
static unsigned long recv_len = 0;

extern bool msp_i2c_error;


/* Send Unix time readout to CUBES */
static bool cubes_send_rtc_time(void*)
{
  unsigned char current_time[4];

  DEBUG_PRINT("Syncing CUBES UTC time");
  DEBUG_PRINT("Invoking MSP_OP_SEND_TIME");
  to_bigendian32(current_time, rtc_get_seconds());
  invoke_send(&exp_link, MSP_OP_SEND_TIME, current_time, 4, BYTES);

  return true;
}

/*
 * CUBES timer sync
 */
static auto sync_timer = timer_create_default();
static uintptr_t sync_task;

// TODO (if no need for stopping the "sync task"):
//  (1) Move the contents of this func. inside cubes_reset()
//  (2) Remove "sync_task" variable.
static void cubes_sync_start()
{
  DEBUG_PRINT("Starting CUBES sync task at 30-second interval.");
  sync_task = sync_timer.every(30000, cubes_send_rtc_time);
}

// TODO: Remove this func.?
static void cubes_sync_tick()
{
  sync_timer.tick();
}

// TODO: Remove this func.?
static void cubes_sync_stop()
{
  DEBUG_PRINT("Stopping CUBES sync task.");
  sync_timer.cancel(sync_task);
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

  cubes_send_rtc_time(NULL);
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

  /* Start sync timer */
  cubes_sync_start();
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

  /* Tick CUBES sync timer */
  cubes_sync_tick();

  /* Process any incoming commands */
  if (Serial1.available()) {
    RS_read(&exp_link);
  }

  /* Finally, request payload on DAQ timeout (if DAQ is running) */
  if (rtc_timed_daq_enabled() && rtc_data_request_timeout()) {

    /* Stop sync and DAQ timers */
    //cubes_sync_stop();
    rtc_enable_timed_daq(false);
    
    /* Request new histogram data and write it to SD card */
    DEBUG_PRINT("Invoking MSP_OP_REQ_PAYLOAD");
    invoke_request(&exp_link, MSP_OP_REQ_PAYLOAD, recv_buf, &recv_len, NONE);

    daq_write_new_file(recv_buf, recv_len);
    
    /* Re-start DAQ */
    DEBUG_PRINT("Invoking MSP_OP_CUBES_DAQ_START");
    invoke_syscommand(&exp_link, MSP_OP_CUBES_DAQ_START);

    rtc_enable_timed_daq(true);
    //cubes_sync_start();
  }
}
