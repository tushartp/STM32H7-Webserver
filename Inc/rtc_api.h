/*
 * rtc_api.h
 *
 *  Created on: Apr 24, 2019
 *      Author: tcpatel
 */

#ifndef RTC_API_H_
#define RTC_API_H_

#ifdef __cplusplus
 extern "C" {
#endif


#include <time.h>




 /** Initialize the RTC peripheral
  *
  * Powerup the RTC in perpetration for access. This function must be called
  * before any other RTC functions are called. This does not change the state
  * of the RTC. It just enables access to it.
  *
  */
void MX_RTC_Init( void );

 /** Get the current time from the RTC peripheral
  *
  * @return The current time in seconds
  *
  */
time_t RTC_READ( void );

/** Write the current time in seconds to the RTC peripheral
 *
 * @param t The current time to be set in seconds.
 *
 */
void RTC_WRITE( time_t t );


#ifdef __cplusplus
}
#endif

#endif /* RTC_API_H_ */
