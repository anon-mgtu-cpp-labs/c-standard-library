#ifndef _FLOAT_H
#define _FLOAT_H

#define FLT_EVAL_METHOD  0
#define FLT_ROUNDS       1

#define FLT_RADIX        2
#define FLT_MANT_DIG     24
#define FLT_DECIMAL_DIG  6
#define FLT_DIG          6
#define FLT_MIN_EXP      (-125)
#define FLT_MIN_10_EXP   (-37)
#define FLT_MAX_EXP      128
#define FLT_MAX_10_EXP   38
#define FLT_MAX          3.402823466e+38F
#define FLT_EPSILON      1.192092896e-07F
#define FLT_MIN          1.175494351e-38F
#define FLT_TRUE_MIN     FLT_MIN

#define DBL_MANT_DIG     53
#define DBL_DECIMAL_DIG  10
#define DBL_DIG          15
#define DBL_MIN_EXP      (-1021)
#define DBL_MIN_10_EXP   (-307)
#define DBL_MAX_EXP      1024
#define DBL_MAX_10_EXP   308
#define DBL_MAX          1.7976931348623158e+308
#define DBL_EPSILON      2.2204460492503131e-016
#define DBL_MIN          2.2250738585072014e-308
#define DBL_TRUE_MIN     DBL_MIN

#define LDBL_MANT_DIG    DBL_MANT_DIG
#define LDBL_DECIMAL_DIG DBL_DECIMAL_DIG
#define LDBL_DIG         DBL_DIG
#define LDBL_MIN_EXP     DBL_MIN_EXP
#define LDBL_MIN_10_EXP  DBL_MIN_10_EXP
#define LDBL_MAX_EXP     DBL_MAX_EXP
#define LDBL_MAX_10_EXP  DBL_MAX_10_EXP
#define LDBL_MAX         DBL_MAX
#define LDBL_EPSILON     DBL_EPSILON
#define LDBL_MIN         DBL_MIN
#define LDBL_TRUE_MIN    DBL_TRUE_MIN

#define DECIMAL_DIG      LDBL_DECIMAL_DIG

#define _FLT_BIT         32
#define _DBL_BIT         64
#define _LDBL_BIT        _DBL_BIT

#define _DBL_MAX_MANT    0xfffffffffffffULL

#define _HEXFLOAT_FRAC_DIG 13

#endif /* _FLOAT_H */
