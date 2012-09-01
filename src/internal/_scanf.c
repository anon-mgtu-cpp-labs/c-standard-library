#include "_fmtspec.h"
#include "_scanf.h"
#include "ctype.h"
#include "errno.h"
#include "limits.h"
#include "locale.h"
#include "stdarg.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

static size_t read_count = 0; /* Total number of characters read */
static size_t converted = 0;  /* Total number of specifiers processed */
static int error_state = 0;   /* The most recently flagged error */

static int load_charvalue(_get_func_t get, _unget_func_t unget, void *src, char s[], size_t n);
static int load_scanvalue(_get_func_t get, _unget_func_t unget, void *src, char s[], size_t n, int *scanset, int exclude);
static int load_strvalue(_get_func_t get, _unget_func_t unget, void *src, char s[], size_t n);
static int load_intvalue(_get_func_t get, _unget_func_t unget, void *src, char s[], size_t n, int is_unsigned, int is_hex);
static int load_fpvalue(_get_func_t get, _unget_func_t unget, void *src, char s[], size_t n, int is_hex);

static size_t integer_end(_get_func_t get, _unget_func_t unget, void *src, size_t n);

static int trim_leading(_get_func_t get, _unget_func_t unget, void *src);
static int match_literal(_get_func_t get, _unget_func_t unget, void *src, char match);

/*
    @description:
        Worker function for the scanf family.
*/
int _scanf(_get_func_t get, _unget_func_t unget, void *src, const char *fmt, va_list args)
{
    _scanspec_t spec = {0};

    read_count = 0;
    converted = 0;

    while (*fmt) {
        if (*fmt != '%') {
            if (isspace(*fmt)) {
                /*
                    Discard all leading whitespace from both the source and format string.
                */
                trim_leading(get, unget, src);
                
                while (isspace(*fmt))
                    ++fmt;
            }
            else {
                /* Try to match a literal character in the format string */
                if (match_literal(get, unget, src, *fmt++) == EOF)
                    break;
            }
        }
        else {
            size_t count = _load_scanspec(&spec, fmt);
            char s[BUFSIZ];
            int n = 0;

            if (count == 0) {
                /* The specifier was invalid */
                error_state = ESFMT;
                break;
            }

            fmt += count; /* Jump past the encoding specifier */

            if (spec.field_width == 0) {
                /* Assume no field width means the maximum possible */
                spec.field_width = INT_MAX;
            }

            if (spec.type == _SPEC_LITERAL) {
                /* Try to match a specifier starter character from the source */
                if (match_literal(get, unget, src, '%') == EOF)
                    break;
            }
            else if (spec.type == _SPEC_COUNT) {
                /* No encoding, the read character count was requested */
                if (!spec.suppressed)
                    *va_arg(args, int*) = read_count;
            }
            else if (spec.type == _SPEC_CHAR || spec.type == _SPEC_STRING || spec.type == _SPEC_SCANSET) {
                /* The three specifiers are similar, select which one to run */
                switch (spec.type) {
                case _SPEC_CHAR:
                    n = load_charvalue(get, unget, src, s, spec.field_width);
                    break;
                case _SPEC_STRING:
                    n = load_strvalue(get, unget, src, s, spec.field_width);
                    break;
                case _SPEC_SCANSET:
                    n = load_scanvalue(get, unget, src, s, spec.field_width, spec.scanset, spec.nomatch);
                    break;
                }

                if (n == 0 && converted == 0)
                    break;

                if (n > 0 && !spec.suppressed) {
                    /* Only %s null terminates the destination */
                    memcpy(va_arg(args, char*), s, n + (spec.type == _SPEC_STRING));
                    ++converted;
                }
            }
            else if (spec.type >= _SPEC_SCHAR && spec.type <= _SPEC_PTRDIFFT) {
                /* Extract and convert signed integer values */
                n = load_intvalue(get, unget, src, s, spec.field_width, false, spec.format == _SPEC_FMT_HEX);

                if (n == 0 && converted == 0)
                    break;

                if (n > 0 && !spec.suppressed) {
                    /* Out of range values invoke undefined behavior, so we'll play DS9000 here */
                    long long value = strtoull(s, 0, spec.format);

                    switch (spec.type) {
                    case _SPEC_SCHAR:    *va_arg(args, signed char*) = (signed char)value;   break;
                    case _SPEC_SHORT:    *va_arg(args, short*) = (short)value;               break;
                    case _SPEC_INT:      *va_arg(args, int*) = (int)value;                   break;
                    case _SPEC_LONG:     *va_arg(args, long*) = (long)value;                 break;
                    case _SPEC_LLONG:    /* Fall through */
                    case _SPEC_INTMAXT:  *va_arg(args, long long*) = value;                  break;
                    case _SPEC_PTRDIFFT: *va_arg(args, int*) = (int)value;                   break;
                    }

                    ++converted;
                }
            }
            else if (spec.type >= _SPEC_UCHAR && spec.type <= _SPEC_PTRDIFFT) {
                /* Extract and convert unsigned integer values */
                n = load_intvalue(get, unget, src, s, spec.field_width, true, spec.format == _SPEC_FMT_HEX);

                if (n == 0 && converted == 0)
                    break;

                if (n > 0 && !spec.suppressed) {
                    /* Out of range values invoke undefined behavior, so we'll play DS9000 here */
                    unsigned long long value = strtoull(s, 0, spec.format);

                    switch (spec.type) {
                    case _SPEC_UCHAR:    *va_arg(args, unsigned char*) = (unsigned char)value;   break;
                    case _SPEC_USHORT:   *va_arg(args, unsigned short*) = (unsigned short)value; break;
                    case _SPEC_UINT:     *va_arg(args, unsigned int*) = (unsigned int)value;     break;
                    case _SPEC_ULONG:    *va_arg(args, unsigned long*) = (unsigned long)value;   break;
                    case _SPEC_ULLONG:   /* Fall through */
                    case _SPEC_UINTMAXT: *va_arg(args, unsigned long long*) = value;             break;
                    case _SPEC_SIZET:    *va_arg(args, unsigned int*) = (unsigned int)value;     break;
                    }

                    ++converted;
                }
            }
            else if (spec.type >= _SPEC_FLOAT && spec.type <= _SPEC_LDOUBLE) {
                /* Extract and convert floating point values */
                n = load_fpvalue(get, unget, src, s, spec.field_width, false);

                if (n == 0 && converted == 0)
                    break;

                if (n > 0 && !spec.suppressed) {
                    /* Out of range values invoke undefined behavior, so we'll play DS9000 here */
                    switch (spec.type) {
                    case _SPEC_FLOAT:   *va_arg(args, float*) = strtof(s, 0);        break;
                    case _SPEC_DOUBLE:  *va_arg(args, double*) = strtod(s, 0);       break;
                    case _SPEC_LDOUBLE: *va_arg(args, long double*) = strtold(s, 0); break;
                    }

                    ++converted;
                }
            }
        }
    }

    return (converted == 0 || error_state) ? EOF : converted;
}

/*
    @description:
        Extracts up to n characters into the specified buffer.
*/
int load_charvalue(_get_func_t get, _unget_func_t unget, void *src, char s[], size_t n)
{
    size_t i;

    for (i = 0; i < n; ++i) {
        int ch = get(src, &read_count);

        if (ch == EOF) {
            unget(&ch, src, &read_count);
            break;
        }

        s[i] = (char)ch;
    }

    return i;
}

/*
    @description:
        Extracts up to n valid scanset characters into the specified buffer.
*/
int load_scanvalue(_get_func_t get, _unget_func_t unget, void *src, char s[], size_t n, int *scanset, int exclude)
{
    size_t i;

    for (i = 0; i < n; ++i) {
        int ch = get(src, &read_count);

        if (ch == EOF || ((exclude && scanset[ch]) || (!exclude && !scanset[ch]))) {
            unget(&ch, src, &read_count);
            break;
        }

        s[i] = (char)ch;
    }

    s[i] = '\0';

    return i;
}

/*
    @description:
        Extracts a whitespace delimited string into the specified buffer.
*/
int load_strvalue(_get_func_t get, _unget_func_t unget, void *src, char s[], size_t n)
{
    if (trim_leading(get, unget, src) == EOF)
        return 0;
    else {
        size_t i;

        for (i = 0; i < n; ++i) {
            int ch = get(src, &read_count);

            if (ch == EOF || isspace(ch)) {
                unget(&ch, src, &read_count);
                break;
            }

            s[i] = (char)ch;
        }

        s[i] = '\0';

        return i;
    }
}

/*
    @description:
        Extracts a valid integer string representation into the specified buffer.
*/
int load_intvalue(_get_func_t get, _unget_func_t unget, void *src, char s[], size_t n, int is_unsigned, int is_hex)
{
    if (trim_leading(get, unget, src) == EOF)
        return 0;
    else {
        int (*is_digit)(int) = is_hex ? isxdigit : isdigit;
        size_t i = 0, iend;

        /* Get a count of valid locale friendly integer characters */
        iend = integer_end(get, unget, src, n);

        if (!iend) {
            /* There are no valid groups */
            return 0;
        }

        /* integer_end returns the index of the last valid character, we want a count */
        n = iend + 1;

        while (n--) {
            int ch = get(src, &read_count);

            if (ch == EOF)
                break;

            /* Further error check anything that's not a thousands separator */
            if (ch != *localeconv()->thousands_sep) {
                if (i == 0 && (ch == '-' || ch == '+')) {
                    if (is_unsigned) {
                        /* The sign isn't in an expected location */
                        unget(&ch, src, &read_count);
                        break;
                    }
                }
                else if (!is_digit(ch)) {
                    if (!(is_hex && i == 1 && tolower(ch) == 'x')) {
                        /* Alternate format characters aren't in an expected location */
                        unget(&ch, src, &read_count);
                        break;
                    }

                    continue; /* Skip over alternate format characters */
                }
            }

            /* Always add even the thousands separator */
            s[i++] = (char)ch;
        }

        s[i] = '\0';

        return i;
    }
}

/*
    @description:
        Extracts a valid floating point string representation into the specified buffer.
*/
int load_fpvalue(_get_func_t get, _unget_func_t unget, void *src, char s[], size_t n, int is_hex)
{
    if (trim_leading(get, unget, src) == EOF)
        return 0;
    else {
        int (*is_digit)(int) = is_hex ? isxdigit : isdigit;
        char exponent = is_hex ? 'p' : 'e';

        bool in_exponent = false;
        bool seen_decimal = false;
        bool seen_digit = false;
        int last = EOF;
        size_t i, iend;

        iend = integer_end(get, unget, src, n);

        for (i = 0; i < n; ++i) {
            int ch = get(src, &read_count);

            if (ch == EOF)
                break;
            else if (is_digit(ch))
                seen_digit = true;
            else {
                /*
                    Only ignore the thousands separator when we're in the
                    integer part. Otherwise all subsequent checks apply 
                    and a thousands separator is erroneous.
                */
                if (i >= iend || ch != *localeconv()->thousands_sep) {
                    if (ch == '+' || ch == '-') {
                        if (last != EOF && !(in_exponent && tolower(last) == exponent)) {
                            /* The sign isn't in an expected location */
                            unget(&ch, src, &read_count);
                            break;
                        }
                    }
                    else if (tolower(ch) == 'x' && !(is_hex && i == 1)) {
                        /* Alternate format characters aren't in an expected location */
                        unget(&ch, src, &read_count);
                        break;
                    }
                    else if (tolower(ch) == exponent) {
                        if (!seen_digit) {
                            /* We can't have an exponent without a value */
                            unget(&ch, src, &read_count);
                            break;
                        }

                        in_exponent = true;
                    }
                    else if (ch == *localeconv()->decimal_point) {
                        if (in_exponent) {
                            /* The decimal isn't in an expected location */
                            unget(&ch, src, &read_count);
                            break;
                        }

                        seen_decimal = true;
                    }
                    else {
                        /* Invalid character */
                        unget(&ch, src, &read_count);
                        break;
                    }
                }
            }

            /* Always add even the thousands separator */
            s[i] = (char)ch;
            last = ch;
        }

        s[i] = '\0';

        return i;
    }
}

/*
    @description:
        Locates the end of the first valid integer string from the source.
        This function is aware of the current locale's LC_NUMERIC setting.
*/
size_t integer_end(_get_func_t get, _unget_func_t unget, void *src, size_t n)
{
    char *grouping = localeconv()->grouping;
    int group_len = 0, group_size = *grouping;
    int stack[BUFSIZ];
    int top = 0;
    size_t i = 0;

    if (!*localeconv()->thousands_sep) {
        /* Avoid potentially a lot of work if the locale doesn't support separators */
        return n;
    }

    /* Find the end of the possible characters */
    while (i++ < n && (stack[top] = get(src, &read_count)) != EOF && !isspace(stack[top]))
        ++top;

    if (i < n) {
        /* We stopped on an invalid character */
        unget(&stack[top--], src, &read_count);
    }

    while (top >= 0) {
        if (top > 0 && group_size && ++group_len == group_size) {
            if (top - 1 == 0 || stack[top - 1] != *localeconv()->thousands_sep) {
                /* Invalid group separator, mark the end and proceed */
                grouping = localeconv()->grouping;
                group_size = *grouping;
                group_len = 0;
                i = top - 1;
            }
            else {
                unget(&stack[top--], src, &read_count);

                if (*grouping && *++grouping)
                    group_size = *grouping;

                group_len = 0;
            }
        }
        else if ((stack[top] == '-' || stack[top] == '+') && top != 0) {
            /* Invalid sign: reset grouping, mark the end and proceed */
            grouping = localeconv()->grouping;
            group_size = *grouping;
            group_len = 0;
            i = top;
        }
        else if (!(stack[top] == '-' || stack[top] == '+') && _digitvalue(stack[top], 10) == -1) {
            /* Invalid digit: reset grouping, mark the end and proceed */
            grouping = localeconv()->grouping;
            group_size = *grouping;
            group_len = 0;
            i = top - 1;
        }

        unget(&stack[top--], src, &read_count);
    }

    return i;
}

/*
    @description:
        Extract and discard leading whitespace from the source.
*/
int trim_leading(_get_func_t get, _unget_func_t unget, void *src)
{
    int ch;

    do {
        if ((ch = get(src, &read_count)) == EOF)
            break;
    } while (isspace(ch));

    /* Push back the non-whitespace character that broke the loop */
    unget((char*)&ch, src, &read_count);

    return ch;
}

/*
    @description:
        Extract and match a specific character value from the source.
*/
int match_literal(_get_func_t get, _unget_func_t unget, void *src, char match)
{
    int ch = get(src, &read_count);

    /* Match a literal character */
    if (ch != match) {
        unget(&ch, src, &read_count);
        return EOF;
    }

    return ch;
}