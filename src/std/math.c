#include "errno.h"
#include "fenv.h"
#include "float.h"
#include "limits.h"
#include "math.h"
#include "stddef.h"
#include "stdint.h"

#define HUGE_EXP_MAX (2 * DBL_MAX_EXP - 1)
#define HUGE_EXP_MIN (-2 * DBL_MAX_EXP)

/* Constants via Cody & Waite */
#define C1           0.693145751953125
#define C2           -2.12194440054690583e-4
#define P1           0.249999999999999993e0
#define P2           0.694360001511792852e-2
#define P3           0.165203300268279130e-4
#define Q1           0.5
#define Q2           0.555538666969001188e-1
#define Q3           0.495862884905441294e-3
#define INV_LN2      1.4426950408889634074
#define RTHALF       0.70710678118654752440

#define LN10         2.30258509299404568401

/* Coefficients for log() */
static double loga[] = {-0.64124943423745581147e2, 0.16383943563021534222e2, -0.78956112887491257267e0};
static double logb[] = {-0.76949932108494879777e3, 0.31203222091924532844e3, -0.35667977739034646171e2, 1.0};

/* 
    ===================================================
                Static helper declarations
    ===================================================
*/

static float fbitpatternf(unsigned long pattern);
static double fbitpatternd(unsigned long long pattern);
static double poly(double value, const double *table, size_t n);

/* 
    ===================================================
              Standard function definitions
    ===================================================
*/

/*
    @description:
        Computes the base-e exponential of value.
*/
double exp(double value)
{
    int sign = signbit(value);
    double xn, g, gp, q, z;
    int n;

    if (isnan(value)) {
        errno = EDOM;
        return value;
    }
    else if (value < C1 * (DBL_MIN_EXP - 1)) {
        errno = ERANGE;
        return 0.0;
    }
    else if (value > C1 * DBL_MAX_EXP) {
        errno = ERANGE;
        return HUGE_VAL;
    }

    if (sign)
        value = -value;

    xn = n = (int)(value * INV_LN2 + 0.5);
    g = (value - xn * C1) - xn * C2;

    if (sign) {
        g = -g;
        n = -n;
    }

    z = g * g;
    gp = ((P3 * z + P2) * z + P1) * g;
    q = (Q3 * z + Q2) * z + Q1;

    return ldexp(0.5 + gp / (q - gp), ++n);
}

/*
    @description:
        Breaks a floating-point number into a normalized fraction 
        and an integral power of 2.
*/
double frexp(double value, int *exp)
{
    _real8_t fpv;
    int bias = 0;

    fpv.fvalue = value;

    if (isnan(value) || isinf(value)) {
        errno = EDOM;
        *exp = 0;
        return value;
    }
    else if (fpclassify(value) == FP_ZERO) {
        *exp = 0;
        return 0;
    }
    else if (fpv.parts.exponent == 0) {
        fpv.fvalue *= (double)ULLONG_MAX + 1;
        bias = _DBL_BIT;
    }

    *exp = (int)(fpv.parts.exponent - (DBL_MAX_EXP - 1) - bias + 1);
    fpv.parts.exponent = (DBL_MAX_EXP - 1) - 1;

    return fpv.fvalue;
}

/*
    @description:
        Multiplies a floating-point number by an integral power of 2.
*/
double ldexp(double value, int exp)
{
    _real8_t fpv;
    unsigned long long old;

    fpv.fvalue = value;
    old = fpv.parts.exponent;

    if (isnan(value) || isinf(value))
        return value;
    else if (fpclassify(value) == FP_ZERO)
        return value;
    else if (exp >= HUGE_EXP_MAX) {
        errno = ERANGE;
        return HUGE_VAL;
    }
    else if (exp <= HUGE_EXP_MIN) {
        errno = ERANGE;
        return 0;
    }

    if (fpv.parts.exponent == 0) {
        fpv.fvalue *= (double)ULLONG_MAX + 1;
        exp = -_DBL_BIT;
        old = fpv.parts.exponent;
    }

    exp = (int)(old + exp);

    if (exp >= HUGE_EXP_MAX) {
        errno = ERANGE;
        return HUGE_VAL;
    }
    else if (exp > 0) {
        fpv.parts.exponent = exp;
        return fpv.fvalue;
    }

    /* Handle in the case of a denormal or underflow */
    fpv.parts.exponent = exp + _DBL_BIT;
    fpv.fvalue /= (double)ULLONG_MAX + 1;

    if (fpclassify(fpv.fvalue) == FP_ZERO)
        errno = ERANGE; /* Underflow */

    return fpv.fvalue;
}

/*
    @description:
        Computes the base-e (natural) logarithm of x.
*/
double log(double value)
{
    double num, den, z, w;
    int exp;

    if (isnan(value)) {
        errno = EDOM;
        return value;
    }
    else if (value < 0) {
        errno = EDOM;
        return -HUGE_VAL;
    }
    else if (value == 0) {
        errno = ERANGE;
        return -HUGE_VAL;
    }

    if (value > DBL_MAX)
        return value; /* Infinity or NaN */

    value = frexp(value, &exp);

    if (value > RTHALF) {
        num = (value - 0.5) - 0.5;
        den = value * 0.5 + 0.5;
    }
    else {
        num = value - 0.5;
        den = num * 0.5 + 0.5;
        --exp;
    }

    z = num / den;
    w = z * z;
    value = z + z * w * (poly(w, loga, 3) / poly(w, logb, 4));
    z = exp;
    value += z * C2;

    return value + z * C1;
}

/*
    @description:
        Computes the base-10 (common) logarithm of x.
*/
double log10(double value)
{
    if (isnan(value)) {
        errno = EDOM;
        return value;
    }
    else if (value < 0) {
        errno = EDOM;
        return -HUGE_VAL;
    }
    else if (value == 0) {
        errno = ERANGE;
        return -HUGE_VAL;
    }

    return log(value) / LN10;
}

/*
    @description:
        Breaks the argument value into integral and fractional parts,
        each of which has the same type and sign as the argument.
*/
double modf(double value, double *ipart)
{
    _real8_t fpv;

    if (-1 < value && value < 1) {
        *ipart = 0;
        return value;
    }

    fpv.fvalue = value;
    fpv.parts.sign = 0;
    fpv.parts.mantissa &= ~(_DBL_MAX_MANT >> (fpv.parts.exponent - (DBL_MAX_EXP - 1)));

    if (isnan(fpv.fvalue))
        return value;
    else if (isinf(fpv.fvalue) || fpclassify(fpv.fvalue) == FP_ZERO)
        return 0;
    else if (value > 0) {
        *ipart = fpv.fvalue;
        return value - *ipart;
    }
    else {
        *ipart = -fpv.fvalue;
        return value + *ipart;
    }
}

/*
    @description:
        Computes the real cube root of value.
*/
double cbrt(double value)
{
    if (value > 0)
        return pow(value, 1.0 / 3.0);
    else
        return -pow(-value, 1.0 / 3.0);
}

/*
    @description:
        Computes the absolute value of x.
*/
double fabs(double value)
{
    if (isnan(value)) {
        errno = EDOM;
        return value;
    }
    else if (isinf(value)) {
        errno = ERANGE;
        return _positive_infinityd();
    }
    else if (fpclassify(value) == FP_ZERO) {
        return 0;
    }

    return value < 0 ? -value : value;
}

/*
    @description:
        Computes x raised to the power of y.
*/
double pow(double value, double y)
{
    double temp;
    int sign = 0;

    if ((value == 0 && y == 0) || (value < 0 && modf(y, &temp))) {
        errno = EDOM;
        return 0;
    }
    else if (value == 0)
        return value;
    else if (value < 0) {
        if (modf(y / 2.0, &temp))
            sign = 1;

        value = -value;
    }

    value = log(value);

    if (value < 0) {
        value = -value;
        y = -y;
    }

    if (value > 1.0 && y > DBL_MAX / value) {
        errno = ERANGE;
        return sign ? -HUGE_VAL : HUGE_VAL;
    }

    value = exp(value * y);

    return sign ? -value: value;
}

/*
    @description:
        Computes the nonnegative square root of value.
*/
double sqrt(double value)
{
    double temp;
    int i, exp;

    if (isnan(value)) {
        errno = EDOM;
        return value;
    }
    else if (isinf(value))
        return value;
    else if (value <= 0) {
        if (value < 0)
            errno = EDOM;

        return 0;
    }

    temp = frexp(value, &exp);

    if (exp % 2) {
        temp *= 2;
        --exp;
    }

    temp = ldexp(temp + 1.0, exp / 2 - 1);

    for (i = 0; i < 5; ++i)
        temp = (temp + value / temp) / 2.0;

    return temp;
}

/*
    @description:
        Computes the smallest integer value not less than x.
*/
double ceil(double value)
{
    double ipart;
    return modf(value, &ipart) > 0 ? ipart + 1.0 : ipart;
}

/*
    @description:
        Computes the largest integer value not greater than x.
*/
double floor(double value)
{
    double ipart;
    return modf(value, &ipart) < 0 ? ipart - 1.0 : ipart;
}

/*
    @description:
        Rounds the argument to an integer value in floating point
        format, using the current rounding direction and without
        raising the "inexact" floating point exception.
*/
double nearbyint(double value)
{
    double rounded;
    fenv_t save;

    feholdexcept(&save);
    rounded = rint(value);
    fesetenv(&save);

    return rounded;
}

/*
    @description:
        Differs from the nearbyint function only in that the
        "inexact" floating point exception may be raised.
*/
double rint(double value)
{
    switch (fegetround()) {
    case FE_TONEAREST:  return round(value);
    case FE_TOWARDZERO: return trunc(value);
    case FE_UPWARD:     return ceil(value);
    case FE_DOWNWARD:   return floor(value);
    default:            return 0; /* Shouldn't happen */
    }
}

/*
    @description:
        Rounds the argument to the nearest integer value, rounding
        according to the current rounding direction.
*/
long lrint(double value)
{
    double rounded = rint(value);

    if (rounded > LONG_MAX) {
        errno = ERANGE;
        return LONG_MAX;
    }
    else if (rounded < LONG_MIN) {
        errno = ERANGE;
        return LONG_MIN;
    }

    return (long)rounded;
}

/*
    @description:
        Rounds the argument to the nearest integer value, rounding
        according to the current rounding direction.
*/
long long llrint(double value)
{
    double rounded = rint(value);

    if (rounded > LLONG_MAX) {
        errno = ERANGE;
        return LLONG_MAX;
    }
    else if (rounded < LLONG_MIN) {
        errno = ERANGE;
        return LLONG_MIN;
    }

    return (long long)rounded;
}

/*
    @description:
        Rounds the argument to the nearest integer in floating point
        format, rounding halfway cases away from zero, regardless of
        current rounding direction.
*/
double round(double value)
{
    int negative = value < 0.0;
    double rounded = negative ? ceil(value) : floor(value);
    double diff = negative ? rounded - value : value - rounded;

    if (diff >= 0.5)
        rounded = negative ? rounded - 1.0 : rounded + 1.0;

    return rounded;
}

/*
    @description:
        Rounds the argument to the nearest integer value, rounding 
        halfway cases away from zero, regardless of current rounding 
        direction.
*/
long lround(double value)
{
    double rounded = round(value);

    if (rounded > LONG_MAX) {
        errno = ERANGE;
        return LONG_MAX;
    }
    else if (rounded < LONG_MIN) {
        errno = ERANGE;
        return LONG_MIN;
    }

    return (long)rounded;
}

/*
    @description:
        Rounds the argument to the nearest integer value, rounding 
        halfway cases away from zero, regardless of current rounding 
        direction.
*/
long long llround(double value)
{
    double rounded = round(value);

    if (rounded > LLONG_MAX) {
        errno = ERANGE;
        return LLONG_MAX;
    }
    else if (rounded < LLONG_MIN) {
        errno = ERANGE;
        return LLONG_MIN;
    }

    return (long long)rounded;
}

/*
    @description:
        Rounds the argument to the integer value, in floating format,
        nearest to but no larger in magnitude than the argument.
*/
double trunc(double value)
{
    double ipart;

    modf(value, &ipart);
    
    return ipart;
}

/*
    @description:
        Computes the floating point remainder of value / y.
*/
double fmod(double value, double y)
{
    double temp;

    if (isnan(value) || isnan(y) || 
        isinf(value) || isinf(y) ||
        fpclassify(y) == FP_ZERO)
    {
        errno = EDOM;

        if (isnan(value))
            return  0;

        return isnan(y) ? y : _quiet_nand();
    }

    return y * modf(value / y, &temp);
}

/*
    @description:
        Produces a value with the magnitude of value and the sign of y.
*/
double copysign(double value, double y)
{
    _real8_t fpv;

    fpv.fvalue = value;
    fpv.parts.sign = signbit(y);

    return fpv.fvalue;
}

/*
    @description:
        Returns a quiet NaN, if available, with content indicated through tagp.
*/
double nan(const char *tagp)
{
    (void)tagp; /* Avoid an unused variable warning */

    /*
        nan() is defined in the standard in terms of strtod(), but
        strtod() simply returns a quiet NaN and ignores the optional
        tag information. Therefore we'll skip that step and just
        return a quiet NaN directly. :-)
    */
    return _quiet_nand();
}

/*
    @description:
        Determines the positive difference between x and y.
*/
double fdim(double x, double y)
{
    double diff;

    if (x <= y)
        return 0;

    diff = x - y;

    if (!isfinite(diff))
        errno = ERANGE;

    return diff;
}

/*
    @description:
        Determines the maximum numeric value of the arguments.
*/
double fmax(double x, double y)
{
    return (isgreaterequal(x, y) || isnan(y)) ? x : y;
}

/*
    @description:
        Determines the minimum numeric value of the arguments.
*/
double fmin(double x, double y)
{
    return (islessequal(x, y) || isnan(y)) ? x : y;
}

/*
    @description:
        Implementation helper for the fpclassify macro for float.
*/
int _fpclassifyf(float value)
{
    _real4_t fpv;

    fpv.fvalue = value;

    if (fpv.parts.exponent == 0)
        return fpv.parts.mantissa == 0 ? FP_ZERO : FP_SUBNORMAL;
    else if (fpv.parts.exponent == 0xFF)
        return fpv.parts.mantissa == 0 ? FP_INFINITE : FP_NAN;

    return FP_NORMAL;
}

/*
    @description:
        Implementation helper for the fpclassify macro for double/long double.
*/
int _fpclassifyd(double value)
{
    _real8_t fpv;

    fpv.fvalue = value;

    if (fpv.parts.exponent == 0)
        return fpv.parts.mantissa == 0 ? FP_ZERO : FP_SUBNORMAL;
    else if (fpv.parts.exponent == 0x7FF)
        return fpv.parts.mantissa == 0 ? FP_INFINITE : FP_NAN;

    return FP_NORMAL;
}

/*
    @description:
        Compares two floating point values.
*/
int _fpcompared(double a, double b)
{
    if (isnan(a) && isnan(b))
        return 0;
    else if (isnan(a))
        return +1;
    else if (isnan(b))
        return -1;
    else {
        _real8_t fpv1, fpv2;

        fpv1.fvalue = a;
        fpv2.fvalue = b;

        if (fpv1.ivalue < fpv2.ivalue)
            return -1;
        else if (fpv1.ivalue > fpv2.ivalue)
            return +1;
        else
            return 0;
    }
}

/*
    @description:
        Checks for an unordered relationship with NaNs.
*/
int _fpunoderedd(double a, double b)
{
    return isnan(a) || isnan(b);
}

/*
    @description:
        Retrieves the IEEE 754 single precision sign bit.
*/
int _signbitf(float value)
{
    _real4_t conv;
    conv.fvalue = value;
    return conv.parts.sign != 0;
}

/*
    @description:
        Retrieves the IEEE 754 double precision sign bit.
*/
int _signbitd(double value)
{
    _real8_t conv;
    conv.fvalue = value;
    return conv.parts.sign != 0;
}

/*
    IEEE 754 special values
*/
float _positive_infinityf(void) { return fbitpatternf(0x7F800000UL); }
float _negative_infinityf(void) { return fbitpatternf(0xFF800000UL); }
double _positive_infinityd(void) { return fbitpatternd(0x7FF0000000000000ULL); }
double _negative_infinityd(void) { return fbitpatternd(0xFFF0000000000000ULL); }
float _quiet_nanf(void) { return fbitpatternf(0x7FC00000UL); }
float _signaling_nanf(void) { return fbitpatternf(0x7F800001UL); }
double _quiet_nand(void) { return fbitpatternd(0x7FF8000020000000ULL); }
double _signaling_nand(void) { return fbitpatternd(0x7FF0000020000000ULL); }

/* 
    ===================================================
                Static helper definitions
    ===================================================
*/

/*
    @description:
        Converts a bit pattern (expressed as an unsigned integer) to floating-point.
*/
float fbitpatternf(unsigned long pattern)
{
    _real4_t conv;
    conv.ivalue = pattern;
    return conv.fvalue;
}

/*
    @description:
        Converts a bit pattern (expressed as an unsigned integer) to floating-point.
*/
double fbitpatternd(unsigned long long pattern)
{
    _real8_t conv;
    conv.ivalue = pattern;
    return conv.fvalue;
}

/*
    @description:
        Compute a polynomial
*/
double poly(double value, const double *table, size_t n)
{
    double y = table[0];
    size_t i;

    for (i = 1; i < n; ++i)
        y = y * value + table[i];

    return y;
}