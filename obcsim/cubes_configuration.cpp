#include "Arduino.h"
#include "SDcard.hpp"
#include "obcsim_configuration.hpp"
#include "obcsim_transactions.hpp"
#include "RTC.hpp"
#include "RS232.hpp"
#include <stdlib.h>

static unsigned char data_SEND_HVPS_CONF[] = "0000000000000000746900C8";
static unsigned char data_SEND_PARAMS[] = {0x50, 0x00, 0x10, 0x22};
static unsigned char data_SEND_CITI_CONF[143];
static unsigned char data_SEND_PROB_CONF[32];
static unsigned char data_seqinit2[] = {0xAA, 0x10, 0x50, 0x30};
static char data_SEND_TIME[10] = "";
static char conf[12] = "conf.txt";
static char prob[12] = "probe.txt";

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
