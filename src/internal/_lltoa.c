#include "_lltoa.h"
#include "locale.h"
#include "stdlib.h"
#include "string.h"

/*
    @description:
        Converts a long long value into its string representation using the given attributes.
            * radix         -- The base of the represented number.
            * show_sign     -- Always show a positive sign.
            * alt_case      -- Represent hexadecimal digits in upper case.
            * ignore_locale -- Ignore the current locale in all cases if true.
*/
char *_lltoa(char s[], long long value, int radix, int show_sign, int alt_case, int ignore_locale)
{
    size_t i = 0;

    if (value < 0 || show_sign)
        s[i++] = (value < 0) ? '-' : '+';

    _ulltoa(&s[i], llabs(value), radix, alt_case, ignore_locale);

    return s;
}

/*
    @description:
        Converts an unsigned long long value into its string representation using the given attributes.
            * radix         -- The base of the represented number.
            * alt_case      -- Represent hexadecimal digits in upper case if true.
            * ignore_locale -- Ignore the current locale in all cases if true.
*/
char *_ulltoa(char s[], unsigned long long value, int radix, int alt_case, int ignore_locale)
{
    const char *digits = alt_case ? "0123456789ABCDEF" : "0123456789abcdef";

    /* Locale-specific grouping and separation information */
    char *grouping = localeconv()->grouping;
    int group_size = *grouping, group_len = 0;
    size_t i = 0;

    if (value == 0)
        s[i++] = '0'; /* Special case to simplify the deconstruction loop */
    else {
        /*
            Desconstruction loop. Build the string in reverse 
            because we don't know how many digits are present.
        */
        while (value) {
            if (!ignore_locale && group_size && group_len++ == group_size) {
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

            s[i++] = digits[value % radix];
            value /= radix;
        }
    }

    s[i--] = '\0';

    _strrev(s); /* Reverse the result, excluding the sign if present */

    return s;
}