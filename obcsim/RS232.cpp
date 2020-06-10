#include <string.h>
#include "Arduino.h"
#include "obcsim_transactions.hpp"
#include "obcsim_configuration.hpp"
#include "rtc.hpp"
#include "RS232.hpp"
#include "daq.hpp"

unsigned char commanddata[143];
static unsigned char recv_buf[REQUEST_BUFFER_SIZE];
static unsigned long recv_len = 0;
static const char* compilation = "OBC: " __DATE__ "," __TIME__;

static boolean fill_commanddata(int expected_len)
{
  int pos = 0;
  int len = 0;
  int loops = 0;
  memset(commanddata, '\0', sizeof(commanddata));
  while (pos < expected_len && loops < 1000) {
    len = Serial1.available();
    if (len) {
      Serial1.readBytes(commanddata + pos, len);
      pos += len;
      len = 0;
      loops++;
    }
  }
  return true;
}


void RS_init(void)
{
  Serial1.begin(115200);
  while (!Serial1);
}

void RS_read(msp_link_t *lnk)
{
  unsigned char command = Serial1.read();
  int len;
  char s[32] = "";
  unsigned char obcsim_status = 0x00;

  Serial.println();

  switch (command) {
    case CMD_SEND_CITIROC_CONF:
      Serial.println("CMD_SEND_CITI_CONF received");
      len = 143;
      if (fill_commanddata(len)) {
        Serial.println("-------- Invoking SEND_CITI_CONF -----");
        invoke_send(lnk, MSP_OP_SEND_CUBES_CITI_CONF, commanddata, len, BYTES);
        Serial.println("--------------------------------------");
      }
      break;
    case CMD_SEND_PROBE_CONF:
      Serial.println("CMD_SEND_PROBE_CONF received");
      len = 32;
      if (fill_commanddata(len)) {
        Serial.println("-------- Invoking SEND_PROBE_CONF ----");
        invoke_send(lnk, MSP_OP_SEND_CUBES_PROBE_CONF, commanddata, len, BYTES);
        Serial.println("--------------------------------------");
      }
      break;
    case CMD_SEND_READ_REG_DEBUG:
      Serial.println("CMD_SEND_READ_REG_DEBUG received");
      len = 1;
      if (fill_commanddata(len)) {
        Serial.println("---- Invoking SEND_READ_REG_DEBUG ----");
        invoke_send(lnk, MSP_OP_SEND_READ_REG_DEBUG, commanddata, len, BYTES);
        Serial.println("--------------------------------------");
      }
      break;
    case CMD_SEND_HVPS_CONF:
      Serial.println("CMD_SEND_HVPS_CONF received");
      len = 13;
      if (fill_commanddata(len)) {
        Serial.println("---- Invoking SEND_CUBES_HVPS_CONF ---");
        invoke_send(lnk, MSP_OP_SEND_CUBES_HVPS_CONF, commanddata, len, BYTES);
        Serial.println("--------------------------------------");
      }
      break;
    case CMD_SEND_HVPS_TMP_VOLT:
      Serial.println("CMD_SEND_HVPS_TMP_VOLT received");
      len = 3;
      if (fill_commanddata(len)) {
        Serial.println("-- Invoking SEND_CUBES_HVPS_TMP_VOLT -");
        invoke_send(lnk, MSP_OP_SEND_CUBES_HVPS_TMP_VOLT, commanddata, len, BYTES);
        Serial.println("--------------------------------------");
      }
      break;
    case CMD_SEND_DAQ_DUR:
      Serial.println("CMD_SEND_DAQ_DUR received");
      len = 1;
      if (fill_commanddata(len)) {
        Serial.println("---- Invoking SEND_CUBES_DAQ_DUR -----");
        invoke_send(lnk, MSP_OP_SEND_CUBES_DAQ_DUR, commanddata, len, BYTES);
        rtc_set_daq_time(commanddata[0]); /* Update timer in arduino code */
        Serial.println("--------------------------------------");
      }
      break;
    case CMD_SEND_GATEWARE_CONF:
      Serial.println("CMD_SEND_GATEWARE_CONF received");
      len = 1;
      if (fill_commanddata(len)) {
        Serial.println("-- Invoking SEND_CUBES_GATEWARE_CONF ---");
        invoke_send(lnk, MSP_OP_SEND_CUBES_GATEWARE_CONF, commanddata, len, BYTES);
      }
      break;
    case CMD_DAQ_START:
      Serial.println("CMD_DAQ_START received");
      Serial.println("----- Invoking CUBES_DAQ_START -------");
      invoke_syscommand(lnk, MSP_OP_CUBES_DAQ_START);
      Serial.println("--------------------------------------");
      rtc_enable_timed_daq(true);
      break;
    case CMD_DAQ_STOP:
      Serial.println("CMD_DAQ_STOP received");
      invoke_syscommand(lnk, MSP_OP_CUBES_DAQ_STOP);
      for (int i = 0; i < 1000; i++) ; /* Wait for gateware to receive command and finish */
      Serial.println("------- Invoking REQ_PAYLOAD ---------");
      invoke_request(lnk, MSP_OP_REQ_PAYLOAD, recv_buf, &recv_len, NONE);
      Serial.println("--------------------------------------");
      rtc_enable_timed_daq(false);
      daq_write_new_file(recv_buf, recv_len);
      break;
    case CMD_DEL_FILES:
      {
        Serial.println("CMD_DEL_FILES received, file deletion in progress");
        int deleted = daq_delete_all_files();
        Serial.print(deleted);
        Serial.println(" files removed.");
        break;
      }
    case CMD_REQ_HK:
      Serial.println("CMD_REQ_HK received");
      len = Serial1.available();
      if (len > 0)
        Serial1.readBytes(commanddata, len); /* Flush incoming data buffer */

      Serial.println("-------- Invoking REQ_HK -------------");
      invoke_request(lnk, MSP_OP_REQ_HK, recv_buf, &recv_len, BYTES);
      Serial.println("--------------------------------------");
      Serial.print("Received HK :");
      Serial.println(recv_len);
      RS_send(recv_buf, recv_len);
      break;
    case CMD_REQ_STATUS:
      Serial.println("CMD_REQ_STATUS received");
      /* Flush incoming data buffer */
      len = Serial1.available();
      if (len > 0)
        Serial1.readBytes(commanddata, len);
      obcsim_status =
        ((daq_new_file_available() ? 1 : 0) << 1) |
        (rtc_timed_daq_enabled() ? 1 : 0);
      RS_send(&obcsim_status, 1);
      break;
    case CMD_REQ_PAYLOAD:
      Serial.println("CMD_REQ_PAYLOAD received");

      len = Serial1.available(); /* Flush incoming data buffer */
      if (len > 0)
        Serial1.readBytes(commanddata, len);

      Serial.println("  Obtaining latest data from SD card...");
      daq_read_last_file((char *)recv_buf, (int *)&recv_len);
      RS_send(recv_buf, recv_len);

      break;
    case CMD_REQ_ID:
      Serial.println("CMD_REQ_ID received");
      Serial1.println(compilation);
      invoke_request(lnk, MSP_OP_REQ_CUBES_ID, recv_buf, &recv_len, BYTES);
      Serial1.print("CUBES: ");
      Serial1.write(recv_buf, recv_len);
      break;
    case CMD_SEND_TIME:
      len = 4;
      if (fill_commanddata(len)) {
        uint32_t timedata = from_bigendian32(commanddata);
        Serial.print("Received unix time: ");
        Serial.println(timedata);
        rtc_set_time(timedata);
        Serial.print("Now programmed: ");
        Serial.println(rtc_get_seconds());
        Serial.println("-------- Invoking SEND_TIME --------");
        invoke_send(lnk, MSP_OP_SEND_TIME, commanddata, len, BYTES);
        Serial.println("------------------------------------\n");
      }
      break;
    default:
      sprintf(s, "Command '%c' (0x%02X) not recognized \n", command, command);
      Serial.println(s);
      break;
  }
}

void RS_send(unsigned char *sends, int len)
{
  DateTime CurrentTime = rtc_get();
  Serial1.print("Unix time: ");
  Serial1.println(CurrentTime.unixtime());
  Serial1.write(sends, len);
  Serial1.println("");
}
