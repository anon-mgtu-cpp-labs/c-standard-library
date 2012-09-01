#include "inttypes.h"
#include "stddef.h"
#include "stdlib.h"

/*
    @description:
        Computes the absolute value of an integer value.
*/
intmax_t imaxabs(intmax_t value)
{
    return llabs(value);
}

/*
    @description:
        Computes numer / denom and numer % denom in a single operation.
*/
imaxdiv_t imaxdiv(intmax_t numer, intmax_t denom)
{
    imaxdiv_t result;

    result.quot = numer / denom;
    result.rem = numer % denom;

    return result;
}

/*
    @description:
        Equivalent to the strto* functions, except for intmax_t.
*/
intmax_t strtoimax(const char * restrict s, char ** restrict end, int base)
{
    return strtoll(s, end, base);
}

/*
    @description:
        Equivalent to the strtou* functions, except for uintmax_t.
*/
uintmax_t strtoumax(const char * restrict s, char ** restrict end, int base)
{
    return strtoull(s, end, base);
}