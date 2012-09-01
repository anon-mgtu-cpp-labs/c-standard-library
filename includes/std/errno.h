#ifndef _ERRNO_H
#define _ERRNO_H

/* Conditional support for Annex K of C11 */
#if defined(__STD_WANT_LIB_EXT1__) && __STD_WANT_LIB_EXT1__ != 0
#ifdef __STD_LIB_EXT1__
#ifndef _HAS_ERRNOT
#define _HAS_ERRNOT
typedef int errno_t;
#endif /* _HAS_ERRNOT */
#endif /* __STD_LIB_EXT1__ */
#endif /* __STD_WANT_LIB_EXT1__ */

extern int *_errno(void);

#define errno  (*_errno())

/*
    Required standard errno constants.
*/
#define EDOM     1    /* Domain error (math functions) */
#define EILSEQ   2    /* Encoding error (illegal byte sequence) */
#define ERANGE   3    /* Out of range error */

/*
    POSIX compliant errno constants.
*/
#define EINVAL   4    /* Invalid argument */
#define ENOENT   5    /* No such file or directory */
#define ENOMEM   6    /* Insufficient memory for the request */
#define EIO      7    /* General I/O error */
#define ENFILE   8    /* Too many open files */
#define EACCESS  9    /* Permission denied */
#define EUNKNOWN 10   /* Unknown error */

/*
    Implementation-defined errno constants.
*/
#define ESETP    8    /* File position seek error */
#define EGETP    9    /* File position tell error */
#define ESFMT    10   /* Unexpected string format */

struct _errno_mapping {
    int         code; /* Equivalent error number to one of the defined macros */
    const char *name; /* Stringized symbolic name for one of the defined macros */
    const char *msg;  /* Descriptive message of the defined macro */
};

extern struct _errno_mapping __errno_map[];

#endif /* _ERRNO_H */
