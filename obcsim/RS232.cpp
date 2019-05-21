#include <string.h>

#include "Arduino.h"
#include "obcsim_transactions.hpp"
#include "obcsim_configuration.hpp"
#include "RTClib.h"
#include "RTC.hpp"
#include "RS232.hpp"

unsigned char commanddata[143];
static unsigned char recv_buff[REQUEST_BUFFER_SIZE];
static unsigned long recv_len=0;

static void fill_commanddata(int expected_len)
{
  int pos = 0;
  int len = 0;
  while (pos < expected_len) {
    len = Serial2.available();
    if (len) {
      Serial2.readBytes(commanddata + pos, len);
      pos += len;
      len = 0;
    }
  }  
}

void RS_init(void) {
  Serial2.begin(115200); //Set 115200 Baud rate, 8 bit 1 stop no parity
  while (!Serial2);
}

void RS_read(msp_link_t *lnk) {
  unsigned char command = Serial2.read();
  int len;
  char s[32];
  
  switch (command) {
    case CMD_CITIROC_CONF:
      Serial2.println("CMD_CITI_CONF received");
      Serial.println("-------- Invoking SEND_CITI_CONF -----\n");
      len = 143;
      fill_commanddata(len);
      invoke_send(lnk, MSP_OP_SEND_CITI_CONF, commanddata, len, BYTES);
      Serial.println("--------------------------------------\n");
      break;
    case CMD_PROBE_CONF:
      Serial2.println("CMD_PROBE_CONF received");
      Serial.println("-------- Invoking SEND_PROBE_CONF ----\n");
      len = 32;
      fill_commanddata(len);
      invoke_send(lnk, MSP_OP_SEND_PROBE_CONF, commanddata, len, BYTES);
      Serial.println("--------------------------------------\n");
      break;
    case CMD_HVPS_CONF:
      Serial2.println("CMD_HVPS_CONF received");
      Serial.println("-------- Invoking SEND_HVPS_CONF -----\n");
      len = 12;
      fill_commanddata(len);
      invoke_send(lnk, MSP_OP_SEND_HVPS_CONF, commanddata, len, BYTES);
      Serial.println("--------------------------------------\n");
      break;
    case CMD_REQ_HK:
      Serial2.println("CMD_HK_REQ received");

      // TODO: Is this really needed?
      // -->
      len = Serial2.available();
      if (len > 0)
        Serial2.readBytes(commanddata, len);
      // <--
      
      Serial.println("-------- Invoking REQ_HK -------------\n");
      invoke_request(lnk, MSP_OP_REQ_HK,recv_buff, &recv_len, STRING);
      Serial.println("--------------------------------------\n");
      RS_send(recv_buff, recv_len);
      break;
    case CMD_REQ_PAYLOAD:
      Serial.println("CMD_REQ_PAYLOAD");

      // TODO: Is this really needed?
      // -->
      len = Serial2.available();
      if (len > 0)
        Serial2.readBytes(commanddata, len);
      // <--
      
      Serial.println("-------- Invoking REQ_PAYLOAD --------\n");
      invoke_request(lnk, MSP_OP_REQ_PAYLOAD,recv_buff, &recv_len, STRING);
      Serial.println("--------------------------------------\n");
      RS_send(recv_buff, recv_len);
      break;
    default:
      sprintf(s, "Command '%c' not recognized \n", command);
      Serial2.println(s);
      break;
  }
}

void RS_send(unsigned char *sends, int len) {
  DateTime CurrentTime = RTC_get();
  Serial2.print("Unix time: ");
  Serial2.println(CurrentTime.unixtime());
  Serial2.write(sends, len);
  Serial2.println("");
}
