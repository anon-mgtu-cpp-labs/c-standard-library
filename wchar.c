#include "wchar.h"

/*
    @description:
        Copies the wide string pointed to by src (include the 
        terminating null character) into the array pointed to by dst.
*/
wchar_t *wcscpy(wchar_t *dst, const wchar_t *src)
{
    wchar_t *p = dst;

    while (*p++ = *src++)
        ;

    return dst;
}

/*
    @description:
        Compares the wide string pointed to by a to the wide 
        string pointed to by b.
*/
int wcscmp(const wchar_t *a, const wchar_t *b)
{
    while (*a == *b) {
        if (*a == L'\0')
            return 0;

        ++a;
        ++b;
    }

    return *(wchar_t*)a < *(wchar_t*)b ? -1 : +1;
}