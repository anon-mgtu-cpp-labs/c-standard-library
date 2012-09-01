#include "_system.h"
#include "errno.h"
#include "signal.h"
#include "stdlib.h"

struct _sig_handler {
    int        sig;  /* The signal being handled */
    _sigfunc_t func; /* Handler function for the signal */
};

/* Hook all signals into the default handler initially */
static struct _sig_handler handlers[] = {
    {SIGINT,  SIG_DFL},
    {SIGILL,  SIG_DFL},
    {SIGFPE,  SIG_DFL},
    {SIGSEGV, SIG_DFL},
    {SIGTERM, SIG_DFL},
    {SIGABRT, SIG_DFL},
};

/*
    @description:
        Sets the handler for the specified signal.
*/
_sigfunc_t signal(int sig, _sigfunc_t func)
{
    size_t i;

    /* Locate the handler for this signal */
    for (i = 0; i < sizeof handlers / sizeof *handlers; ++i) {
        if (sig == handlers[i].sig)
            return handlers[i].func = func;
    }

    /* The signal wasn't found */
    errno = EINVAL;

    return SIG_ERR;
}

/*
    @description:
        Executes the handler specified by sig.
*/
int raise(int sig)
{
    _sigfunc_t func = SIG_ERR;
    size_t i;

    /* Locate the handler for this signal */
    for (i = 0; i < sizeof handlers / sizeof *handlers; ++i) {
        if (sig == handlers[i].sig) {
            func = handlers[i].func;
            break;
        }
    }

    if (func == SIG_ERR) {
        /* The signal wasn't found, or someone registered SIG_ERR... */
        return -1;
    }
    else if (func == SIG_IGN) {
        /* Ignore this signal */
        return 0;
    }
    else if (func == SIG_DFL) {
        /* Default behavior is failure termination */
        _sys_exit(EXIT_FAILURE);
    }
    else {
        /* Run the registered handler */
        func(sig);
    }

    return 0;
}

/*
    @description:
        Unregister the specified signal such that raising it will be a no-op.
*/
void _sig_unregister(int sig)
{
    size_t i;

    /* Locate the handler for this signal */
    for (i = 0; i < sizeof handlers / sizeof *handlers; ++i) {
        if (sig == handlers[i].sig) {
            /* Setting the signal to be ignored is easiest */
            handlers[i].func = SIG_IGN;
            break;
        }
    }
}

/*
    @description:
        Unregister all signals such that raising them will be a no-op.
*/
void _sig_unregister_all(void)
{
    size_t i;

    for (i = 0; i < sizeof handlers / sizeof *handlers; ++i) {
        /* Setting the signal to be ignored is easiest */
        handlers[i].func = SIG_IGN;
    }
}