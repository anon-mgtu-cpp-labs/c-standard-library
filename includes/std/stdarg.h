#ifndef _STDARG_H
#define _STDARG_H

/* Also defined in stdio.h */
#ifndef _HAS_VALIST
#define _HAS_VALIST
typedef char *va_list;
#endif

/* Expand the type size to an int boundary */
#define _INTALIGNEDSIZ(type) ((sizeof(type) + sizeof(int) - 1) & ~(sizeof(int) - 1))

#define va_start(ap,param)   ((ap) = (va_list)&(param) + _INTALIGNEDSIZ(param))
#define va_copy(dst,src)     ((dst) = (src))
#define va_arg(ap,type)      (*(type*)(((ap) += _INTALIGNEDSIZ(type)) - _INTALIGNEDSIZ(type)))
#define va_end(ap)

#endif /* _STDARG_H */
