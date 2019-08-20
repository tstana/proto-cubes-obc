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
#include "RTC.hpp"
#include "obcsim_transactions.hpp"
#include "obcsim_configuration.hpp"

#define I2C_SPEED (400L*1000L)
#define I2C_TIMEOUT (100L*1000L)

static msp_link_t exp_link;
static unsigned char exp_buf[EXP_MTU + 5];
static unsigned char recv_buf[REQUEST_BUFFER_SIZE];
static unsigned long recv_len = 0;

/* Arduino Setup */
void setup()
{
  /* Start up debug connection on programming USB port */
  //Serial.begin(115200);
  //Serial.println("-------------------------");
  //Serial.println(" Proto-CUBES OBC started");
  //Serial.println("-------------------------");

  /* Create link to the experiment & start I2C */
  exp_link = msp_create_link(EXP_ADDR, msp_seqflags_init(), exp_buf, EXP_MTU);
  msp_i2c_start(I2C_SPEED, I2C_TIMEOUT);

  /* Init RS-232 connection, RTC and Proto-CUBES DAQ */
  RS_init();
  RTC_init();
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

  sequence_loop(&exp_link);

  if (Serial1.available()) {
    RS_read(&exp_link);
  }

  if (RTC_timed_daq_enabled() && RTC_data_request_timer()) {
    //Serial.println("-------- Invoking REQ_PAYLOAD --------");
    invoke_request(&exp_link, MSP_OP_REQ_PAYLOAD, recv_buf, &recv_len, NONE);
    //Serial.println("--------------------------------------");
    daq_write_new_file(recv_buf, recv_len);
    invoke_syscommand(&exp_link, MSP_OP_CUBES_DAQ_START);
  }
}
