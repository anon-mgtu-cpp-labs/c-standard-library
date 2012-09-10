#include "fenv.h"
#include "float.h"

static fenv_t default_environment = 0;
static fenv_t current_environment = 0;
static int round_direction = FLT_ROUNDS;

/*
    @description:
        Gets the curent rounding direction.
*/
int fegetround(void)
{
    return round_direction;
}

/*
    @description:
        Sets the current rounding direction.
*/
int fesetround(int round)
{
    if (round < FE_TOWARDZERO || round > FE_DOWNWARD)
        return -1;

    round_direction = round;

    return 0;
}

/*
    @description:
        Attempts to store the current floating point 
        environment in the object pointed to by envp.
*/
int fegetenv(fenv_t *envp)
{
    *envp = *_current_fenv();
    return 0;
}

/*
    @description:
        Saves the current floating point environemnt
        in the object pointed to by envp, clears the
        floating point status flags, and then installs
        a non-stop mode, if available, for all exceptions.
*/
int feholdexcept(fenv_t *envp)
{
    *envp = *_current_fenv();
    return 0;
}

/*
    @description:
        Attempts to establish the floating point environment
        represented by the object pointed to by envp.
*/
int fesetenv(const fenv_t *envp)
{
    current_environment = *envp;
    return 0;
}

/*
    @description:
        Attempts to save the currently raised floating point
        exceptions in its automatic storage, install the floating
        point environment represented by the object pointed to by
        envp, and then raise the saved floating point exceptions.
*/
int feupdateenv(const fenv_t *envp)
{
    current_environment = *envp;
    return 0;
}

/*
    @description:
        Retrieves the default floating point environment.
*/
fenv_t *_default_fenv()
{
    return &default_environment;
}

/*
    @description:
        Retrieves the current floating point environment.
*/
fenv_t *_current_fenv()
{
    return &current_environment;
}