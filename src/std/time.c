#include "_system.h"
#include "_systime.h"
#include "_time.h"
#include "stdbool.h"
#include "stdio.h"
#include "limits.h"
#include "locale.h"
#include "time.h"

/* 
    ===================================================
                     Static helpers
    ===================================================
*/

static bool normalize_calendar_time(struct tm *timeptr);
static void normalize(int *tens, int *units, int base);
static void correct_cycling_values(int *large, int *small, int small_max);
static char *copystr(char *dst, size_t n, const char *src);
static char *copyint(char *dst, size_t n, int value, int width, char pad);

/* 
    ===================================================
               Public function definitions
    ===================================================
*/

/*
    @description:
        Determines the processor time used.
*/
clock_t clock(void)
{
    return _sys_getticks() - __clock_base;
}

/*
    @description:
        Determines the current calendar time.
*/
time_t time(time_t *timer)
{
    time_t result = _sys_getseconds();

    if (timer)
        *timer = result;

    return result;
}

/*
    @description:
        Converts the broken-down time, expressed as local time, in the
        structure pointed to by timeptr into a calendar time.
*/
time_t mktime(struct tm *timeptr)
{
    if (!normalize_calendar_time(timeptr))
        return (time_t)-1;
    else {
        time_t cal_year = _YEAR_BASE + timeptr->tm_year;
        time_t year = timeptr->tm_year - _TM_EPOCH;
        time_t adjusted;

        if (cal_year < _EPOCH || cal_year > _YEAR_MAX)
            return (time_t)-1;

        /* Add accumulated seconds for every full year (including leap days) */
        adjusted = year * _SECS_PER_YEAR + _LEAP_SINCE_EPOCH(cal_year) * _SECS_PER_DAY;

        /* Add the accumulated seconds for every day in the present year */
        adjusted += (timeptr->tm_yday * _SECS_PER_DAY);

         /* Add the accumulated seconds for the present day */
        adjusted += ((_SECS_PER_HOUR * timeptr->tm_hour) + (_SECS_PER_MINUTE * timeptr->tm_min) + timeptr->tm_sec);

        /* mktime returns a local time, so adjust to the local timezone */
        adjusted += _sys_timezone_offset();

        /* Account for daylight savings time */
        if (timeptr->tm_isdst > 0 || (timeptr->tm_isdst < 0 && _sys_isdst(timeptr)))
            adjusted += _sys_dst_offset();

        return adjusted;
    }
}

/*
    @description:
        Returns the difference expressed in seconds as a double.
*/
double difftime(time_t time1, time_t time0)
{
    return (double)(time1 - time0);
}

/*
    @description:
        Sets the interval pointed to by ts to hold the current 
        calendar time based on the specified time base.
*/
int timespec_get(struct timespec *ts, int base)
{
    if (base == TIME_UTC) {
        ts->tv_sec = _sys_getseconds();
        ts->tv_nsec = 0;
        return base;
    }

    return 0;
}

/*
    @description:
        Converts the broken-down time in the structure pointed to by
        timeptr into a string in the form "Sun Sep 16 01:03:52 1973\n\0".
*/
char *asctime(const struct tm *timeptr)
{
    static char result[26];

    sprintf(result, "%.3s %.3s%3d %.2d:%.2d:%.2d %d\n",
        _localetime()->wday_name[timeptr->tm_wday],
        _localetime()->mon_name[timeptr->tm_mon],
        timeptr->tm_mday, 
        timeptr->tm_hour,
        timeptr->tm_min, 
        timeptr->tm_sec,
        1900 + timeptr->tm_year);

    return result;
}

/*
    @description:
        converts the calendar time pointed to by timer
        to local time in the form of a string.
*/
char *ctime(const time_t *timer)
{
    return asctime(localtime(timer));
}

/*
    @description:
        Converts the calendar time pointed to by timer 
        into a brokendown time, expressed as UTC.
*/
struct tm *gmtime(const time_t *timer)
{
    time_t seconds = *timer, year, tm_year, cal_year;
    time_t tm_yday, tm_mon = 0, tm_hour, tm_min;
    static struct tm result;
    
    year = seconds / _SECS_PER_YEAR;  /* Calculate years since the epoch */
    tm_year = year + _TM_EPOCH;       /* Add the epoch offset to get a tm_year value */
    cal_year = _YEAR_BASE + tm_year;  /* Get the calendar year for easier leap calculations */

    /* Reduce to seconds in the current year */
    seconds -= (year * _SECS_PER_YEAR + _LEAP_SINCE_EPOCH(cal_year) * _SECS_PER_DAY);

    /* Extract elapsed days in the present year and reduce to seconds in the current day */
    tm_yday = seconds / _SECS_PER_DAY;
    seconds -= (tm_yday * _SECS_PER_DAY);

    /* Find the hour of the current day and reduce to seconds in the hour */
    tm_hour = seconds / _SECS_PER_HOUR;
    seconds -= (tm_hour * _SECS_PER_HOUR);

    /* Find the minute of the current hour and reduce to seconds in the minute */
    tm_min = seconds / _SECS_PER_MINUTE;
    seconds -= (tm_min * _SECS_PER_MINUTE);

    /*
        Finding the number of days in the month doesn't depend on secons,
        so we can do it last. Search up to the point where the month's
        year days exceed the year days from timer. The previous month
        at that point is the one we want.
    */
    while (yeardays[_LEAP_YEAR(cal_year)][tm_mon + 1] < tm_yday)
        ++tm_mon;

    /* Fill in the tm structure result */
    result.tm_year = (int)tm_year;
    result.tm_yday = (int)tm_yday;
    result.tm_mon = (int)tm_mon;
    result.tm_mday = (int)(tm_yday - yeardays[_LEAP_YEAR(cal_year)][tm_mon]);
    result.tm_wday = (_first_wday((int)cal_year, 1) + tm_yday) % 7;
    result.tm_hour = (int)tm_hour;
    result.tm_min = (int)tm_min;
    result.tm_sec = (int)seconds;

    result.tm_isdst = 0; /* UTC never acknowledges DST */

    return &result;
}

/*
    @description:
        Converts the calendar time pointed to by timer 
        into a broken-down time, expressed as local time.
*/
struct tm *localtime(const time_t *timer)
{
    time_t tzoffset = _sys_timezone_offset();
    time_t temp = *timer - tzoffset;
    struct tm *result;
    
    /*
        _sys_isdst requires a valid tm object, so we need 
        to generate it from only the timezone offset first.
    */
    result = gmtime(&temp);

    if (result && _sys_isdst(result)) {
        /*
            Now we can do a daylight savings time offset, but the tm
            object needs to be recalculated after changing the time_t.
        */
        temp -= _sys_dst_offset();
        result = gmtime(&temp);

        if (result)
            result->tm_isdst = 1;
    }

    return result;
}

/*
    @description:
        Places characters into the array pointed to by s 
        as controlled by the string pointed to by fmt.
*/
size_t strftime(char * restrict s, size_t n, 
    const char * restrict fmt, const struct tm * restrict timeptr)
{
    struct _ltime *loc = _localetime();
    char *start = s;

    while (*fmt && (size_t)(s - start) < n) {
        if (*fmt != '%')
            *s++ = *fmt++;
        else {
            size_t max = n - (s - start);
            char alt = '\0';

            ++fmt; /* Skip over the '%' */

            /* Recognize and save a locale-alternative specifier */
            if (*fmt == 'E' || *fmt == 'O')
                alt = *fmt++;

            switch (*fmt++) {
            case 'a': s = copystr(s, max, loc->wday_name[timeptr->tm_wday]); break;
            case 'A': s = copystr(s, max, loc->wday_name_long[timeptr->tm_wday]); break;
            case 'b': s = copystr(s, max, loc->mon_name[timeptr->tm_mon]); break;
            case 'B': s = copystr(s, max, loc->mon_name_long[timeptr->tm_mon]); break;
            case 'c': s += strftime(s, max, loc->datetime_format, timeptr); break;
            case 'C': s = copyint(s, max, (int)(timeptr->tm_year / 100.0), 2, '0'); break;
            case 'd': s = copyint(s, max, timeptr->tm_mday, 2, '0'); break;
            case 'D': s += strftime(s, max, "%m/%d/%y", timeptr); break;
            case 'e': s = copyint(s, max, timeptr->tm_mday, 2, ' '); break;
            case 'F': s += strftime(s, max, "%Y-%m-%d", timeptr); break;
            case 'g':
            case 'G':
                {
                    int year = timeptr->tm_year + _YEAR_BASE;
                    int month = timeptr->tm_mon + 1;
                    int day = timeptr->tm_mday;

                    _iso_week(&year, month, day);
                    s = copyint(s, max, year, fmt[-1] == 'g' ? 2 : 4, '0');

                    break;
                }
            case 'h': s += strftime(s, max, "%b", timeptr); break;
            case 'H': s = copyint(s, max, timeptr->tm_hour, 2, '0'); break;
            case 'I': s = copyint(s, max, 1 + timeptr->tm_hour % 12, 2, '0'); break;
            case 'j': s = copyint(s, max, 1 + timeptr->tm_yday, 3, '0'); break;
            case 'm': s = copyint(s, max, timeptr->tm_mon, 2, '0'); break;
            case 'M': s = copyint(s, max, timeptr->tm_min, 2, '0'); break;
            case 'n': *s++ = '\n'; break;
            case 'p': s = copystr(s, max, (timeptr->tm_hour > 11 ? loc->pm : loc->am)); break;
            case 'r': s += strftime(s, max, loc->time12_format, timeptr); break;
            case 'R': s += strftime(s, max, "%H:%M", timeptr); break;
            case 'S': s = copyint(s, max, timeptr->tm_sec, 2, '0'); break;
            case 't': *s++ = '\t'; break;
            case 'T':  s += strftime(s, max, "%H:%M:%S", timeptr); break;
            case 'u': s = copyint(s, max, 1 + timeptr->tm_wday, 1, '0'); break;
            case 'U': s = copyint(s, max, (timeptr->tm_yday + 7 - timeptr->tm_wday) / 7, 2, '0'); break;
            case 'V':
                {
                    int year = timeptr->tm_year + _YEAR_BASE;
                    int month = timeptr->tm_mon + 1;
                    int day = timeptr->tm_mday;

                    s = copyint(s, max, _iso_week(&year, month, day), 2, '0');

                    break;
                }
            case 'w': s = copyint(s, max, timeptr->tm_wday, 1, '0'); break;
            case 'W':
                {
                    int wday = timeptr->tm_wday - 1;
                    
                    if (wday == 0)
                        wday = 6;

                    s = copyint(s, max, (timeptr->tm_yday + 7 - wday) / 7, 2, '0');

                    break;
                }
            case 'x': s += strftime(s, max, loc->date_format, timeptr); break;
            case 'X': s += strftime(s, max, loc->time_format, timeptr); break;
            case 'y': s = copyint(s, max, timeptr->tm_year % 100, 2, '0'); break;
            case 'Y': s = copyint(s, max, _YEAR_BASE + timeptr->tm_year, 4, '0'); break;
            case 'z':
                {
                    long offset = _sys_timezone_offset();

                    s = copystr(s, max, offset < 0 ? "-" : "+");
                    s = copyint(s, max, offset / _SECS_PER_HOUR, 2, '0');
                    s = copyint(s, max, (offset % _SECS_PER_HOUR) / _SECS_PER_MINUTE, 2, '0');

                    break;
                }
            case 'Z': s = copystr(s, max, _sys_timezone_name()); break;
            case '%': *s++ = '%'; break;
            }
        }
    }

    if ((size_t)(s - start) < n) {
        *s = '\0';
        return s - start;
    }

    return 0;
}

/* 
    ===================================================
                Static helper definitions
    ===================================================
*/

/*
    @description:
        Corrects out of range values in timeptr by adjusting
        each into range and propagating the adjustment up.
*/
bool normalize_calendar_time(struct tm *timeptr)
{
    int cal_year; /* For less verbose leap year checks */

    /* Simple odometer style cases */
    normalize(&timeptr->tm_min, &timeptr->tm_sec, 60);
    normalize(&timeptr->tm_hour, &timeptr->tm_min, 60);
    normalize(&timeptr->tm_mday, &timeptr->tm_hour, 24);
    normalize(&timeptr->tm_year, &timeptr->tm_mon, 12);

    cal_year = _YEAR_BASE + timeptr->tm_year;

    /*
        The month day is harder because the adjustment isn't 
        consistent; it depends on the month in question. We'll
        start with forcing it to be positive by rolling back
        the year.
    */
    while (timeptr->tm_mday <= 0) {
        cal_year = _YEAR_BASE + --timeptr->tm_year;
        timeptr->tm_mday += _DAYS_IN_YEAR(cal_year);
    }

    /*
        Now roll the month and year forward until month days
        are within range of the current month.
    */
    while (timeptr->tm_mday > monthdays[_LEAP_YEAR(cal_year)][timeptr->tm_mon]) {
        timeptr->tm_mday -= monthdays[_LEAP_YEAR(cal_year)][timeptr->tm_mon];

        if (++timeptr->tm_mon >= 12) {
            timeptr->tm_mon = 0;
            cal_year = _YEAR_BASE + ++timeptr->tm_year;
        }
    }

    timeptr->tm_yday = timeptr->tm_mday + yeardays[_LEAP_YEAR(cal_year)][timeptr->tm_mon];
    timeptr->tm_wday = (_first_wday(cal_year, 1) + timeptr->tm_yday) % 7;

    timeptr->tm_isdst = _sys_isdst(timeptr);

    /* Boolean return for the future, but for now we can't fail */
    return true;
}

/*
    @description:
        Normalize two values in the style of an odometer.
*/
void normalize(int *tens, int *units, int base)
{
    if (*units >= base) {
		*tens += *units / base;
		*units %= base;
	} else if (*units < 0) {
		--(*tens);
		*units += base;

		if (*units < 0) {
			*tens = *tens - 1 + (-*units / base);
			*units = base - (-*units % base);
		}
	}
}

/*
    @description:
        Copies up to n characters from src into dst and returns
        a pointer to the one past the last written character.
*/
char *copystr(char *dst, size_t n, const char *src)
{
    size_t count = 0;

    while (count++ < n && *src)
        *dst++ = *src++;

    return dst;
}

/*
    @description:
        Copies up to n characters from value into dst and returns
        a pointer to the one past the last written character. Value
        may be padded by the specified character if its width is
        less than the specified field width.
*/
char *copyint(char *dst, size_t n, int value, int width, char pad)
{
    char buf[BUFSIZ];
    size_t i;

    sprintf(buf, "%*d", width, value);

    /* Fill in padding */
    if (pad != ' ') {
        for (i = 0; buf[i] && buf[i] == ' '; ++i)
            buf[i] = pad;
    }

    for (i = 0; i < n && buf[i]; ++i)
        *dst++ = buf[i];

    return dst;
}