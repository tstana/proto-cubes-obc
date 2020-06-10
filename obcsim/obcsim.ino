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

#define I2C_SPEED (400L*1000L)
#define I2C_TIMEOUT (100L*1000L)

#define CUBES_RESET_PIN   13

static msp_link_t exp_link;
static unsigned char exp_buf[EXP_MTU + 5];
static unsigned char recv_buf[REQUEST_BUFFER_SIZE];
static unsigned long recv_len = 0;

extern bool msp_i2c_error;


/* Reset CUBES board */
static void cubes_reset()
{
  const int dly = 100;
  unsigned char current_time[4];

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

  Serial.println("-------- Invoking SEND_TIME --------");
  to_bigendian32(current_time, rtc_get_seconds());
  invoke_send(&exp_link, MSP_OP_SEND_TIME, current_time, 4, BYTES);
  Serial.println("------------------------------------");
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
}


/* Arduino Loop */
void loop()
{
  static bool initiated = false;

  if (!initiated) {
    sequence_init(&exp_link);
    initiated = true;
  }

  /*
   * In case MSP throws I2C error, assert CUBES reset and restart DAQ if it was
   * running. The DAQ time also needs to be resent to CUBES, since the reset
   * cleared anything already stored.
   * TODO: Add comment on Arduino side of "reset"...
   */
  if (msp_i2c_error) {
    msp_i2c_error = false;
    
    Serial.println("***************************************");
    Serial.println("MSP I2C error...");
    Serial.println(">>> Re-starting Arduino I2C comms. ");
    msp_i2c_stop();
    msp_i2c_start(I2C_SPEED, I2C_TIMEOUT);

    cubes_reset();

    if (rtc_timed_daq_enabled()) {
      rtc_enable_timed_daq(false);
      Serial.println("---- Invoking SEND_CUBES_DAQ_DUR -----");
      uint8_t daq_time = rtc_get_daq_time();
      invoke_send(&exp_link, MSP_OP_SEND_CUBES_DAQ_DUR, &daq_time, 1, BYTES);
      Serial.println("--------------------------------------");
      Serial.println("----- Invoking CUBES_DAQ_START -------");
      invoke_syscommand(&exp_link, MSP_OP_CUBES_DAQ_START);
      Serial.println("--------------------------------------");
      rtc_enable_timed_daq(true);
    }

    Serial.println("***************************************");
  }

  /* Process any incoming commands */
  if (Serial1.available()) {
    RS_read(&exp_link);
  }

  /* Finally, request payload on DAQ timeout (if DAQ is running) */
  if (rtc_timed_daq_enabled() && rtc_data_request_timeout()) {

    rtc_enable_timed_daq(false);
    
    Serial.println("-------- Invoking REQ_PAYLOAD --------");
    invoke_request(&exp_link, MSP_OP_REQ_PAYLOAD, recv_buf, &recv_len, NONE);
    Serial.println("--------------------------------------");
    daq_write_new_file(recv_buf, recv_len);
    Serial.println("----- Invoking CUBES_DAQ_START -------");
    invoke_syscommand(&exp_link, MSP_OP_CUBES_DAQ_START);
    Serial.println("--------------------------------------");

    rtc_enable_timed_daq(true);
  }
}
