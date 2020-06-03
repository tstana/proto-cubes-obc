#include "Arduino.h"
#include "daq.hpp"
#include "obcsim_configuration.hpp"
#include "obcsim_transactions.hpp"
#include "RTC.hpp"
#include "RS232.hpp"
#include <stdlib.h>

static char data_SEND_TIME[10] = "";

void sequence_init(msp_link_t *lnk)
{
  Serial.println("-------- Invoking ACTIVE --------");
  invoke_syscommand(lnk, MSP_OP_ACTIVE);
  Serial.println("---------------------------------\n");
}

void sequence_loop(msp_link_t *lnk)
{
}
