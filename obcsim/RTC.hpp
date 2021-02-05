#ifndef __RTC_HPP__
#define __RTC_HPP__

/**
 * @brief Initialize the RTC_PCF8523
 */
void rtc_init();


/**
 * @brief Set the time of the RTC with a provided Unix time stamp
 * 
 * @param uint32_t Unix time stamp value
 * 
 * @return None
 */
void rtc_set_time(uint32_t unix_timestamp);


/**
 * @brief Return the current Unix timestamp (seconds since 1st of january 1970)
 * 
 * @param None
 * 
 * @return uint32_t Unix timestamp
 */
uint32_t rtc_get_time(void);

#endif //__RTC_HPP__