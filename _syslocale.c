#include "_syslocale.h"
#include "locale.h"
#include "stdlib.h"
#include "string.h"
#include "wchar.h"
#include <windows.h>

/* 
    ===================================================
                Static helper declarations
    ===================================================
*/

static char *locale_sinfo_nobuf(unsigned long lcid, unsigned long lctype);
static unsigned long locale_iinfo(unsigned long lcid, unsigned long lctype);
static wchar_t *mbstowcs_nobuf(const char *s);
static char *wcstombs_nobuf(const wchar_t *s);
static int translate_grouping_format(const char *src, char *dst, int size);
static int translate_date_format(const char *src, char *dst, int size);
static int translate_time_format(const char *src, char *dst, int size);
static BOOL CALLBACK find_locale(LPWSTR name, DWORD flags, LPARAM requested_locale);

/* 
    ===================================================
              Public function definitions
    ===================================================
*/

/*
    @description:
        Retrieves the default locale name for the system as a shared string.
*/
char *_sys_local_localename(void)
{
    static wchar_t buffer[1024];

    if (GetUserDefaultLocaleName(buffer, 1024) == 0)
        return NULL;

    return wcstombs_nobuf(buffer);
}

/*
    @description:
        Looks for an installed locale with the given name, and if found,
        populates the provided locale object with detailed locale information.
*/
int _sys_load_locale(const char *name, _locale *loc) {
    _locale temp_loc = {0};

    temp_loc.name = _strdup(name);

    if (!EnumSystemLocalesEx(find_locale, LOCALE_ALL, (LPARAM)&temp_loc, 0) || temp_loc.lcid == 0) {
        free(temp_loc.name);
        return 0;
    }

    *loc = temp_loc;

    /* Fill in locale-specific information */
    loc->numeric.decimal_point = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SDECIMAL));
    loc->numeric.thousands_sep = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_STHOUSAND));

    loc->numeric.grouping = (char*)malloc(10);
    translate_grouping_format(locale_sinfo_nobuf(loc->lcid, LOCALE_SGROUPING), loc->numeric.grouping, 10);

    loc->numeric.mon_decimal_point = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SDECIMAL));
    loc->numeric.mon_thousands_sep = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_STHOUSAND));

    loc->numeric.mon_grouping = (char*)malloc(10);
    translate_grouping_format(locale_sinfo_nobuf(loc->lcid, LOCALE_SGROUPING), loc->numeric.mon_grouping, 10);

    loc->numeric.positive_sign = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SPOSITIVESIGN));
    loc->numeric.negative_sign = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SNEGATIVESIGN));
    loc->numeric.currency_symbol = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SCURRENCY));
    loc->numeric.frac_digits = (char)locale_iinfo(loc->lcid, LOCALE_IDIGITS);
    loc->numeric.p_cs_precedes = (char)locale_iinfo(loc->lcid, LOCALE_IPOSSYMPRECEDES);
    loc->numeric.n_cs_precedes = (char)locale_iinfo(loc->lcid, LOCALE_INEGSYMPRECEDES);
    loc->numeric.p_sep_by_space = (char)locale_iinfo(loc->lcid, LOCALE_IPOSSEPBYSPACE);
    loc->numeric.n_sep_by_space = (char)locale_iinfo(loc->lcid, LOCALE_INEGSEPBYSPACE);
    loc->numeric.p_sign_posn = (char)locale_iinfo(loc->lcid, LOCALE_IPOSSIGNPOSN);
    loc->numeric.n_sign_posn = (char)locale_iinfo(loc->lcid, LOCALE_INEGSIGNPOSN);

    /* C11 additions for international ISO 4217, basically ignored for now */
    loc->numeric.int_curr_symbol = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SCURRENCY));
    loc->numeric.int_frac_digits = (char)locale_iinfo(loc->lcid, LOCALE_IDIGITS);
    loc->numeric.int_p_cs_precedes = (char)locale_iinfo(loc->lcid, LOCALE_IPOSSYMPRECEDES);
    loc->numeric.int_n_cs_precedes = (char)locale_iinfo(loc->lcid, LOCALE_INEGSYMPRECEDES);
    loc->numeric.int_p_sep_by_space = (char)locale_iinfo(loc->lcid, LOCALE_IPOSSEPBYSPACE);
    loc->numeric.int_n_sep_by_space = (char)locale_iinfo(loc->lcid, LOCALE_INEGSEPBYSPACE);
    loc->numeric.int_p_sign_posn = (char)locale_iinfo(loc->lcid, LOCALE_IPOSSIGNPOSN);
    loc->numeric.int_n_sign_posn = (char)locale_iinfo(loc->lcid, LOCALE_INEGSIGNPOSN);

    /* Month names */
    loc->datetime.mon_name_long[0] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SMONTHNAME1));
    loc->datetime.mon_name_long[1] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SMONTHNAME2));
    loc->datetime.mon_name_long[2] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SMONTHNAME3));
    loc->datetime.mon_name_long[3] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SMONTHNAME4));
    loc->datetime.mon_name_long[4] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SMONTHNAME5));
    loc->datetime.mon_name_long[5] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SMONTHNAME6));
    loc->datetime.mon_name_long[6] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SMONTHNAME7));
    loc->datetime.mon_name_long[7] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SMONTHNAME8));
    loc->datetime.mon_name_long[8] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SMONTHNAME9));
    loc->datetime.mon_name_long[9] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SMONTHNAME10));
    loc->datetime.mon_name_long[10] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SMONTHNAME11));
    loc->datetime.mon_name_long[11] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SMONTHNAME12));

    loc->datetime.mon_name[0] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SABBREVMONTHNAME1));
    loc->datetime.mon_name[1] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SABBREVMONTHNAME2));
    loc->datetime.mon_name[2] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SABBREVMONTHNAME3));
    loc->datetime.mon_name[3] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SABBREVMONTHNAME4));
    loc->datetime.mon_name[4] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SABBREVMONTHNAME5));
    loc->datetime.mon_name[5] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SABBREVMONTHNAME6));
    loc->datetime.mon_name[6] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SABBREVMONTHNAME7));
    loc->datetime.mon_name[7] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SABBREVMONTHNAME8));
    loc->datetime.mon_name[8] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SABBREVMONTHNAME9));
    loc->datetime.mon_name[9] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SABBREVMONTHNAME10));
    loc->datetime.mon_name[10] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SABBREVMONTHNAME11));
    loc->datetime.mon_name[11] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SABBREVMONTHNAME12));

    /* Week names */
    loc->datetime.wday_name_long[0] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SDAYNAME1));
    loc->datetime.wday_name_long[1] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SDAYNAME2));
    loc->datetime.wday_name_long[2] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SDAYNAME3));
    loc->datetime.wday_name_long[3] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SDAYNAME4));
    loc->datetime.wday_name_long[4] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SDAYNAME5));
    loc->datetime.wday_name_long[5] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SDAYNAME6));
    loc->datetime.wday_name_long[6] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SDAYNAME7));

    loc->datetime.wday_name[0] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SABBREVDAYNAME1));
    loc->datetime.wday_name[1] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SABBREVDAYNAME2));
    loc->datetime.wday_name[2] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SABBREVDAYNAME3));
    loc->datetime.wday_name[3] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SABBREVDAYNAME4));
    loc->datetime.wday_name[4] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SABBREVDAYNAME5));
    loc->datetime.wday_name[5] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SABBREVDAYNAME6));
    loc->datetime.wday_name[6] = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_SABBREVDAYNAME7));

    loc->datetime.date_format = (char*)malloc(80);
    translate_date_format(locale_sinfo_nobuf(loc->lcid, LOCALE_SSHORTDATE), loc->datetime.date_format, 80);

    loc->datetime.time_format = (char*)malloc(80);
    translate_time_format(locale_sinfo_nobuf(loc->lcid, LOCALE_STIMEFORMAT), loc->datetime.time_format, 80);

    /* There's no Windows locale specifier for combined date and time */
    loc->datetime.datetime_format = (char*)malloc(160 + 1);
    strcpy(loc->datetime.datetime_format, loc->datetime.date_format);
    strcat(loc->datetime.datetime_format, " ");
    strcat(loc->datetime.datetime_format, loc->datetime.time_format);

    loc->datetime.time12_format = _strdup(loc->datetime.time_format);

    /* Replace the 24 hour specifier with the 12 hour specifier if necessary */
    {
        char *p = strstr(loc->datetime.time12_format, "%H");

        if (p)
            p[1] = 'I';
    }

    loc->datetime.am = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_S1159));
    loc->datetime.pm = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_S2359));

    loc->ctype.codepage = _strdup(locale_sinfo_nobuf(loc->lcid, LOCALE_IDEFAULTCODEPAGE));

    /* Populate the codepage character table */
    if (atoi(loc->ctype.codepage)) {
        CPINFO info = {0};
        int i;

        if (GetCPInfo(atoi(loc->ctype.codepage), &info)) {
            for (i = 0; i < 256; i++) {
                if (!(loc->ctype.ctype[i] & 0x8000))
                    GetStringTypeA(loc->lcid, CT_CTYPE1, (char*)&i, 1, &loc->ctype.ctype[i]);
            }
        }
    }

    return 1;
}

/*
    @description:
        Compares two strings according to the specified locale ID.
*/
int _sys_strcoll(unsigned long lcid, const char *a, const char *b)
{
    int diff = CompareStringA(lcid, 0, a, strlen(a), b, strlen(b));

    if (diff) {
        /* See documentation for CompareStringEx */
        diff -= 2;
    }

    return diff;
}

/*
    @description:
        Compares two strings according to the specified locale ID.
*/
int _sys_strxfrm(unsigned long lcid, const char *dst, const char *src)
{
    return LCMapStringA(lcid, LCMAP_LOWERCASE, src, strlen(src), (char*)dst, strlen(dst));
}

/* 
    ===================================================
                Static helper definitions
    ===================================================
*/

/*
    @description:
        Retrieves the specified locale information as a shared string.
*/
char *locale_sinfo_nobuf(unsigned long lcid, unsigned long lctype) {
    static char buf[1024];
    int n = GetLocaleInfoA(lcid, lctype, buf, 0);

    if (n) {
        buf[GetLocaleInfoA(lcid, lctype, buf, n)] = '\0';
        return buf;
    }

    return NULL;
}

/*
    @description:
        Retrieves the specified locale information as an integer.
*/
unsigned long locale_iinfo(unsigned long lcid, unsigned long lctype) {
    unsigned long value;

    GetLocaleInfoA(lcid, lctype, (char*)&value, sizeof value);

    return value;
}

/*
    @description:
        Same as mbstowcs except using a shared buffer instead of 
        a buffer parameter. Returns NULL if mbstowcs would have
        returned -1.
*/
wchar_t *mbstowcs_nobuf(const char *s)
{
    static wchar_t buf[1024];

    if (mbstowcs(buf, s, (size_t)-1) == (size_t)-1)
        return NULL;

    return buf;
}

/*
    @description:
        Same as wcstombs except using a shared buffer instead of 
        a buffer parameter. Returns NULL if wcstombs would have
        returned -1.
*/
char *wcstombs_nobuf(const wchar_t *s)
{
    static char buf[1024];

    if (wcstombs(buf, s, (size_t)-1) == (size_t)-1)
        return NULL;

    return buf;
}

/*
    @description:
        Translates a standard Windows grouping format into ISO C format.
*/
int translate_grouping_format(const char *src, char *dst, int size)
{
    int i, k = 0;

    dst[0] = '\0';

    for (i = 0; i < size && src[i]; ++i) {
        if (src[i] != ';')
            *dst++ = src[i] - '0';
    }

    dst[k] = '\0';

    return k;
}

/*
    @description:
        Translates a standard Windows date format into a strftime-compatible format.
*/
int translate_date_format(const char *src, char *dst, int size)
{
    int i, k = 0;

    dst[0] = '\0';

    for (i = 0; i < size && src[i]; ++i) {
        char ch = src[i];

        switch (ch) {
        case 'd':
            if (src[i + 1] != ch || src[++i + 1] != ch)
                strcat(dst, "%d"); /* Numeric day of the month */
            else if (src[++i + 1] != ch)
                strcat(dst, "%a"); /* Abbreviated day of the week name */
            else
                strcat(dst, "%A"); /* Full day of the week name */

            k += 2;
            break;
        case 'M':
            if (src[i + 1] != ch || src[++i + 1] != ch)
                strcat(dst, "%m"); /* Numeric month of the year */
            else if (src[++i + 1] != ch)
                strcat(dst, "%b"); /* Abbreviated month name */
            else
                strcat(dst, "%B"); /* Full month name */

            k += 2;
            break;
        case 'y':
            if (src[i + 1] != ch || src[++i + 1] != ch)
                strcat(dst, "%y"); /* 2 digit year */
            else if (src[++i + 1] != ch)
                strcat(dst, "%Y"); /* At least 3 digit year (always use 4 digits) */
            else if (src[++i + 1] != ch)
                strcat(dst, "%Y"); /* 4 digit year */
            else {
                strcat(dst, "0%Y"); /* 5 digit year (always prepend 0) */
                ++k; /* To account for the leading zero */
            }

            k += 2;
            break;

        /* Ignore all time parts just in case */
        case 'h': case 'H': case 'm': case 's':
        case 'f': case 'F': case 'g': case 'G':
        case 'K': case 't': case 'T': case 'z':
            while (src[i + 1] == ch)
                ++i;

            break;
        default:
            dst[k++] = ch; 
            dst[k] = '\0';
            break;
        }
    }

    dst[k] = '\0';

    /* Trim trailing whitespace */
    while (dst[k - 1] == ' ')
        dst[--k] = '\0';

    return k;
}

/*
    @description:
        Translates a standard Windows time format into a strftime-compatible format.
*/
int translate_time_format(const char *src, char *dst, int size)
{
    int i, k = 0;

    dst[0] = '\0';

    for (i = 0; i < size && src[i]; ++i) {
        int skip_double = 0;
        char ch = src[i];

        switch (ch) {
        case 'h':
            strcat(dst, "%I");
            k += 2;
            skip_double = 1;
            break;
        case 'H':
            strcat(dst, "%H");
            k += 2;
            skip_double = 1;
            break;
        case 'm':
            strcat(dst, "%M");
            k += 2;
            skip_double = 1;
            break;
        case 's':
            strcat(dst, "%S");
            k += 2;
            skip_double = 1;
            break;
        case 'f':
        case 'F':
            /* Ignore fractions of a second (see below) */
        case 'g':
        case 'G':
            /* Ignore era (see below) */
        case 'K':
            /* Ignore time zone (see below) */
        case 't':
        case 'T':
            /* Ignore AM/PM (see below) */
        case 'z':
            /* Ignore UTC offset */
            while (src[i + 1] == ch)
                ++i;

            break;
        default: 
            dst[k++] = ch; 
            dst[k] = '\0';
            break;
        }

        /* Take care not to skip duplicates unless we matched a specifier */
        if (skip_double && src[i + 1] == ch)
            ++i;
    }

    dst[k] = '\0';

    /* Trim trailing whitespace */
    while (dst[k - 1] == ' ')
        dst[--k] = '\0';

    return k;
}

/*
    @description:
        Callback for EnumSystemLocalesEx. Attempts to match an installed
        locale with the requested locale information from the caller.
*/
BOOL CALLBACK find_locale(LPWSTR name, DWORD flags, LPARAM requested_locale)
{
    _locale *loc = (_locale*)requested_locale;

    (void)flags; /* Suppress unused variable warning */

    if (wcscmp(name, mbstowcs_nobuf(loc->name)) == 0) {
        loc->lcid = LocaleNameToLCID(name, 0);
        return FALSE;
    }

    return TRUE;
}