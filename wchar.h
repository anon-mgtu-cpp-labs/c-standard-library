#ifndef _WCHAR_H
#define _WCHAR_H

/* NULL is defined in multiple headers */
#ifndef NULL
#define NULL 0
#endif

#ifndef _HAS_WCHARMAX
#define _HAS_WCHARMAX
#define WCHAR_MIN     0
#define WCHAR_MAX     65535
#endif

#define WEOF (-1)

/* wchar_t is defined in multiple headers */
#ifndef _HAS_WCHART
#define _HAS_WCHART
typedef unsigned short wchar_t;
#endif

/* size_t is defined in multiple headers */
#ifndef _HAS_SIZET
#define _HAS_SIZET
typedef unsigned size_t;
#endif

typedef int mbstate_t;
typedef int wint_t;

struct tm;

extern wchar_t *wcscpy(wchar_t *dst, const wchar_t *src);
extern int wcscmp(const wchar_t *a, const wchar_t *b);

#endif /* _WCHAR_H */
