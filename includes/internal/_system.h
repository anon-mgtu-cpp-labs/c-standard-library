#ifndef __SYSTEM_H
#define __SYSTEM_H

/* Concrete definitions taken from windows.h for low coupling */

#define _SYS_BADHANDLE   ((void*)(long)-1) /* Symbolic wrapper for an invalid handle */
#define _SYS_CMDPROCVAR  "COMSPEC"         /* Name of the command processor for system() */
#define _SYS_CMDPROC     "cmd.exe"         /* Executable file name of the command processor */
#define _SYS_CMDPROCARGS " /C "            /* Necessary arguments to the command processor */
#define _SYS_LOC_MAX     86                /* Maximum locale name length */

typedef void *_sys_handle_t; /* Symbolic wrapper for the system's handle type */

/* wchar_t is defined in multiple headers */
#ifndef _HAS_WCHART
#define _HAS_WCHART
typedef unsigned short wchar_t;
#endif

extern _sys_handle_t __sys_heap;   /* The central heap for malloc and friends */
extern _sys_handle_t __sys_stdin;  /* Underlying handle for stdin */
extern _sys_handle_t __sys_stdout; /* Underlying handle for stdout */
extern _sys_handle_t __sys_stderr; /* Underlying handle for stderr */

extern _sys_handle_t _sys_stdin(void);
extern _sys_handle_t _sys_stdout(void);
extern _sys_handle_t _sys_stderr(void);

extern char *_sys_commandline(void);

extern void _sys_exit(int status);

extern char *_sys_getenv(const char *name);
extern int _sys_system(const char *cmd_proc, char *cmd_line);

extern void *_sys_alloc(unsigned bytes);
extern void  _sys_free(void *p);

extern _sys_handle_t _sys_heapcreate();
extern void _sys_heapdestroy(_sys_handle_t *heap);
extern void *_sys_heapalloc(_sys_handle_t heap, unsigned bytes);
extern void *_sys_heaprealloc(_sys_handle_t heap, void *p, unsigned bytes);
extern void  _sys_heapfree(_sys_handle_t heap, void *p);

extern int _sys_temppath(char *buf, int n);
extern unsigned _sys_tempfilename(const char* path, const char *prefix, char *buf);

extern int _sys_parse_openmode(const char *mode, unsigned *flag, int *orient, int *attr, int *share);
extern _sys_handle_t _sys_openfile(const char *filename, int orient, int attr, int share);
extern int _sys_closefile(_sys_handle_t fd);

extern int _sys_read(_sys_handle_t fd, void *p, int n);
extern int _sys_write(_sys_handle_t fd, void *p, int n);

extern int _sys_tell(_sys_handle_t fd, long long *pos);
extern int _sys_seek(_sys_handle_t fd, long long offset, int whence);

extern int _sys_unlink(const char *filename);
extern int _sys_move(const char *old_name, const char *new_name);

extern int _sys_mbtowc(wchar_t *wc, const char *s, int n);
extern int _sys_wctomb(char *s, const wchar_t *wc, int n);

#endif /* __SYSTEM_H */
