#ifndef __SCANF_H
#define __SCANF_H

/* Also defined in stdarg.h and stdio.h */
#ifndef _HAS_VALIST
#define _HAS_VALIST
typedef char *va_list;
#endif

/* size_t is defined in multiple headers */
#ifndef _HAS_SIZET
#define _HAS_SIZET
typedef unsigned size_t;
#endif

typedef int (*_get_func_t)(void *src, size_t *count);
typedef int (*_unget_func_t)(void *data, void *src, size_t *count);

extern int _scanf(_get_func_t get, _unget_func_t unget, void *src, const char *fmt, va_list args);

#endif /* __SCANF_H */
