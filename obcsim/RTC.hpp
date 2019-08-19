#include "RTClib.h"
#define REQUEST_TIME 60

/*
 * RTC_init
 * Initializes the RTC clock
 */
void RTC_init();

/*
 * RTC_get
 * Returns a datetime variable with the current time of request
 * 
 */
DateTime RTC_get(void);
/* RTC_time_since_last
 *  Returns a boolean if the time has passed the threshold of delaytime since last it triggered a true statement
 *  Input the delaytime required
 *  Can not be used for 2 timers simultaneous
 *  Non-blocking
 */

boolean RTC_time_since_last(int delaytime);

/**
 * RTC_set_time
 * Sets the time of the RTC with a provided Unix time stamp.
 * 
 * @param uint32_t unix time stamp value
 * 
 */
void RTC_set_time(uint32_t timestamp);

/*
 * RTC_get_seconds
 * Returns a long with the current time in unixtime (seconds since 1st of january 1970)
 */
long RTC_get_seconds(void);


/*
* RTC_data_request_timer
* Returns a boolean if timer target has been hit (not interrupt, so might not be 100% accurate)
*
*/
boolean RTC_data_request_timer(void);

/*
* RTC_change_timer
* Changes the time target for the timer in RTC, called whenever DAQDUR is used 
*/

void RTC_change_timer(int request_timer);

void RTC_enable_timed_daq(bool enable);

bool RTC_timed_daq_enabled(void);
