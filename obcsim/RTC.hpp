#include "RTClib.h"
#define REQUEST_TIME 60

void RTC_init();
DateTime RTC_get(void);
boolean RTC_time_since_last();
long RTC_get_seconds(void);
