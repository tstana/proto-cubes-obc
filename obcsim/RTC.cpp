#include <RTClib.h>
#include <string.h>

#include "rtc.hpp"
#include "debug.hpp"

RTC_PCF8523 rtc;
static uint8_t daq_time = 0;
static long time_daq_start = 0;
static bool timed_daq_en = false;

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
    time_daq_start = rtc_get_seconds();
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

boolean rtc_data_request_timeout(void)
{
  long time_now = rtc_get_seconds();
  long time_delta = time_now - time_daq_start;
  
  return (time_delta >= daq_time) ? true : false;
}

void rtc_set_daq_time(uint8_t request_timer)
{
  daq_time = request_timer + 1;
}

uint8_t rtc_get_daq_time()
{
  return daq_time;
}

void rtc_enable_timed_daq(bool enable)
{
  /*
   * If timed DAQ is disabled, set time_daq_start to 2^32-1 so that timed DAQ "never
   * triggers" while off.
   * 
   * If timed DAQ is enablde, set time_daq_start to the current time, so that data
   * is requested from the CUBES PCB after DAQ_DUR (plus any eventual offset
   * after it).
   */
  if (!enable)
    time_daq_start = 4294967295;
  else
    time_daq_start = rtc_get_seconds();

  /* Finally, set the local variable and exit. */
  timed_daq_en = enable;
}

bool rtc_timed_daq_enabled()
{
  return timed_daq_en;
}
