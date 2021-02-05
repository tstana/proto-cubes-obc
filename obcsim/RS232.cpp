#include <string.h>
#include <Arduino.h>

#include "obcsim_transactions.hpp"
#include "obcsim_configuration.hpp"
#include "rtc.hpp"
#include "RS232.hpp"
#include "daq.hpp"
#include "debug.hpp"

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
  char s[64] = "";
  unsigned char obcsim_status = 0x00;

  switch (command) {
    /*-------- Send commands --------*/
    case CMD_SEND_TIME:
      DEBUG_PRINT("CMD_SEND_TIME received");
      len = 4;
      if (fill_commanddata(len)) {
        uint32_t timedata = from_bigendian32(commanddata);
        sprintf(s, "Attempting to program to RTC the received Unix time %d", timedata);
        DEBUG_PRINT(s);
        rtc_set_time(timedata);
        sprintf(s, "RTC time after programming is %d", rtc_get_time());
        DEBUG_PRINT(s);
        DEBUG_PRINT("Invoking MSP_OP_SEND_TIME");
        invoke_send(lnk, MSP_OP_SEND_TIME, commanddata, len, BYTES);
      }
      break;

    case CMD_SEND_CITIROC_CONF:
      DEBUG_PRINT("CMD_SEND_CITI_CONF received");
      len = 143;
      if (fill_commanddata(len)) {
        DEBUG_PRINT("Invoking MSP_OP_SEND_CUBES_CITI_CONF");
        invoke_send(lnk, MSP_OP_SEND_CUBES_CITI_CONF, commanddata, len, BYTES);
      }
      break;

    case CMD_SEND_PROBE_CONF:
      DEBUG_PRINT("CMD_SEND_PROBE_CONF received");
      len = 32;
      if (fill_commanddata(len)) {
        DEBUG_PRINT("Invoking MSP_OP_SEND_CUBES_PROBE_CONF");
        invoke_send(lnk, MSP_OP_SEND_CUBES_PROBE_CONF, commanddata, len, BYTES);
      }
      break;

    case CMD_SEND_READ_REG_DEBUG:
      DEBUG_PRINT("CMD_SEND_READ_REG_DEBUG received");
      len = 1;
      if (fill_commanddata(len)) {
        DEBUG_PRINT("Invoking MSP_OP_SEND_READ_REG_DEBUG");
        invoke_send(lnk, MSP_OP_SEND_READ_REG_DEBUG, commanddata, len, BYTES);
      }
      break;

    case CMD_SEND_HVPS_CONF:
      DEBUG_PRINT("CMD_SEND_HVPS_CONF received");
      len = 13;
      if (fill_commanddata(len)) {
        DEBUG_PRINT("Invoking MSP_OP_SEND_CUBES_HVPS_CONF");
        invoke_send(lnk, MSP_OP_SEND_CUBES_HVPS_CONF, commanddata, len, BYTES);
      }
      break;

    case CMD_SEND_HVPS_TMP_VOLT:
      DEBUG_PRINT("CMD_SEND_HVPS_TMP_VOLT received");
      len = 3;
      if (fill_commanddata(len)) {
        DEBUG_PRINT("Invoking MSP_OP_SEND_CUBES_HVPS_TMP_VOLT");
        invoke_send(lnk, MSP_OP_SEND_CUBES_HVPS_TMP_VOLT, commanddata, len, BYTES);
      }
      break;

    case CMD_SEND_DAQ_CONF:
      DEBUG_PRINT("CMD_SEND_DAQ_CONF received");
      len = 7;  // 1 byte for DAQ_DUR + 6 bytes for BIN_CFG
      if (fill_commanddata(len)) {
        DEBUG_PRINT("Invoking MSP_OP_SEND_CUBES_DAQ_CONF");
        invoke_send(lnk, MSP_OP_SEND_CUBES_DAQ_CONF, commanddata, len, BYTES);
        daq_set_dur(commanddata[0]);
      }
      break;

    case CMD_SEND_GATEWARE_CONF:
      DEBUG_PRINT("CMD_SEND_GATEWARE_CONF received");
      len = 1;
      if (fill_commanddata(len)) {
        DEBUG_PRINT("Invoking MSP_OP_SEND_CUBES_GATEWARE_CONF");
        invoke_send(lnk, MSP_OP_SEND_CUBES_GATEWARE_CONF, commanddata, len, BYTES);
      }
      break;

    /*-------- Request commands --------*/
    case CMD_REQ_ID:
      DEBUG_PRINT("CMD_REQ_ID received");
      Serial1.println(compilation);
      DEBUG_PRINT("Invoking MSP_OP_REQ_CUBES_ID");
      invoke_request(lnk, MSP_OP_REQ_CUBES_ID, recv_buf, &recv_len, BYTES);
      Serial1.print("CUBES: ");
      Serial1.write(recv_buf, recv_len);
      break;

    case CMD_REQ_HK:
      DEBUG_PRINT("CMD_REQ_HK received");
      len = Serial1.available();
      if (len > 0)
        Serial1.readBytes(commanddata, len); /* Flush incoming data buffer */

      DEBUG_PRINT("Invoking MSP_OP_REQ_HK");
      invoke_request(lnk, MSP_OP_REQ_HK, recv_buf, &recv_len, BYTES);
      sprintf(s, "Received HK, %d bytes", recv_len);
      DEBUG_PRINT("Received HK :");
      RS_send(recv_buf, recv_len);
      break;

    case CMD_REQ_STATUS:
      DEBUG_PRINT("CMD_REQ_STATUS received");
      /* Flush incoming data buffer */
      len = Serial1.available();
      if (len > 0)
        Serial1.readBytes(commanddata, len);
      /* ... and reply with status */
      obcsim_status =
        ((daq_new_file_available() ? 1 : 0) << 1) |
        (daq_running() ? 1 : 0);
      sprintf(s, "Sending obcsim_status %d", obcsim_status);
      DEBUG_PRINT(s);
      RS_send(&obcsim_status, 1);
      break;

    case CMD_REQ_PAYLOAD:
      DEBUG_PRINT("CMD_REQ_PAYLOAD received");

      len = Serial1.available(); /* Flush incoming data buffer */
      if (len > 0)
        Serial1.readBytes(commanddata, len);

      DEBUG_PRINT("Obtaining latest data from SD card");
      daq_read_last_file((char *)recv_buf, (int *)&recv_len);
      RS_send(recv_buf, recv_len);

      break;

    /*-------- System commands --------*/
    case CMD_DAQ_START:
      DEBUG_PRINT("CMD_DAQ_START received");
      DEBUG_PRINT("Invoking MSP_OP_CUBES_DAQ_START");
      invoke_syscommand(lnk, MSP_OP_CUBES_DAQ_START);
      daq_start_timers();
      break;

    case CMD_DAQ_STOP:
      DEBUG_PRINT("CMD_DAQ_STOP received");
      DEBUG_PRINT("Invoking MSP_OP_CUBES_DAQ_STOP");
      invoke_syscommand(lnk, MSP_OP_CUBES_DAQ_STOP);
      for (int i = 0; i < 1000; i++) ; /* Wait for gateware to receive command and finish */
      DEBUG_PRINT("Invoking MSP_OP_REQ_PAYLOAD");
      invoke_request(lnk, MSP_OP_REQ_PAYLOAD, recv_buf, &recv_len, NONE);
      daq_stop_timers();
      daq_write_new_file(recv_buf, recv_len);
      break;

    case CMD_DEL_FILES:
      {
        DEBUG_PRINT("CMD_DEL_FILES received");
        DEBUG_PRINT("Deleting ALL files on SD card...");
        int num_deleted = daq_delete_all_files();
        sprintf(s, "Done; %d files removed", num_deleted);
        DEBUG_PRINT(s);
        break;
      }


    /*-------- Should (ideally) never reach here --------*/
    default:
      sprintf(s, "Command '%c' (0x%02X) not recognized \n",
          command, command);
      DEBUG_PRINT(s);
      break;
  }
}


void RS_send(unsigned char *sends, int len)
{
  char s[64];
  long ut = rtc_get_time();

  sprintf(s, "Sending reply to REQ, %d bytes", len);
  DEBUG_PRINT(s);

  Serial1.print("Unix time: ");
  Serial1.println(ut);
  Serial1.write(sends, len);
  Serial1.println("");
}
