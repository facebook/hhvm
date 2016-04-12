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

#include "cs_config.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif

#include "neo_bool.h"
#include "neo_misc.h"
#include "neo_err.h"
#include "neo_str.h"
#include "ulist.h"

#ifndef va_copy
#ifdef __va_copy
# define va_copy(dest,src) __va_copy(dest,src)
#else
# define va_copy(dest,src) ((dest) = (src))
#endif
#endif

char *neos_strip (char *s)
{
  int x;

  x = strlen(s) - 1;
  while (x>=0 && isspace(s[x])) s[x--] = '\0';

  while (*s && isspace(*s)) s++;

  return s;
}


void string_init (NEOSTRING *str)
{
  str->buf = NULL;
  str->len = 0;
  str->max = 0;
}

void string_clear (NEOSTRING *str)
{
  if (str->buf != NULL)
    free(str->buf);
  string_init(str);
}

static NEOERR* string_check_length (NEOSTRING *str, int l)
{
  if (str->buf == NULL)
  {
    if (l * 10 > 256)
      str->max = l * 10;
    else
      str->max = 256;
    str->buf = (char *) malloc (sizeof(char) * str->max);
    if (str->buf == NULL)
      return nerr_raise (NERR_NOMEM, "Unable to allocate render buf of size %d",
	  str->max);
   /*  ne_warn("Creating string %x at %d (%5.2fK)", str, str->max, (str->max / 1024.0)); */
  }
  else if (str->len + l >= str->max)
  {
    void *new_ptr;
    int new_max = str->max;
    do
    {
      new_max *= 2;
    } while (str->len + l >= new_max);
    new_ptr = realloc (str->buf, sizeof(char) * new_max);
    if (new_ptr == NULL) {
      return nerr_raise (NERR_NOMEM, "Unable to allocate NEOSTRING buf of size %d",
	                       new_max);
    }
    str->buf = (char *) new_ptr;
    str->max = new_max;
    /* ne_warn("Growing string %x to %d (%5.2fK)", str, str->max, (str->max / 1024.0)); */
  }
  return STATUS_OK;
}

NEOERR *string_append (NEOSTRING *str, const char *buf)
{
  NEOERR *err;
  int l;

  l = strlen(buf);
  err = string_check_length (str, l);
  if (err != STATUS_OK) return nerr_pass (err);
  strcpy(str->buf + str->len, buf);
  str->len += l;

  return STATUS_OK;
}

NEOERR *string_appendn (NEOSTRING *str, const char *buf, int l)
{
  NEOERR *err;

  err = string_check_length (str, l+1);
  if (err != STATUS_OK) return nerr_pass (err);
  memcpy(str->buf + str->len, buf, l);
  str->len += l;
  str->buf[str->len] = '\0';

  return STATUS_OK;
}

/* this is much more efficient with C99 snprintfs... */
NEOERR *string_appendvf (NEOSTRING *str, const char *fmt, va_list ap)
{
  NEOERR *err;
  char buf[4096];
  int bl, size;
  va_list tmp;

  va_copy(tmp, ap);
  /* determine length */
  size = sizeof (buf);
  bl = vsnprintf (buf, size, fmt, tmp);
  if (bl > -1 && bl < size)
    return string_appendn (str, buf, bl);

  /* Handle non-C99 snprintfs (requires extra malloc/free and copy) */
  if (bl == -1)
  {
    char *a_buf;

    va_copy(tmp, ap);
    a_buf = vnsprintf_alloc(size*2, fmt, tmp);
    if (a_buf == NULL)
      return nerr_raise(NERR_NOMEM,
	  "Unable to allocate memory for formatted string");
    err = string_append(str, a_buf);
    free(a_buf);
    return nerr_pass(err);
  }

  err = string_check_length (str, bl+1);
  if (err != STATUS_OK) return nerr_pass (err);
  va_copy(tmp, ap);
  vsprintf (str->buf + str->len, fmt, tmp);
  str->len += bl;
  str->buf[str->len] = '\0';

  return STATUS_OK;
}

NEOERR *string_appendf (NEOSTRING *str, const char *fmt, ...)
{
  NEOERR *err;
  va_list ap;

  va_start (ap, fmt);
  err = string_appendvf (str, fmt, ap);
  va_end (ap);
  return nerr_pass(err);
}

NEOERR *string_append_char (NEOSTRING *str, char c)
{
  NEOERR *err;
  err = string_check_length (str, 1);
  if (err != STATUS_OK) return nerr_pass (err);
  str->buf[str->len] = c;
  str->buf[str->len + 1] = '\0';
  str->len += 1;

  return STATUS_OK;
}

/* Mostly used by vprintf_alloc for non-C99 compliant snprintfs,
 * this is like vsprintf_alloc except it takes a "suggested" size */
int vnisprintf_alloc (char **buf, int start_size, const char *fmt, va_list ap)
{
  int bl, size;
  va_list tmp;

  *buf = NULL;
  size = start_size;

  *buf = (char *) malloc (size * sizeof(char));
  if (*buf == NULL) return 0;
  while (1)
  {
    void *new_ptr;
    va_copy(tmp, ap);
    bl = vsnprintf (*buf, size, fmt, tmp);
    if (bl > -1 && bl < size)
      return bl;
    if (bl > -1)
      size = bl + 1;
    else
      size *= 2;
    new_ptr = realloc (*buf, size * sizeof(char));
    if (new_ptr == NULL) {
      free(*buf);
      *buf = NULL;
      return 0;
    }
    *buf = (char *) new_ptr;
  }
}

char *vnsprintf_alloc (int start_size, const char *fmt, va_list ap)
{
  char *r;
  vnisprintf_alloc(&r, start_size, fmt, ap);
  return r;
}

/* This works better with a C99 compliant vsnprintf, but should work ok
 * with versions that return a -1 if it overflows the buffer */
int visprintf_alloc (char **buf, const char *fmt, va_list ap)
{
  char ibuf[4096];
  int bl, size;
  va_list tmp;

/* PPC doesn't like you re-using a va_list... and it might not be
 * supposed to work at all */
  va_copy(tmp, ap);

  size = sizeof (ibuf);
  bl = vsnprintf (ibuf, sizeof (ibuf), fmt, tmp);
  if (bl > -1 && bl < size)
  {
    *buf = (char *) calloc(bl+1, sizeof(char));
    if (*buf == NULL) return 0;
    strncpy(*buf, ibuf, bl);
    return bl;
  }

  if (bl > -1)
    size = bl + 1;
  else
    size *= 2;

  return vnisprintf_alloc(buf, size, fmt, ap);
}

char *vsprintf_alloc (const char *fmt, va_list ap)
{
  char *r;
  visprintf_alloc(&r, fmt, ap);
  return r;
}

char *repr_string_alloc (const char *s)
{
  int l,x,i;
  int nl = 0;
  char *rs;

  if (s == NULL)
  {
    return strdup("NULL");
  }

  l = strlen(s);
  for (x = 0; x < l; x++)
  {
    if (isprint(s[x]) && s[x] != '"' && s[x] != '\\')
    {
      nl++;
    }
    else
    {
      if (s[x] == '\n' || s[x] == '\t' || s[x] == '\r' || s[x] == '"' ||
	  s[x] == '\\')
      {
	nl += 2;
      }
      else nl += 4;
    }
  }

  rs = (char *) malloc ((nl+3) * sizeof(char));
  if (rs == NULL)
    return NULL;

  i = 0;
  rs[i++] = '"';
  for (x = 0; x < l; x++)
  {
    if (isprint(s[x]) && s[x] != '"' && s[x] != '\\')
    {
      rs[i++] = s[x];
    }
    else
    {
      rs[i++] = '\\';
      switch (s[x])
      {
	case '\n':
	  rs[i++] = 'n';
	  break;
	case '\t':
	  rs[i++] = 't';
	  break;
	case '\r':
	  rs[i++] = 'r';
	  break;
	case '"':
	  rs[i++] = '"';
	  break;
	case '\\':
	  rs[i++] = '\\';
	  break;
	default:
	  sprintf(&(rs[i]), "%03o", (s[x] & 0377));
	  i += 3;
	  break;
      }
    }
  }
  rs[i++] = '"';
  rs[i] = '\0';
  return rs;
}
