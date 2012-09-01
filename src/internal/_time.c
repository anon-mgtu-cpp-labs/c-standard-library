#include "_systime.h"
#include "_time.h"
#include "time.h"

clock_t __clock_base;

const int yeardays[2][13] = {
    { -1, 30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333, 364 },
    { -1, 30, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 }
};

const int monthdays[2][13] = {
    { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
    { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

/*
    @description:
        Initializes the time subsystem (called at the entry point).
*/
void _init_clock(void)
{
    __clock_base = _sys_getticks();
}

/*
    @description:
        Calculates the weekday of the first day of the month in the given year.
*/
int _first_wday(int year, int month)
{
    int ydays, base_dow;

    /* Correct out of range months by shifting them into range (in the same year) */
    month = (month < 1) ? 1 : month;
    month = (month > 12) ? 12 : month;
    
    /* Find the number of days up to the first day of the month */
    ydays = 1 + yeardays[_LEAP_YEAR(year)][month - 1];

    /* Find the day of the week for January 1st */
    base_dow = (year * 365 + _LEAP_COUNT(year)) % 7;

    /* Shift to month 1st and cycle to the correct weekday */
    return (base_dow + ydays) % 7;
}

/*
    @description:
        Calculates the ISO 8601 week number for the given date.

        If the week number rolls to another year, the year
        parameter is both [in/out] and will be updated
        accordingly.
*/
int _iso_week(int *year, int month, int day)
{
    time_t t = time(NULL);
    struct tm *info = localtime(&t);

    info->tm_year = *year - 1900;
    info->tm_mon = month - 1;
    info->tm_mday = day;

    if (mktime(info) == (time_t)-1)
        return -1;
    else {
        int wday = info->tm_wday;
        int week_start, week;
        
        if (wday == 0)
            wday = 7;

        week_start = ((info->tm_yday + 1) - wday);
        week = week_start / 7 + 1;

        if (week_start < 0)
            week = 0;

        if (week_start % 7 < 0)
            week_start += 7;

        if (week_start % 7 > 3)
            ++week;

        if (week == 53) {
            if (!((_LEAP_YEAR(*year) && week_start % 7 == 5) || week_start % 7 == 4)) {
                week = 1;
                ++*year;
            }
        }

        if (week < 1) {
            switch (week_start % 7) {
            case 1: week = 52;                                break;
            case 2: week = (_LEAP_YEAR(*year - 1)) ? 53 : 52; break;
            case 3: week = 53;                                break;
            }

            --*year;
        }

        return week;
    }
}