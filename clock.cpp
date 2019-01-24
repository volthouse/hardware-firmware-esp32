#include "clock.h"


#define TZ       -1       // (utc+) TZ in hours
#define DST_MN   60 // use 60mn for summer time in some countries
#define RTC_TEST 1510592820 // 1510592825 = Monday 13 November 2017 17:07:05 UTC

#define TZ_MN    ((TZ)*60)
#define TZ_SEC   ((TZ)*3600)
#define DST_SEC  ((DST_MN)*60)

#define MAGIC_NO 0xABCDEFAB


ClockClass::ClockClass(uint32_t sleepTimeMs) 
  : sleepTimeMs(sleepTimeMs)
{
/*  
  ESP.rtcUserMemoryRead(0, (uint32_t*) &rtc_time_desc, sizeof(rtc_time_desc));
  
	if(rtc_time_desc.magic != MAGIC_NO) {       
    memset(&rtc_time_desc, 0, sizeof(rtc_time_desc));
    rtc_time_desc.magic = MAGIC_NO;
    Serial.println("First Boot");
  } else {    
    timeval tv;
    timeval tv_add;
    memset(&tv_add, 0, sizeof(tv_add));
    tv_add.tv_sec = sleepTimeMs;
    add(rtc_time_desc.tv, tv_add, tv);
    timezone tz = { TZ_MN + DST_MN, 0 };
    settimeofday(&tv, &tz);
  }
*/

    timeval tv;
    timeval tv_add;
    memset(&tv_add, 0, sizeof(tv_add));
    tv_add.tv_sec = sleepTimeMs;
    add(rtc_time_desc.tv, tv_add, tv);
    timezone tz = { TZ_MN + DST_MN, 0 };
    settimeofday(&tv, &tz);
}

uint32_t ClockClass::hasSecondsChanged()
{
  static uint32_t lastSec = 0;
  timeval tv;  
  
  gettimeofday(&tv, nullptr);
  
  if(lastSec != tv.tv_sec) {
    lastSec = tv.tv_sec;
    return true;
  }
  
  return false;
}

void ClockClass::setClock(uint32_t year, uint32_t month, uint32_t day, uint32_t hours, uint32_t minutes, uint32_t seconds) {
	  struct tm tm;
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hours;
    tm.tm_min = minutes;
    tm.tm_sec = seconds;
    time_t t = mktime(&tm);    
    struct timeval now = { .tv_sec = t };
    timezone tz = { TZ_MN + DST_MN, 0 };
    settimeofday(&now, &tz);
}

void ClockClass::add(const timeval& a, const timeval& b, timeval& result)
{
  result.tv_sec = a.tv_sec + b.tv_sec;
  result.tv_usec = a.tv_usec + b.tv_usec;

  if (result.tv_usec >= 1000000) {
    result.tv_sec++;
    result.tv_usec -= 1000000;
  }
}

void ClockClass::save()
{
  timeval tv;
  
  gettimeofday(&tv, nullptr);
  rtc_time_desc.count++;
  memcpy(&rtc_time_desc.tv, &tv, sizeof(tv));
  //ESP.rtcUserMemoryWrite(0, (uint32_t*) &rtc_time_desc, sizeof(rtc_time_desc));  
}

uint32_t ClockClass::getRebootCount()
{
  return  rtc_time_desc.count;
}

String ClockClass::toString()
{
  time_t now = time(nullptr);  
  char b[100];
  ctime_r(&now, b);
  return String(b);
}
