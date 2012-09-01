#include "ctype.h"
#include "locale.h"
#include "string.h"

static int istype(int c, int flags);

int isalnum(int c) { return istype(c, _ALPHA | _DIGIT); }
int isalpha(int c) { return istype(c, _ALPHA); }
int isblank(int c) { return istype(c, _BLANK); }
int iscntrl(int c) { return istype(c, _CNTRL); }
int isdigit(int c) { return istype(c, _DIGIT); }
int isgraph(int c) { return istype(c, _PUNCT | _ALPHA | _DIGIT); }
int islower(int c) { return istype(c, _LOWER); }
int isprint(int c) { return istype(c, _BLANK | _PUNCT | _ALPHA | _DIGIT); }
int ispunct(int c) { return istype(c, _PUNCT); }
int isspace(int c) { return istype(c, _SPACE); }
int isupper(int c) { return istype(c, _UPPER); }
int isxdigit(int c) { return istype(c, _HEX); }

int tolower(int c) { return !isupper(c) ? c : c + ('a' - 'A'); }
int toupper(int c) { return !islower(c) ? c : c - ('a' - 'A'); }

/*
    @description:
        Converts a character into its corresponding decimal 
        value according to the specified numeric base.
*/
int _digitvalue(int c, int base)
{
    char *p = "0123456789abcdefghijklmnopqrstuvwxyz";
    char *q = strchr(p, tolower(c));

    if (!q || (q - p) >= base)
        return -1;

    return (int)(q - p);
}

/*
    @description:
        Convenience helper to make reading the ctype functions easier.
*/
static int istype(int c, int flags)
{
    return _localectype()[(unsigned char)(c & 0xff)] & flags;
}