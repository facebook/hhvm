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

#ifndef incl_HPHP_NEO_STR_H_
#define incl_HPHP_NEO_STR_H_ 1

__BEGIN_DECLS

#include <stdarg.h>
#include <stdio.h>
#include "hphp/neo/neo_bool.h"
#include "hphp/neo/neo_misc.h"

/* This modifies the string its called with by replacing all the white
 * space on the end with \0, and returns a pointer to the first
 * non-white space character in the string
 */
char *neos_strip (char *s);

char *vsprintf_alloc (const char *fmt, va_list ap);
char *vnsprintf_alloc (int start_size, const char *fmt, va_list ap);

/* Versions of the above which actually return a length, necessary if
 * you expect embedded NULLs */
int vnisprintf_alloc (char **buf, int start_size, const char *fmt, va_list ap);
int visprintf_alloc (char **buf, const char *fmt, va_list ap);

typedef struct _string
{
  char *buf;
  int len;
  int max;
} NEOSTRING;

/* At some point, we should add the concept of "max len" to these so we
 * can't get DoS'd by someone sending us a line without an end point,
 * etc. */
void string_init (NEOSTRING *str);
NEOERR *string_append (NEOSTRING *str, const char *buf);
NEOERR *string_appendn (NEOSTRING *str, const char *buf, int l);
NEOERR *string_append_char (NEOSTRING *str, char c);
NEOERR *string_appendf (NEOSTRING *str, const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);
NEOERR *string_appendvf (NEOSTRING *str, const char *fmt, va_list ap);
void string_clear (NEOSTRING *str);


char *repr_string_alloc (const char *s);


__END_DECLS

#endif /* incl_HPHP_NEO_STR_H_ */
