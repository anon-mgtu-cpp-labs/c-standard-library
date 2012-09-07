#include "_system.h"
#include "conio.h"
#include "stdbool.h"
#include "stdio.h"
#include "wchar.h"
#include <Windows.h>

static bool unget_avail;  /* True if a character is available for ungetch */
static int unget_char;

/*
    @description:
        Clears the visible console output buffer.
*/
void _clrscr(void)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    COORD topleft = {0};
    DWORD size, written;

    if (!GetConsoleScreenBufferInfo(__sys_stdout, &info))
        return;

    size = info.dwSize.X * info.dwSize.Y;

    /* Overwrite the visible buffer with blank spaces */
    FillConsoleOutputCharacter(__sys_stdout, ' ', size, topleft, &written);

    /* If the first call succeeded, this one should too */
    GetConsoleScreenBufferInfo(__sys_stdout, &info);

    /*
        Fix the character attributes (color, etc...) for the "whitespace"
        if they weren't set to defaults. Otherwise they would be lost.
        Finally, reset the cursor position.
    */
    FillConsoleOutputAttribute(__sys_stdout, info.wAttributes, size, topleft, &written);
    SetConsoleCursorPosition(__sys_stdout, topleft);
}

/*
    @description:
        Sets the position of the console cursor.
*/
void _gotoxy(int x, int y)
{
    COORD pos;

    /* Conio positions are 1-based, but Win32 positions are 0-based */
    pos.X = (short)x - 1;
    pos.Y = (short)y - 1;

    SetConsoleCursorPosition(__sys_stdout, pos);
}

/*
    @description:
        Gets the X coordinate of the console cursor.
*/
int _wherex(void)
{
    CONSOLE_SCREEN_BUFFER_INFO info;

    if (!GetConsoleScreenBufferInfo(__sys_stdout, &info))
        return -1;

    /* Conio positions are 1-based, but Win32 positions are 0-based */
    return info.dwCursorPosition.X + 1;
}

/*
    @description:
        Gets the X coordinate of the console cursor.
*/
int _wherey(void)
{
    CONSOLE_SCREEN_BUFFER_INFO info;

    if (!GetConsoleScreenBufferInfo(__sys_stdout, &info))
        return -1;

    /* Conio positions are 1-based, but Win32 positions are 0-based */
    return info.dwCursorPosition.Y + 1;
}

/*
    @description:
        Checks the console (keyboard) buffer for a key stroke.
*/
int _kbhit(void)
{
    DWORD saved_mode, read, pending;
    PINPUT_RECORD info;
    int key_ready = 0;
    size_t i;

    /* Save the current mode and switch to raw mode */
    GetConsoleMode(__sys_stdin, &saved_mode);
    SetConsoleMode(__sys_stdin, 0);

    /* Get the number of pending input events (including key presses) */
    if (!GetNumberOfConsoleInputEvents(__sys_stdin, &pending) || !pending)
        return 0;

    if (!(info = _sys_alloc(pending * sizeof *info)))
        return 0;

    /* Peek at and check each pending input event for a key press */
    if (PeekConsoleInput(__sys_stdin, info, pending, &read) && read) {
        for (i = 0; i < read; ++i) {
            if (info[i].EventType == KEY_EVENT && info[i].Event.KeyEvent.bKeyDown) {
                key_ready = 1;
                break;
            }
        }
    }

    _sys_free(info);

    /* Restore the original console mode */
    SetConsoleMode(__sys_stdin, saved_mode);

    return key_ready;
}

/*
    @description:
        Retrieves a character directly from the console (keyboard) buffer without echo.
*/
int _getch(void)
{
    if (unget_avail) {
        /* There was a preceding ungetch */
        unget_avail = false;
        return unget_char;
    }
    else {
        /* No ungetch, so we need to pull from the console buffer */
        DWORD saved_mode, read;
        INPUT_RECORD info;
        int ch = EOF;

        /* Save the current mode and switch to raw mode */
        GetConsoleMode(__sys_stdin, &saved_mode);
        SetConsoleMode(__sys_stdin, 0);

        /* Read a raw character */
        while (ch == EOF) {
            if (!ReadConsoleInput(__sys_stdin, &info, 1, &read) || !read) {
                ch = EOF;
                break;
            }
            else {
                KEY_EVENT_RECORD key = info.Event.KeyEvent;
                size_t i;

                if (info.EventType == KEY_EVENT && key.bKeyDown) {
                    /* Save the current code (may be an escape for extended codes) */
                    ch = key.uChar.AsciiChar;
                
                    if (ch == 0 || ch == 0xe0) {
                        /* Push the extended code for the next call */
                        unget_avail = true;
                        unget_char = key.wVirtualScanCode;
                    }
                }
            }
        }

        /* Restore the original console mode */
        SetConsoleMode(__sys_stdin, saved_mode);

        return ch;
    }
}

/*
    @description:
        Retrieves a character directly from the console (keyboard) buffer with echo.
*/
int _getche(void)
{
    return _putch(_getch());
}

/*
    @description:
        Returns a previously retrieved console character such 
        that the next getch will retrieve it again.
*/
int _ungetch(int c)
{
    if (unget_avail)
        return EOF; /* Only allow one ungetch at a time */

    unget_avail = true;
    unget_char = c;

    return c;
}

/*
    @description:
        Writes a character directly to the console (screen) buffer.
*/
int _putch(int c)
{
    DWORD written;

    if (WriteFile(__sys_stdout, &c, 1, &written, 0))
        return EOF;

    return c;
}