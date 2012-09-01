#ifndef _LOCALE_H
#define _LOCALE_H

/* NULL is defined in multiple headers */
#ifndef NULL
#define NULL 0
#endif

#define LC_ALL       0
#define LC_COLLATE   1
#define LC_CTYPE     2
#define LC_MONETARY  3
#define LC_NUMERIC   4
#define LC_TIME      5

#define _LC_MIN      0
#define _LC_MAX      5

#define _UPPER  0x0001
#define _LOWER  0x0002
#define _DIGIT  0x0004
#define _SPACE  0x0008
#define _PUNCT  0x0010
#define _CNTRL  0x0020
#define _BLANK  0x0040
#define _HEX    0x0080
#define _ALPHA  0x0100

struct lconv {
    char *decimal_point;       /* (Non-monetary) Decimal point character */
    char *thousands_sep;       /* (Non-monetary) Group separator */
    char *grouping;            /* (Non-monetary) Size of digit groups */
    char *mon_decimal_point;   /* (Monetary) Decimal point character */
    char *mon_thousands_sep;   /* (Monetary) Group separator */
    char *mon_grouping;        /* (Monetary) Size of digit groups */
    char *positive_sign;       /* (Monetary) Symbol used to indicate a positive value */
    char *negative_sign;       /* (Monetary) Symbol used to indicate a negative value */
    char *currency_symbol;     /* (Monetary) Currency symbol */
    char frac_digits;          /* (Monetary) Number of digits after the decimal point */
    char p_cs_precedes;        /* (Monetary) Currency symbol is before or after the value */
    char n_cs_precedes;        /* (Monetary) Currency symbol is before or after the value */
    char p_sep_by_space;       /* (Monetary) Space exists between the sign value */
    char n_sep_by_space;       /* (Monetary) Space exists between the sign value */
    char p_sign_posn;          /* (Monetary) Position of the sign in a value */
    char n_sign_posn;          /* (Monetary) Position of the sign in a value */
    char *int_curr_symbol;     /* (International Monetary) Currency symbol */
    char int_frac_digits;      /* (International Monetary) Number of digits after the decimal point */
    char int_p_cs_precedes;    /* (International Monetary) Currency symbol is before or after the value */
    char int_n_cs_precedes;    /* (International Monetary) Currency symbol is before or after the value */
    char int_p_sep_by_space;   /* (International Monetary) Space exists between the sign value */
    char int_n_sep_by_space;   /* (International Monetary) Space exists between the sign value */
    char int_p_sign_posn;      /* (International Monetary) Position of the sign in a value */
    char int_n_sign_posn;      /* (International Monetary) Position of the sign in a value */
};

struct _ltime {
    char *mon_name[12];        /* Abbreviated month name */
    char *mon_name_long[12];   /* Full month name */
    char *wday_name[7];        /* Abbreviated week name */
    char *wday_name_long[12];  /* Full week name */
    char *datetime_format;     /* Combined date and time format */
    char *date_format;         /* Date format */
    char *time12_format;       /* 12 hour time format */
    char *time_format;         /* Time format (12 or 24, depending on locale) */
    char *am;                  /* AM string */
    char *pm;                  /* PM string */
};

struct _lctype {
    char  *codepage;           /* Character codepage name */
    unsigned short ctype[256]; /* Codepage contents */
};

typedef struct {
    int             refcount;  /* Reference count of instances with shared pointers */
    unsigned long   lcid;      /* Internal locale ID */
    char           *name;      /* Locale name */
    struct lconv    numeric;   /* Numeric/monetary information for this locale */
    struct _ltime   datetime;  /* Date/time information for this locale */
    struct _lctype  ctype;     /* Character type information for this locale */
} _locale;

extern char *setlocale(int category, const char *locale);
extern struct lconv *localeconv(void);

extern unsigned long _localeid(int category);
extern struct _ltime *_localetime(void);
extern char *_localecodepage(void);
extern unsigned short *_localectype(void);

extern void _unload_all_locales(void);

#endif /* _LOCALE_H */
