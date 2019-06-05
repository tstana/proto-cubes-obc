#include <string.h>

#include "Arduino.h"
#include "obcsim_transactions.hpp"
#include "obcsim_configuration.hpp"
#include "RTClib.h"
#include "RTC.hpp"
#include "RS232.hpp"

unsigned char commanddata[143];
static unsigned char recv_buf[REQUEST_BUFFER_SIZE];
static unsigned long recv_len = 0;

static boolean fill_commanddata(int expected_len)
{
  int pos = 0;
  int len = 0;
  int TU  = 0;
  int transfer_size = 32;
  for (int i=0; i<expected_len/transfer_size; i++){
    len = Serial.available();
    if(len){
      if(len<transfer_size)
        TU = len;
      else
        TU = transfer_size;
      
      Serial.readBytes(commanddata + pos, TU);
      pos += TU;
      len = 0;
    }  
  }
  if(pos != expected_len){
     Serial.println("Data stream interrupted, command not executed");
     memset(commanddata, '\0', 143);
     return false;
    }
  /*while (pos < expected_len) {
    len = Serial.available();
    if (len) {
      Serial.readBytes(commanddata + pos, len);
      pos += len;
      len = 0;
    }
  }*/
  return true;
}

void RS_init(void) {
  Serial.begin(115200); //Set 115200 Baud rate, 8 bit 1 stop no parity
  while (!Serial);
}

void RS_read(msp_link_t *lnk) {
  unsigned char command = Serial.read();
  int len;
  char s[32];
  
  switch (command) {
    case CMD_SEND_CITIROC_CONF:
      Serial.println("CMD_SEND_CITI_CONF received");
      len = 143;
      if(fill_commanddata(len)){
        Serial.println("-------- Invoking SEND_CITI_CONF -----");
        invoke_send(lnk, MSP_OP_SEND_CITI_CONF, commanddata, len, BYTES);
        Serial.println("--------------------------------------");
      }
      break;
    case CMD_SEND_PROBE_CONF:
      Serial.println("CMD_SEND_PROBE_CONF received");
      len = 32;
      if(fill_commanddata(len)){
        Serial.println("-------- Invoking SEND_PROBE_CONF ----");
        invoke_send(lnk, MSP_OP_SEND_PROBE_CONF, commanddata, len, BYTES);
        Serial.println("--------------------------------------");
      }
      break;
    case CMD_SEND_HVPS_CONF:
      Serial.println("CMD_SEND_HVPS_CONF received");
      len = 12;
      if(fill_commanddata(len)){
        Serial.println("-------- Invoking SEND_HVPS_CONF -----");
        invoke_send(lnk, MSP_OP_SEND_HVPS_CONF, commanddata, len, BYTES);
        Serial.println("--------------------------------------");
      }
      break;
    case CMD_SEND_DAQ_DUR_AND_START:
      Serial.println("CMD_SEND_DAQ_DUR_AND_START received");
      len = 1;
      if(fill_commanddata(len)){
        Serial.println("-- Invoking SEND_DAQ_DUR_AND_START ---");
        invoke_send(lnk, MSP_OP_SEND_DAQ_DUR_AND_START, commanddata, len, BYTES);
        Serial.println("--------------------------------------");
      }
      break;
    case CMD_REQ_HK:
      Serial.println("CMD_HK_REQ received");

      // TODO: Is this really needed?
      // -->
      len = Serial.available();
      if (len > 0)
        Serial.readBytes(commanddata, len);
      // <--
      
      Serial.println("-------- Invoking REQ_HK -------------");
      invoke_request(lnk, MSP_OP_REQ_HK, recv_buf, &recv_len, STRING);
      Serial.println("--------------------------------------");
      RS_send(recv_buf, recv_len);
      break;
    case CMD_REQ_PAYLOAD:
      Serial.println("CMD_REQ_PAYLOAD");

      // TODO: Is this really needed?
      // -->
      len = Serial.available();
      if (len > 0)
        Serial.readBytes(commanddata, len);
      // <--
      
      Serial.println("-------- Invoking REQ_PAYLOAD --------");
      invoke_request(lnk, MSP_OP_REQ_PAYLOAD, recv_buf, &recv_len, STRING);
      Serial.println("--------------------------------------");
      RS_send(recv_buf, recv_len);
      break;
    default:
      sprintf(s, "Command '%c' not recognized \n", command);
      Serial.println(s);
      break;
  }
}

void RS_send(unsigned char *sends, int len) {
  DateTime CurrentTime = RTC_get();
  Serial.print("Unix time: ");
  Serial.println(CurrentTime.unixtime());
  Serial.write(sends, len);
  Serial.println("");
}
