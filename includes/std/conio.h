#ifndef _CONIO_H
#define _CONIO_H

extern void _clrscr(void);

extern void _gotoxy(int x, int y);
extern int _wherex(void);
extern int _wherey(void);

extern int _kbhit(void);
extern int _getch(void);
extern int _getche(void);
extern int _ungetch(int c);

extern int _putch(int c);

#endif /* _CONIO_H */
