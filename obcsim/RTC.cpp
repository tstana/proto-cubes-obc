#include <RTClib.h>
#include <string.h>

#include "rtc.hpp"
#include "debug.hpp"

static RTC_PCF8523 rtc;

void rtc_init(void)
{
  char s[32];
  DEBUG_PRINT("RTC initializing");
  if (!rtc.begin()) {
    Serial.println("RTC could not be found");
  }
  if (!rtc.initialized()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    DateTime now = rtc.now();
    sprintf(s, "RTC set to %d/%d/%d, %02d:%02d:%02d",
        now.year(), now.month(), now.day(),
        now.hour(), now.minute(), now.second());
    DEBUG_PRINT(s);
    sprintf(s, "Unix time is now %d", now.unixtime());
    DEBUG_PRINT(s);
  }
}

void rtc_set_time(uint32_t timestamp){
  rtc.adjust(DateTime(timestamp));
}

long rtc_get_time(void)
{
  return (rtc.now()).unixtime();
}
