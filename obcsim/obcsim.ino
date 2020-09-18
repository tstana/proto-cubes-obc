/*
   obcsim.ino

   Main file for the OBC simulator.
*/

#include "Arduino.h"
#include "msp_obc.h"
#include "daq.hpp"
#include "obcsim_configuration.hpp"
#include "obcsim_transactions.hpp"
#include "RS232.hpp"
#include "rtc.hpp"
#include "obcsim_transactions.hpp"
#include "obcsim_configuration.hpp"

#include <arduino-timer.h>

#define I2C_SPEED (400L*1000L)
#define I2C_TIMEOUT (100L*1000L)

#define CUBES_RESET_PIN   13

static msp_link_t exp_link;
static unsigned char exp_buf[EXP_MTU + 5];
static unsigned char recv_buf[REQUEST_BUFFER_SIZE];
static unsigned long recv_len = 0;

extern bool msp_i2c_error;

/* Send Unix time readout to CUBES */
static bool cubes_send_time_from_rtc(void*)
{
  unsigned char current_time[4];

    Serial.print(">>> @");
    Serial.println(rtc_get_seconds());

  Serial.println();
  Serial.println("Setting CUBES UTC time...");
  Serial.println("-------- Invoking SEND_TIME --------");
  to_bigendian32(current_time, rtc_get_seconds());
  invoke_send(&exp_link, MSP_OP_SEND_TIME, current_time, 4, BYTES);
  Serial.println("------------------------------------");

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
  Serial.println();
  Serial.print("Starting CUBES sync task at 30-second interval.");
  Serial.println();
  sync_task = sync_timer.every(30000, cubes_send_time_from_rtc);
}

// TODO: Remove this func.?
static void cubes_sync_tick()
{
  sync_timer.tick();
}

// TODO: Remove this func.?
static void cubes_sync_stop()
{
  Serial.println();
  Serial.println("Stopping CUBES sync task.");
  Serial.println();
  sync_timer.cancel(sync_task);
}

/* Reset CUBES board */
static void cubes_reset()
{
  const int dly = 100;

  Serial.print(">>> Asserting CUBES reset for "); Serial.print(dly);
  /*...*/ Serial.println(" ms!");
  digitalWrite(CUBES_RESET_PIN, LOW);
  delay(dly);
  digitalWrite(CUBES_RESET_PIN, HIGH);

  Serial.println(">>> CUBES start-up delay (1 second)...");
  delay(1000);

  Serial.println("-------- Invoking ACTIVE --------");
  invoke_syscommand(&exp_link, MSP_OP_ACTIVE);
  Serial.println("---------------------------------\n");

  cubes_send_time_from_rtc(NULL);
}


/* Arduino Setup */
void setup()
{
  /* Start up debug connection on programming USB port */
  Serial.begin(115200);
  Serial.println("-------------------------");
  Serial.println(" Proto-CUBES OBC started");
  Serial.println("-------------------------");

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
    
    Serial.println("***************************************");
    Serial.println("MSP I2C error...");
    Serial.println(">>> Re-starting Arduino I2C comms. ");
    msp_i2c_stop();
    msp_i2c_start(I2C_SPEED, I2C_TIMEOUT);
    Serial.println("***************************************");
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
    
    DateTime CurrentTime = rtc_get();
    Serial.print(">>> @");
    Serial.println(rtc_get().unixtime());

    /* Request new histogram data and write it to SD card */
    Serial.println("-------- Invoking REQ_PAYLOAD --------");
    invoke_request(&exp_link, MSP_OP_REQ_PAYLOAD, recv_buf, &recv_len, NONE);
    Serial.println("--------------------------------------");

    daq_write_new_file(recv_buf, recv_len);
    
    CurrentTime = rtc_get();
    Serial.print(">>> @");
    Serial.println(rtc_get().unixtime());
    
    /* Re-start DAQ */
    Serial.println("----- Invoking CUBES_DAQ_START -------");
    invoke_syscommand(&exp_link, MSP_OP_CUBES_DAQ_START);
    Serial.println("--------------------------------------");

    rtc_enable_timed_daq(true);
    //cubes_sync_start();
  }
}
