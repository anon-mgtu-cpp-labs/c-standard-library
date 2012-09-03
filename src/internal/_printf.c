#include "_fmtspec.h"
#include "_dtoa.h"
#include "_lltoa.h"
#include "_printf.h"
#include "ctype.h"
#include "errno.h"
#include "float.h"
#include "limits.h"
#include "locale.h"
#include "math.h"
#include "stdarg.h"
#include "stdbool.h"
#include "stddef.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

static size_t write_limit = 0; /* Total number of written characters allowed */
static size_t write_count = 0; /* Total number of characters written */
static int error_state = 0;    /* The most recently flagged error */

static char *intfmt_nobuf(va_list *args, _printspec_t spec);
static char *fpfmt_nobuf(double value, int flag, int precision, int alt_fmt, int alt_case, int show_sign);

static int pad_buffer(_put_func_t put, void *dst, char *buffer, _printspec_t *spec);
static void put_padding(_put_func_t put, void *dst, char pad, intmax_t len);
static void put_buffer(_put_func_t put, void *dst, char *buffer, intmax_t len);
static void put_sign(_put_func_t put, void *dst, char **buffer);

/*
    @description:
        Worker function for the printf family.
*/
int _printf(_put_func_t put, void *dst, const char *fmt, size_t n, va_list args)
{
    _printspec_t spec = {0};

    write_limit = n;
    write_count = 0;

    while (*fmt) {
        if (*fmt != '%') {
            /* Easy case: literal format string character */
            char ch = *fmt++;

            if (!put(&ch, dst, 1, &write_count, write_limit)) {
                error_state = EIO;
                break;
            }
        }
        else {
            size_t count = _load_printspec(&spec, fmt);
            char tempbuf[2] = {0}; /* Temporary buffer for single character output */
            char *padbuf = NULL;   /* Full string encoding (buffer used) */

            if (count == 0) {
                /* The specifier was invalid */
                error_state = ESFMT;
                break;
            }

            fmt += count; /* Jump past the encoding specifier */

            if (spec.field_width == _ARG_SPECIFIED) {
                /* Extract a user-provided field width */
                spec.field_width = va_arg(args, int);
            }

            if (spec.precision == _ARG_SPECIFIED) {
                /* Extract a user-provided precision */
                spec.precision = va_arg(args, int);
            }

            /* Encode or process the argument */
            if (spec.type == _SPEC_STRING) {
                /* No encoding needed, direct string output */
                padbuf = va_arg(args, char*);
            }
            else if (spec.type == _SPEC_CHAR) {
                /* No encoding needed, direct single character output */
                tempbuf[0] = (unsigned char)va_arg(args, unsigned char);
                padbuf = tempbuf;
            }
            else if (spec.type == _SPEC_LITERAL) {
                /* No encoding needed, escaped specifier starter */
                padbuf = "%";
            }
            else if (spec.type == _SPEC_COUNT) {
                /* No output, the written character count was requested */
                *va_arg(args, int*) = write_count;
            }
            else if (spec.type >= _SPEC_SCHAR && spec.type <= _SPEC_POINTER) {
                /* Encode all integral types */

                if (spec.type >= _SPEC_UCHAR && spec.type <= _SPEC_POINTER) {
                    /* Disable showing the sign on unsigned types */
                    spec.flags &= ~_SHOW_SIGN;
                }

                padbuf = intfmt_nobuf(&args, spec);

                if (spec.format == _SPEC_FMT_OCTAL || spec.format == _SPEC_FMT_HEX) {
                    /* Don't use alternate formats on a zero value*/
                    if (spec.flags & _ALT_FORMAT && padbuf[0] == '0')
                        spec.flags &= ~_ALT_FORMAT;
                }
            }
            else if (spec.type >= _SPEC_FLOAT && spec.type <= _SPEC_LDOUBLE) {
                /* Encode all floating point types */
                padbuf = fpfmt_nobuf(
                    va_arg(args, double), 
                    spec.format, 
                    spec.precision, 
                    spec.flags & _ALT_FORMAT, 
                    spec.alt_case, 
                    spec.flags & _SHOW_SIGN);
            }

            if (padbuf) {
                /* We encoded a full buffer, so it needs to be padded */
                if (!pad_buffer(put, dst, padbuf, &spec))
                    break;
            }
        }
    }

    return error_state ? error_state : write_count;
}

// char *fpfmt_nobuf(double value, int flag, int precision, int alt_fmt, int alt_case, int show_sign)
char *intfmt_nobuf(va_list *args, _printspec_t spec)
{
    static char s[BUFSIZ];

    unsigned long long utemp = 0;
    long long stemp = 0;

    /* Load the temporary value with the next provided argument */
    switch (spec.type) {
    case _SPEC_SCHAR:    stemp = (signed short)va_arg(*args, int);            break;
    case _SPEC_SHORT:    stemp = (short)va_arg(*args, int);                   break;
    case _SPEC_INT:      stemp = (int)va_arg(*args, int);                     break;
    case _SPEC_LONG:     stemp = (long)va_arg(*args, long);                   break;
    case _SPEC_LLONG:    /* Fall through to intmax_t */
    case _SPEC_INTMAXT:  stemp = va_arg(*args, long long);                    break;
    case _SPEC_PTRDIFFT: stemp = (ptrdiff_t)va_arg(*args, ptrdiff_t);         break;
    case _SPEC_UCHAR:    stemp = (unsigned short)va_arg(*args, unsigned);     break;
    case _SPEC_USHORT:   utemp = (unsigned short)va_arg(*args, unsigned);     break;
    case _SPEC_UINT:     utemp = (unsigned)va_arg(*args, unsigned);           break;
    case _SPEC_ULONG:    utemp = (unsigned long)va_arg(*args, unsigned long); break;
    case _SPEC_ULLONG:   /* Fall through to uintmax_t */
    case _SPEC_UINTMAXT: utemp = va_arg(*args, unsigned long long);           break;
    case _SPEC_SIZET:    utemp = (size_t)va_arg(*args, size_t);               break;
    case _SPEC_POINTER:  utemp = (uintptr_t)va_arg(*args, void*);             break;
    }

    if (spec.type >= _SPEC_UCHAR && spec.type <= _SPEC_POINTER) {
        return _ulltoa(s, utemp, spec.format, spec.alt_case, false);
    }
    else {
        /* Always show a sign and let the padding algorithm deal with it */
        return _lltoa(s, stemp, spec.format, true, spec.alt_case, false);
    }
}

/*
    @description:
        Converts a double to a string using printf rules.
*/
char *fpfmt_nobuf(double value, int flag, int precision, int alt_fmt, int alt_case, int show_sign)
{
    static char s[BUFSIZ];

    if (flag == _SPEC_FMT_HEXFLOAT) {
        /* No special rules for hexfloat, just defer to _hdtoa */
        _hdtoa(s, value, precision, show_sign, alt_fmt, alt_case);
    }
    else {
        int show_zeros = 1;   /* Fill leftover precision with '0' */
        int show_radix = 0;   /* Show a decimal point even without precision */
        int force_normal = 1; /* Assume scientific only until otherwise noted */

        /* Set a reasonable default if there's no precision */
        if (precision == _NOT_SPECIFIED)
            precision = FLT_DECIMAL_DIG;

        if (flag == _SPEC_FMT_NO_FORMAT) {
            /* FMT_NO_FORMAT means either _SPEC_FMT_FLOAT or _SPEC_FMT_NORMAL, ie. %g */
            show_radix = alt_fmt ? 1 : 0;
            force_normal = 0;

            /* %g requires treating a 0 precision as 1 */
            if (precision == 0)
                precision = 1;
        }
        else {
            /* %e and %f have the same alternate format rules */
            if (precision == 0 && alt_fmt)
                show_radix = 1;

            /*
                Turn off scientific output for %f. Technically  
                not necessary, but it clarifies the intention.
            */
            if (flag == _SPEC_FMT_FLOAT)
                force_normal = 0;
        }

        if (flag == _SPEC_FMT_FLOAT) {
            /* We can always use the straight standard conversion for %f */
            _dtoa(s, value, precision, show_sign, show_zeros, show_radix, alt_case);
        }
        else {
            /*
                We can't slack with %e or %g because both produce
                (if only conditionally) normalized scientific strings.
            */
            _dtoa_normal(s, value, precision, show_sign, alt_case, force_normal);
        }
    }

    return s;
}

/*
    @description:
        Given a partially converted argument, complete the conversion
        by adding padding and prefix values according to the specifier
        The result is sent directly to the destination.
*/
int pad_buffer(_put_func_t put, void *dst, char *buffer, _printspec_t *spec)
{
    intmax_t len = strlen(buffer); /* Default our precision to the length of the string */
    intmax_t width = spec->field_width;

    if (spec->type == _SPEC_STRING) {
        /* Fit the length to a valid precision */
        if (spec->precision < len && spec->precision != -1)
            len = spec->precision;

        width -= len;

        /* String padding is *very* straightforward */
        if (spec->flags & _LEFT_JUSTIFY) {
            put_buffer(put, dst, buffer, len);
            put_padding(put, dst, ' ', width);
        }
        else {
            put_padding(put, dst, ' ', width);
            put_buffer(put, dst, buffer, len);
        }
    }
    else {
        char *prefix = "";
        size_t prefix_len = 0;

        /* Add a special alternate form prefix for octal and hexadecimal if specified */
        if (spec->flags & _ALT_FORMAT) {
            if (spec->format == _SPEC_FMT_OCTAL) {
                prefix = "0";
                prefix_len = 1;
            }
            else if (spec->format == _SPEC_FMT_HEX) {
                prefix = spec->alt_case ? "0X" : "0x";
                prefix_len = 2;
            }
        }

        if (spec->format == _SPEC_FMT_HEXFLOAT) {
            /* The leading hexadecimal starter is unconditional for hexfloat */
            prefix = spec->alt_case ? "0X" : "0x";
            prefix_len = 2;
        }

        /* Strip a leading positive sign if necessary */
        if (*buffer == '+' && !(spec->flags & _SHOW_SIGN)) {
            ++buffer;
            --len;
        }

        width -= prefix_len + len;

        /* Add a leading space if necessary */
        if (width <= 0 && spec->flags & _SHOW_SPACE && !(spec->flags & _SHOW_SIGN))
            ++width;

        /*
            Padding with leading zeros for the precision isn't specified for
            floating-point, but because the precision won't ever be greater 
            than the length of the buffer in that case, it's a non-issue.
        */
        if (spec->precision > len)
            width -= spec->precision - len;

        if (spec->flags & _LEFT_JUSTIFY) {
            /* No special padding rules apply when left justified */
            put_buffer(put, dst, prefix, prefix_len);

            /* Pad leading zeros if precision > length */
            put_padding(put, dst, '0', (intmax_t)spec->precision - len);
            put_buffer(put, dst, buffer, len);
            put_padding(put, dst, spec->pad, width);
        }
        else {
            /*
                When right justified, we need to account for leading 
                zeros after the sign rather than before.
            */
            if (spec->pad != ' ') {
                /* Show the sign first, then pad */
                put_sign(put, dst, &buffer);
                put_buffer(put, dst, prefix, prefix_len);
                put_padding(put, dst, spec->pad, width);
            }
            else {
                /* Pad first, then show the sign */
                put_padding(put, dst, spec->pad, width);
                put_sign(put, dst, &buffer);
                put_buffer(put, dst, prefix, prefix_len);
            }

            /* Pad leading zeros if precision > length */
            put_padding(put, dst, '0', (intmax_t)spec->precision - len);
            put_buffer(put, dst, buffer, len);
        }
    }

    return error_state == 0;
}

/*
    @description:
        Sends len characters specified by pad to the destination.
*/
void put_padding(_put_func_t put, void *dst, char pad, intmax_t len)
{
    if (error_state)
        return;

    while (--len >= 0) {
        if (!put(&pad, dst, 1, &write_count, write_limit)) {
            error_state = EIO;
            break;
        }
    }
}

/*
    @description:
        Sends up to len characters in the specified buffer to the destination.
*/
void put_buffer(_put_func_t put, void *dst, char *buffer, intmax_t len)
{
    intmax_t n = strlen(buffer);

    if (error_state)
        return;

    if (len < n)
        n = len;

    if (!put(buffer, dst, (size_t)n, &write_count, write_limit))
        error_state = EIO;
}

/*
    @description:
        Sends an optional sign character to the destination and
        skips over it using the buffer pointer. buffer is expected
        to point to a pointer rather than an array.
*/
void put_sign(_put_func_t put, void *dst, char **buffer)
{
    if (error_state)
        return;

    if (**buffer == '-' || **buffer == '+') {
        if (!put(*buffer, dst, 1, &write_count, write_limit))
            error_state = EIO;

        ++(*buffer);
    }
}