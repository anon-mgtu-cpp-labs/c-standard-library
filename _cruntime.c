#include "_deque.h"
#include "_system.h"
#include "_time.h"
#include "locale.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"

static char **sys_argv = 0;
static int    sys_argc = 0;

static void init_stdio();
static void finalize_stdio();
static void set_defaults();
static void parse_cmdline(const char *s);

int main();
int main(int argc, char **argv);

int _main_init()
{
#define _HEAP_ALLOC_ERROR 13859

    int rc;

    _init_clock();

    /* Allocate a central heap for the C memory manager */
    if (!(__sys_heap = _sys_heapcreate()))
        _sys_exit(_HEAP_ALLOC_ERROR);

    init_stdio();
    set_defaults();

    parse_cmdline(_sys_commandline());
    rc = main(sys_argc, sys_argv);

    /* Tidy up the memory for argv */
    while (--sys_argc > 0)
        _sys_free(sys_argv[sys_argc]);
    _sys_free(sys_argv);

    _unload_all_locales();
    finalize_stdio();

    _sys_heapdestroy(&__sys_heap);
    
    return rc;

#undef _HEAP_ALLOC_ERROR
}

void init_stdio()
{
    __io_buf[0].fd = __sys_stdin = _sys_stdin();
    _deque_init(__io_buf[0].buf, __stdin_buf, BUFSIZ);
    _deque_init(__io_buf[0].unget, __stdin_unget, _UNGETSIZ);
    __io_buf[0].flag = _LBF | _READ | _TEXT | _OPEN;

    __io_buf[1].fd = __sys_stdout = _sys_stdout();
    _deque_init(__io_buf[1].buf, __stdout_buf, BUFSIZ);
    __io_buf[1].flag = _LBF | _WRITE | _TEXT | _OPEN;

    __io_buf[2].fd = __sys_stderr = _sys_stderr();
    _deque_init(__io_buf[2].buf, __stdin_buf, BUFSIZ);
    __io_buf[2].flag = _NBF | _WRITE | _TEXT | _OPEN;
}

void finalize_stdio()
{
    int i;

    for (i = 0; i < FOPEN_MAX; ++i)
        fclose(&__io_buf[i]);
}

void set_defaults()
{
    setlocale(LC_ALL, "C");
    srand(1);
}

static void parse_cmdline(const char *s)
{
#define _CMDLINE_DELIM(c) ((c) == ' ' || (c) == '\t')
#define _CMDLINE_QUOTE(c) ((c) == '"')

    char arg[1024];
    char **temp;
    int n = 0;

    /* Always allocate at least one spot for the trailing null pointer */
    sys_argv = (char**)_sys_alloc(sizeof *sys_argv);

    while (*s) {
        if (_CMDLINE_QUOTE(*s)) {
            /* Restart the counter and copy everything up to the next unescaped quote */
            for (n = 0; *++s && !_CMDLINE_QUOTE(*s); ++n)
                arg[n] = (*s == '\\' && _CMDLINE_QUOTE(*(s + 1))) ? '"' : *s;

            /* Leave the trailing quote for the next step */
        }
        
        if (*s && !_CMDLINE_QUOTE(*s) && !_CMDLINE_DELIM(*s))
            arg[n++] = *s++;
        else {
            arg[n++] = '\0'; /* Finalize the temporary argument string */

            /* Allocate and populate the next argument string */
            sys_argv[sys_argc] = (char*)_sys_alloc(n);
            memcpy(sys_argv[sys_argc], arg, n);

            n = 0; /* Don't forget to reset the counter */

            /* Make room for the next argument (not forgetting the trailing null pointer) */
            temp = (char**)_sys_alloc((++sys_argc + 1) * sizeof *temp);
            memcpy(temp, sys_argv, sys_argc * sizeof *temp);
            _sys_free(sys_argv);
            sys_argv = temp;

            /* Trim the trailing quote that we left before */
            if (_CMDLINE_QUOTE(*s))
                ++s;

            /* Skip over any delimiter characters */
            while (*s && _CMDLINE_DELIM(*s))
                ++s;
        }
    }

    sys_argv[sys_argc] = NULL; /* argv is always terminated with a null pointer */

#undef _CMDLINE_DELIM
#undef _CMDLINE_QUOTE
}