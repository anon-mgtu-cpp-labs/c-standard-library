#ifndef _SORT_H
#define _SORT_H

/* size_t is defined in multiple headers */
#ifndef _HAS_SIZET
#define _HAS_SIZET
typedef unsigned size_t;
#endif

extern void _sort(void *base, size_t n, size_t size, int (*cmp)(const void*, const void*));

#endif /* _SORT_H */
