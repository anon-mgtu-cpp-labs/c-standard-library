#ifndef __DTOA_H
#define __DTOA_H

extern char *_dtoa(char s[], double value, int precision, int show_sign, int show_zeros, int show_radix, int alt_case);
extern char *_dtoa_normal(char s[], double value, int precision, int show_sign, int alt_case, int force_normal);
extern char *_hdtoa(char s[], double value, int precision, int show_sign, int show_radix, int alt_case);

#endif /* __DTOA_H */
