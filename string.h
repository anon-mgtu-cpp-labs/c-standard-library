#ifndef _STRING_H
#define _STRING_H

#include "_stdc11.h"

/* size_t is defined in multiple headers */
#ifndef _HAS_SIZET
#define _HAS_SIZET
typedef unsigned size_t;
#endif

/* Conditional support for Annex K of C11 */
#if defined(__STD_WANT_LIB_EXT1__) && __STD_WANT_LIB_EXT1__ != 0
#ifdef __STD_LIB_EXT1__

#ifndef _HAS_ERRNOT
#define _HAS_ERRNOT
typedef int errno_t;
#endif

#ifndef _HAS_RSIZET
#define _HAS_RSIZET
typedef size_t rsize_t;
#endif

#endif /* __STD_LIB_EXT1__ */
#endif /* __STD_WANT_LIB_EXT1__ */

/* NULL is defined in multiple headers */
#ifndef NULL
#define NULL 0
#endif

extern void *memcpy(void * restrict dst, const void * restrict src, size_t n);
extern void *memmove(void * restrict dst, const void * restrict src, size_t n);
extern int memcmp(const void *a, const void *b, size_t n);
extern void *memchr(const void *s, int c, size_t n);
extern void *memset(void *s, int c, size_t n);

extern char *strcpy(char * restrict dst, const char * restrict src);
extern char *strncpy(char * restrict dst, const char * restrict src, size_t n);
extern char *strcat(char * restrict dst, const char * restrict src);
extern char *strncat(char * restrict dst, const char * restrict src, size_t n);

extern int strcmp(const char *a, const char *b);
extern int strcoll(const char *a, const char *b);
extern int strncmp(const char *a, const char *b, size_t n);

extern size_t strxfrm(char * restrict a, const char * restrict b, size_t n);

extern char *strchr(const char *s, int c);
extern char *strrchr(const char *s, int c);

extern size_t strspn(const char *s, const char *match);
extern size_t strcspn(const char *s, const char *match);
extern char *strpbrk(const char *s, const char *match);
extern char *strstr(const char *s, const char *match);

extern char *strtok(char * restrict s, const char * restrict delim);

extern size_t strlen(const char *s);
extern char *strerror(int errnum);

/* Conditional support for Annex K of C11 */
#if defined(__STD_WANT_LIB_EXT1__) && __STD_WANT_LIB_EXT1__ != 0
#ifdef __STD_LIB_EXT1__

extern errno_t memcpy_s(void * restrict dst, rsize_t dstmax, const void * restrict src, rsize_t n);
extern errno_t memmove_s(void *dst, rsize_t dstmax, const void *src, rsize_t n);
extern errno_t memset_s(void *s, rsize_t smax, int c, rsize_t n);

extern errno_t strcpy_s(char * restrict dst, rsize_t dstmax, const char * restrict src);
extern errno_t strncpy_s(char * restrict dst, rsize_t dstmax, const char * restrict src, rsize_t n);

extern errno_t strcat_s(char * restrict dst, rsize_t dstmax, const char * restrict src);
extern errno_t strncat_s(char * restrict dst, rsize_t dstmax, const char * restrict src, rsize_t n);

extern char *strtok_s(char * restrict s, rsize_t * restrict smax, const char * restrict delim, char ** restrict token);

extern size_t strnlen_s(const char *s, size_t maxsize);
extern errno_t strerror_s(char *s, rsize_t maxsize, errno_t errnum);
extern size_t strerrorlen_s(errno_t errnum);

#endif /* __STD_LIB_EXT1__ */
#endif /* __STD_WANT_LIB_EXT1__ */

extern int _stricmp(const char *a, const char *b);
extern int _strnicmp(const char *a, const char *b, size_t n);

extern char *_strdup(const char *s);
extern char *_strrev(char *s);
extern char *_strupr(char *s);
extern char *_strlwr(char *s);
extern char *_strltrim(char *s, const char *s2);
extern char *_strrtrim(char *s, const char *s2);

#endif /* _STRING_H */
