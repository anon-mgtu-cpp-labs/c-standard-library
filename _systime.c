#include "_system.h"
#include "_systime.h"
#include "_time.h"
#include "limits.h"
#include "stdbool.h"
#include "string.h"
#include "time.h"
#include "wchar.h"
#include <Windows.h>

/* 
    ===================================================
                      Private helpers
    ===================================================
*/

typedef struct {
    int  yday;  /* Days up to a DST boundary in the year */
    long dsec;  /* Seconds after midnight */
} _dsttime_t;

static TIME_ZONE_INFORMATION tzinfo;
static DWORD tzinfo_dst = TIME_ZONE_ID_UNKNOWN;
static bool tzinfo_init = false;

static _dsttime_t extract_dst_info(SYSTEMTIME info, int year);

/* 
    ===================================================
                    Public definitions
    ===================================================
*/

/*
    @description:
        Retrieves the process' startup time in clock ticks.
*/
long long _sys_getticks(void)
{
    return GetTickCount64();
}

/*
    @description:
        Retrieves the number of seconds since 1970, normalized to UTC.
*/
long long _sys_getseconds(void)
{
    SYSTEMTIME past_systime = { 1970, 1, 4, 1, 0, 0, 0, 0 }; /* Epoch */
    SYSTEMTIME curr_systime;
    union { 
        FILETIME       data; 
        ULARGE_INTEGER raw; 
    } past_filetime, curr_filetime;

    GetSystemTime(&curr_systime);
    
    if (!SystemTimeToFileTime(&past_systime, (LPFILETIME)&past_filetime) ||
        !SystemTimeToFileTime(&curr_systime, (LPFILETIME)&curr_filetime))
    {
        return -1;
    }
    else {
        /* Get the time expressed as 100 nanosecond intervals */
        long long secs = curr_filetime.raw.QuadPart - past_filetime.raw.QuadPart;

        /* Correct to seconds */
        secs /= 10000000LL;

        return secs;
    }
}

/*
    @description:
        Retrieves the timezone name, dependent on daylight savings time.
*/
const char *_sys_timezone_name(void)
{
    static char namebuf[64];
    const wchar_t *s;

    if (!tzinfo_init) {
        tzinfo_dst = GetTimeZoneInformation(&tzinfo);
        tzinfo_init = true;
    }

    if (tzinfo_dst == TIME_ZONE_ID_UNKNOWN)
        return "";

    s = (tzinfo_dst == TIME_ZONE_ID_STANDARD) ? tzinfo.StandardName : tzinfo.DaylightName;

    if (_sys_wctomb(namebuf, s, 32) == -1)
        return "";
    
    return namebuf;
}

/*
    @description:
        Retrieves the current timezone bias in seconds.
*/
long _sys_timezone_offset(void)
{
    if (!tzinfo_init) {
        tzinfo_dst = GetTimeZoneInformation(&tzinfo);
        tzinfo_init = true;
    }

    if (tzinfo_dst == TIME_ZONE_ID_UNKNOWN)
        return 0;

    return tzinfo.Bias * _SECS_PER_MINUTE;
}

/*
    @description:
        Retrieves the current daylight savings time bias.
*/
long _sys_dst_offset(void)
{
    if (!tzinfo_init) {
        tzinfo_dst = GetTimeZoneInformation(&tzinfo);
        tzinfo_init = true;
    }

    if (tzinfo_dst == TIME_ZONE_ID_UNKNOWN)
        return 0;

    return (tzinfo.DaylightBias - tzinfo.StandardBias) * _SECS_PER_MINUTE;
}

/*
    @description:
        Determines whether the given date is within daylight savings time.
*/
int _sys_isdst(struct tm *timeptr)
{
    /*
        Assume standard USA daylight savings time rules if we can't
        get anything useful from the system. Default to current rules.
    */
    SYSTEMTIME begin = {0, 3, 1, 2, 2, 0, 0, 0}; /* 2nd Sunday of Mar at 02:00 */
    SYSTEMTIME end = {0, 11, 1, 1, 2, 0, 0, 0};  /* 1st Sunday of Nov at 02:00 */
    _dsttime_t dst_begin, dst_end;
    long sec;

    if (tzinfo_dst == TIME_ZONE_ID_INVALID && timeptr->tm_year <= 107) {
        /*
            The rules changed post 2007; correct for earlier years
        */
        begin.wMonth = 4, begin.wDay = 1; /* 1st Sunday of Apr at 02:00 */
        end.wMonth = 10, end.wDay = 5;    /* Last Sunday of Oct at 02:00 */
    }
    else {
        /* Use the available system provided dates */
        begin = tzinfo.DaylightDate;
        end = tzinfo.StandardDate;
    }

    dst_begin = extract_dst_info(begin, _YEAR_BASE + timeptr->tm_year);
    dst_end = extract_dst_info(end, _YEAR_BASE + timeptr->tm_year);

    /* dst_end represents the last day in daylight savings time, but we want one past that */
    dst_end.dsec += _sys_dst_offset();

    /* Correct for rollover in the day's seconds */
    if (dst_end.dsec >= _SECS_PER_DAY) {
        /* the DST offset was positive */
        dst_end.dsec -= _SECS_PER_DAY;
        ++dst_end.yday;
    }
    else {
        /* the DST offset was negative */
        dst_end.dsec += _SECS_PER_DAY;
        --dst_end.yday;
    }

    /*
        Check to see if the provided year day is in range of daylight savings time.
    */
    if (dst_begin.yday < dst_end.yday) {
        if (timeptr->tm_yday < dst_begin.yday || timeptr->tm_yday > dst_end.yday)
            return 0;

        if (timeptr->tm_yday > dst_begin.yday && timeptr->tm_yday < dst_end.yday)
            return 1;
    }
    else {
        if (timeptr->tm_yday < dst_end.yday || timeptr->tm_yday > dst_begin.yday)
            return 1;

        if (timeptr->tm_yday > dst_end.yday && timeptr->tm_yday < dst_begin.yday)
            return 0;
    }

    /* The year day being tested is a transition date; check the time of day */
    sec = (_SECS_PER_HOUR * timeptr->tm_hour) + (_SECS_PER_MINUTE * timeptr->tm_min) + timeptr->tm_sec;

    if (timeptr->tm_yday == dst_begin.yday) {
        /* The transition date is moving into daylight savings time */
        return sec >= dst_begin.dsec;
    }
    else {
        /* The transition date is moving out of daylight savings time */
        return sec < dst_end.dsec;
    }
}

/* 
    ===================================================
                Static helper definitions
    ===================================================
*/

/*
    @description:
        Calculates the transition time in year days and day 
        seconds for the given DST date in the given year.
*/
_dsttime_t extract_dst_info(SYSTEMTIME info, int year)
{
    int yday = yeardays[_LEAP_YEAR(year)][info.wMonth - 1];
    int mday = info.wDay;
    _dsttime_t result;

    /*
        Via TIME_ZONE_INFORMATION documentation from MSDN: "If the wYear member 
        is not zero, the transition date is absolute; it will only occur one time. 
        Otherwise, it is a relative date that occurs yearly."

        Absolute in this case means that wDay has the usual meaning, so
        a simple offset into the yday using wDay is sufficient.
    */
    if (info.wYear == 0) {
        /*
            Relative means that wDay encodes the week number of the month 
            rather than the day of the month. We need to find the number
            of days from the first day of the month to the wDayOfWeek of
            the nth week specified by wDay.
        */
        mday = (info.wDay - 1) * 7 + info.wDayOfWeek; /* Assume the month starts on Sunday */
        mday -= _first_wday(year, info.wMonth);   /* Correct for months not starting on Sunday */
        mday += 1;                                    /* Weekdays are 0-based; mday will be short by 1 */
    }

    result.yday = yday + mday;
    result.dsec = (_SECS_PER_HOUR * info.wHour) + (_SECS_PER_MINUTE * info.wMinute) + info.wSecond;

    return result;
}