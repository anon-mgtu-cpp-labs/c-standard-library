#ifndef __PRINTF_H
#define __PRINTF_H

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

typedef int (*_put_func_t)(void *data, void *dst, size_t n, size_t *count, size_t limit);

extern int _printf(_put_func_t put, void *dst, const char *fmt, size_t n, va_list args);

#endif /* __PRINTF_H */
