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
  int loops = 0;
  memset(commanddata, '\0', sizeof(commanddata));
  while (pos < expected_len && loops <1000) {
    len = Serial2.available();
    if (len) {
      Serial2.readBytes(commanddata + pos, len);
      pos += len;
      len = 0;
      loops++;
    }
  }
  return true;
}


void RS_init(void)
{
  Serial2.begin(115200);
  while (!Serial2);
}

void RS_read(msp_link_t *lnk)
{
  unsigned char command = Serial2.read();
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
      len = Serial2.available();
      if (len > 0)
        Serial2.readBytes(commanddata, len);
      // <--
      
      Serial.println("-------- Invoking REQ_HK -------------");
      invoke_request(lnk, MSP_OP_REQ_HK, recv_buf, &recv_len, STRING);
      Serial.println("--------------------------------------");
      RS_send(recv_buf, recv_len);
      break;
    case CMD_REQ_PAYLOAD:
    {                                                 // <<<<< TODO: Remove me!
      Serial.println("CMD_REQ_PAYLOAD received");

      // TODO: Is this really needed?
      // -->
      Serial.println("CMD_REQ_PAYLOAD");
      len = Serial2.available();
      if (len > 0)
        Serial2.readBytes(commanddata, len);
      // <--
      
      Serial.println("-------- Invoking REQ_PAYLOAD --------");
      //invoke_request(lnk, MSP_OP_REQ_PAYLOAD, recv_buf, &recv_len, STRING);

      // TODO: Remove from here --->
      /* Fill in histogram data */
      uint16_t i = 0;
      uint16_t val = 0;

      // Histogram header
      recv_buf[0] = (unsigned char)'C';
      recv_buf[1] = (unsigned char)'1';
      for (i = 2; i < 254; ++i) {
        recv_buf[i] = 0;
      }
      val = 2048;
      recv_buf[254] = (val >> 8) & 0xFF;
      recv_buf[255] = val & 0xFF;

      // Histogram data
      for (int j = 0; j < 6; ++j) {
        val = 0;
        for (i = 0; i < 2048; ++i) {
          recv_buf[256 + 4096*j + i*2    ] = (val >> 8) & 0xFF;
          recv_buf[256 + 4096*j + i*2 + 1] = val & 0xFF;
          val += 32;
        }
      }

      // Print data
      // uint16_t histo[REQUEST_BUFFER_SIZE/2];
      // for (i = 0; i < REQUEST_BUFFER_SIZE/2; ++i) {
      //   histo[i] = (recv_buf[i*2] << 8) | (recv_buf[i*2 + 1]);
      //   sprintf(s, "histo[%d] = % 5d", i, histo[i]);
      //   Serial.println(s);
      // }

      recv_len = REQUEST_BUFFER_SIZE-25; //-25 to allow for "Unix time: xxxxxxx\r\n - DATADATADATA - \r\n"
      // TODO: Remove to here <---

      Serial.println("--------------------------------------");
      RS_send(recv_buf, recv_len);
      break;
    }                                                 // <<<<< TODO: Remove me!
    default:
      sprintf(s, "Command '%c' (0x%02X) not recognized \n", command, command);
      Serial.println(s);
      break;
    memset(recv_buf, '\0', sizeof(recv_len));
  }
}

void RS_send(unsigned char *sends, int len)
{
  DateTime CurrentTime = RTC_get();
  Serial2.print("Unix time: ");
  Serial2.println(CurrentTime.unixtime());
  Serial2.write(sends, len);
  Serial2.println("");
}
