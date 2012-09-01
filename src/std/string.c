#include "_syslocale.h"
#include "ctype.h"
#include "errno.h"
#include "limits.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"

/*
    @description:
        Copies n characters from the object pointed to by src
        into the object pointed to by dst. Overlapping blocks
        are implicitly supported (technically undefined behavior).
*/
void *memcpy(void * restrict dst, const void * restrict src, size_t n)
{
    return memmove(dst, src, n);
}

/*
    @description:
        Copies n characters from the object pointed to by src
        into the object pointed to by dst. Overlapping blocks
        are explicitly supported.
*/
void *memmove(void * restrict dst, const void * restrict src, size_t n)
{
    const uint8_t *from = (const uint8_t*)src;
    uint8_t *to = (uint8_t*)dst;
    size_t i;

    if (to <= from || to >= from + n) {
        /* Non overlapped blocks: left to right copy */
        for (i = 0; i < n; ++i)
            *to++ = *from++;
    }
    else {
        from = from + n - 1;
        to = to + n - 1;

        /* Overlapped blocks: right to left copy */
        for (i = 0; i < n; ++i)
            *to-- = *from--;
    }

    return dst;
}

/*
    @description:
        Compares the first n characters of the object pointed to by
        a to the first n characters of the object pointed to by b.
*/
int memcmp(const void *a, const void *b, size_t n)
{
    const uint8_t *lhs = (const uint8_t*)a;
    const uint8_t *rhs = (const uint8_t*)b;
    size_t i;

    for (i = 0; i < n && *lhs == *rhs; ++i)
        ;

    return *lhs - *rhs;
}

/*
    @description:
        Locates the first occurrence of c (converted to unsigned char)
        in the initial n characters of the object pointed to by s.
*/
void *memchr(const void *s, int c, size_t n)
{
    const uint8_t *p = (const uint8_t*)s;
    size_t i;

    for (i = 0; i < n && *p != (uint8_t)c; ++i)
        ++p;

    return i < n ? (void*)p : NULL;
}

/*
    @description:
        Copies the value of c (converted to unsigned char) into
        each of the first n character of the object pointed to by s.
*/
void *memset(void *s, int c, size_t n)
{
    uint8_t *p = (uint8_t*)s;
    size_t i;

    for (i = 0; i < n; ++i)
        *p++ = (uint8_t)c;

    return s;
}

/*
    @description:
        Copies the string pointed to by src (include the terminating 
        null character) into the array pointed to by dst.
*/
char *strcpy(char * restrict dst, const char * restrict src)
{
    char *p = dst;

    while (*p++ = *src++)
        ;

    return dst;
}

/*
    @description:
        Copies not more than n characters (up to and including the first null 
        character) from the array pointed to by src to the array pointed to by dst.
*/
char *strncpy(char * restrict dst, const char * restrict src, size_t n)
{
    char *p = dst;

    /* Copy up to n legit characters from src */
    while (n && (*p++ = *src++))
        --n;

    /* Null pad up to n characters if src was shorter */
    if (n) {
        while (--n)
            *dst++ = '\0';
    }

    return dst;
}

/*
    @description:
        Appends a copy of the string pointed to by src (including the terminating
        null character) to the end of the string pointed to by dst. The initial
        character of src overwrites the null character at the end of dst.
*/
char *strcat(char * restrict dst, const char * restrict src)
{
    char *p = dst;

    /* Find the terminating null character in dst */
    while (*p)
        ++p;

    /* Copy from the null to append src */
    while (*p++ = *src++)
        ;

    return dst;
}

/*
    @description:
        Appends not more than n characters (up to and including the first null
        character) from the array pointed to by src to the end of the string 
        pointed to by dst. The initial character of src overwrites the null 
        character at the end of dst.
*/
char *strncat(char * restrict dst, const char * restrict src, size_t n)
{
    char *p = dst;

    /* Find the terminating null character in dst */
    while (*p)
        ++p;

    /* Copy from the null to append src */
    while (n-- && *src)
        *p++ = *src++;

    *p = '\0'; /* strncat() doesn't pad nulls like strncpy() */

    return dst;
}

/*
    @description:
        Compares the string pointed to by a to the string pointed to by b.
*/
int strcmp(const char *a, const char *b)
{
    while (*a == *b) {
        if (*a == '\0')
            return 0;

        ++a;
        ++b;
    }

    return *(uint8_t*)a < *(uint8_t*)b ? -1 : +1;
}

/*
    @description:
        Comparese the string pointed to by a to the string pointed to by b, both
        interpreted as appropriate to the LC_COLLATE category of the current locale.
*/
int strcoll(const char *a, const char *b)
{
    unsigned long lcid = _localeid(LC_COLLATE);

    if (lcid)
        return _sys_strcoll(lcid, a, b);
    
    /* We're in the "C" locale, so it's just a strcmp */
    return strcmp(a, b);
}

/*
    @description:
        Compares not more than n characters (up to the first null character)
        from the array pointed to by a to the array pointed to by b.
*/
int strncmp(const char *a, const char *b, size_t n)
{
    size_t i;

    for (i = 0; i < n; ++i, ++a, ++b) {
        if (*a != *b)
            return *(uint8_t*)a < *(uint8_t*)b ? -1 : +1;
        else if (*a == '\0')
            return 0;
    }

    return 0;
}

/*
    @description: 
        Transforms the string pointed to by b and places the resulting string
        into the array pointed to by a. The transformation is such that if the
        strcmp() function is applied to two transformed strings, it returns a 
        value greater than, equal to, or less than zero, corresponding to the
        result of the strcoll() function applied to the same two strings.

        No more than n characters are placed into the resulting array,
        including the null terminating character.
*/
size_t strxfrm(char * restrict a, const char * restrict b, size_t n)
{
    unsigned long lcid = _localeid(LC_COLLATE);

    if (lcid)
        return _sys_strxfrm(lcid, a, b);

    /* We're in the "C" locale, so fake it */
    *a = '\0';
    strncat(a, b, n);

    return strlen(a);
}

/*
    @description: 
        Locates the first occurence of c (converted to char) 
        in the string pointed to by s.
*/
char *strchr(const char *s, int c)
{
    while (*s && *s != (char)c)
        ++s;

    return *s == (char)c ? (char*)s : NULL;
}

/*
    @description: 
        Locates the last occurrence of c (converted to char)
        in the string pointed to by s.
*/
char *strrchr(const char *s, int c)
{
    char *p;
    
    /* Do a single pass search and save the most recent hit */
    for (p = 0; *s; ++s) {
        if (*s == c)
            p = (char*)s;
    }

    return p;
}

/*
    @description: 
        Computes the length of the maximum initial segment of the
        string pointed to by s which consists entirely of characters
        from the string pointed to by match.
*/
size_t strspn(const char *s, const char *match)
{
    const char *p = s;

    while (*p && strchr(match, *p))
        ++p;

    return p - s;
}

/*
    @description: 
        Computes the length of the maximum initial segment of the
        string pointed to by s which consists entirely of characters
        *not* from the string pointed to by match.
*/
size_t strcspn(const char *s, const char *match)
{
    const char *p = s;

    while (*p && !strchr(match, *p))
        ++p;

    return p - s;
}

/*
    @description: 
        Locates the first occurrence in the string pointed to by s
        of any character from the string pointed to by match.
*/
char *strpbrk(const char *s, const char *match)
{
    for (; *s; ++s) {
        if (strchr(match, *s))
            return (char*)s;
    }

    return NULL;
}

/*
    @description: 
        Locates the first occurrence in the string pointed to by s
        of the sequence of characters (excluding the terminating null
        character) in the string pointed to by match.
*/
char *strstr(const char *s, const char *match)
{
    size_t len;

    if (!*match)
        return (char*)s; /* Nothing to search for, the whole string matches */

    for (len = strlen(match); *s; ++s) {
        /* Only call strncmp() if there's a possibility of a match */
        if (*s == *match && strncmp(s, match, len) == 0)
            return (char*)s;
    }

    return NULL;
}

/*
    @description: 
        Breaks the string pointed to by s into a sequence of tokens,
        each of which is delimited by a character from the string
        pointed to by delim. The first call in the sequence has a 
        non-null first argument; subsequent calls in the sequence
        have a null first argument.
*/
char *strtok(char * restrict s, const char * restrict delim)
{
    static char *save = NULL;

    /* Continue from save if we're in a subsequent call */
    s = s ? s : save;

    /* Skip leading delimiters */
    s += strspn(s, delim);

    if (!*s)
        return NULL;

    /* Find the end of the token */
    save = s + strcspn(s, delim);

    /* Only terminate the token if it's *not* the last */
    if (*save)
        *save++ = '\0';

    return s;
}

/*
    @description:
        Computes the length of the string pointed to by s.
*/
size_t strlen(const char *s)
{
    size_t n = 0;

    while (*s++)
        ++n;

    return n;
}

/*
    @description: 
        Maps the number in errnum to a message string.
*/
char *strerror(int errnum)
{
    size_t i;

    /* Simple case for no error */
    if (errnum == 0)
        return "No error";

    /* Look for a matching errno code */
    for (i = 0; __errno_map[i].code >= 0; ++i) {
        if (__errno_map[i].code == errnum)
            return (char*)__errno_map[i].msg;
    }

    return "Unrecognized error"; /* Give up ;) */
}

/*
    @description:
        Compares two strings in a case insensitive manner.
*/
int _stricmp(const char *a, const char *b)
{
    while (tolower(*a) == tolower(*b)) {
        if (*a == '\0')
            return 0;

        ++a;
        ++b;
    }

    return tolower(*(uint8_t*)a) < tolower(*(uint8_t*)b) ? -1 : +1;
}

/*
    @description:
        Compares not more than n characters (up to the 
        first null character) in a case insensitive manner.
*/
int _strnicmp(const char *a, const char *b, size_t n)
{
    size_t i;

    for (i = 0; i < n; ++i, ++a, ++b) {
        if (tolower(*a) != tolower(*b))
            return tolower(*(uint8_t*)a) < tolower(*(uint8_t*)b) ? -1 : +1;
        else if (*a == '\0')
            return 0;
    }

    return 0;
}

/*
    @description:
        Copies the string pointed to by s into a dynamically allocated array.
*/
char *_strdup(const char *s)
{
    char *mem = (char*)malloc(strlen(s) + 1);

    if (mem)
        strcpy(mem, s);

    return mem;
}

/*
    @description:
        Reverses the string pointed to by s in place.
*/
char *_strrev(char *s)
{
    size_t front, back;
    char temp;

    if (*s) {
        for (front = 0, back = strlen(s) - 1; front < back; ++front, --back) {
            temp = s[front];
            s[front] = s[back];
            s[back] = temp;
        }
    }

    return s;
}

/*
    @description:
        Converts the given string to upper case.
*/
char *_strupr(char *s)
{
    char *start = s;

    while (*s)
        *s++ = (char)toupper(*s);

    return start;
}

/*
    @description:
        Converts the given string to lower case.
*/
char *_strlwr(char *s)
{
    char *start = s;

    while (*s)
        *s++ = (char)tolower(*s);

    return start;
}

/*
    @description:
        Removes all characters matching any character,
        up to the first non-match, in s2 from the left 
        hand side of s.
*/
char *_strltrim(char *s, const char *s2)
{
    char *p = s, *q = s;

    while (*p && strchr(s2, *p))
        ++p;

    if (p != q) {
        while (*q++ = *p++)
            ;
    }

    return s;
}

/*
    @description:
        Removes all characters matching any character,
        up to the first non-match, in s2 from the right 
        hand side of s.
*/
char *_strrtrim(char *s, const char *s2)
{
    char *p = s + strlen(s);

    while (p != s && strchr(s2, *p))
        --p;

    if (p != s)
        ++p;

    *p = '\0';

    return s;
}