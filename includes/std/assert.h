#ifndef _ASSERT_H
#define _ASSERT_H

#ifdef NDEBUG

#define assert(expr) ((void)0)

#else /* NDEBUG */

#include "_stdc11.h"

#define _ASSERT_STR(x)   _ASSERT_DOSTR(x)
#define _ASSERT_DOSTR(x) #x

#define assert(expr) ((expr) ? (void)0 : _assert(__FILE__ "(" _ASSERT_STR(__LINE__) ") @" __func__ , "'" #expr "'"))

extern void _assert(const char *loc, const char *expr);

#endif /* NDEBUG */

#define static_assert _Static_assert

#endif /* _ASSERT_H */
