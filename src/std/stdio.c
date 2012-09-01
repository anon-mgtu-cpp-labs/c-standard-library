#include "_system.h"
#include "_deque.h"
#include "_printf.h"
#include "_scanf.h"
#include "errno.h"
#include "stdarg.h"
#include "stdbool.h"
#include "stdio.h"
#include "string.h"

FILE __io_buf[FOPEN_MAX];      /* Regular files opened with fopen (and standard streams) */
FILE __io_tmp[TMP_MAX];        /* Files opened for temporary use (ie. tmpfile) */

char __tmp_name[L_tmpnam];     /* Runtime owned temporary file name */
char __stdin_buf[BUFSIZ];      /* Main input buffer for stdin */
char __stdin_unget[_UNGETSIZ]; /* Unget buffer for stdin */
char __stdout_buf[BUFSIZ];     /* Main output buffer for stdout */
char __stderr_buf[1];          /* "Buffer" for stderr for simplicity */

/* 
    ===================================================
                Static helper declarations
    ===================================================
*/

static char *get_temp_name(char *buf);
static FILE *get_unused_handle(FILE handles[], size_t limit);
static FILE *file_open(FILE *file, const char * restrict filename, const char * restrict mode, bool tempfile);
static size_t count_newline_diff(struct _deque *buf, bool as_stack);
static size_t count_buffer_bytes(FILE *stream);
static bool intern_tell(FILE *stream, fpos_t *new_pos);
static bool intern_seek(FILE *stream, fpos_t offset, int whence);
static int peekbuf(FILE *in);
static bool fillbuf(FILE *in);
static bool flushbuf(FILE *out);
static size_t compact_newlines(FILE *in, char *buf, size_t n);
static size_t expand_newlines(FILE *out, char *output);
static int write_stream(void *data, void *dst, size_t n, size_t *count, size_t limit);
static int write_string(void *data, void *dst, size_t n, size_t *count, size_t limit);
static int write_nothing(void *data, void *dst, size_t n, size_t *count, size_t limit);
static int read_stream(void *src, size_t *count);
static int unget_stream(void *data, void *src, size_t *count);
static int read_string(void *src, size_t *count);
static int unget_string(void *data, void *src, size_t *count);

/* 
    ===================================================
              Standard function definitions
    ===================================================
*/

/*
    @description:
        Causes the file whose name is the string pointed to by
        filename to be no longer accessible by that name.
*/
int remove(const char *filename)
{
    return _sys_unlink(filename) == 0;
}

/*
    @description:
        Causes the file whose name is the string pointed to by
        old_name to be henceforth known by the name given by
        the string pointed to by new_name.
*/
int rename(const char *old_name, const char *new_name)
{
    return _sys_move(old_name, new_name);
}

/*
    @description:
        Creates a temporary binary file that is different
        from any other existing file and that will automatically
        be removed when it is closed or at program termination.
*/
FILE *tmpfile(void)
{
    char buf[L_tmpnam];
    FILE *fp = NULL;

    if (get_temp_name(buf))
        fp = file_open(NULL, buf, "rb+", true);

    return fp;
}

/*
    @description:
        Generates a string that is a valid file name and 
        that is not the same name of an existing file.
*/
char *tmpnam(char *s)
{
    return get_temp_name(!s ? __tmp_name : s);
}

/*
    @description:
        Opens the file whose name is the string pointed to by 
        filename and associates a stream with it.
*/
FILE *fopen(const char * restrict filename, const char * restrict mode)
{
    return file_open(NULL, filename, mode, false);
}

/*
    @description:
        Opens the file whose name is the string pointed to by
        filename and associates the stream pointed to by stream
        with it.
*/
FILE *freopen(const char * restrict filename, const char * restrict mode, FILE * restrict stream)
{
    /*
        It's implementation-defined what mode changes are supported
        when filename is a null pointer, and this implementation
        chooses to support none. ;)  -JRD
    */
    if (filename) {
        fclose(stream); /* It's required to ignore close errors */
        stream = file_open(stream, filename, mode, false);
    }

    return stream;
}

/*
    @description:
        Causes the stream pointed to by stream to be flushed
        and the associated file to be closed.
*/
int fclose(FILE *stream)
{
    int rc = 0;

    if (stream->flag & _OPEN) {
        /* Try to flush if the stream is in write mode */
        if (stream->flag & _WRITE && !flushbuf(stream))
            rc = EOF;

        /* Try to close the underlying handle */
        if (!_sys_closefile(stream->fd))
            rc = EOF;

        /* Release owned main buffer memory */
        if (stream->flag & _OWNED)
            _sys_free(stream->buf.base);

        /* unget is always owned, but may not be dynamically allocated */
        if (stream->flag & _ALLOC)
            _sys_free(stream->unget.base);

        if (stream->flag & _TEMP) {
            remove(stream->tmp);    /* Temporary files are always deleted */
            _sys_free(stream->tmp); /* Don't forget we allocated the filename */
        }

        /* Mark the stream as unused */
        stream->flag = 0;
    }

    return rc;
}

/*
    @description:
        Equivalent to setvbuf where mode is _IOFBF and size is 
        BUFSIZ if buf is a null pointer, otherwise mode is _IONBF.
*/
void setbuf(FILE * restrict stream, char * restrict buf)
{
    setvbuf(stream, buf, (buf ? _IOFBF : _IONBF), (buf ? BUFSIZ : 1));
}

/*
    @description:
        Replaces the buffer for the specified stream, using the 
        specified mode. If buf is an array, it will be used as 
        the unowned buffer with the size of the array denoted
        by size. If buf is a null pointer, size will be used 
        to create an owned buffer.
*/
int setvbuf(FILE * restrict stream, char * restrict buf, int mode, size_t size)
{
    bool owned = (buf == NULL);

    /* Without a size, all kinds of gremlins can pop up */
    if (size == 0)
        return -1;

    if (owned) {
        /* Try to make an owned buffer */
        if (!(buf = (char*)_sys_alloc(size)))
            return -1;
    }

    /* With a valid buf, the old buffer can be safely freed */
    if (stream->flag & _OWNED)
        _sys_free(stream->buf.base);

    /* Make sure the owned state is properly set */
    stream->flag = owned ? (stream->flag | _OWNED) : (stream->flag & ~_OWNED);

    /* Set the new buffering flag */
    stream->flag &= ~(_LBF | _NBF);
    stream->flag |= mode;

    /* Finally, apply the new buffer */
    _deque_init(stream->buf, buf, size);

    return 0;
}

/*
    @description:
        Sets the mbstate_t object (if any) and file position indicator
        for the stream pointed to by stream according to the value of 
        the object pointed to by pos, which shall be a value obtained 
        from an earlier successful call to the fgetpos function on a 
        stream associated with the same file.
*/
int fsetpos(FILE *stream, const fpos_t *pos)
{
    return intern_seek(stream, *pos, SEEK_SET);
}

/*
    @description:
        Stores the current values of the parse state (if any)
        and file position indicator for the stream pointed to
        by stream in the object pointed to by pos.
*/
int fgetpos(FILE * restrict stream, fpos_t * restrict pos)
{
    return intern_tell(stream, pos);
}

/*
    @description:
        Obtains the current value of the file position indicator
        for the stream pointed to by stream.
*/
long ftell(FILE *stream)
{
    fpos_t pos;

    return intern_tell(stream, &pos) ? -1L : (long)pos;
}

/*
    @description:
        Sets the file position indicator for the stream pointed
        to by stream to an offset specified by offset starting
        from the position specified by whence.
*/
int fseek(FILE *stream, long offset, int whence)
{
    /*
        intern_seek uses fpos_t, but that's alright because long
        is a subset of long long. Everything works out correctly.
    */
    return intern_seek(stream, offset, whence);
}

/*
    @description:
        Equivalent to fseek with an offset of 0 and whence of SEEK_SET.
*/
void rewind(FILE *stream)
{
    /* I've chosen to interpret "equivalent" as "identical". ;)  -JRD */
    fseek(stream, 0L, SEEK_SET);
}

/*
    @description:
        Reads the next character from the stream pointed to by in, if available.
*/
int fgetc(FILE *in)
{
    /* The stream must be both open and in read mode */
    if (!(in->flag & _OPEN) || in->flag & _WRITE)
        return EOF;

    /* Reset the stream to read mode */
    in->flag &= ~_WRITE;
    in->flag |= _READ;

    if (_deque_ready(in->unget) && !_deque_empty(in->unget)) {
        /* Pull from a non-empty unget buffer */
        return _deque_popf(in->unget);
    }
    else {
        /* Pull from the main buffer (refill if necessary) */
        if ((in->flag & _NBF || _deque_empty(in->buf)) && !fillbuf(in))
            return EOF;

        return _deque_popb(in->buf);
    }
}

/*
    @description:
        Writes the character specified by c (converted to unsigned char)
        to the output stream pointed to by out.
*/
int fputc(int c, FILE *out)
{
    /* The stream must be both open and in write mode */
    if (!(out->flag & _OPEN) || out->flag & _READ)
        return EOF;

    /* Reset the stream to write mode */
    out->flag &= ~_READ;
    out->flag |= _WRITE;

    _deque_pushf(out->buf, (char)c);

    /*
        1) Flush on a newline.
        2) Always flush if buffering is turned off.
        3) Flush if the buffer is full
    */
    if ((out->flag & _LBF && c == '\n') || out->flag & _NBF || _deque_full(out->buf)) {
        if (!flushbuf(out))
            return EOF;
    }

    return c;
}

/*
    @description:
        Pushes the character specified by c (converted to unsigned char)
        back onto the input stream pointed to by in. Pushed back characters
        will be returned by subsequent reads on the stream in reverse order
        of pushshing.
*/
int ungetc(int c, FILE *in)
{
    /* The stream must be both open and in read mode */
    if (!(in->flag & _OPEN) || in->flag & _WRITE)
        return EOF;

    /* Reset the stream to read mode */
    in->flag &= ~_WRITE;
    in->flag |= _READ;

    /* The unget buffer may have been lazy initialized (see fopen), create it now */
    if (!_deque_ready(in->unget)) {
        char *buf = (char*)_sys_alloc(_UNGETSIZ);

        if (!buf)
            return EOF;

        _deque_init(in->unget, buf, _UNGETSIZ);
        in->flag |= _ALLOC;
    }

    /* Make sure an unget can be performed with the current buffer */
    if (_deque_full(in->unget))
        return EOF;

    _deque_pushf(in->unget, (char)c);
    in->flag &= ~_EOF; /* A pushback clears the EOF state */

    return c;
}

/*
    @description:
        Causes unwritten data for the specified stream (or all open streams
        meeting requirements if the stream pointer is null) to be delivered 
        to the host environment to be written to the file.
*/
int fflush(FILE *out)
{
    int rc = 0;

    if (out) {
        /* The stream must be both open and in write mode */
        if (!(out->flag & _OPEN) || out->flag & _READ || !flushbuf(out))
            rc = EOF;
    }
    else {
        size_t i;

        /* Flush ALL the things! :D */
        for (i = 0; i < FOPEN_MAX; ++i) {
            out = &__io_buf[i]; /* Sacrifice a little for notational consistency */

            /* The stream must be both open and in write mode */
            if (!(out->flag & _OPEN) || out->flag & _READ || !flushbuf(out))
                rc = EOF;
        }
    }

    return rc;
}

/*
    @description:
        Reads at most one less than the number of characters specified by n
        from the stream pointed to by in into the array pointed to by s. No
        additional characters are read after a newline character (which is retained)
        or after end-of-file.
*/
char *fgets(char * restrict s, int n, FILE * restrict in)
{
    char *p = s;
    int ch;

    while (--n > 0) {
        if ((ch = fgetc(in)) == EOF || (*p++ = (char)ch) == '\n')
            break;
    }

    if (p != s && !ferror(in)) {
        *p = '\0';
        return s;
    }
    
    return NULL;
}

/*
    @description:
        Writes the string pointed to by s to the stream pointed to by stream.
*/
int fputs(const char * restrict s, FILE * restrict out)
{
    while (*s)
        fputc(*s++, out);

    return ferror(out);
}

/*
    @description:
        Writes the string pointed to by s to the stream pointed to by stdout.
*/
int puts(const char *s)
{
    fputs(s, stdout);
    fputc('\n', stdout);

    return ferror(stdout);
}

/*
    @description:
        Reads, into the array pointed to by p, up to n elements whose
        size is specified by size, from the stream pointed to by stream.
*/
size_t fread(void * restrict p, size_t size, size_t n, FILE * restrict in)
{
    if (size == 0 || n == 0)
        return 0;
    else {
        size_t bytes = n * size;
        size_t count = 0;
        char *dst = (char*)p;
        int ch;

        while (count < bytes && (ch = fgetc(in)) != EOF)
            *dst++ = (char)ch;

        return count;
    }
}

/*
    @description:
        Writes, from the array pointed to by p, up to n elements whose
        size is specified by size, to the stream pointed to by stream.
*/
size_t fwrite(const void * restrict p, size_t size, size_t n, FILE * restrict out)
{
    if ( size == 0 || n == 0 )
        return 0;
    else {
        size_t bytes = n * size;
        size_t count = 0;
        const char *temp = (const char*)p;

        while (count < bytes && fputc(*temp++, out) != EOF)
            ++count;

        return count;
    }
}

/*
    @description:
        Writes output to the stream pointed to by out, under control of the
        string pointed to by fmt that specifies how subsequent arguments are
        converted for output.
*/
int fprintf(FILE * restrict out, const char * restrict fmt, ...)
{
    va_list args;
    int rv;

    va_start(args, fmt);
    rv = vfprintf(out, fmt, args);
    va_end(args);

    return rv;
}

/*
    @description:
        Equivalent to fprintf with the first argument being stdout.
*/
int printf(const char * restrict fmt, ...)
{
    va_list args;
    int rv;

    va_start(args, fmt);
    rv = vfprintf(stdout, fmt, args);
    va_end(args);

    return rv;
}

/*
    @description:
        Equivalent to fprintf, except that the output is written into
        an array (specified by argument s) rather than to a stream. If
        n is zero, nothing is written, and s may be a null pointer.
        Otherwise, up to n - 1 characters are written.
*/
int snprintf(char * restrict s, size_t n, const char * restrict fmt, ...)
{
    va_list args;
    int rv;

    va_start(args, fmt);
    rv = vsnprintf(s, n, fmt, args);
    va_end(args);

    return rv;
}

/*
    @description:
        Equivalent to fprintf, except that the output is written into
        an array (specified by the argument s) rather than to a stream.
*/
int sprintf(char * restrict s, const char * restrict fmt, ...)
{
    va_list args;
    int rv;

    va_start(args, fmt);
    rv = vsprintf(s, fmt, args);
    va_end(args);

    return rv;
}

/*
    @description:
        Equivalent to fprintf, with the variable argument list replaced by args.
*/
int vfprintf(FILE * restrict out, const char * restrict fmt, va_list args)
{
    return _printf(write_stream, out, fmt, (size_t)-1, args);
}

/*
    @description:
        Equivalent to printf, with the variable argument list replaced by args.
*/
int vprintf(const char * restrict fmt, va_list args)
{
    return _printf(write_stream, stdout, fmt, (size_t)-1, args);
}

/*
    @description:
        Equivalent to snprintf, with the variable argument list replaced by args.
*/
int vsnprintf(char * restrict s, size_t n, const char * restrict fmt, va_list args)
{
    if (n == 0)
        return _printf(write_nothing, s, fmt, (size_t)-1, args);
    else
        return _printf(write_string, s, fmt, n - 1, args);
}

/*
    @description:
        Equivalent to sprintf, with the variable argument list replaced by args.
*/
int vsprintf(char * restrict s, const char * restrict fmt, va_list args)
{
    int n = _printf(write_string, s, fmt, (size_t)-1, args);

    s[n] = '\0';

    return n;
}

/*
    @description:
        Reads input from the stream pointed to by in, under control of the string
        pointed to by fmt that specifies the admissible input sequences and how
        they are to be converted for assignment, using subsequent arguments as
        pointers to the objects to receive the converted input.
*/
int fscanf(FILE * restrict in, const char * restrict fmt, ...)
{
    va_list args;
    int rv;

    va_start(args, fmt);
    rv = vfscanf(in, fmt, args);
    va_end(args);

    return rv;
}

/*
    @description:
        Equivalent to fscanf with the first argument being stdin.
*/
int scanf(const char * restrict fmt, ...)
{
    va_list args;
    int rv;

    va_start(args, fmt);
    rv = vfscanf(stdin, fmt, args);
    va_end(args);

    return rv;
}

/*
    @description:
        Equivalent to fscanf, except that input is 
        obtained from a string rather than from a stream.
*/
int sscanf(const char * restrict s, const char * restrict fmt, ...)
{
    va_list args;
    int rv;

    va_start(args, fmt);
    rv = vsscanf(s, fmt, args);
    va_end(args);

    return rv;
}

/*
    @description:
        Equivalent to fscanf, with the variable argument list replaced by args.
*/
int vfscanf(FILE * restrict in, const char * restrict fmt, va_list args)
{
    return _scanf(read_stream, unget_stream, in, fmt, args);
}

/*
    @description:
        Equivalent to scanf, with the variable argument list replaced by args.
*/
int vscanf(const char * restrict fmt, va_list args)
{
    return _scanf(read_stream, unget_stream, stdin, fmt, args);
}

/*
    @description:
        Equivalent to sscanf, with the variable argument list replaced by args.
*/
int vsscanf(const char * restrict s, const char * restrict fmt, va_list args)
{
    return _scanf(read_string, unget_string, (void*)s, fmt, args);
}

/*
    @description:
        Clears the end-of-file and error indicators
        for the stream pointed to by stream.
*/
void clearerr(FILE *stream)
{
    stream->flag &= ~(_EOF | _ERR);
}

/*
    @description:
        Tests the end-of-file indicator for the stream pointed to by stream.
*/
int feof(FILE *stream)
{
    return stream->flag & _OPEN && stream->flag & _EOF;
}

/*
    @description:
        Tests the error indicator for the stream pointed to by stream.
*/
int ferror(FILE *stream)
{
    return stream->flag & _OPEN && stream->flag & _ERR;
}

/*
    @description:
        Maps the error number in errno to an error message.
*/
void perror(const char *s)
{
    if (s && *s) {
        fputs(s, stdout);
        fputs(": ", stdout);
    }

    puts(strerror(errno));
}

/* 
    ===================================================
                Static helper definitions
    ===================================================
*/

/*
    @description:
        Generates the full path for a temporary file.
*/
char *get_temp_name(char *buf)
{
    int len = _sys_temppath(buf, L_tmpnam);

    if (len > L_tmpnam || len == 0 || _sys_tempfilename(buf, "", buf + len) == 0)
        return NULL;

    return buf;
}

/*
    @description:
        Retrieves an unused handle from the requested collection.
*/
FILE *get_unused_handle(FILE handles[], size_t limit)
{
    size_t i;

    for (i = 0; i < limit; i++) {
        /* Only unused handles will have a totally clear flag */
        if (handles[i].flag == 0)
            return &handles[i];
    }

    return NULL;
}

/*
    @description:
        Opens or reopens a file with the specified mode.
*/
FILE *file_open(FILE *file, const char * restrict filename, const char * restrict mode, bool tempfile)
{
    if (!file) {
        FILE *handles = tempfile ? __io_tmp : __io_buf;
        size_t limit = tempfile ? TMP_MAX : FOPEN_MAX;

        /* Create a new FILE instead of reopening an old one */
        file = get_unused_handle(handles, limit);
    }
    
    if (file) {
        int orient, attr, share;

        if (_sys_parse_openmode(mode, &file->flag, &orient, &attr, &share)) {
            if ((file->fd = _sys_openfile(filename, orient, attr, share)) != _SYS_BADHANDLE) {
                char *buf = (char*)_sys_alloc(BUFSIZ);

                if (!buf || (tempfile && !(file->tmp = (char*)_sys_alloc(L_tmpnam)))) {
                    _sys_free(buf);
                    _sys_closefile(file->fd);
                }
                else {
                    /* Finalize the temporary status now that all memory is prepped */
                    if (tempfile) {
                        strcpy(file->tmp, filename);
                        file->flag |= _TEMP;
                    }

                    /* Initialize the buffer, set the initial flags, and we're good to go */
                    _deque_init(file->buf, buf, BUFSIZ);
                    file->flag |= (_OWNED | _LBF | _OPEN);
                    
                    return file;
                }
            }
        }
    }

    return NULL;
}

/*
    @description:
        Counts the number of bytes currently stored in a deque
        as if newlines were expanded to the system representation.

        The count can be performed as either a stack or as a queue.
*/
size_t count_newline_diff(struct _deque *buf, bool as_stack)
{
    size_t i = as_stack ? buf->front : buf->back;
    size_t diff = 0;
    size_t n = 0;

    while (n++ < buf->fill) {
        if (as_stack)
            _wrap_incr(i, buf->size);
        else
            _wrap_decr(i, buf->size);

        if (buf->base[i] == '\n')
            ++diff;
    }

    return diff;
}

/*
    @description:
        Counts the number of bytes currently stored in the stream 
        buffer as if newlines were expanded to the system representation.
*/
size_t count_buffer_bytes(FILE *stream)
{
    size_t bytes = stream->buf.fill;
    size_t unget = stream->unget.fill;

    /* Account for newline compaction on text streams */
    if (stream->flag & _TEXT) {
        bytes += count_newline_diff(&stream->buf, false);
        unget += count_newline_diff(&stream->unget, true);
    }

    return bytes + unget;
}

/*
    @description:
        Gets the current file position indicator for the specified stream.
*/
bool intern_tell(FILE *stream, fpos_t *new_pos)
{
    if (!_sys_tell(stream->fd, new_pos)) {
        errno = EGETP;
        return false;
    }

    *new_pos -= count_buffer_bytes(stream);

    return true;
}

/*
    @description:
        Sets the current file position indicator for the specified stream.
*/
bool intern_seek(FILE *stream, fpos_t offset, int whence)
{
    /*
        SEEK_CUR is a pain, especially with padding and text expansion.
        Ignore it completely and work from SEEK_SET instead.
    */
    if (whence == SEEK_CUR) {
        fpos_t curr;

        if (!intern_tell(stream, &curr)) {
            errno = ESETP;
            return false;
        }

        offset += curr;
        whence = SEEK_SET;
    }

    /* Flush output streams in write mode, discard otherwise */
    if (stream->flag & _WRITE)
        flushbuf(stream);
    else
        _deque_init(stream->buf, stream->buf.base, stream->buf.size);

    /* Clear the unget buffer if present */
    if (_deque_ready(stream->unget))
        _deque_init(stream->unget, stream->unget.base, stream->unget.size);

    /* The next operation may be a read or a write */
    stream->flag &= ~(_READ | _WRITE);
    stream->flag &= ~_EOF;

    if (!_sys_seek(stream->fd, offset, whence)) {
        errno = ESETP;
        return false;
    }

    return true;
}

/*
    @description:
        Peeks ahead by one character in the source device.
*/
int peekbuf(FILE *in)
{
    if (_sys_read(in->fd, &in->peek, 1) < 1)
        return EOF;

    in->flag |= _PEEK;

    return in->peek;
}

/*
    @description:
        Fills the buffer with characters from the source device.
*/
bool fillbuf(FILE *in)
{
    char *temp = (char*)_sys_alloc(in->buf.size);

    if (!temp)
        in->flag |= _ERR;
    else {
        bool has_peek = (in->flag & _PEEK) != 0;
        size_t nread;

        /* Grab the peeked character if present */
        if (has_peek)
            *temp++ = in->peek;

        /*
            Fill the temporary buffer from the system stream, taking care
            not to overwrite or over read due to a peeked character.
        */
        nread = _sys_read(in->fd, temp + has_peek, in->buf.size - has_peek);

        if (nread < 0)
            in->flag |= _ERR; /* There was a stream error */
        else if (nread == 0 && !has_peek)
            in->flag |= _EOF; /* We hit end-of-file immediately */
        else {
            char *buf = in->buf.base;

            nread += has_peek; /* Account for a peeked character */

            /* Finalize the temporary buffer by compacting newlines */
            if (in->flag & _TEXT)
                nread = compact_newlines(in, temp, nread);

            /* Set up a new owned buffer if necessary */
            if (!buf && in->flag & _OWNED)
                buf = (char*)_sys_alloc(in->buf.size);

            if (!buf)
                in->flag |= _ERR; /* Allocation failed or a non-owned buffer was NULL */
            else {
                size_t i;

                _deque_init(in->buf, buf, in->buf.size); /* Reset the buffer for refill */

                /* Fill the stream buffer with the finished temporary buffer */
                for (i = 0; i < nread && i < in->buf.size; ++i)
                    _deque_pushf(in->buf, temp[i]);
            }
        }

        _sys_free(temp);
    }

    return (in->flag & (_ERR | _EOF)) == 0;
}

/*
    @description:
        Sends buffered characters to the destination device.
*/
bool flushbuf(FILE *out)
{
    /* Assuming every character may be a newline for expansion */
    char *temp = (char*)_sys_alloc(out->buf.size * 2);

    if (!temp)
        out->flag |= _ERR;
    else {
        size_t write_size = out->buf.size;

        /* Expand newlines for the output device */
        if (out->flag & _TEXT)
            write_size = expand_newlines(out, temp);

        if (_sys_write(out->fd, temp, write_size) < 0)
            out->flag |= _ERR; /* There was a stream error */
        else {
            /* Reset the buffer so that we don't double flush */
            _deque_init(out->buf, out->buf.base, out->buf.size);
        }

        _sys_free(temp);
    }

    return (out->flag & _ERR) == 0;
}

/*
    @description:
        Converts a platform-dependent line break sequence into '\n'.
*/
size_t compact_newlines(FILE *in, char *buf, size_t n)
{
    char *it = buf;
    char *save = buf;

    while (it != &buf[n]) {
        /* Compact CRLF into LF */
        if (*it == '\r') {
            if (&it[1] == &buf[n]) {
                /*
                    CR at the end of the buffer should be relatively rare, 
                    so we can  comfortably peek on a per character basis.
                */

                /* If we couldn't peek, CR will be saved */
                if (peekbuf(in) == '\n') {
                    /* We found LF; overwrite the CR so that LF is saved */
                    *it = '\n';

                    /* Reset the peek status so the extra LF is discarded */
                    in->flag &= ~_PEEK;
                }
            }
            else if (it[1] == '\n') {
                ++it; /* The next character is LF so jump over the CR and continue */
            }
        }

        *save++ = *it++;
    }

    return save - buf;
}

/*
    @description:
        Converts '\n' into a suitable platform-dependent line break sequence.
*/
size_t expand_newlines(FILE *out, char *output)
{
    char *save = output;

    while (!_deque_empty(out->buf)) {
        char ch = _deque_popb(out->buf);

        /* Expand LF into CRLF */
        if (ch == '\n')
            *save++ = '\r';

        *save++ = ch;
    }

    return save - output;
}

/*
    @description:
        Concrete implementation of _put_func_t for fprintf variants.
*/
int write_stream(void *data, void *dst, size_t n, size_t *count, size_t limit)
{
    size_t written = 0;
    
    if (limit == (size_t)-1) {
        /* No limit, just write until done */
        written = fwrite(data, 1, n, (FILE*)dst);
    }
    else {
        /* Write up to the limit */
        size_t remaining = limit - *count;
        size_t safen = n < remaining ? n : remaining;

        if (safen > 0 && (written = fwrite(data, 1, safen, (FILE*)dst)) == safen) {
            /* Trick the return value into reporting success */
            written = n;
        }
    }

    *count += n;

    return written == n;
}

/*
    @description:
        Concrete implementation of _put_func_t for sprintf variants.
*/
int write_string(void *data, void *dst, size_t n, size_t *count, size_t limit)
{
    if (limit == (size_t)-1) {
        /* No limit, just write until done */
        memcpy((char*)dst + *count, data, n);
    }
    else {
        /* Write up to the limit */
        size_t remaining = limit - *count;
        size_t safen = n < remaining ? n : remaining;

        if (safen > 0)
            memcpy((char*)dst + *count, data, safen);
    }

    *count += n;

    return 1;
}

/*
    @description:
        Concrete implementation of _put_func_t for nprintf variants.
*/
int write_nothing(void *data, void *dst, size_t n, size_t *count, size_t limit)
{
    /* Suppress unused parameter warnings */
    (void)data;
    (void)dst;
    (void)limit;

    *count += n;

    return 1;
}

/*
    @description:
        Concrete implementation of _get_func_t for fscanf variants.
*/
int read_stream(void *src, size_t *count)
{
    int rv = fgetc((FILE*)src);

    if (rv != EOF)
        ++(*count);

    return rv;
}

/*
    @description:
        Concrete implementation of _unget_func_t for fscanf variants.
*/
int unget_stream(void *data, void *src, size_t *count)
{
    if (*(char*)data == EOF)
        return EOF;
    else {
        int rv = ungetc(*(char*)data, (FILE*)src);

        if (rv != EOF)
            --(*count);

        return rv;
    }
}

/*
    @description:
        Concrete implementation of _get_func_t for sscanf variants.
*/
int read_string(void *src, size_t *count)
{
    int ch = ((char*)src)[*count];

    if (ch == '\0')
        ch = EOF;
    else
        ++(*count);

    return ch;
}

/*
    @description:
        Concrete implementation of _unget_func_t for sscanf variants.
*/
int unget_string(void *data, void *src, size_t *count)
{
    char ch = *(char*)data;

    /* Suppressing unused parameter warnings */
    (void)src;

    if (ch != EOF)
        --(*count);

    return ch;
}