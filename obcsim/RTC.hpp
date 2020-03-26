#include "RTClib.h"
#define REQUEST_TIME 60

/*
 * rtc_init
 * Initializes the RTC
 */
void rtc_init();

/*
 * rtc_get
 * Returns a datetime variable with the current time of request
 * 
 */
DateTime rtc_get(void);
/* rtc_time_since_last
 *  Returns a boolean if the time has passed the threshold of delaytime since
 *      last it triggered a true statement
 *  Input the delaytime required
 *  Can not be used for 2 timers simultaneously
 *  Non-blocking
 */

boolean rtc_time_since_last(int delaytime);

/**
 * rtc_set_time
 * Sets the time of the RTC with a provided Unix time stamp.
 * 
 * @param uint32_t unix time stamp value
 * 
 */
void rtc_set_time(uint32_t timestamp);

/*
 * rtc_get_seconds
 * Returns a long with the current time in unixtime (seconds since 1st of january 1970)
 */
long rtc_get_seconds(void);


/*
* rtc_data_request_timeout
* Returns a boolean if timer target has been hit (not interrupt, so might not be 100% accurate)
*
*/
boolean rtc_data_request_timeout(void);

/*
* rtc_change_timer
* Changes the time target for the timer in RTC, called whenever DAQDUR is used 
*/

void rtc_change_timer(int request_timer);

void rtc_enable_timed_daq(bool enable);

bool rtc_timed_daq_enabled(void);



void rtc_print_timer_delta(void);

