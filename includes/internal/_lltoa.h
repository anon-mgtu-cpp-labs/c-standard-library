#ifndef __LLTOA_H
#define __LLTOA_H

extern char *_lltoa(char s[], long long value, int radix, int show_sign, int alt_case, int thousands_sep);
extern char *_ulltoa(char s[], unsigned long long value, int radix, int alt_case, int ignore_locale);

#endif /* __LLTOA_H */
