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
UINT32 ne_crc (UINT8 *data, UINT32 bytes);

__END_DECLS

#endif /* incl_HPHP_NEO_MISC_H_ */
