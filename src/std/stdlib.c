#include "_rand.h"
#include "_sort.h"
#include "_system.h"
#include "ctype.h"
#include "errno.h"
#include "float.h"
#include "limits.h"
#include "locale.h"
#include "math.h"
#include "signal.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"

#define _EXIT_FUNC_MAX 32 /* Minimum requirement */
#define _BASE_MAX      36 /* Upper limit for integer conversions */

/* Registered exit function stacks */
static _exitfunc_t qexit_funcs[_EXIT_FUNC_MAX];
static _exitfunc_t exit_funcs[_EXIT_FUNC_MAX];
static size_t qexit_count;
static size_t exit_count;

/* Shared buffer for _ecvt and _fcvt */
static char cvtbuf[2 * DBL_MAX_10_EXP + 10];

/* 
    ===================================================
                Static helper declarations
    ===================================================
*/

static char *fpcvt(double value, int precision, int *radix, int *sign, char *buf, int all_digits);
static const char *integer_end(const char *first, const char *last, int base);
static int max_digits(int bits, int base);
static char *round_numeric(char *s);

/* 
    ===================================================
              Standard function definitions
    ===================================================
*/

/*
    @description:
        Converts the initial portion of the string pointed to by s to floating point.
            * Behavior is undefined if the string cannot be represented.
*/
double atof(const char *s)
{
    return strtod(s, NULL);
}

/*
    @description:
        Converts the initial portion of the string pointed to by s to an integer.
            * Behavior is undefined if the string cannot be represented.
*/
int atoi(const char *s)
{
    return (int)strtol(s, NULL, 10);
}

/*
    @description:
        Converts the initial portion of the string pointed to by s to an integer.
            * Behavior is undefined if the string cannot be represented.
*/
long atol(const char *s)
{
    return strtol(s, NULL, 10);
}

/*
    @description:
        Converts the initial portion of the string pointed to by s to an integer.
            * Behavior is undefined if the string cannot be represented.
*/
long long atoll(const char *s)
{
    return strtoll(s, NULL, 10);
}

/*
    @description:
        Converts the initial portion of the string pointed to by s to floating point.
*/
float strtof(const char * restrict s, char ** restrict end)
{
    return (float)strtod(s, end);
}

/*
    @description:
        Converts the initial portion of the string pointed to by s to floating point.
*/
double strtod(const char * restrict s, char ** restrict end)
{
    const char *radix = localeconv()->decimal_point;
    const char *sep = localeconv()->thousands_sep;
    size_t sep_len = strlen(sep);
    int (*is_digit)(int) = &isdigit;
    char exp_char = 'e';
    const char *it = s, *last;

    long long ipart = 0, fpart = 0, epart = 0;
    int sign = 0, esign = 0;
    int fdigits = 0;
    int base = 10;

    /* Strip leading whitespace */
    while (isspace(*it))
        ++it;

    /* Check for an integer part sign */
    if (*it == '-' || *it == '+')
        sign = (*it++ == '-');

    /* Check for a literal NAN */
    if (_strnicmp(it, "nan", 3) == 0)
        return _quiet_nand();

    /* Check for a literal INFINITY */
    if (_strnicmp(it, "inf", 3) == 0 || _strnicmp(it, "infinity", 8) == 0)
        return sign ? _negative_infinityd() : _positive_infinityd();

    if (_strnicmp(it, "0x", 2) == 0) {
        /* This will be a hexfloat representation */
        sep = "";
        sep_len = 0;
        exp_char = 'p';
        is_digit = &isxdigit;
        base = 16;

        it += 2;
    }

    /* Find the locale friendly end of the integer part */
    last = integer_end(it, it + strlen(it), base);

    /* Build the locale friendly integer part */
    while (it != last) {
        if (memcmp(it, sep, sep_len) == 0)
            it += sep_len; /* Skip over a thousands separator */
        else {
            int digit = _digitvalue(*it++, base);

            if (ipart > LLONG_MAX / (base + digit)) {
                /* If the intermediate overflows, the final result would */
                errno = ERANGE;
                return HUGE_VAL;
            }

            ipart = base * ipart + digit;
        }
    }

    if (*it == *radix) {
        /* Find the locale independent fractional part */
        while (is_digit(*++it)) {
            int digit = _digitvalue(*it, base);

            if (fpart > LLONG_MAX / (base + digit)) {
                /* If the intermediate overflows, the final result would */
                errno = ERANGE;
                return HUGE_VAL;
            }

            fpart = base * fpart + digit;
            ++fdigits;
        }
    }

    if (tolower(*it) == exp_char) {
        /* Check for an exponent part sign */
        if (*++it == '-' || *it == '+')
            esign = (*it++ == '-');

        /* Find the locale independent exponent part */
        while (isdigit(*it)) {
            epart = ((base == 10) ? 10 : 2) * epart + (*it++ - '0');

            if ((esign && -epart < DBL_MIN_EXP) || (!esign && epart > DBL_MAX_EXP)) {
                errno = ERANGE;
                return HUGE_VAL;
            }
        }

        if (esign)
            epart = -epart;
    }

    /* Save the stopping point */
    if (end)
        *end = (char*)it;

    if (base == 16) {
        /*
            Since we have the bit values for an IEEE 754 double, we 
            might as well use them to build the value directly.
        */
        unsigned long long mantissa = fpart;
        unsigned long long exponent = epart + (DBL_MAX_EXP - 1);
        _real8_t fpv;

        /* fpart needs to be in the most significant bit position */
        while ((mantissa & (1ULL << 51)) == 0)
            mantissa <<= 1;

        fpv.parts.sign = sign;
        fpv.parts.mantissa = mantissa;
        fpv.parts.exponent = exponent;

        return fpv.fvalue;
    }
    else {
        double result = (double)fpart; /* Start with the unscaled fractional part */

        /* Now scale the fractional part down to the correct decimal place */
        while (fdigits--)
            result /= base;

        result += ipart; /* Add the integer part */

        /* Next, scale the entire value according to the exponent */
        while (epart) {
            result = epart < 0 ? result / 10 : result * 10;
            epart = epart < 0 ? epart + 1 : epart - 1;
        }

        /* Finally, fix the sign and we're done */
        if (sign)
            result = -result;

        return result;
    }
}

/*
    @description:
        Converts the initial portion of the string pointed to by s to floating point.
*/
long double strtold(const char * restrict s, char ** restrict end)
{
    return strtod(s, end);
}

/*
    @description:
        Converts the initial portion of the string pointed to by s to an integer.
*/
long strtol(const char * restrict s, char ** restrict end, int base)
{
    unsigned long long temp;
    const char *it = s;

    /* Skip leading whitespace (looking for a sign) */
    while (isspace(*it))
        ++it;

    /* Using the original pointer because we must return it on failure */
    temp = strtoull(s, end, base);

    /* Handle underflow/overflow */
    if (*it == '-' && temp <= LONG_MAX) {
        errno = ERANGE;
        return LONG_MIN;
    }
    else if (*it != '-' && temp >= LONG_MAX) {
        errno = ERANGE;
        return LONG_MAX;
    }

    return (long)temp;
}

/*
    @description:
        Converts the initial portion of the string pointed to by s to an integer.
*/
long long strtoll(const char * restrict s, char ** restrict end, int base)
{
    unsigned long long temp;
    const char *it = s;

    /* Skip leading whitespace (looking for a sign) */
    while (isspace(*it))
        ++it;

    /* Using the original pointer because we must return it on failure */
    temp = strtoull(s, end, base);

    /* Handle underflow/overflow */
    if (*it == '-' && temp <= LLONG_MAX) {
        errno = ERANGE;
        return LLONG_MIN;
    }
    else if (*it != '-' && temp >= LLONG_MAX) {
        errno = ERANGE;
        return LLONG_MAX;
    }

    return (long long)temp;
}

/*
    @description:
        Converts the initial portion of the string pointed to by s to an integer.
*/
unsigned long strtoul(const char * restrict s, char ** restrict end, int base)
{
    unsigned long long temp = strtoull(s, end, base);

    /* Handle overflow */
    if (temp >= ULONG_MAX) {
        errno = ERANGE;
        return ULONG_MAX;
    }

    return (unsigned long)temp;
}

/*
    @description:
        Converts the initial portion of the string pointed to by s to an integer.
*/
unsigned long long strtoull(const char * restrict s, char ** restrict end, int base)
{
    const char *digits = "0123456789abcdefghijklmnopqrstuvwxyz";
    unsigned long long temp = 0, prev = 0;
    const char *it = s, *match, *num_end;
    char *sep = localeconv()->thousands_sep;
    size_t sep_len = strlen(sep);
    int sign = 0, n;

    /* Skip leading whitespace */
    while (isspace(*it))
        ++it;

    /* Check for and skip over any sign */
    if (*it == '-' || *it == '+')
        sign = (*it++ == '-');

    /* Handle unrealistic bases (excluding 0 because it has special meaning) */
    if (base < 0 || base == 2 || base > _BASE_MAX) {
        if (end)
            *end = (char*)s;

        return 0;
    }

    /* Extrapolate the base if it's 0 */
    if (base == 0) {
        if (*it == '0' && (it[1] && tolower(it[1]) == 'x')) {
            base = 16;

            /* Skip a leading 0x because it'll interfere later */
            it += 2;
        }
        else if (*it == '0') {
            base = 8;
        }
        else {
            base = 10;
        }
    }
    else {
        if (base == 16 && *it == '0' && (it[1] && tolower(it[1]) == 'x')) {
            /* Skip a leading 0x because it'll interfere later */
            it += 2;
        }
    }

    /* Skip leading zeros */
    while (*it == '0')
        ++it;

    /* Find the end of the first locale-friendly numeric string */
    num_end = integer_end(it, it + strlen(it), base);

    if (num_end == it) {
        /* There are no valid groups */
        if (end)
            *end = (char*)s;

        return 0;
    }

    /* Build the value */
    for (n = 0; it != num_end; ++n, ++it) {
        if (memcmp(it, sep, sep_len) == 0)
            it += sep_len; /* Skip over a thousands separator */

        match = (const char*)memchr(digits, tolower(*it), base);
        prev = temp;
        temp = base * temp + (match - digits);
    }

    if (n == 0) {
        /* No valid digits in the string */
        if (end)
            *end = (char*)s;

        return 0;
    }
    else {
        match = (const char*)memchr(digits, tolower(it[-1]), base);

        if (end)
            *end = (char*)it;

        /* Check for overflow */
        if (n >= max_digits(sizeof(unsigned long long) * CHAR_BIT, base) ||
            (temp - (match - digits)) / base != prev)
        {
            errno = ERANGE;
            return ULLONG_MAX;
        }

        if (sign)
            temp = -temp;

        return temp;
    }
}

/*
    @description:
        Causes abnormal program termination to occur.
*/
_Noreturn void abort(void)
{
    raise(SIGABRT);

    /* Catch the case where a SIGABRT handler doesn't exit */
    _Exit(EXIT_FAILURE);
}

/*
    @description:
        Registers the function pointed to by func, to be 
        called without arguments should quick_exit be called.
*/
int at_quick_exit(_exitfunc_t func)
{
    if (qexit_count == _EXIT_FUNC_MAX)
        return -1;

    qexit_funcs[qexit_count++] = func;

    return 0;
}

/*
    @description:
        Causes normal program termination to occur.

        Runs signal handlers: NO
        Runs at_quick_exit:   YES
        Runs atexit:          NO
*/
_Noreturn void quick_exit(int status)
{
    _sig_unregister_all();

    /* Run at_quick_exit functions */
    while (qexit_count-- > 0)
        (*qexit_funcs[qexit_count])();

    _Exit(status);
}

/*
    @description:
        Registers the function pointed to by func, to be 
        called without arguments at normal program termination.
*/
int atexit(_exitfunc_t func)
{
    if (exit_count == _EXIT_FUNC_MAX)
        return -1;

    exit_funcs[exit_count++] = func;

    return 0;
}

/*
    @description:
        Causes normal program termination to occur.
        
        Runs signal handlers: YES
        Runs at_quick_exit:   NO
        Runs atexit:          YES
*/
_Noreturn void exit(int status)
{
    /* Run atexit functions */
    while (exit_count-- > 0)
        (*exit_funcs[exit_count])();

    _Exit(status);
}

/*
    @description:
        Causes normal program termination to occur.
        
        Runs signal handlers: NO
        Runs at_quick_exit:   NO
        Runs atexit:          NO
*/
_Noreturn void _Exit(int status)
{
    _sig_unregister_all();
    _sys_exit(status);
}

/*
    @description:
        Searches an environment list, provided by the host environment, 
        for a string that matches the string pointed to by name.
*/
char *getenv(const char *name)
{
    return _sys_getenv(name);
}

/*
    @description:
        Passes the string pointed to by command to the system's command processor.
*/
int system(const char *command)
{
    char *cmd_proc = getenv(_SYS_CMDPROCVAR);
    char *cmd_line;
    int rc, cmd_line_len;

    /* C11 has some awkward return rules as concerns nullity */
    if (!command)
        return cmd_proc ? -1 : 0;

    if (!cmd_proc) {
        if (!(cmd_proc = _strdup(_SYS_CMDPROC))) {
            errno = ENOMEM;
            return -1;
        }
    }

    /* Set up the full command line */
    cmd_line_len = strlen(cmd_proc) + strlen(_SYS_CMDPROCARGS) + strlen(command) + 1;

    /* Using malloc here for consistency with strdup from above */
    if (!(cmd_line = (char*)malloc(cmd_line_len))) {
        free(cmd_proc);
        errno = ENOMEM;
        return -1;
    }

    strcpy(cmd_line, cmd_proc);
    strcat(cmd_line, _SYS_CMDPROCARGS);
    strcat(cmd_line, command);

    rc = _sys_system(cmd_proc, cmd_line);

    free(cmd_line);
    free(cmd_proc);

    return rc;
}

/*
    @description:
        Allocates space for an object whose alignment is specified by
        alignment, whose size is specified by size.
*/
void *aligned_alloc(size_t alignment, size_t size)
{
    uintptr_t mem;

    if (!(mem = (uintptr_t)malloc(alignment + size)))
        return NULL;

    mem += alignment - sizeof(size_t);
    *(size_t*)mem = alignment; /* Custom alignment */
    mem += sizeof(size_t);

    return (void*)mem;
}

/*
    @description:
        Allocates space for an object whose size is specified by size.
*/
void *malloc(size_t size)
{
    uintptr_t mem = (uintptr_t)_sys_heapalloc(__sys_heap, size + sizeof(size_t));

    *(size_t*)mem = 0; /* General alignment */
    mem += sizeof(size_t);

    return (void*)mem;
}

/*
    @description:
        Allocates space for an array of n objects, each of whose size 
        is specified by size.
*/
void *calloc(size_t n, size_t size)
{
    void *mem = malloc(n * size);

    if (mem)
        memset(mem, 0, n * size);

    return mem;
}

/*
    @description:
        Deallocates the old object pointed to by p and returns a
        pointer to a new object that has the size specified by size.
        The contents of the new object shall be the same as that of
        the old object prior to deallocation, up to the lesser of
        the new and old sizes.
*/
void *realloc(void *p, size_t size)
{
    /*
        realloc acts like all of malloc, free, and realloc 
        depending on the values of p and size.
    */
    if (!p) {
        /* p is a null pointer; act like malloc */
        return malloc(size);
    }
    else if (size == 0) {
        /* p is not a null pointer, but size is 0; act like free */
        free(p);
        return NULL;
    }
    else {
        /* p and size are valid, reallocate the memory with a new size */

        /*
            Take into account a pointer from aligned_alloc, but reallocate 
            to general alignment because there's no aligned_realloc.
        */
        size_t align = ((size_t*)p)[-1];
        uintptr_t old = (uintptr_t)p - (align + sizeof(size_t));
        uintptr_t mem = (uintptr_t)_sys_heaprealloc(__sys_heap, (void*)old, size - align + sizeof(size_t));

        *(size_t*)mem = 0; /* General alignment */
        mem += sizeof(size_t);

        return (void*)mem;
    }
}

/*
    @description:
        Causes the space pointed to by p to be deallocated.
*/
void free(void *p)
{
    if (p) {
        size_t align = ((size_t*)p)[-1];
        uintptr_t old = (uintptr_t)p - (align + sizeof(size_t));

        _sys_heapfree(__sys_heap, (void*)old);
    }
}

/*
    @description:
        Searches an array of n objects, the initial element of which
        is pointed to by base, for an element that matches the object
        pointed to by key. The size of each element is specified by size.
*/
void *bsearch(const void *key, const void *base, size_t n, size_t size, _cmp_func_t cmp)
{
    /* Conventional binary search */
    const char *p = (const char*)base;
    size_t lo = 0, hi = n, mid;
    int diff;

    while (lo < hi) {
        mid = lo + (hi - lo) / 2;

        if ((diff = cmp(key, p + mid * size)) == 0)
            return (void*)(p + mid * size);

        diff < 0 ? (hi = mid) : (lo = mid + 1);
    }

    return NULL;
}

/*
    @description:
        Sorts an array of n objects, the initial element of which is
        pointed to by base. The size of each element is specified by size.
*/
void qsort(void *base, size_t n, size_t size, _cmp_func_t cmp)
{
    _sort(base, n, size, cmp);
}

/*
    @description:
        Computes the absolute value of the integer value.
*/
int abs(int value)
{
    return value < 0 ? -value : value;
}

/*
    @description:
        Computes the absolute value of the integer value.
*/
long labs(long value)
{
    return value < 0L ? -value : value;
}

/*
    @description:
        Computes the absolute value of the integer value.
*/
long long llabs(long long value)
{
    return value < 0LL ? -value : value;
}

/*
    @description:
        Computes numer / denom and numer % denom in a single operation.
*/
div_t div(int numer, int denom)
{
    div_t result;

    result.quot = numer / denom;
    result.rem = numer % denom;

    return result;
}

/*
    @description:
        Computes numer / denom and numer % denom in a single operation.
*/
ldiv_t ldiv(long numer, long denom)
{
    ldiv_t result;

    result.quot = numer / denom;
    result.rem = numer % denom;

    return result;
}

/*
    @description:
        Computes numer / denom and numer % denom in a single operation.
*/
lldiv_t lldiv(long long numer, long long denom)
{
    lldiv_t result;

    result.quot = numer / denom;
    result.rem = numer % denom;

    return result;
}

/*
    @description:
        Generates a random number in the range of [0,RAND_MAX).
*/
int rand(void)
{
    return _rand();
}

/*
    @description:
        Seeds the random number generator.
*/
void srand(unsigned seed)
{
    _srand(seed);
}

/*
    @description:
        Determines the number of bytes contained in the 
        multibyte character pointed to by s.
*/
int mblen(const char *s, size_t n)
{
    if (!s)
        return 0;

    return _sys_mbtowc(0, s, n);
}

/*
    @description:
        Converts a multibyte array into a wide character.
*/
int mbtowc(wchar_t * restrict wc, const char * restrict s, size_t n)
{
    return _sys_mbtowc(wc, s, n);
}

/*
    @description:
        Converts the wide character into a multibyte array.
*/
int wctomb(char *s, wchar_t wc)
{
    return _sys_wctomb(s, &wc, 1);
}

/*
    @description:
        Converts a string of multibyte characters into a string of wide characters.
*/
size_t mbstowcs(wchar_t * restrict ws, const char * restrict s, size_t n)
{
    size_t ret = _sys_mbtowc(ws, s, n);

    if (ret != -1)
        ws[ret] = L'\0';

    return ret;
}

/*
    @description:
        Converts a string of wide characters into a string of multibyte characters.
*/
size_t wcstombs(char * restrict s, const wchar_t * restrict ws, size_t n)
{
    size_t ret = _sys_wctomb(s, ws, n);

    if (ret != -1)
        s[ret] = '\0';

    return ret;
}

/*
    @description:
        Returns the digits of the floating point value up to precision 
        for all digits. The position of the radix and sign are stored
        in corresponding [out] parameters.
*/
char *_ecvt(double value, int precision, int *radix, int *sign)
{
    return fpcvt(value, precision, radix, sign, cvtbuf, 1);
}

/*
    @description:
        Returns the digits of the floating point value up to precision 
        for all digits past the radix. The position of the radix and 
        sign are stored in corresponding [out] parameters.
*/
char *_fcvt(double value, int precision, int *radix, int *sign)
{
    return fpcvt(value, precision, radix, sign, cvtbuf, 0);
}

/* 
    ===================================================
                Static helper definitions
    ===================================================
*/

/*
    @description:
        Heavy lifter for _ecvt() and _fcvt(). Copies only the digits of the
        floating point value into a buffer. The radix position and sign of
        the value are stored in the corresponding [out] parameters.
*/
char *fpcvt(double value, int precision, int *radix, int *sign, char *buf, int all_digits)
{
    int i = 0;

    *radix = 0;
    *sign = (value < 0);
    
    value = fabs(value);

    /* Pre-scale the value into a "normal" floating style */
    while (value && value < 1) { value *= 10; --(*radix); }
    while (value >= 10) { value /= 10; ++(*radix); }

    /* Since the integer part is fixed to 1 digit, grab it directly */
    buf[i++] = ((int)value % 10) + '0';

    /* Update the radix to account for the digit we just extracted */
    ++(*radix);

    if (!all_digits) {
        /* Only count precision after the radix */
        precision += *radix;
    }

    /* Pull precision digits from the value to the requested amount */
    while (i < precision) {
        value = (value - (int)value) * 10;
        buf[i++] = (char)value + '0';
    }

    buf[i] = '\0';

    return round_numeric(buf);
}

/*
    @description:
        Locates the end of the first valid integer string starting at s.
        This function is aware of the current locale's LC_NUMERIC setting.
*/
const char *integer_end(const char *first, const char *last, int base)
{
    char *grouping = localeconv()->grouping;
    int group_len = 0, group_size = *grouping;

    const char *end = last;
    const char *it = end - 1;

    for (;;) {
        if (it == first && group_size && _digitvalue(*end, base) != -1)
            end = first;
        else if (it != first && group_size && ++group_len == group_size) {
            /* Check for a group separator */
            if (it - 1 == first || it[-1] != *localeconv()->thousands_sep) {
                /* Invalid group: reset grouping, mark the end and proceed */
                grouping = localeconv()->grouping;
                group_size = *grouping;
                group_len = 0;
                end = it; /* Save 1 past the last valid character */
            }
            else {
                /* Valid group: move to the next grouping level */
                if (*grouping && *++grouping)
                    group_size = *grouping;

                group_len = 0;

                /* Skip over the separator so we don't error on the next iteration */
                --it;
            }
        }
        else if ((*it == '-' || *it == '+') && it != first) {
            /* Invalid sign: reset grouping, mark the end and proceed */
            grouping = localeconv()->grouping;
            group_size = *grouping;
            group_len = 0;
            end = it; /* Save 1 past the last valid character */
        }
        else if (!(*it == '-' || *it == '+') && _digitvalue(*it, base) == -1) {
            /* Invalid digit: reset grouping, mark the end and proceed */
            grouping = localeconv()->grouping;
            group_size = *grouping;
            group_len = 0;
            end = it; /* Save 1 past the last valid character */
        }

        if (it == first)
            break;

        --it;
    }

    return end;
}

/*
    @description:
        Calculates the maximum number of decimal digits that can be stored
        in a value of up to the specified bits in the specified base.
*/
int max_digits(int bits, int base)
{
    return (base > 1) ? 1 + (int)(log(pow(2.0, bits)) / log((double)base)) : 0;
}

/*
    @description:
        Round up digits from the end of a numeric string to the beginning.
*/
char *round_numeric(char *s)
{
    size_t last = strlen(s) - 1;

    if (s[last] >= '5') {
        /* Kick off the rounding from 5 or higher */
        s[last] = (s[last] == '9') ? '0' : s[last] + 1;

        /* Keep going as long as there was a carry or the next digit needs rounding */
        while (s[last] == '0' || s[last - 1] >= '9') {
            if (--last == 0) {
                /* We can't extend the buffer, so hard stop at the 0th index */
                s[last] = (s[last] == '9') ? '1' : s[last] + 1;
                break;
            }

            s[last] = (s[last] == '9') ? '0' : s[last] + 1;
        }
    }

    return s;
}