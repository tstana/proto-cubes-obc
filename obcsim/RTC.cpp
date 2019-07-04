//#include "RTClib.h"
#include "RTC.hpp"

RTC_PCF8523 rtc;
static int timer_request = 30;
static long timer_last = 0;

void RTC_init(void) {
  Serial.println("RTC initializing");
  if (!rtc.begin()) {
    Serial.println("RTC could not be found");
  }
  if (!rtc.initialized()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    DateTime now = rtc.now();
    Serial.print("RTC set to: ");
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(", ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    Serial.print("Unix time: ");
    Serial.println(now.unixtime());
    timer_last = RTC_get_seconds();
  }
}

DateTime RTC_get(void) {
  return rtc.now();
}

long RTC_get_seconds(void) {
  return (rtc.now()).unixtime();
}

boolean RTC_time_since_last(int delaytime) {
  long time_now = 0;
  static long time_last = 0;
  time_now = RTC_get_seconds();
  int time_since = (int) (time_now - time_last);
  if (time_since > delaytime) {
    time_last = time_now;
    return true;
  }
  return false;
}


boolean RTC_data_request_timer(void) {
  long time_now = RTC_get_seconds();
  int timer_delta = time_now - timer_last;
  if (timer_delta >= timer_request) {
    timer_last = time_now;
    return true;
  }
  return false;
}

void RTC_change_timer(int request_timer) {
  timer_request = request_timer + 1;
}

void RTC_switch_timer_status(char bitflip) {
  static char timerstatus = 0;
  static int  currentdur = 0;
  if (bitflip = 0 && timerstatus == 1) {
    timer_last = 4294967295; /* Set timer_last to 2^32-1 to that it never triggers while off */
    timerstatus = 0;
  }
  else if (bitflip = 1 && timerstatus == 0) {
    timerstatus = 1;
    timer_last = RTC_get_seconds(); /* Sets timer to start at delta = 0, so that it requests data after DAQ_DUR+1 seconds */
  }
}
