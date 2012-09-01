#include "assert.h"
#include "stdio.h"
#include "stdlib.h"

/*
    @description:
        Performs the work of the assert macro by printing
        the message and calling abort.
*/
void _assert(const char *loc, const char *expr)
{
    fputs(loc, stderr);
    fputs(" -- assertion failed: ", stderr);
    fputs(expr, stderr);
    fputc('\n', stderr);
    abort();
}