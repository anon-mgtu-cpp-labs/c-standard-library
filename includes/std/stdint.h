#ifndef _STDINT_H
#define _STDINT_H

typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

typedef int8_t             int_least8_t;
typedef int16_t            int_least16_t;
typedef int32_t            int_least32_t;
typedef int64_t            int_least64_t;

typedef uint8_t            uint_least8_t;
typedef uint16_t           uint_least16_t;
typedef uint32_t           uint_least32_t;
typedef uint64_t           uint_least64_t;

typedef int8_t             int_fast8_t;
typedef int16_t            int_fast16_t;
typedef int32_t            int_fast32_t;
typedef int64_t            int_fast64_t;

typedef uint8_t            uint_fast8_t;
typedef uint16_t           uint_fast16_t;
typedef uint32_t           uint_fast32_t;
typedef uint64_t           uint_fast64_t;

typedef int32_t            intptr_t;
typedef uint32_t           uintptr_t;

typedef int64_t            intmax_t;
typedef uint64_t           uintmax_t;

#define INT8_MIN           (-128)
#define INT16_MIN          (-32768)
#define INT32_MIN          (-2147483648)
#define INT64_MIN          (-9223372036854775808)

#define INT8_MAX           127
#define INT16_MAX          32767
#define INT32_MAX          2147483647
#define INT64_MAX          9223372036854775807

#define UINT8_MAX          255
#define UINT16_MAX         65535
#define UINT32_MAX         4294967295
#define UINT64_MAX         18446744073709551615

#define INT_LEAST8_MIN     INT8_MIN
#define INT_LEAST16_MIN    INT16_MIN
#define INT_LEAST32_MIN    INT32_MIN
#define INT_LEAST64_MIN    INT64_MIN

#define INT_LEAST8_MAX     INT8_MAX
#define INT_LEAST16_MAX    INT16_MAX
#define INT_LEAST32_MAX    INT32_MAX
#define INT_LEAST64_MAX    INT64_MAX

#define UINT_LEAST8_MAX    UINT8_MAX
#define UINT_LEAST16_MAX   UINT16_MAX
#define UINT_LEAST32_MAX   UINT32_MAX
#define UINT_LEAST64_MAX   UINT64_MAX

#define INT_FAST8_MIN      INT8_MIN
#define INT_FAST16_MIN     INT16_MIN
#define INT_FAST32_MIN     INT32_MIN
#define INT_FAST64_MIN     INT64_MIN

#define INT_FAST8_MAX      INT8_MAX
#define INT_FAST16_MAX     INT16_MAX
#define INT_FAST32_MAX     INT32_MAX
#define INT_FAST64_MAX     INT64_MAX

#define UINT_FAST8_MAX     UINT8_MAX
#define UINT_FAST16_MAX    UINT16_MAX
#define UINT_FAST32_MAX    UINT32_MAX
#define UINT_FAST64_MAX    UINT64_MAX

#define INTPTR_MIN         INT32_MIN
#define INTPTR_MAX         INT32_MAX
#define UINTPTR_MAX        UINT32_MAX

#define INTMAX_MIN         INT64_MIN
#define INTMAX_MAX         INT64_MAX
#define UINTMAX_MAX        UINT64_MAX

#define PTRDIFF_MIN        INT32_MIN
#define PTRDIFF_MAX        INT32_MAX

#define SIG_ATOMIC_MIN     INT_MIN
#define SIG_ATOMIC_MAX     INT_MAX

#undef  SIZE_MAX
#define SIZE_MAX           UINT32_MAX

/* WCHAR_MIN/WCHAR_MAX are also defined in wchar.h */
#ifndef _HAS_WCHARMAX
#define _HAS_WCHARMAX
#define WCHAR_MIN          0
#define WCHAR_MAX          UINT16_MAX
#endif

#define WINT_MIN           0
#define WINT_MAX           UINT16_MAX

#if defined(__STD_WANT_LIB_EXT1__) && __STD_WANT_LIB_EXT1__ != 0
#ifdef __STD_LIB_EXT1__
#define RSIZE_MAX          (SIZE_MAX >> 1)
#endif
#endif

#define INT8_C(value)      value
#define INT16_C(value)     value
#define INT32_C(value)     value##L
#define INT64_C(value)     value##LL

#define UINT8_C(value)     value
#define UINT16_C(value)    value##U
#define UINT32_C(value)    value##UL
#define UINT64_C(value)    value##ULL

#define INTMAX_C           INT64_C
#define UINTMAX_C          UINT64_C

#endif /* _STDINT_H */
