#ifndef _STDLIB_H
#define _STDLIB_H

#include "_stdc11.h"

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

/* NULL is defined in multiple headers */
#ifndef NULL
#define NULL 0
#endif

#define EXIT_FAILURE (-1)
#define EXIT_SUCCESS 0

#define RAND_MAX     2147483647

#define MB_CUR_MAX   ((size_t)1)

typedef struct _div_t   { int       quot, rem; } div_t;
typedef struct _ldiv_t  { long      quot, rem; } ldiv_t;
typedef struct _lldiv_t { long long quot, rem; } lldiv_t;

typedef int (*_cmp_func_t)(const void*, const void*);
typedef void (*_exitfunc_t)(void);

extern double atof(const char *s);

extern int atoi(const char *s);
extern long atol(const char *s);
extern long long atoll(const char *s);

extern float strtof(const char * restrict s, char ** restrict end);
extern double strtod(const char * restrict s, char ** restrict end);
extern long double strtold(const char * restrict s, char ** restrict end);

extern long strtol(const char * restrict s, char ** restrict end, int base);
extern long long strtoll(const char * restrict s, char ** restrict end, int base);
extern unsigned long strtoul(const char * restrict s, char ** restrict end, int base);
extern unsigned long long strtoull(const char * restrict s, char ** restrict end, int base);

extern _Noreturn void abort(void);
extern int at_quick_exit(_exitfunc_t func);
extern _Noreturn void quick_exit(int status);
extern int atexit(_exitfunc_t func);
extern _Noreturn void exit(int status);
extern _Noreturn void _Exit(int status);

extern char *getenv(const char *name);
extern int system(const char *command);

extern void *aligned_alloc(size_t alignment, size_t size);
extern void *malloc(size_t size);
extern void *calloc(size_t n, size_t size);
extern void *realloc(void *p, size_t size);
extern void free(void *p);

extern void *bsearch(const void *key, const void *base, size_t n, size_t size, _cmp_func_t cmp);
extern void qsort(void *base, size_t n, size_t size, _cmp_func_t cmp);

extern int abs(int value);
extern long labs(long value);
extern long long llabs(long long value);

extern div_t div(int numer, int denom);
extern ldiv_t ldiv(long numer, long denom);
extern lldiv_t lldiv(long long numer, long long denom);

extern int rand(void);
extern void srand(unsigned seed);

extern int mblen(const char *s, size_t n);

extern int mbtowc(wchar_t * restrict wc, const char * restrict s, size_t n);
extern int wctomb(char *s, wchar_t wc);

extern size_t mbstowcs(wchar_t * restrict ws, const char * restrict s, size_t n);
extern size_t wcstombs(char * restrict s, const wchar_t * restrict ws, size_t n);

extern char *_ecvt(double arg, int ndigits, int *decpt, int *sign);
extern char *_fcvt(double arg, int ndigits, int *decpt, int *sign);

#endif /* _STDLIB_H */
