#ifndef _FENV_H
#define _FENV_H

typedef unsigned fenv_t;
typedef unsigned fexcept_t;

#if 0 /* math_errhandling & MATH_ERREXCEPT == 0 */
#define FE_DIVBYZERO  0x01
#define FE_INEXACT    0x02
#define FE_INVALID    0x04
#define FE_OVERFLOW   0x08
#define FE_UNDERFLOW  0x10
#endif

#define FE_ALL_EXCEPT 0 /* No error macros defined */

/* Corresponds to FLT_ROUNDS values */
#define FE_TOWARDZERO 0
#define FE_TONEAREST  1
#define FE_UPWARD     2
#define FE_DOWNWARD   3

#define FE_DFL_ENV    (_default_fenv())

extern int fegetround(void);
extern int fesetround(int round);

extern int fegetenv(fenv_t *envp);
extern int feholdexcept(fenv_t *envp);
extern int fesetenv(const fenv_t *envp);
extern int feupdateenv(const fenv_t *envp);

extern fenv_t *_default_fenv();
extern fenv_t *_current_fenv();

#endif /* _FENV_H */
