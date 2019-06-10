#include <RTClib.h>
#include "RTC.hpp"

RTC_PCF8523 rtc;
int TIMER_REQUEST;

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
  int time_since = (int) (time_now-time_last);
  if(time_since > delaytime){
    time_last = time_now;
    return true;
  }
  return false;
}


boolean RTC_data_request_timer(void){
  long time_now = RTC_get_seconds();
  static long timer_last = 0;
  int timer_delta = time_now-timer_last;
  if(timer_delta > TIMER_REQUEST){
    timer_last = time_now;
    return true;
  }
  return false;
}

void RTC_change_timer(int request_timer){
  TIMER_REQUEST = request_timer+1;
}