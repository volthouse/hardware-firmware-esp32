#include "clock.h"
#include "rom/rtc.h"

#define TZ       -1       // (utc+) TZ in hours
#define DST_MN   60 // use 60mn for summer time in some countries
#define RTC_TEST 1510592820 // 1510592825 = Monday 13 November 2017 17:07:05 UTC

#define TZ_MN    ((TZ)*60)
#define TZ_SEC   ((TZ)*3600)
#define DST_SEC  ((DST_MN)*60)

// Pulse counter value, stored in RTC_SLOW_MEM
static size_t RTC_DATA_ATTR m_pulse_count;


ClockClass::ClockClass(uint32_t sleepTimeMs) 
  : sleepTimeMs(sleepTimeMs)
{
    m_pulse_count++;
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

uint32_t ClockClass::getRebootCount()
{
  return  m_pulse_count;
}

String ClockClass::toString()
{
  time_t now = time(nullptr);  
  char b[100];
  ctime_r(&now, b);
  return String(b);
}
