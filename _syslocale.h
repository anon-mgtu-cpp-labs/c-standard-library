#ifndef __SYSLOCALE_H
#define __SYSLOCALE_H

#include "locale.h"

extern char *_sys_local_localename(void);
extern int _sys_load_locale(const char *name, _locale *loc);
extern int _sys_strcoll(unsigned long lcid, const char *a, const char *b);
extern int _sys_strxfrm(unsigned long lcid, const char *dst, const char *src);

#endif /* __SYSLOCALE_H */