#ifndef _SIGNAL_H
#define _SIGNAL_H

typedef int sig_atomic_t;
typedef void (*_sigfunc_t)(int);

#define SIGINT  2    /* Receipt of an interactive attention signal */
#define SIGILL  4    /* Detection of an illegal function image, such as an invalid instruction */
#define SIGFPE  8    /* Erroneous arithmatic operation */
#define SIGSEGV 11   /* Invalid access to storage */
#define SIGTERM 15   /* Termination request sent to the program */
#define SIGABRT 22   /* Abnormal termination */

#define SIG_DFL ((_sigfunc_t)0)  /* Default handler return from signal */
#define SIG_ERR ((_sigfunc_t)-1) /* Error handler return from signal */
#define SIG_IGN ((_sigfunc_t)1)  /* Ignore handler return from signal */

extern _sigfunc_t signal(int sig, _sigfunc_t func);
extern int raise(int sig);

extern void _sig_unregister(int sig);
extern void _sig_unregister_all(void);

#endif /* _SIGNAL_H */
