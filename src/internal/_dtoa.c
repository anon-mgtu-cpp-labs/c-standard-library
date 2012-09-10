#include "_dtoa.h"
#include "_lltoa.h"
#include "ctype.h"
#include "float.h"
#include "locale.h"
#include "math.h"
#include "stdbool.h"
#include "stdlib.h"
#include "string.h"

static bool special_value(char s[], double value, int alt_case);

/*
    @description:
        Converts a double value into its standard string representation using the given attributes.
            * precision  -- The number of decimal places in the fractional part.
            * show_sign  -- Always show a positive sign.
            * show_zeros -- Zero fill up to the requested precision.
            * show_radix -- Show a radix character if the fractional part is zero.
            * alt_case   -- Alters the case of special values.
*/
char *_dtoa(char s[], double value, int precision, int show_sign, int show_zeros, int show_radix, int alt_case)
{
    if (!special_value(s, value, alt_case)) {
        /* Locale-specific grouping and separation information */
        char *grouping = localeconv()->grouping;
        int group_size = *grouping, group_len = 0;
        double ipart, fpart;
        size_t i = 0;

        /* Set and skip any explicit sign */
        if (value < 0 || show_sign)
            s[i++] = (value < 0) ? '-' : '+';

        /* Get the integer and fractional part of the value */
        fpart = modf(fabs(value), &ipart);

        /* Build the locale-friendly integer part */
        while (fabs(ipart - 0) > 1) {
            if (group_size && group_len++ == group_size) {
                s[i++] = *localeconv()->thousands_sep;

                /*
                    Only move to the next group if it exists, 
                    otherwise use the last valid group size
                */
                if (*grouping && *++grouping)
                    group_size = *grouping;

                /* Reset the group (to 1, since we're still copying a digit) */
                group_len = 1;
            }

            s[i++] = (char)(fmod(ipart, 10) + '0');
            ipart /= 10;
        }

        /* There weren't any integer part digits */
        if (i == 0 || (!isdigit(s[0]) && i == 1))
            s[i++] = (char)((int)fmod(ipart, 10) + '0');

        s[i] = '\0';                               /* Close the string for _strrev */
        _strrev(s + (s[0] == '-' || s[0] == '+')); /* Reverse the integer part */
        strcat(s, localeconv()->decimal_point);    /* Set the radix */

        /* Build the fractional part with zero fill up to the precision */
        for (i = strlen(s); --precision >= 0; ++i) {
            fpart *= 10;
            s[i] = (char)((int)fmod(fpart, 10) + '0');
        }

        /* Remove trailing zeros if necessary */
        if (!show_zeros) {
            while (s[i - 1] == '0')
                --i;
        }

        /* Remove a trailing radix if necessary */
        if (!show_radix && s[i - 1] == *localeconv()->decimal_point)
            --i;

        s[i] = '\0';
    }

    return s;
}

/*
    @description:
        Converts a double value into its scientific string representation using the 
        given attributes. For small magnitude values unless forcing normal representation,
        the conversion will defer to _dtoa().
            * precision    -- The number of decimal places in the fractional part.
            * show_sign    -- Always show a positive sign.
            * alt_case     -- Alters the case of special values and exponent characters.
            * force_normal -- Show all values as scientific, even smaller magnitudes.
*/
char *_dtoa_normal(char s[], double value, int precision, int show_sign, int alt_case, int force_normal)
{
    if (!special_value(s, value, alt_case)) {
        int exponent = 0;
        double normal = value;

        /* Scale the value into a normal form */
        while (abs((int)normal) >= 10) { ++exponent; normal /= 10; }
        while (normal < 0.1) { --exponent; normal *= 10; }

        /* The value of -4 here is in conformance with fprintf specifications for the %g specifier */
        if (!force_normal && (precision != 0 && exponent < precision && exponent > -4)) {
            /*
                For smaller magnitude values when not forcing normal
                representation, use the standard representation.
            */
            return _dtoa(s, value, precision, show_sign, true, true, alt_case);
        }
        else {
            int radix, sign, i = 0;
            char *p;

            /* Translate the value into a string of nothing but digits */
            p = _ecvt(value, DBL_MAX_10_EXP, &radix, &sign);
            
            /* Set and skip any explicit sign */
            if (sign || show_sign)
                s[i++] = sign ? '-' : '+';

            s[i++] = radix ? *p++ : '0';           /* The integer part is always one digit */
            s[i++] = *localeconv()->decimal_point; /* Set the radix */

            /* Fill in the fractional part (with absolute zero fill to precision) */
            while (precision--)
                s[i++] = *p ? *p++ : '0';

            s[i++] = alt_case ? 'E' : 'e';
            s[i++] = (exponent < 0) ? '-' : '+';

            exponent = abs(exponent); /* Trim the sign now that we're done with it */

            /* The exponent needs to be at least three digits for consistency */
            if (exponent < 100) s[i++] = '0';
            if (exponent < 10) s[i++] = '0';

            /* Defer to _lltoa since the exponent is an integer */
            _lltoa(&s[i], exponent, 10, false, false, true);
        }
    }

    return s;
}

/*
    @description:
        Converts a double value into its hexadecimal string representation using the 
        given attributes.
            * precision  -- The number of decimal places in the fractional part.
            * show_sign  -- Always show a positive sign.
            * show_radix -- Show a radix character if the fractional part is zero.
            * alt_case   -- Alters the case of special values and exponent characters.
*/
char *_hdtoa(char s[], double value, int precision, int show_sign, int show_radix, int alt_case)
{
    if (!special_value(s, value, alt_case)) {
        int digits, end, exp;
        _real8_t fpv;

        fpv.fvalue = value; /* Break the value down into component parts */

        if (fpv.parts.sign || show_sign)
            strcat(s, fpv.parts.sign ? "-" : "+");

        /* Always produce a normalized fraction */
        strcat(s, "1");
        strcat(s, localeconv()->decimal_point);

        /* Defer to _ulltoa since we're working on 64-bit integers */
        _ulltoa(s + strlen(s), fpv.parts.mantissa, 16, alt_case, true);

        /* Set precision to the max if the caller defers to us with -1 */
        digits = precision >= 0 ? precision : _HEXFLOAT_FRAC_DIG;

        end = strlen(s) - 1; /* Locate the current end of the string */
        digits -= (end + 2); /* Determine remaining precision characters */

        if (precision > 0) {
            /* Pad with zeros to meet the requested precision */
            while (digits--)
                s[end++] = '0';
        }
        else {
            /* Truncate to meet the requested precision. Needs rounding. */
            while (digits++ && s[end] == '0')
                s[end--] = '\0';
        }

        /* Handle a trailing radix with no precision digits */
        if (!show_radix && s[end] == *localeconv()->decimal_point)
            s[end] = '\0';

        strcat(s, alt_case ? "P" : "p");

        /* This is raw IEEE 754, don't forget to trim the exponent bias */
        exp = (int)(fpv.parts.exponent - (DBL_MAX_EXP - 1));

        /* Exponents are always in base 10, defer to _lltoa because it's an int */
        _lltoa(s + strlen(s), exp, 10, true, alt_case, true);
    }

    return s;
}

/*
    @description:
        Populates s with the corresponding string representation
        and returns true if value is a special value. Otherwise
        returns false and s is set to an empty string.
*/
static bool special_value(char s[], double value, int alt_case)
{
    s[0] = '\0';

    if (isnan(value))
        strcpy(s, alt_case ? "QNAN" : "qnan");
    else if (value == _positive_infinityd())
        strcpy(s, alt_case ? "+INF" : "+inf");
    else if (value == _negative_infinityd())
        strcpy(s, alt_case ? "-INF" : "-inf");

    return s[0] != '\0';
}