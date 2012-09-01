#ifndef _MATH_H
#define _MATH_H

#include "_stdc11.h"

#pragma STDC FP_CONTRACT (off)

#define HUGE_VAL         (_positive_infinityd())
#define HUGE_VALF        (_positive_infinityf())
#define HUGE_VALL        HUGE_VAL

#define INFINITY         HUGE_VAL
#define NAN              (_quiet_nanf())

#define FP_INFINITE      0
#define FP_NAN           1
#define FP_NORMAL        2
#define FP_SUBNORMAL     3
#define FP_ZERO          4

#define FP_ILOGB0        (-2147483648)
#define FP_ILOGBNAN      FP_ILOGB0

#define MATH_ERRNO       1
#define MATH_ERREXCEPT   2

#define math_errhandling MATH_ERRNO

#define fpclassify(x)    (sizeof(x) == sizeof(float)  ? _fpclassifyf((float)(x)) : _fpclassifyd((double)(x)))
#define isfinite(x)      (fpclassify(x) != FP_NAN && fpclassify(x) != FP_INFINITE)
#define isinf(x)         (fpclassify(x) == FP_INFINITE)
#define isnan(x)         (fpclassify(x) == FP_NAN)
#define isnormal(x)      (fpclassify(x) == FP_NORMAL)
#define signbit(x)       (sizeof(x) == sizeof(float) ? _signbitf((float)(x)) : _signbitd((double)(x)))

typedef float float_t;
typedef double double_t;

typedef union _real4 {
    unsigned ivalue;
    float    fvalue;
    struct {
        unsigned mantissa : 23;
        unsigned exponent : 8;
        unsigned sign     : 1;
    } parts;
} _real4_t;

typedef union _real8 {
    unsigned long long ivalue;
    double             fvalue;
    struct {
        unsigned long long mantissa : 52;
        unsigned long long exponent : 11;
        unsigned long long sign     : 1;
    } parts;
} _real8_t;

typedef _real8_t _real10_t;

extern double exp(double value);
extern double frexp(double value, int *exp);
extern double ldexp(double value, int exp);
extern double log(double value);
extern double log10(double value);
extern double modf(double value, double *ipart);
extern double cbrt(double value);
extern double fabs(double value);
extern double pow(double value, double y);
extern double sqrt(double value);
extern double ceil(double value);
extern double floor(double value);
extern double fmod(double value, double y);

extern int _fpclassifyf(float value);
extern int _fpclassifyd(double value);

extern int _signbitf(float value);
extern int _signbitd(double value);

extern float _positive_infinityf(void);
extern float _negative_infinityf(void);
extern double _positive_infinityd(void);
extern double _negative_infinityd(void);
extern float _quiet_nanf(void);
extern float _signaling_nanf(void);
extern double _quiet_nand(void);
extern double _signaling_nand(void);

#endif /* _MATH_H */
