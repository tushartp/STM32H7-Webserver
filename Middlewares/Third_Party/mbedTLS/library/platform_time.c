


#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)

#include "mbedtls/platform.h"

#if defined(MBEDTLS_PLATFORM_TIME_ALT)

#if defined(STM32H753xx)
#include "main.h"
#include "string.h"
#include "stm32h7xx_hal.h"

extern RTC_HandleTypeDef hrtc;
//time_t time_stamp ( time_t *timer );
//mbedtls_time_t (*mbedtls_time)( mbedtls_time_t* time ) = time_stamp;

time_t time ( time_t *timer )
{
	RTC_TimeTypeDef currentTime;
	RTC_DateTypeDef currentDate;
	time_t timestamp = 0;
	struct tm currTime;

	HAL_RTC_GetTime(&hrtc, &currentTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &currentDate, RTC_FORMAT_BIN);

	currTime.tm_year = currentDate.Year + 100;  // In fact: 2000 + 18 - 1900
	currTime.tm_mday = currentDate.Date;
	currTime.tm_mon  = currentDate.Month - 1;

	currTime.tm_hour = currentTime.Hours;
	currTime.tm_min  = currentTime.Minutes;
	currTime.tm_sec  = currentTime.Seconds;

	timestamp = mktime(&currTime);

	return timestamp;
}

#endif
#endif /* MBEDTLS_PLATFORM_TIME_ALT */
#endif /* MBEDTLS_PLATFORM_C */
