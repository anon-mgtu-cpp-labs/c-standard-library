#ifndef __SYSTIME_H
#define __SYSTIME_H

struct tm; /* Forward declaration for _sys_isdst */

extern long long _sys_getticks(void);
extern long long _sys_getseconds(void);

extern const char *_sys_timezone_name(void);

extern long _sys_timezone_offset(void);
extern long _sys_dst_offset(void);

extern int _sys_isdst(struct tm *timeptr);

#endif /* __SYSTIME_H */
