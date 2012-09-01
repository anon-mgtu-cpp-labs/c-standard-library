#ifndef _INTTYPES_H
#define _INTTYPES_H

#include "_stdc11.h"
#include "stdint.h"

/* wchar_t is defined in multiple headers */
#ifndef _HAS_WCHART
#define _HAS_WCHART
typedef unsigned short wchar_t;
#endif

#define PRId8        "hhd"
#define PRIi8        "hhi"
#define PRIdLEAST8   "hhd"
#define PRIiLEAST8   "hhi"
#define PRIdFAST8    "hhd"
#define PRIiFAST8    "hhi"

#define PRId16       "hd"
#define PRIi16       "hi"
#define PRIdLEAST16  "hd"
#define PRIiLEAST16  "hi"
#define PRIdFAST16   "hd"
#define PRIiFAST16   "hi"

#define PRId32       "d"
#define PRIi32       "i"
#define PRIdLEAST32  "d"
#define PRIiLEAST32  "i"
#define PRIdFAST32   "d"
#define PRIiFAST32   "i"

#define PRId64       "lld"
#define PRIi64       "lli"
#define PRIdLEAST64  "lld"
#define PRIiLEAST64  "lli"
#define PRIdFAST64   "lld"
#define PRIiFAST64   "lli"

#define PRIdMAX      "lld"
#define PRIiMAX      "lli"

#define PRIdPTR      "d"
#define PRIiPTR      "i"

#define PRIo8        "hho"
#define PRIu8        "hhu"
#define PRIx8        "hhx"
#define PRIX8        "hhX"
#define PRIoLEAST8   "hho"
#define PRIuLEAST8   "hhu"
#define PRIxLEAST8   "hhx"
#define PRIXLEAST8   "hhX"
#define PRIoFAST8    "hho"
#define PRIuFAST8    "hhu"
#define PRIxFAST8    "hhx"
#define PRIXFAST8    "hhX"

#define PRIo16       "ho"
#define PRIu16       "hu"
#define PRIx16       "hx"
#define PRIX16       "hX"
#define PRIoLEAST16  "ho"
#define PRIuLEAST16  "hu"
#define PRIxLEAST16  "hx"
#define PRIXLEAST16  "hX"
#define PRIoFAST16   "ho"
#define PRIuFAST16   "hu"
#define PRIxFAST16   "hx"
#define PRIXFAST16   "hX"

#define PRIo32       "o"
#define PRIu32       "u"
#define PRIx32       "x"
#define PRIX32       "X"
#define PRIoLEAST32  "o"
#define PRIuLEAST32  "u"
#define PRIxLEAST32  "x"
#define PRIXLEAST32  "X"
#define PRIoFAST32   "o"
#define PRIuFAST32   "u"
#define PRIxFAST32   "x"
#define PRIXFAST32   "X"

#define PRIo64       "llo"
#define PRIu64       "llu"
#define PRIx64       "llx"
#define PRIX64       "llX"
#define PRIoLEAST64  "llo"
#define PRIuLEAST64  "llu"
#define PRIxLEAST64  "llx"
#define PRIXLEAST64  "llX"
#define PRIoFAST64   "llo"
#define PRIuFAST64   "llu"
#define PRIxFAST64   "llx"
#define PRIXFAST64   "llX"

#define PRIoMAX      "llo"
#define PRIuMAX      "llu"
#define PRIxMAX      "llx"
#define PRIXMAX      "llX"

#define PRIoPTR      "o"
#define PRIuPTR      "u"
#define PRIxPTR      "x"
#define PRIXPTR      "X"

#define SCNd8        "d"
#define SCNi8        "i"
#define SCNdLEAST8   "d"
#define SCNiLEAST8   "i"
#define SCNdFAST8    "d"
#define SCNiFAST8    "i"

#define SCNd16       "hd"
#define SCNi16       "hi"
#define SCNdLEAST16  "hd"
#define SCNiLEAST16  "hi"
#define SCNdFAST16   "hd"
#define SCNiFAST16   "hi"

#define SCNd32       "ld"
#define SCNi32       "li"
#define SCNdLEAST32  "ld"
#define SCNiLEAST32  "li"
#define SCNdFAST32   "ld"
#define SCNiFAST32   "li"

#define SCNd64       "lld"
#define SCNi64       "lli"
#define SCNdLEAST64  "lld"
#define SCNiLEAST64  "lli"
#define SCNdFAST64   "lld"
#define SCNiFAST64   "lli"

#define SCNdMAX      "lld"
#define SCNiMAX      "lli"

#define SCNdPTR      "ld"
#define SCNiPTR      "li"

#define SCNo8        "o"
#define SCNu8        "u"
#define SCNx8        "x"
#define SCNX8        "X"
#define SCNoLEAST8   "o"
#define SCNuLEAST8   "u"
#define SCNxLEAST8   "x"
#define SCNXLEAST8   "X"
#define SCNoFAST8    "o"
#define SCNuFAST8    "u"
#define SCNxFAST8    "x"
#define SCNXFAST8    "X"

#define SCNo16       "ho"
#define SCNu16       "hu"
#define SCNx16       "hx"
#define SCNX16       "hX"
#define SCNoLEAST16  "ho"
#define SCNuLEAST16  "hu"
#define SCNxLEAST16  "hx"
#define SCNXLEAST16  "hX"
#define SCNoFAST16   "ho"
#define SCNuFAST16   "hu"
#define SCNxFAST16   "hx"
#define SCNXFAST16   "hX"

#define SCNo32       "lo"
#define SCNu32       "lu"
#define SCNx32       "lx"
#define SCNX32       "lX"
#define SCNoLEAST32  "lo"
#define SCNuLEAST32  "lu"
#define SCNxLEAST32  "lx"
#define SCNXLEAST32  "lX"
#define SCNoFAST32   "lo"
#define SCNuFAST32   "lu"
#define SCNxFAST32   "lx"
#define SCNXFAST32   "lX"

#define SCNo64       "llo"
#define SCNu64       "llu"
#define SCNx64       "llx"
#define SCNX64       "llX"
#define SCNoLEAST64  "llo"
#define SCNuLEAST64  "llu"
#define SCNxLEAST64  "llx"
#define SCNXLEAST64  "llX"
#define SCNoFAST64   "llo"
#define SCNuFAST64   "llu"
#define SCNxFAST64   "llx"
#define SCNXFAST64   "llX"

#define SCNoMAX      "llo"
#define SCNuMAX      "llu"
#define SCNxMAX      "llx"
#define SCNXMAX      "llX"

#define SCNoPTR      "lo"
#define SCNuPTR      "lu"
#define SCNxPTR      "lx"
#define SCNXPTR      "lX"

typedef struct {
    intmax_t quot;
    intmax_t rem;
} imaxdiv_t;

extern intmax_t imaxabs(intmax_t value);
extern imaxdiv_t imaxdiv(intmax_t numer, intmax_t denom);
extern intmax_t strtoimax(const char * restrict s, char ** restrict end, int base);
extern uintmax_t strtoumax(const char * restrict s, char ** restrict end, int base);

extern intmax_t wcstoimax(const wchar_t * restrict s, wchar_t ** restrict end, int base);
extern uintmax_t wcstoumax(const wchar_t * restrict s, wchar_t ** restrict end, int base);

#endif /* _INTTYPES_H */
