#include "Arduino.h"
#include "SDcard.hpp"
#include "obcsim_configuration.hpp"
#include "obcsim_transactions.hpp"
#include "RTC.hpp"
#include "RS232.hpp"
#include <stdlib.h>

static char data_SEND_TIME[10] = "";

void sequence_init(msp_link_t *lnk)
{
  delay(5000);
  Serial.println("-------- Invoking ACTIVE --------");
  invoke_syscommand(lnk, MSP_OP_ACTIVE);
  //SD_read(data_SEND_CITI_CONF, conf);
  //SD_read(data_SEND_PROB_CONF, prob);
  Serial.println("---------------------------------\n");
  Serial.println("-------- Invoking SEND_TIME --------");
  ltoa(RTC_get_seconds(), data_SEND_TIME, 10);
  invoke_send(lnk, MSP_OP_SEND_TIME, (unsigned char *) data_SEND_TIME, sizeof(data_SEND_TIME), BYTES);
  Serial.println("------------------------------------\n");
  delay(2000);
}

void sequence_loop(msp_link_t *lnk)
{
//    Serial.println("-------- Invoking SEND_CITI_CONF --------");
//    invoke_send(lnk, MSP_OP_SEND_CITI_CONF, data_SEND_CITI_CONF, sizeof(data_SEND_CITI_CONF), NONE);
//    Serial.println("-----------------------------------------\n");
//    delay(5000);
//
//    if(RTC_time_since_last(REQUEST_TIME)){
//      Serial.println("-------- Invoking REQ_PAYLOAD --------");
//      invoke_request(lnk, MSP_OP_REQ_PAYLOAD, recv_buf, &buf_len, STRING);
//      RS_send(recv_buf, buf_len);
//      SD_send(recv_buf, buf_len);
//      Serial.println("--------------------------------------\n");
//    }
}
