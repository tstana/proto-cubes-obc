//#include "RTClib.h"
#include "RTC.hpp"

RTC_PCF8523 rtc;

void RTC_init(void) {
  if (!rtc.begin()) {
    Serial2.println("RTC could not be found");
  }
  if (!rtc.initialized()) {
    Serial2.println("RTC initializing");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    DateTime now = rtc.now();
    Serial2.print("RTC set to: ");
    Serial2.print(now.year(), DEC);
    Serial2.print('/');
    Serial2.print(now.month(), DEC);
    Serial2.print('/');
    Serial2.print(now.day(), DEC);
    Serial2.print(", ");
    Serial2.print(now.hour(), DEC);
    Serial2.print(':');
    Serial2.print(now.minute(), DEC);
    Serial2.print(':');
    Serial2.print(now.second(), DEC);
    Serial2.println();
    Serial2.print("Unix time: ");
    Serial2.println(now.unixtime());
  }
}

DateTime RTC_get(void) {
  return rtc.now();
}

long RTC_get_seconds(void){
  return (rtc.now()).unixtime();
}

boolean RTC_time_since_last(void){
  long time_now=0;
  static long time_last = 0;
  time_now =RTC_get_seconds();
  int time_since = time_now-time_last;
  if(time_since > REQUEST_TIME){
    time_last = time_now;
    return true;
  }
  return false;
}
