/*
 * rtc_time.c
 *
 *  Created on: Apr 24, 2019
 *      Author: tcpatel
 */

#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include "rtc_time.h"
#include "cmsis_os.h"



static osMutexId rtc_mutex = NULL;
static void rtc_mutex_init( void );

int _gettimeofday (struct timeval * tp, struct timezone * tzp)
{
	if(rtc_mutex == NULL)
	{
		rtc_mutex_init();
	}

  /* Return fixed data for the timezone.  */
  if (tzp)
    {
      tzp->tz_minuteswest = 0;
      tzp->tz_dsttime = 0;
    }
  time_t t = 0;
  if(osMutexWait(rtc_mutex, 300) == osOK)
  {
	  t = RTC_READ();

	  tp->tv_sec = t;
	  tp->tv_usec = 0;

	  osMutexRelease(rtc_mutex);

	  if(t < 0)
		  return -1;

	  return 0;
  }

  return -1;
}


int _settimeofday(struct timeval * tp, struct timezone * tzp)
{
	if(rtc_mutex == NULL)
	{
		rtc_mutex_init();
	}

  if(osMutexWait(rtc_mutex, 300) == osOK)
  {
	RTC_WRITE(tp->tv_sec);
	osMutexRelease(rtc_mutex);
	return 0;
  }


	return -1;
}


static void rtc_mutex_init( void )
{
	osMutexDef(rtc);
	rtc_mutex = osMutexCreate(osMutex(rtc));
}
