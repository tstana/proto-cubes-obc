#include <RTClib.h>
#include "rtc.hpp"

RTC_PCF8523 rtc;
static int timer_request = 30;
static long timer_last = 0;
static bool timed_daq_en = false;

void rtc_init(void)
{
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
    timer_last = rtc_get_seconds();
  }
}

DateTime rtc_get(void)
{
  return rtc.now();
}

void rtc_set_time(uint32_t timestamp){
  rtc.adjust(DateTime(timestamp));
}

long rtc_get_seconds(void)
{
  return (rtc.now()).unixtime();
}

boolean rtc_time_since_last(int delaytime)
{
  long time_now = 0;
  static long time_last = 0;
  time_now = rtc_get_seconds();
  int time_since = (int)(time_now - time_last);
  if (time_since > delaytime) {
    time_last = time_now;
    return true;
  }
  return false;
}

static long timer_delta;

boolean rtc_data_request_timeout(void)
{
  long time_now = rtc_get_seconds();
  
  timer_delta = time_now - timer_last;
  
  if (timer_delta >= timer_request)
  {
    timer_last = time_now;
    return true;
  }
  return false;
}

void rtc_print_timer_delta(void)
{
  Serial.print(">>> timer_delta = ");
  Serial.println(timer_delta);
}

void rtc_change_timer(int request_timer)
{
  timer_request = request_timer + 1;
}

void rtc_enable_timed_daq(bool enable)
{
  /*
   * If timed DAQ is disabled, set timer_last to 2^32-1 so that timed DAQ "never
   * triggers" while off.
   * 
   * If timed DAQ is enablde, set timer_last to the current time, so that data
   * is requested from the CUBES PCB after DAQ_DUR (plus any eventual offset
   * after it).
   */
  if (!enable)
    timer_last = 4294967295;
  else
    timer_last = rtc_get_seconds();

  /* Finally, set the local variable and exit. */
  timed_daq_en = enable;
}

bool rtc_timed_daq_enabled()
{
  return timed_daq_en;
}
