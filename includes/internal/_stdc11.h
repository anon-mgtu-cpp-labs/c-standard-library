#ifndef __STDC11_H
#define __STDC11_H

#if __STDC_VERSION__ < 199901L

/* Introduce stuff from C99 */
#define restrict   __restrict
#define _Pragma    __pragma
#define _Complex
#define _Imaginary

typedef unsigned char _Bool;

#define __func__    __FUNCTION__
#define STDC
#define FP_CONTRACT fp_contract

#endif

#if __STDC_VERSION__ <= 199901L

/* Introduce stuff from C11 */
#define _Alignas
#define _Alignof
#define _Atomic
#define _Generic
#define _Noreturn
#define _Static_assert
#define _Thread_local

#endif

#endif
