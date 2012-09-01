#ifndef FMTSPEC_H
#define FMTSPEC_H

#include "stddef.h"

/* Special format flags for printing */
#define _LEFT_JUSTIFY 0x01 /* '-' flag */
#define _SHOW_SIGN    0x02 /* '+' flag */
#define _SHOW_SPACE   0x04 /* ' ' flag */
#define _ALT_FORMAT   0x08 /* '#' flag */

#define _NOT_SPECIFIED -1
#define _ARG_SPECIFIED -2

typedef enum {
    /* Default initialization or unrecognized type */
    _SPEC_NO_MATCH,
    /* Non-null terminated pointers to char */
    _SPEC_CHAR,
    /* Signed integers */
    _SPEC_SCHAR,
    _SPEC_SHORT, 
    _SPEC_INT, 
    _SPEC_LONG,
    _SPEC_LLONG,
    _SPEC_INTMAXT,
    _SPEC_PTRDIFFT,
    /* Unsigned integers */
    _SPEC_UCHAR,
    _SPEC_USHORT, 
    _SPEC_UINT, 
    _SPEC_ULONG,
    _SPEC_ULLONG,
    _SPEC_UINTMAXT,
    _SPEC_SIZET,
    /* Pointer values */
    _SPEC_POINTER,
    /* Floating-point types */
    _SPEC_FLOAT, 
    _SPEC_DOUBLE, 
    _SPEC_LDOUBLE,
    /* Null terminated pointers to char */
    _SPEC_STRING,
    /* Sequences of characters to match */
    _SPEC_SCANSET,
    /* Character input counts of the stream */
    _SPEC_COUNT,
    /* Escaped '%' literal characters */
    _SPEC_LITERAL
} _spec_type_t;

typedef enum {
    _SPEC_FMT_NO_FORMAT,    /* Default initialization */
    _SPEC_FMT_DECIMAL = 10, /* Decimal (base 10) format */
    _SPEC_FMT_OCTAL = 8,    /* Octal (base 8)  format */
    _SPEC_FMT_HEX = 16,     /* Hexadecimal (base 16) format */
    _SPEC_FMT_NORMAL,       /* Scientific floating point format */
    _SPEC_FMT_FLOAT,        /* Standard floating point format */
    _SPEC_FMT_HEXFLOAT      /* IEEE 754 hexadecimal floating point format */
} _spec_fmt_t;

typedef struct _scanspec {
    int  suppressed;  /* Conversion has been suppressed with '*' */
    int  field_width; /* The maximum number of characters to read */
    int  type;        /* Type of expected input (including length) */
    int  format;      /* Expected format of the type */
    int  skipws;      /* Used if the type discards leading whitespace */
    int  nomatch;     /* Used if the scanset specifies '^' */
    int *scanset;     /* Lookup table for the scanset */
} _scanspec_t;

typedef struct _printspec {
    int  field_width;   /* The minimum field width */
    int  precision;     /* Type-defined precision */
    int  type;          /* Type of value to print */
    int  format;        /* Special formatting of the type */
    int  flags;         /* Special formatting flags */
    int  alt_case;      /* 'e' or 'E', and such */
    char pad;           /* Field width padding character */
} _printspec_t;

extern int _load_scanspec(_scanspec_t *spec, const char *fmt);
extern int _load_printspec(_printspec_t *spec, const char *fmt);

#endif /* FMTSPEC_H */