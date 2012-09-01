#include "_fmtspec.h"
#include "ctype.h"
#include "float.h"
#include "stdbool.h"
#include "stdlib.h"
#include "limits.h"

static int _load_scanset(_scanspec_t *spec, const char *fmt, size_t pos);

/*
    @description:
        Parse and extract a scanf format string specifier.
*/
int _load_scanspec(_scanspec_t *spec, const char *fmt)
{
    bool done = false;
    int length = 0; /* Offset from _SPEC_*INT or _SPEC_FLOAT for the corresponding type */
    size_t pos = 0;
    char ch = 0;

    if (fmt[pos] != '%')
        return 0;

    /* Reset to suitable defaults */
    spec->suppressed = 0;
    spec->field_width = 0;
    spec->type = _SPEC_NO_MATCH;
    spec->format = _SPEC_FMT_NO_FORMAT;
    spec->skipws = 1; /* All specifications skip by default */
    spec->nomatch = 0;

    free(spec->scanset);
    spec->scanset = NULL;

    /* Get the non-type flags */
    while (!done && (ch = fmt[++pos])) {
        if (isdigit (ch))
            spec->field_width = 10 * spec->field_width + (ch - '0');
        else {
            switch (ch) {
            case '*': spec->suppressed = 1; break;
            case 'h':
                length = -1;

                if (fmt[pos + 1] == 'h') {
                    --length;
                    ++pos;
                }

                break;
            case 'l':
                length = +1;

                if (fmt[pos + 1] == 'l') {
                    ++length;
                    ++pos;
                }

                break;
            case 'j': length = +3; break;
            case 'z': length = +4; break;
            case 't': length = +4; break;
            case 'L': length = +2; break;
            default:  done = true; break;
            }
        }
    }

    /* A, E, F, G, and X behave the same as a, e, f, g, and x */
    ch = (char)tolower(ch);

    /* Fill in the type data */
    switch (ch) {
    case 'd':
        spec->type = _SPEC_INT + length;
        spec->format = _SPEC_FMT_DECIMAL;
        break;
    case 'i': spec->type = _SPEC_INT + length; break;
    case 'o':
        spec->type = _SPEC_UINT + length;
        spec->format = _SPEC_FMT_OCTAL;
        break;
    case 'u':
        spec->type = _SPEC_UINT + length;
        spec->format = _SPEC_FMT_DECIMAL;
        break;
    case 'x':
        spec->type = _SPEC_UINT + length;
        spec->format = _SPEC_FMT_HEX;
        break;
    case 'e':
    case 'f':
    case 'g': spec->type = _SPEC_FLOAT + length; break;
    case 'c':
        spec->type = _SPEC_CHAR;
        spec->skipws = 0;

        /* Default to 1 character */
        if (spec->field_width == 0)
            spec->field_width = 1;

        break;
    case 's': spec->type = _SPEC_STRING; break;
    case '[':
        pos = _load_scanset(spec, fmt, pos);
        spec->type = _SPEC_SCANSET;
        spec->skipws = 0;
        break;
    case 'p':
        spec->type = _SPEC_POINTER;
        spec->format = _SPEC_FMT_HEX;
        break;
    case 'n':
        spec->type = _SPEC_COUNT;
        spec->skipws = 0;
        break;
    case '%': spec->type = _SPEC_LITERAL; break;
    default:  return 0; /* Invalid specifier */
    }

    return pos + 1;
}

/*
    @description:
        Parse and extract a printf format string specifier.
*/
int _load_printspec(_printspec_t *spec, const char *fmt)
{
    bool found_precision = false, skip_precision = false;
    bool done = false;
    int length = 0; /* Offset from _SPEC_*INT or _SPEC_FLOAT for the corresponding type */
    size_t pos = 0;
    char ch = 0;

    if (fmt[pos] != '%')
        return 0;

    /* Reset to suitable defaults */
    spec->field_width = _NOT_SPECIFIED;
    spec->precision = _NOT_SPECIFIED;
    spec->type = _SPEC_NO_MATCH;
    spec->format = _SPEC_FMT_NO_FORMAT;
    spec->flags = 0;
    spec->pad = ' ';

    /* Get the special flags first */
    while (!done && (ch = fmt[++pos])) {
        switch (ch) {
        case '-': spec->flags |= _LEFT_JUSTIFY; break;
        case '+': spec->flags |= _SHOW_SIGN;    break;
        case ' ': spec->flags |= _SHOW_SPACE;   break;
        case '#': spec->flags |= _ALT_FORMAT;   break;
        case '0': spec->pad = '0';              break;
        default:  done = true;                  break;
        }
    }

    done = 0;

    /* Get the non-type flags */
    while (!done) {
        if (isdigit(ch)) {
            if (found_precision && !skip_precision) {
                if (spec->precision == _NOT_SPECIFIED)
                    spec->precision = 0;

                spec->precision = 10 * spec->precision + (ch - '0');
            }
            else if (!found_precision) {
                if (spec->field_width == _NOT_SPECIFIED)
                    spec->field_width = 0;

                spec->field_width = 10 * spec->field_width + (ch - '0');
            }
        }
        else {
            switch (ch) {
            case '*':
                if (found_precision)
                    spec->precision = _ARG_SPECIFIED;
                else
                    spec->field_width = _ARG_SPECIFIED;

                break;
            case '-':
                if (!found_precision)
                    return 0; /* Invalid specifier */
                else {
                    spec->precision = 0;
                    skip_precision = 1;
                }

                break;
            case '.': found_precision = 1; break;
            case 'h':
                length = -1;

                if (fmt[pos + 1] == 'h') {
                    --length;
                    ++pos;
                }

                break;
            case 'l':
                length = +1;

                if (fmt[pos + 1] == 'l') {
                    ++length;
                    ++pos;
                }

                break;
            case 'j': length = +3; break;
            case 'z': length = +4; break;
            case 't': length = +4; break;
            case 'L': length = +2; break;
            default:
                if (found_precision && spec->precision == _NOT_SPECIFIED)
                    spec->precision = 0;

                done = 1;
                break;
            }
        }

        if (!done && !(ch = fmt[++pos]))
            break;
    }

    if (spec->flags & _LEFT_JUSTIFY)
        spec->pad = ' ';

    /* Fill in the type data */
    switch (ch) {
    case 'd':
    case 'i':
        spec->format = _SPEC_FMT_DECIMAL;
        spec->type = _SPEC_INT + length;
        break;
    case 'u':
        spec->format = _SPEC_FMT_DECIMAL;
        spec->type = _SPEC_UINT + length;
        break;
    case 'o':
        spec->alt_case = 1;
        spec->format = _SPEC_FMT_OCTAL;
        spec->type = _SPEC_UINT + length;
        break;
    case 'X':
        spec->alt_case = 1;
    case 'x':
        spec->format = _SPEC_FMT_HEX;
        spec->type = _SPEC_UINT + length;
        break;
    case 'A':
        spec->alt_case = 1; /* Fall through */
    case 'a':
        spec->format = _SPEC_FMT_HEXFLOAT;

        if (!found_precision) {
            /* It's already set this way, just being explicit */
            spec->precision = _NOT_SPECIFIED;
        }

        if (length == 0)
            length = 1;

        spec->type = _SPEC_FLOAT + length;
        break;
    case 'E':
    case 'F':
        spec->alt_case = 1; /* Fall through */
    case 'e':
    case 'f':
        spec->format = (tolower(ch) == 'f') ? _SPEC_FMT_FLOAT : _SPEC_FMT_NORMAL;

        if (!found_precision)
            spec->precision = FLT_DECIMAL_DIG;

        if (length == 0)
            length = 1;

        spec->type = _SPEC_FLOAT + length;
        break;
    case 'G':
        spec->alt_case = 1; /* Fall through */
    case 'g':
        spec->format = _SPEC_FMT_NO_FORMAT;

        if (!found_precision)
            spec->precision = FLT_DECIMAL_DIG;

        if (length == 0)
            length = 1;

        spec->type = _SPEC_FLOAT + length;
        break;
    case 'c': spec->type = _SPEC_CHAR; break;
    case 's':
        spec->pad = ' ';
        spec->type = _SPEC_STRING;
        break;
    case 'p':
        spec->alt_case = 1;
        spec->precision = 8;
        spec->format = _SPEC_FMT_HEX;
        spec->type = _SPEC_POINTER;
        break;
    case 'n': spec->type = _SPEC_COUNT; break;
    case '%': spec->type = _SPEC_LITERAL; break;
    default: return 0; /* Invalid specifier */
    }

    return pos + 1;
}

/*
    @description:
        Parse and extract a scanf scanset.
*/
int _load_scanset(_scanspec_t *spec, const char *fmt, size_t pos)
{
    size_t start = pos;
    bool done = false;

    if (fmt[pos] != '[')
        return 0;

    /* Create a lookup table for every character */
    if (!spec->scanset&& !(spec->scanset = (int*)calloc(UCHAR_MAX + 1, sizeof *spec->scanset)))
        return 0;

    /* Fill up the lookup table */
    while (!done && fmt[++pos] != '\0') {
        bool firstchar = (pos == start + 1 || (pos == start + 2 && fmt[pos - 1] == '^'));

        /*
            A hat character can be in the scanset, but if it's the
            first character, the scanset denotes a negative lookup
            and the hat is *not* stored.
        */
        if (fmt[pos] == '^' && pos == start + 1) {
            spec->nomatch = 1;
            continue;
        }

        /*
            A closing square bracket can be in the scanset, but
            only immediately after the the opening "[" or "[^".
        */
        if (fmt[pos] == ']' && !firstchar) {
            done = true;
            continue;
        }

        if (!(fmt[pos] == '-' && !firstchar && fmt[pos + 1] != ']'))
            ++spec->scanset[fmt[pos]]; /* Add a non-range character to the lookup table */
        else {
            /*
                It's implementation-defined how *scanf treats a '-' character 
                anywhere except the beginning of the scanset. That clause
                practically begs the implementation to employ a range, so
                this implementation does so. :)
            */
            if (fmt[pos] == '-' && !firstchar && fmt[pos + 1] != ']') {
                int begin = fmt[pos - 1], end = fmt[++pos];

                if (begin < end) {
                    while (++begin < end)
                        ++spec->scanset[begin];
                }
                else {
                    while (--begin > end)
                        ++spec->scanset[begin];
                }
            }
        }
    }

    return pos;
}