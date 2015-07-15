/*
 * Copyright 2001-2004 Brandon Long
 * All Rights Reserved.
 *
 * ClearSilver Templating System
 *
 * This code is made available under the terms of the ClearSilver License.
 * http://www.clearsilver.net/license.hdf
 *
 */

#ifndef incl_HPHP_NEO_MISC_H_
#define incl_HPHP_NEO_MISC_H_ 1

#include <stdlib.h>
#include <time.h>
#include <limits.h>

/* In case they didn't start from ClearSilver.h. */
#ifndef incl_HPHP_CS_CONFIG_H_
#include "hphp/neo/cs_config.h"
#endif

/* Fix Up for systems that don't define these standard things */
#ifndef __BEGIN_DECLS
#ifdef __cplusplus
#define __BEGIN_DECLS extern "C" {
#define __END_DECLS }
#else
#define __BEGIN_DECLS
#define __END_DECLS
#endif
#endif

#define PATH_BUF_SIZE 512

#ifdef _MSC_VER
#include <sys/stat.h>
#define S_IXUSR 0
#define S_IWUSR _S_IWRITE
#define S_IRUSR _S_IREAD
#endif

#ifndef S_IXGRP
#define S_IXGRP S_IXUSR
#endif
#ifndef S_IWGRP
#define S_IWGRP S_IWUSR
#endif
#ifndef S_IRGRP
#define S_IRGRP S_IRUSR
#endif
#ifndef S_IXOTH
#define S_IXOTH S_IXUSR
#endif
#ifndef S_IWOTH
#define S_IWOTH S_IWUSR
#endif
#ifndef S_IROTH
#define S_IROTH S_IRUSR
#endif

/* Format string checking for compilers that support it (GCC style) */

#ifndef ATTRIBUTE_PRINTF
#if __GNUC__ > 2 || __GNUC__ == 2 && __GNUC_MINOR__ > 6
#define ATTRIBUTE_PRINTF(a1,a2) __attribute__((__format__ (__printf__, a1, a2)))
#else
#define ATTRIBUTE_PRINTF(a1,a2)
#endif
#endif


__BEGIN_DECLS

#ifndef HAVE_STRTOK_R
char * strtok_r (char *s,const char * delim, char **save_ptr);
#endif

#ifndef HAVE_LOCALTIME_R
struct tm *localtime_r (const time_t *timep, struct tm *ttm);
#endif

#ifndef HAVE_GMTIME_R
struct tm *gmtime_r(const time_t *timep, struct tm *ttm);
#endif

#ifndef HAVE_MKSTEMP
int mkstemp(char *path);
#endif

#ifndef HAVE_SNPRINTF
int snprintf (char *str, size_t count, const char *fmt, ...)
              ATTRIBUTE_PRINTF(3,4);
#endif

#ifndef HAVE_VSNPRINTF
int vsnprintf (char *str, size_t count, const char *fmt, va_list arg);
#endif

#include <stdarg.h>
#include <sys/types.h>

typedef unsigned int UINT32;
typedef unsigned short int UINT16;
typedef short int INT16;
typedef unsigned char UINT8;
/* This was conflicting with a cygwin header definition */
#if defined(__CYGWIN__) || defined(_MSC_VER)
typedef signed char INT8;
#else
typedef char INT8;
#endif

#ifndef MIN
#define MIN(x,y)        (((x) < (y)) ? (x) : (y))
#endif

void ne_vwarn (const char *fmt, va_list ap)
               ATTRIBUTE_PRINTF(1,0);
void ne_warn (const char *fmt, ...)
              ATTRIBUTE_PRINTF(1,2);
void ne_set_log (int level);
void ne_log (int level, const char *fmt, ...)
             ATTRIBUTE_PRINTF(2,3);
UINT32 python_string_hash (const char *s);
UINT8 *ne_stream4 (UINT8  *dest, UINT32 num);
UINT8 *ne_unstream4 (UINT32 *pnum, UINT8 *src);
UINT8 *ne_stream2 (UINT8  *dest, UINT16 num);
UINT8 *ne_unstream2 (UINT16 *pnum, UINT8 *src);
UINT8 *ne_stream_str (UINT8 *dest, const char *s, int l);
UINT8 *ne_unstream_str (char *s, int l, UINT8 *src);
double ne_timef (void);
UINT32 ne_crc (UINT8 *data, UINT32 bytes);

__END_DECLS

#endif /* incl_HPHP_NEO_MISC_H_ */
