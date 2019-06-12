//#include "RTClib.h"
#include "RTC.hpp"

RTC_PCF8523 rtc;

void RTC_init(void) {
  SerialUSB.println("RTC initializing");
  if (!rtc.begin()) {
    SerialUSB.println("RTC could not be found");
  }
  if (!rtc.initialized()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    DateTime now = rtc.now();
    SerialUSB.print("RTC set to: ");
    SerialUSB.print(now.year(), DEC);
    SerialUSB.print('/');
    SerialUSB.print(now.month(), DEC);
    SerialUSB.print('/');
    SerialUSB.print(now.day(), DEC);
    SerialUSB.print(", ");
    SerialUSB.print(now.hour(), DEC);
    SerialUSB.print(':');
    SerialUSB.print(now.minute(), DEC);
    SerialUSB.print(':');
    SerialUSB.print(now.second(), DEC);
    SerialUSB.println();
    SerialUSB.print("Unix time: ");
    SerialUSB.println(now.unixtime());
  }
}

DateTime RTC_get(void) {
  return rtc.now();
}

long RTC_get_seconds(void){
  return (rtc.now()).unixtime();
}

boolean RTC_time_since_last(int delaytime){
  long time_now=0;
  static long time_last = 0;
  time_now =RTC_get_seconds();
  int time_since = time_now-time_last;
  if(time_since > delaytime){
    time_last = time_now;
    return true;
  }
  return false;
}
