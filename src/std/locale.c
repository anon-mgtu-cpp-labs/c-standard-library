#include "_system.h"
#include "_syslocale.h"
#include "limits.h"
#include "locale.h"
#include "stdlib.h"
#include "string.h"

/* 
    ===================================================
                Static helper declarations
    ===================================================
*/

typedef struct {
    char    *name; /* Name of the locale category */
    _locale *loc;  /* Assigned locale object */
} _locale_category;

/* Current locale settings for each category */
static _locale_category __curr_category[_LC_MAX];

/* Default values defined in _locale_default.c for brevity */
extern struct lconv   __default_lconv;
extern struct _ltime  __default_ltime;
extern struct _lctype __default_ctype;

static void unload_locale(_locale **loc);

/* 
    ===================================================
              Public function definitions
    ===================================================
*/

/*
    @description:
        Selects the appropriate portion of the program's locale
        as specified by the category and locale arguments.
*/
char *setlocale(int category, const char *locale)
{
    int is_default;
    _locale *loc;
    size_t i;

    /* Sanity check the category */
    if (category < _LC_MIN || category > _LC_MAX)
        return NULL;

    /* Retrieving the current locale? */
    if (!locale)
        return __curr_category[category].name;

    is_default = strcmp(locale, "C") == 0;

    if (__curr_category[category].loc) {
        /*
            Check to see if the locale has changed for the specified category.
            If it hasn't then we don't need to do anthing, just return the
            current locale name as if the requested locale were NULL.
        */
        if (category != LC_ALL && strcmp(__curr_category[category].name, locale) == 0) {
            /* Requesting the same locale from a non-LC_ALL category */
            return __curr_category[category].name;
        }
        else if (category == LC_ALL) {
            /* True if any category is different than the requested locale */
            int new_locale = 0;

            for (i = _LC_MIN; i <= _LC_MAX; ++i) {
                if (strcmp(__curr_category[category].name, locale) != 0) {
                    /* At least one category will be changing to a new locale */
                    new_locale = 1;
                    break;
                }
            }

            if (!new_locale) {
                /*
                    Requesting the same locale from LC_ALL, and no other
                    categories have been reset since the last LC_ALL.
                */
                return __curr_category[category].name;
            }
        }
    }

    if (!*locale) {
        /* Handle a request for the "" locale */
        locale = _sys_local_localename();

        if (!locale)
            return NULL;
    }

    if (!(loc = (_locale*)malloc(sizeof *loc)))
        return NULL;
    
    if (is_default) {
        /* Handle a request for the "C" locale */
        loc->refcount = -1; /* Disable freeing of memory */
        loc->lcid = 0;
        loc->name = "C";
        loc->ctype = __default_ctype;
        loc->numeric = __default_lconv;
        loc->datetime = __default_ltime;
    }
    else {
        /* Try to get the requested locale from the system */
        if (!_sys_load_locale(locale, loc)) {
            free(loc);
            return NULL;
        }
    }

    if (category != LC_ALL) {
        unload_locale(&__curr_category[category].loc);

        __curr_category[category].loc = loc;
        __curr_category[category].name = __curr_category[category].loc->name;
    }
    else {
        _unload_all_locales();

        /* Update all categories to the selected locale */
        for (i = _LC_MIN; i <= _LC_MAX; ++i) {
            __curr_category[i].loc = loc;
            __curr_category[i].name = __curr_category[i].loc->name;

            if (loc->refcount != -1)
                ++loc->refcount;
        }
    }

    return __curr_category[category].name;
}

/*
    @description:
        Sets the components of an object with type struct lconv with
        values appropriate for the formatting of numeric quantities
        according to the rules of the current locale.
*/
struct lconv *localeconv(void)
{
    return &__curr_category[LC_NUMERIC].loc->numeric;
}

/*
    @description:
        Retrieves the internal locale ID for the specified category.
*/
unsigned long _localeid(int category)
{
    return __curr_category[category].loc->lcid;
}

/*
    @description:
        Sets the components of an object with type struct _ltime
        with values appropriate for the formatting of dates and
        times according to the rules of the current locale.
*/
struct _ltime *_localetime(void)
{
    return &__curr_category[LC_TIME].loc->datetime;
}

/*
    @description:
        Retrieves the current locale's codepage identifier.
*/
char *_localecodepage(void)
{
    return __curr_category[LC_CTYPE].loc->ctype.codepage;
}

/*
    @description:
        Retrieve's the current locale's codepage.
*/
unsigned short *_localectype(void)
{
    return __curr_category[LC_CTYPE].loc->ctype.ctype;
}

/*
    @description:
        Unload the locale object in every category.
*/
void _unload_all_locales(void)
{
    int i;

    for (i = _LC_MIN; i <= _LC_MAX; ++i)
        unload_locale(&__curr_category[i].loc);
}

/* 
    ===================================================
                Static helper definitions
    ===================================================
*/

/*
    @description:
        Release all memory owned by a locale object, or decrement
        the reference count if multiple references exist.
*/
void unload_locale(_locale **loc)
{
    _locale *ploc = *loc;

    if (!ploc || ploc->refcount == -1)
        return;

    if (--ploc->refcount == 0) {
        int i;

        /* Release memory owned by the locale object */
        free(ploc->name);
        free(ploc->numeric.decimal_point);
        free(ploc->numeric.thousands_sep);
        free(ploc->numeric.grouping);
        free(ploc->numeric.mon_decimal_point);
        free(ploc->numeric.mon_thousands_sep);
        free(ploc->numeric.mon_grouping);
        free(ploc->numeric.positive_sign);
        free(ploc->numeric.negative_sign);
        free(ploc->numeric.currency_symbol);
        free(ploc->numeric.int_curr_symbol);

        for (i = 0; i < 12; ++i)
            free(ploc->datetime.mon_name_long[i]);

        for (i = 0; i < 12; ++i)
            free(ploc->datetime.mon_name[i]);

        for (i = 0; i < 7; ++i)
            free(ploc->datetime.wday_name_long[i]);

        for (i = 0; i < 7; ++i)
            free(ploc->datetime.wday_name[i]);

        free(ploc->datetime.date_format);
        free(ploc->datetime.time_format);
        free(ploc->datetime.time12_format);
        free(ploc->datetime.datetime_format);
        free(ploc->datetime.am);
        free(ploc->datetime.pm);

        free(ploc->ctype.codepage);

        free(*loc);
        *loc = NULL;
    }
}