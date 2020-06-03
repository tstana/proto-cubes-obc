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

static void cubes_reset()
{
  const int dly = 100;

  Serial.print(">>> Asserting CUBES reset for "); Serial.print(dly);
  /*...*/ Serial.println(" ms!");
  digitalWrite(CUBES_RESET_PIN, LOW);
  delay(dly);
  digitalWrite(CUBES_RESET_PIN, HIGH);
}

/* Arduino Setup */
void setup()
{
  /* Init CUBES reset pin and assert CUBES reset */
  pinMode(CUBES_RESET_PIN, OUTPUT);
  cubes_reset();

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
}

/* Arduino Loop */
void loop()
{
  static bool initiated = false;

  if (!initiated) {
    sequence_init(&exp_link);
    initiated = true;
  }

  /* Assert CUBES reset in case MSP craps out with I2C error */
  if (msp_i2c_error) {
    cubes_reset();
    msp_i2c_error = false;
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
    invoke_syscommand(&exp_link, MSP_OP_CUBES_DAQ_START);

    rtc_enable_timed_daq(true);
  }
}
