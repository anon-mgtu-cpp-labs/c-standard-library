#include "_sort.h"
#include "_system.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"

static void do_heap(uint8_t *base, size_t i, size_t n, size_t size, _cmp_func_t cmp);

/*
    @description:
        qsort-friendly implementation of heapsort for guaranteed time complexity.
*/
void _sort(void *base, size_t n, size_t size, _cmp_func_t cmp)
{
    /* Not much can be done when size is unknown */
    void *temp = _sys_alloc(size);
    uint8_t *p = (uint8_t*)base;
    int i = n / 2;

    /* Heapify the array */
    while (i-- > 0)
        do_heap(p, i, n, size, cmp);

    /* Extrac the heap max and place it in sorted position */
    while (--n < (size_t)-1) {
        /* Swap the heap max with base[n] */
        memcpy(temp, p, size);
        memcpy(p, &p[n * size], size);
        memcpy(&p[n * size], temp, size);

        /* Re-heapify after removing the heap max */
        do_heap(p, 0, n, size, cmp);
    }

    _sys_free(temp);
}

/*
    @description:
        Heapify helper for _sort.
*/
void do_heap(uint8_t *base, size_t i, size_t n, size_t size, _cmp_func_t cmp)
{
    /* Not much can be done when size is unknown */
    void *temp = _sys_alloc(size);
    size_t k;

    /* Save the node that's being overwritten below */
    memcpy(temp, &base[i * size], size);

    /* Move all subsequent child nodes into place to maintain the heap property */
    for (k = i * 2 + 1; k < n; k = i * 2 + 1) {
        /* Find the next child node */
        if (k + 1 < n && cmp(&base[k * size], &base[(k + 1) * size]) < 0)
            ++k;

        /* Are we at the saved node's sorted position? */
        if (cmp(temp, &base[k * size]) >= 0)
            break;

        /* Overwrite the parent with one of its children */
        memcpy(&base[i * size], &base[k * size], size);
        i = k;
    }

    /* Copy the saved node into its final resting place */
    memcpy(&base[i * size], temp, size);
    _sys_free(temp);
}