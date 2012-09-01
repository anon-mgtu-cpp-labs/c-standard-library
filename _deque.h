#ifndef __DEQUE_H
#define __DEQUE_H

/* Increment or decrement an index with wraparound */
#define _wrap_decr(i,n)      ((i) = ((i) == 0 ? (n) - 1 : (i) - 1))
#define _wrap_incr(i,n)      ((i) = ((i) == (n) - 1 ? 0 : (i) + 1))

#define _deque_ready(d)      ((d).base != 0)

#define _deque_full(d)       ((d).fill == (d).size)
#define _deque_empty(d)      ((d).fill == 0)

/* Generic push and pop. Calling code defines the index and update logic */
#define _deque_push(d,x,i,f) (++(d).fill, (d).base[(i)] = (x), f((i), (d).size))
#define _deque_pop(d,i,f)    (--(d).fill, (d).base[f((i), (d).size)])

/* Concrete push/pop front and push/pop back */
#define _deque_pushf(d,x)    (_deque_push((d), (x), (d).front, _wrap_decr))
#define _deque_pushb(d,x)    (_deque_push((d), (x), (d).back, _wrap_incr))
#define _deque_popf(d)       (_deque_pop((d), (d).front, _wrap_incr))
#define _deque_popb(d)       (_deque_pop((d), (d).back, _wrap_decr))

#define _deque_init(d,buf,n) do { \
  (d).base = (buf);               \
  (d).size = (n);                 \
  (d).fill = 0;                   \
  (d).front = 0;                  \
  (d).back = 1;                   \
} while (0)

struct _deque {
  char     *base;
  unsigned  size;
  unsigned  fill;
  unsigned  front;
  unsigned  back;
};

#endif
