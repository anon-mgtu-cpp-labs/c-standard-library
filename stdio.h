#ifndef _STDIO_H
#define _STDIO_H

#include "_stdc11.h"
#include "_system.h"
#include "_deque.h"

/* size_t is defined in multiple headers */
#ifndef _HAS_SIZET
#define _HAS_SIZET
typedef unsigned size_t;
#endif

/* Also defined in stdarg.h */
#ifndef _HAS_VALIST
#define _HAS_VALIST
typedef char *va_list;
#endif

/* NULL is defined in multiple headers */
#ifndef NULL
#define NULL 0
#endif

typedef long long fpos_t;

#define _UNGETSIZ    64

#define BUFSIZ       512
#define EOF          (-1)

/* Based on Windows XP limits */
#define FOPEN_MAX    512
#define FILENAME_MAX 255

#define L_tmpnam     FILENAME_MAX
#define TMP_MAX      FOPEN_MAX

/* Conditional support for Annex K of C11 */
#if defined(__STD_WANT_LIB_EXT1__) && __STD_WANT_LIB_EXT1__ != 0
#ifdef __STD_LIB_EXT1__

#ifndef _HAS_ERRNOT
#define _HAS_ERRNOT
typedef int errno_t;
#endif

#ifndef _HAS_RSIZET
#define _HAS_RSIZET
typedef size_t rsize_t;
#endif

#define L_tmpname_s L_tmpname
#define TMP_MAX_S   TMP_MAX

#endif /* __STD_LIB_EXT1__ */
#endif /* __STD_WANT_LIB_EXT1__ */

#define _OWNED  0x0001 /* Buffer was allocated internally */
#define _LBF    0x0002 /* Buffer is synchronized on transmission of a newline */
#define _NBF    0x0004 /* Buffer is synchronized on every transmission */
#define _APPEND 0x0008 /* All writes append to the buffer */
#define _READ   0x0010 /* Buffer is in read mode */
#define _WRITE  0x0020 /* Buffer is in write mode */
#define _TEXT   0x0040 /* Buffer must perform text translations */
#define _EOF    0x0080 /* Buffer source has reached end-of-file */
#define _ERR    0x0100 /* Buffer has reported an error */
#define _OPEN   0x0200 /* Handle points to an open "file" */
#define _PEEK   0x0400 /* Buffer has a peeked character */
#define _TEMP   0x0800 /* File is to be deleted on closing */
#define _ALLOC  0x1000 /* Unget buffer was dynamically allocated */

#define _IOFBF 0    /* Fully buffered */
#define _IOLBF _LBF /* Line buffered */
#define _IONBF _NBF /* Unbuffered */

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

struct _buffer {
  _sys_handle_t fd;    /* Character source for the buffer */
  struct _deque buf;   /* Queue of buffered characters */
  struct _deque unget; /* Stack of pushed back characters */
  char          peek;  /* Next unread but not yet buffered character */
  unsigned      flag;  /* State of the buffer */
  char         *tmp;   /* The name of the file (if it's temporary) */
};

typedef struct _buffer FILE;

extern FILE __io_buf[];
extern FILE __io_tmp[];

extern char __tmp_name[];
extern char __stdin_buf[];
extern char __stdin_unget[];
extern char __stdout_buf[];
extern char __stderr_buf[];

#define stdin  (&__io_buf[0])
#define stdout (&__io_buf[1])
#define stderr (&__io_buf[2])

#define getc(in)    (fgetc(in))
#define putc(c,out) (fputc(c, out))
#define getchar()   (getc(stdin))
#define putchar(c)  (putc(c, stdout))

extern int remove(const char *filename);
extern int rename(const char *old_name, const char *new_name);

extern FILE *tmpfile(void );
extern char *tmpnam(char *s);

extern FILE *fopen(const char * restrict filename, const char * restrict mode);
extern FILE *freopen(const char * restrict filename, const char * restrict mode, FILE * restrict stream);
extern int fclose(FILE *stream);

extern void setbuf(FILE * restrict stream, char * restrict buf);
extern int setvbuf(FILE * restrict stream, char * restrict buf, int mode, size_t size);

extern int fsetpos(FILE *stream, const fpos_t *pos);
extern int fgetpos(FILE * restrict stream, fpos_t * restrict pos);

extern long ftell(FILE *stream);
extern int fseek(FILE *stream, long offset, int whence);

extern void rewind(FILE *stream);

extern int fgetc(FILE *in);
extern int fputc(int c, FILE *out);
extern int ungetc(int c, FILE *in);
extern int fflush(FILE *out);

extern char *fgets(char * restrict s, int n, FILE * restrict in);
extern int fputs(const char * restrict s, FILE * restrict out);
extern int puts(const char *s);

extern size_t fread(void * restrict p, size_t size, size_t n, FILE * restrict in);
extern size_t fwrite(const void * restrict p, size_t size, size_t n, FILE * restrict out);

extern int fprintf(FILE * restrict out, const char * restrict fmt, ...);
extern int printf(const char * restrict fmt, ...);
extern int snprintf(char * restrict s, size_t n, const char * restrict fmt, ...);
extern int sprintf(char * restrict s, const char * restrict fmt, ...);
extern int vfprintf(FILE * restrict out, const char * restrict fmt, va_list args);
extern int vprintf(const char * restrict fmt, va_list args);
extern int vsnprintf(char * restrict s, size_t n, const char * restrict fmt, va_list args);
extern int vsprintf(char * restrict s, const char * restrict fmt, va_list args);

extern int fscanf(FILE * restrict in, const char * restrict fmt, ...);
extern int scanf(const char * restrict fmt, ...);
extern int sscanf(const char * restrict s, const char * restrict fmt, ...);
extern int vfscanf(FILE * restrict in, const char * restrict fmt, va_list args);
extern int vscanf(const char * restrict fmt, va_list args);
extern int vsscanf(const char * restrict s, const char * restrict fmt, va_list args);

extern void clearerr(FILE *stream);
extern int feof(FILE *stream);
extern int ferror(FILE *stream);
extern void perror(const char *s);

/* Conditional support for Annex K of C11 */
#if defined(__STD_WANT_LIB_EXT1__) && __STD_WANT_LIB_EXT1__ != 0
#ifdef __STD_LIB_EXT1__

extern errno_t tmpfile_s(FILE * restrict * restrict streamptr);
extern errno_t tmpnam_s(char *s, rsize_t maxsize);

extern errno_t fopen_s(FILE * restrict * restrict streamptr, 
    const char * restrict filename, const char * restrict mode);
extern errno_t freopen_s(FILE * restrict * restrict newstreamptr, 
    const char * restrict filename, const char * restrict mode, FILE * restrict stream);

extern char *gets_s(char *s, rsize_t n);

extern int fprintf_s(FILE * restrict out, const char * restrict fmt, ...);
extern int printf_s(const char * restrict fmt, ...);
extern int snprintf_s(const char * restrict s, rsize_t n, const char * restrict fmt, ...);
extern int sprintf_s(char * restrict s, rsize_t n, const char * restrict fmt, ...);
extern int vfprintf_s(FILE * restrict out, const char * restrict fmt, va_list args);
extern int vprintf_s(const char * restrict fmt, va_list args);
extern int vsnprintf_s(char * restrict s, rsize_t n, const char * restrict fmt, va_list arg);
extern int vsprintf_s(char * restrict s, rsize_t n, const char * restrict fmt, va_list arg);

extern int fscanf_s(FILE * restrict in, const char * restrict fmt, ...);
extern int scanf_s(const char * restrict fmt, ...);
extern int sscanf_s(const char * restrict s, const char * restrict fmt, ...);
extern int vfscanf_s(FILE * restrict in, const char * restrict fmt, va_list args);
extern int vscanf_S(const char * restrict fmt, va_list args);
extern int vsscanf_s(const char * restrict s, const char * restrict fmt, va_list arg);

#endif /* __STD_LIB_EXT1__ */
#endif /* __STD_WANT_LIB_EXT1__ */

#endif /* _STDIO_H */
