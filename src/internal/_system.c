#include "_system.h"
#include "errno.h"
#include "string.h"
#include "wchar.h"
#include <Windows.h>

_sys_handle_t __sys_heap;
_sys_handle_t __sys_stdin;
_sys_handle_t __sys_stdout;
_sys_handle_t __sys_stderr;

/* 
    ===================================================
              Public function definitions
    ===================================================
*/

/*
    @description:
        Retrieves the system standard input handle.
*/
_sys_handle_t _sys_stdin(void)
{
    return GetStdHandle(STD_INPUT_HANDLE);
}

/*
    @description:
        Retrieves the system standard output handle.
*/
_sys_handle_t _sys_stdout(void)
{
    return GetStdHandle(STD_OUTPUT_HANDLE);
}

/*
    @description:
        Retrieves the system standard error handle.
*/
_sys_handle_t _sys_stderr(void)
{
    return GetStdHandle(STD_ERROR_HANDLE);
}

/*
    @description:
        Retrieves the command line string.
*/
char *_sys_commandline(void)
{
    return GetCommandLine();
}

/*
    @description:
        Exit the current thread with the specified status.
*/
void _sys_exit(int status)
{
    TerminateThread(GetCurrentThread(), status);
}

/*
    @description:
        Retrieve the requested environment variable by name.
*/
char *_sys_getenv(const char *name)
{
    static char buf[32767];

    if (GetEnvironmentVariable(name, buf, sizeof buf) == 0)
        return 0;

    return buf;
}

/*
    @description:
        Run the command line as a separate process.
*/
int _sys_system(const char *cmd_proc, char *cmd_line)
{
    PROCESS_INFORMATION pi;
    STARTUPINFO si = {0};
    DWORD exit_code;

    si.cb = sizeof si;
    si.wShowWindow = SW_SHOWDEFAULT;

    if (!CreateProcessA(cmd_proc, cmd_line, 0, 0, TRUE, 0, 0, 0, &si, &pi))
        return -1;

    CloseHandle(pi.hThread);

    if (WaitForSingleObject(pi.hProcess, INFINITE) != WAIT_OBJECT_0)
        return -1;

    if (!GetExitCodeProcess(pi.hProcess, &exit_code))
        return -1;
    
    CloseHandle(pi.hProcess);

    return (int)exit_code;
}

/*
    @description:
        Allocate the specified number of bytes from the global store.
*/
void *_sys_alloc(unsigned bytes)
{
    return GlobalAlloc(0, bytes);
}

/*
    @description:
        Free the specified block from the global store.
        Do nothing if the pointer to the block is NULL.
*/
void _sys_free(void *p)
{
    if (p)
        GlobalFree(p);
}

/*
    @description:
        Create a new heap for use with _sys_heapalloc, _sys_heaprealloc, and _sysheapfree.
*/
_sys_handle_t _sys_heapcreate()
{
    return HeapCreate(0, 0, 0);
}

/*
    @description:
        Destroy and invalidate a heap returned by _sys_heapcreate.
*/
void _sys_heapdestroy(_sys_handle_t *heap)
{
    HeapDestroy(*heap);
    *heap = _SYS_BADHANDLE;
}

/*
    @description:
        Allocate the specified number of bytes from the specified heap.
*/
void *_sys_heapalloc(_sys_handle_t heap, unsigned bytes)
{
    return HeapAlloc(heap, 0, bytes);
}

/*
    @description:
        Reallocate the specified number of bytes for the block
        pointed to by p from the specified heap.
*/
void *_sys_heaprealloc(_sys_handle_t heap, void *p, unsigned bytes)
{
    return HeapReAlloc(heap, 0, p, bytes);
}

/*
    @description:
        Free the specified block from the specified heap.
        Do nothing if the pointer to the block is NULL.
*/
void _sys_heapfree(_sys_handle_t heap, void *p)
{
    if (p)
        HeapFree(heap, 0, p);
}

/*
    @description:
        Retrieve a temporary file path.
*/
int _sys_temppath(char *buf, int n)
{
    return GetTempPath(n, buf);
}

/*
    @description:
        Generate a unique filename for the specified path.
*/
unsigned _sys_tempfilename(const char* path, const char *prefix, char *buf)
{
    return GetTempFileName(path, prefix, 0, buf);
}

/*
    @description:
        Convert a mode string into Windows specific open flags
*/
int _sys_parse_openmode(const char *mode, unsigned *flag, int *orient, int *attr, int *share)
{
    int exclusive_available = 0;
    int modes_found = 0;

    *flag |= 0x0040; /* Assume text mode (see stdio.h) */

    /* Determine the open orientation (read, write, or append) */
    switch (*mode) {
    case 'r':
        *orient = GENERIC_READ;
        *attr = OPEN_EXISTING;
        *share = FILE_SHARE_READ;
        break;
    case 'w':
        *orient = GENERIC_WRITE;
        *attr = CREATE_NEW | TRUNCATE_EXISTING;
        *share = FILE_SHARE_WRITE;
        exclusive_available = 1;
        break;
    case 'a':
        *orient = 
            FILE_APPEND_DATA | 
            FILE_WRITE_ATTRIBUTES | 
            FILE_WRITE_EA | 
            STANDARD_RIGHTS_WRITE | 
            SYNCHRONIZE;
        *attr = CREATE_NEW | OPEN_EXISTING;
        *share = FILE_SHARE_WRITE;
        break;
    default:
        return 0; /* Invalid mode */
    }

    /*
        Determine subsequent attributes for the orientation.

        There are 4 general cases for valid attributes: "+", "+b", "b", "b+". If
        the initial character is "w", then the mode may end with "x" to specify 
        exclusive access.
    */
    while (*++mode && ++modes_found != 4) {
        switch (*mode) {
        case '+':
            /* Make sure that GENERIC_WRITE is not assigned for append mode */
            *orient |= (*orient == GENERIC_READ) ? GENERIC_WRITE : GENERIC_READ;
            break;
        case 'b':
            /* Switch to binary mode (see stdio.h) */
            *flag &= ~0x0040;
            break;
        case 'x':
            /* Ensure that we're in write mode and this is the last attribute */
            if (!exclusive_available || mode[1])
                return 0; /* Invalid mode */

            /* Disable sharing */
            *share = 0;
            break;
        default:
            return 0; /* Invalid mode */
        }
    }

    return 1;
}

/*
    @description:
        Attempt to open a file represented by filename with the specified attributes.
*/
_sys_handle_t _sys_openfile(const char *filename, int orient, int attr, int share)
{
    _sys_handle_t handle = CreateFile(filename, orient, share, 0, attr, FILE_ATTRIBUTE_NORMAL, 0);

    if (handle == _SYS_BADHANDLE) {
        int error = GetLastError();
        switch (error) {
        case ERROR_PATH_NOT_FOUND:      errno = ENOENT;   break;
        case ERROR_FILE_NOT_FOUND:      errno = ENOENT;   break;
        case ERROR_INVALID_NAME:        errno = ENOENT;   break;
        case ERROR_TOO_MANY_OPEN_FILES: errno = ENFILE;   break;
        case ERROR_ACCESS_DENIED:       errno = EACCESS;  break;
        default:                        errno = EUNKNOWN; break;
        }
    }

    return handle;
}

/*
    @description:
        Close a handle previously opened with _sys_openfile.
*/
int _sys_closefile(_sys_handle_t fd)
{
    return CloseHandle(fd);
}

/*
    @description:
        Attempt to read n bytes from the specified file into the array pointed to by p.
*/
int _sys_read(_sys_handle_t fd, void *p, int n)
{
    DWORD nread;

    if (!ReadFile(fd, p, n, &nread, 0))
        return -1;

    return nread;
}

/*
    @description:
        Attempt to write n bytes from the array pointed to by p to the specified file.
*/
int _sys_write(_sys_handle_t fd, void *p, int n)
{
    DWORD nwritten = 0;

    if (n > 0 && !WriteFile(fd, p, n, (LPDWORD)&nwritten, 0))
        return -1;

    return nwritten;
}

/*
    @description:
        Retrieve the current file position indicator for the specified file.
*/
int _sys_tell(_sys_handle_t fd, long long *pos)
{
    LARGE_INTEGER sys_pos;

    sys_pos.QuadPart = 0;
    sys_pos.LowPart = SetFilePointer(fd, sys_pos.LowPart, &sys_pos.HighPart, FILE_CURRENT);

    *pos = sys_pos.QuadPart;
    
    return sys_pos.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR;
}

/*
    @description:
        Set the current file position indicator for the specified file.
*/
int _sys_seek(_sys_handle_t fd, long long offset, int whence)
{
    LARGE_INTEGER pos;

    pos.QuadPart = offset;
    pos.LowPart = SetFilePointer(fd, pos.LowPart, &pos.HighPart, whence);

    return pos.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR;
}

/*
    @description:
        Delete the specified file by name.
*/
int _sys_unlink(const char *filename)
{
    return DeleteFile(filename) == 0;
}

/*
    @description:
        Rename the specified file.
*/
int _sys_move(const char *old_name, const char *new_name)
{
    return MoveFile(old_name, new_name) == 0;
}

/*
    @description:
        Converts a sequence of multibyte characters to wide characters.
*/
int _sys_mbtowc(wchar_t *wc, const char *s, int n)
{
    int len;
    
    if (!s)
        return -1;

    len = MultiByteToWideChar(CP_THREAD_ACP, MB_ERR_INVALID_CHARS | MB_PRECOMPOSED, s, n, 0, 0);

    if (len == 0)
        return -1;

    return MultiByteToWideChar(CP_THREAD_ACP, MB_ERR_INVALID_CHARS | MB_PRECOMPOSED, s, n, wc, len);
}

/*
    @description:
        Converts a sequence of wide characters to multibyte characters.
*/
int _sys_wctomb(char *s, const wchar_t *wc, int n)
{
    int len;

    if (!s)
        return -1;
    
    if (*wc == L'\0') {
        *s = '\0';
        return 1;
    }

    len = WideCharToMultiByte(CP_THREAD_ACP, 0, wc, n, 0, 0, '\0', 0);

    if (len == 0)
        return -1;

    return WideCharToMultiByte(CP_THREAD_ACP, 0, wc, n, s, len, '\0', 0);
}