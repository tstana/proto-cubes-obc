#include "Arduino.h"
#include "obcsim_transactions.hpp"
#include "obcsim_configuration.hpp"
#include "RTClib.h"
#include "RTC.hpp"
#include "RS232.hpp"
unsigned char commanddata[143];
int len = 0;
static unsigned char recv_buff[REQUEST_BUFFER_SIZE];
static unsigned long recv_len=0;
void RS_init(void) {
  Serial2.begin(115200); //Set 115200 Baud rate, 8bit 1stop no parity
  while (!Serial2);

}

void RS_read(msp_link_t *lnk) {
  unsigned char command = Serial2.read();
  switch (command) {
    case 'a': /* CITIROC CONF */
      Serial.println("CITI_CONF received");
      len = Serial2.available();
      Serial2.readBytes(commanddata, len);
      invoke_send(lnk, MSP_OP_SEND_CITI_CONF, commanddata, len, NONE);
      break;
    case 'b': /* HVPS CONF */
      Serial.println("HVPS_CONF received");
      len = Serial2.available();
      Serial2.readBytes(commanddata, len);
      invoke_send(lnk, MSP_OP_SEND_HVPS_CONF, commanddata, len, NONE);
      break;
    case 'c': /* REQUEST HOUSEKEEPING */
      Serial.println("HK_REQ received");
      len = Serial2.available();
      if (len > 0)
        Serial2.readBytes(commanddata, len);
      invoke_request(lnk, MSP_OP_REQ_HK,recv_buff, &recv_len, STRING);
      RS_send(recv_buff, recv_len);
      break;
    case 'd': /* Request payload */
      Serial.println("Payload request received");
      len = Serial2.available();
      if (len > 0)
        Serial2.readBytes(commanddata, len);
      invoke_request(lnk, MSP_OP_REQ_PAYLOAD,recv_buff, &recv_len, STRING);
      RS_send(recv_buff, recv_len);
      break;
    default:
      Serial2.println("Command not recognized");
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
