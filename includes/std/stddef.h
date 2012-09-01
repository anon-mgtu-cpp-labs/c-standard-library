#ifndef _STDDEF_H
#define _STDDEF_H

typedef int       ptrdiff_t;
typedef long long max_align_t;

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

#if defined(__STD_WANT_LIB_EXT1__) && __STD_WANT_LIB_EXT1__ != 0
#ifdef __STD_LIB_EXT1__
#ifndef _HAS_RSIZET
#define _HAS_RSIZET
typedef size_t rsize_t;
#endif
#endif
#endif

/* NULL is defined in multiple headers */
#ifndef NULL
#define NULL 0
#endif

#define offsetof(type,memb) ((size_t)&(((type*)0)->memb))

#endif /* _STDDEF_H */
