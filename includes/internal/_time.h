#ifndef __TIME_H
#define __TIME_H

#include "time.h"

#define _SECS_PER_MINUTE  60L
#define _SECS_PER_HOUR    3600L
#define _SECS_PER_DAY     86400L
#define _SECS_PER_YEAR    31536000L

#define _YEAR_BASE        1900 /* Base year for _TM_EPOCH */
#define _YEAR_MAX         2100 /* Maximum supported year */

#define _EPOCH            1970 /* Year of the epoch (with base offset) */
#define _TM_EPOCH         70   /* Year of the epoch (without base offset) */

#define _LEAP_YEAR(year)        (((year) > 0) && !((year) % 4) && (((year) % 100) || !((year) % 400)))
#define _LEAP_COUNT(year)       ((((year) - 1) / 4) - (((year) - 1) / 100) + (((year) - 1) / 400))
#define _LEAP_SINCE_EPOCH(year) (_LEAP_COUNT(year) - _LEAP_COUNT(_EPOCH))
#define _DAYS_SINCE_EPOCH(year) (((year) - _EPOCH) * 365)
#define _DAYS_IN_YEAR(year)     (_LEAP_YEAR(year) ? 366 : 365)

extern clock_t __clock_base;

extern const int yeardays[2][13];
extern const int monthdays[2][13];

extern void _init_clock(void);
extern int _first_wday(int year, int month);
extern int _iso_week(int *year, int month, int day);

#endif /* __TIME_H */
