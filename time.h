#ifndef _TIME_H
#define _TIME_H

#include "_stdc11.h"

/* NULL is defined in multiple headers */
#ifndef NULL
#define NULL 0
#endif

#define CLOCKS_PER_SEC 1000
#define TIME_UTC       1

/* size_t is defined in multiple headers */
#ifndef _HAS_SIZET
#define _HAS_SIZET
typedef unsigned size_t;
#endif

typedef long long clock_t;
typedef long long time_t;

struct timespec {
    time_t tv_sec;  /* Whole seconds >= 0 */
    long   tv_nsec; /* Nanoseconds [0,999999999] */
};

struct tm {
    int tm_sec;   /* Seconds after the minute [0,60] */
    int tm_min;   /* Minutes after the hour [0,59] */
    int tm_hour;  /* Hours since midnight [0,23] */
    int tm_mday;  /* Day of the month [1,31] */
    int tm_mon;   /* Months since January [0,11] */
    int tm_year;  /* Years since 1900 */
    int tm_wday;  /* Days since Sunday [0,6] */
    int tm_yday;  /* Days since January 1st [0,365] */
    int tm_isdst; /* Daylight Savings Time flag */
};

extern clock_t clock(void);
extern time_t time(time_t *timer);

extern time_t mktime(struct tm *timeptr);
extern double difftime(time_t time1, time_t time0);

extern int timespec_get(struct timespec *ts, int base);

extern char *asctime(const struct tm *timeptr);
extern char *ctime(const time_t *timer);

extern struct tm *gmtime(const time_t *timer);
extern struct tm *localtime(const time_t *timer);

extern size_t strftime(char * restrict s, size_t n,
    const char * restrict fmt, const struct tm * restrict timeptr);

#endif /* _TIME_H */
