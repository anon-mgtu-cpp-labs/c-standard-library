#include "errno.h"

extern struct _errno_mapping __errno_map[] = {
    {EDOM,     "EDOM",     "Domain error"},
    {EILSEQ,   "EILSEQ",   "Encoding error"},
    {ERANGE,   "ERANGE",   "Range error"},
    {EINVAL,   "EINVAL",   "Invalid argument"},
    {ENOENT,   "ENOENT",   "No such file or directory"},
    {ENOMEM,   "ENOMEM",   "Not enough memory"},
    {EIO,      "EIO",      "I/O error"},
    {ENFILE,   "ENFILE",   "Too many open files"},
    {EACCESS,  "EACCESS",  "Permission denied"},
    {EUNKNOWN, "EUNKNOWN", "Unknown error"},
    {ESETP,    "ESETP",    "Error setting file position"},
    {EGETP,    "EGETP",    "Error getting file position"},
    {ESFMT,    "ESFMT",    "Unexpected string format"},
    {-1,       "",         ""} /* Sentinel record with an invalid errno value */
};

/*
    @description:
        Acquire and return the error code object for errno. This
        process is more complicated than an extern variable because
        each thread must have its own unique copy of errno.
*/
int *_errno(void)
{
    /* Until the threading subsystem is ready, we can just use a static */
    static int code = 0;
    return &code;
}