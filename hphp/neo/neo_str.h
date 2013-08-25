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
#include "hphp/neo/neo_misc.h"

/* This modifies the string its called with by replacing all the white
 * space on the end with \0, and returns a pointer to the first
 * non-white space character in the string
 */
char *neos_strip (char *s);

void neos_lower (char *s);

char *sprintf_alloc (const char *fmt, ...) ATTRIBUTE_PRINTF(1,2);
char *nsprintf_alloc (int start_size, const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);
char *vsprintf_alloc (const char *fmt, va_list ap);
char *vnsprintf_alloc (int start_size, const char *fmt, va_list ap);

/* Versions of the above which actually return a length, necessary if
 * you expect embedded NULLs */
int vnisprintf_alloc (char **buf, int start_size, const char *fmt, va_list ap);
int visprintf_alloc (char **buf, const char *fmt, va_list ap);
int isprintf_alloc (char **buf, const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);

typedef struct _string
{
  char *buf;
  int len;
  int max;
} NEOSTRING;

typedef struct _string_array
{
  char **entries;
  int count;
  int max;
} NEOSTRING_ARRAY;

/* At some point, we should add the concept of "max len" to these so we
 * can't get DoS'd by someone sending us a line without an end point,
 * etc. */
void string_init (NEOSTRING *str);
NEOERR *string_set (NEOSTRING *str, const char *buf);
NEOERR *string_append (NEOSTRING *str, const char *buf);
NEOERR *string_appendn (NEOSTRING *str, const char *buf, int l);
NEOERR *string_append_char (NEOSTRING *str, char c);
NEOERR *string_appendf (NEOSTRING *str, const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);
NEOERR *string_appendvf (NEOSTRING *str, const char *fmt, va_list ap);
NEOERR *string_readline (NEOSTRING *str, FILE *fp);
void string_clear (NEOSTRING *str);

/* typedef struct _ulist ULIST; */
#include "hphp/neo/ulist.h"
/* s is not const because we actually temporarily modify the string
 * during split */
NEOERR *string_array_split (ULIST **list, char *s, const char *sep,
                            int max);

BOOL reg_search (const char *re, const char *str);

/* NEOS_ESCAPE details the support escape contexts/modes handled
 * by various NEOS helper methods and reused in CS itself. */
typedef enum
{
  NEOS_ESCAPE_UNDEF    =  0,    /* Used to force eval-time checking */
  NEOS_ESCAPE_NONE     =  1<<0,
  NEOS_ESCAPE_HTML     =  1<<1,
  NEOS_ESCAPE_SCRIPT   =  1<<2,
  NEOS_ESCAPE_URL      =  1<<3,
  NEOS_ESCAPE_FUNCTION =  1<<4  /* Special case used to override the others */
} NEOS_ESCAPE;

NEOERR* neos_escape(UINT8 *buf, int buflen, char esc_char, const char *escape,
                    char **esc);
UINT8 *neos_unescape (UINT8 *s, int buflen, char esc_char);

char *repr_string_alloc (const char *s);

/* This is the "super" escape call which will call the proper helper
 * variable escape function based on the passed in context. */
NEOERR *neos_var_escape (NEOS_ESCAPE context,
                         const char *in,
                         char **esc);

/* Generic data escaping helper functions used by neos_contextual_escape
 * and cs built-ins. */
NEOERR *neos_url_escape (const char *in, char **esc,
                         const char *other);

NEOERR *neos_js_escape (const char *in, char **esc);

NEOERR *neos_html_escape (const char *src, int slen,
                          char **out);

NEOERR *neos_url_validate (const char *in, char **esc);

__END_DECLS

#endif /* incl_HPHP_NEO_STR_H_ */
