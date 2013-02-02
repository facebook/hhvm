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

#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <onigposix.h>
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

char *neos_rstrip (char *s)
{
  int n = strlen (s)-1;

  while (n >= 0 && isspace(s[n]))
  {
    s[n] = '\0';
    n--;
  }

  return s;
}

void neos_lower(char *s)
{
  while(*s != 0) {
    *s = tolower(*s);
    s++;
  }
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

NEOERR *string_set (NEOSTRING *str, const char *buf)
{
  str->len = 0;
  return nerr_pass (string_append (str, buf));
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

void string_array_init (NEOSTRING_ARRAY *arr)
{
  arr->entries = NULL;
  arr->count = 0;
  arr->max = 0;
}

NEOERR *string_array_split (ULIST **list, char *s, const char *sep,
                            int max)
{
  NEOERR *err;
  char *p, *n, *f;
  int sl;
  int x = 0;

  if (sep[0] == '\0')
    return nerr_raise (NERR_ASSERT, "separator must be at least one character");

  err = uListInit (list, 10, 0);
  if (err) return nerr_pass(err);

  sl = strlen(sep);
  p = (sl == 1) ? strchr (s, sep[0]) : strstr (s, sep);
  f = s;
  while (p != NULL)
  {
    if (x >= max) break;
    *p = '\0';
    n = strdup(f);
    *p = sep[0];
    if (n) err = uListAppend (*list, n);
    else err = nerr_raise(NERR_NOMEM,
	"Unable to allocate memory to split %s", s);
    if (err) goto split_err;
    f = p+sl;
    p = (sl == 1) ? strchr (f, sep[0]) : strstr (f, sep);
    x++;
  }
  /* Handle remainder */
  n = strdup(f);
  if (n) err = uListAppend (*list, n);
  else err = nerr_raise(NERR_NOMEM,
      "Unable to allocate memory to split %s", s);
  if (err) goto split_err;
  return STATUS_OK;

split_err:
  uListDestroy(list, ULIST_FREE);
  return err;
}

void string_array_clear (NEOSTRING_ARRAY *arr)
{
  int x;

  for (x = 0; x < arr->count; x++)
  {
    if (arr->entries[x] != NULL) free (arr->entries[x]);
    arr->entries[x] = NULL;
  }
  free (arr->entries);
  arr->entries = NULL;
  arr->count = 0;
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

/* technically, sprintf's can have null values, so we need to be able to
 * return a length also like real sprintf */
int isprintf_alloc (char **buf, const char *fmt, ...)
{
  va_list ap;
  int r;

  va_start (ap, fmt);
  r = visprintf_alloc (buf, fmt, ap);
  va_end (ap);
  return r;
}

char *sprintf_alloc (const char *fmt, ...)
{
  va_list ap;
  char *r;

  va_start (ap, fmt);
  r = vsprintf_alloc (fmt, ap);
  va_end (ap);
  return r;
}

/* This is mostly just here for completeness, I doubt anyone would use
 * this (its more efficient (time-wise) if start_size is bigger than the
 * resulting string.  Its less efficient than sprintf_alloc if we have a
 * C99 snprintf and it doesn't fit in start_size.
 * BTW: If you are really worried about the efficiency of these
 * functions, maybe you shouldn't be using them in the first place... */
char *nsprintf_alloc (int start_size, const char *fmt, ...)
{
  va_list ap;
  char *r;

  va_start (ap, fmt);
  r = vnsprintf_alloc (start_size, fmt, ap);
  va_end (ap);
  return r;
}

BOOL reg_search (const char *re, const char *str)
{
  regex_t search_re;
  int errcode;
  char buf[256];

  if ((errcode = regcomp(&search_re, re, REG_ICASE | REG_EXTENDED | REG_NOSUB)))
  {
    regerror (errcode, &search_re, buf, sizeof(buf));
    ne_warn ("Unable to compile regex %s: %s", re, buf);
    return FALSE;
  }
  errcode = regexec (&search_re, str, 0, NULL, 0);
  regfree (&search_re);
  if (errcode == 0)
    return TRUE;
  return FALSE;
}

NEOERR *string_readline (NEOSTRING *str, FILE *fp)
{
  NEOERR *err;

  /* minimum size for a readline is 256 above current position */
  err = string_check_length (str, str->len + 256);
  if (err != STATUS_OK) return nerr_pass (err);

  while (fgets(str->buf + str->len, str->max - str->len, fp) != NULL)
  {
    str->len = strlen(str->buf);
    if (str->buf[str->len-1] == '\n') break;
    err = string_check_length (str, str->len + 256);
    if (err != STATUS_OK) return nerr_pass (err);
  }
  return STATUS_OK;
}

NEOERR* neos_escape(UINT8 *buf, int buflen, char esc_char, const char *escape,
                    char **esc)
{
  int nl = 0;
  int l = 0;
  int x = 0;
  char *s;
  int match = 0;

  while (l < buflen)
  {
    if (buf[l] == esc_char)
    {
      nl += 2;
    }
    else
    {
      x = 0;
      while (escape[x])
      {
	if (escape[x] == buf[l])
	{
	  nl +=2;
	  break;
	}
	x++;
      }
    }
    nl++;
    l++;
  }

  s = (char *) malloc (sizeof(char) * (nl + 1));
  if (s == NULL)
    return nerr_raise (NERR_NOMEM, "Unable to allocate memory to escape %s",
	buf);

  nl = 0; l = 0;
  while (l < buflen)
  {
    match = 0;
    if (buf[l] == esc_char)
    {
      match = 1;
    }
    else
    {
      x = 0;
      while (escape[x])
      {
	if (escape[x] == buf[l])
	{
	  match = 1;
	  break;
	}
	x++;
      }
    }
    if (match)
    {
      s[nl++] = esc_char;
      s[nl++] = "0123456789ABCDEF"[buf[l] / 16];
      s[nl++] = "0123456789ABCDEF"[buf[l] % 16];
      l++;
    }
    else
    {
      s[nl++] = buf[l++];
    }
  }
  s[nl] = '\0';

  *esc = s;
  return STATUS_OK;
}

UINT8 *neos_unescape (UINT8 *s, int buflen, char esc_char)
{
  int i = 0, o = 0;

  if (s == NULL) return s;
  while (i < buflen)
  {
    if (s[i] == esc_char && (i+2 < buflen) &&
	isxdigit(s[i+1]) && isxdigit(s[i+2]))
    {
      UINT8 num;
      num = (s[i+1] >= 'A') ? ((s[i+1] & 0xdf) - 'A') + 10 : (s[i+1] - '0');
      num *= 16;
      num += (s[i+2] >= 'A') ? ((s[i+2] & 0xdf) - 'A') + 10 : (s[i+2] - '0');
      s[o++] = num;
      i+=3;
    }
    else {
      s[o++] = s[i++];
    }
  }
  if (i && o) s[o] = '\0';
  return s;
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

/* List of all characters that must be escaped
 * List based on http://www.blooberry.com/indexdot/html/topics/urlencoding.htm
 */
static char EscapedChars[] = "$&+,/:;=?@ \"<>#%{}|\\^~[]`'";

/* Check if a single character needs to be escaped */
static BOOL is_reserved_char(char c)
{
  int i = 0;

  if (c < 32 || c > 122) {
    return TRUE;
  } else {
    while (EscapedChars[i]) {
      if (c == EscapedChars[i]) {
        return TRUE;
      }
      ++i;
    }
  }
  return FALSE;
}

NEOERR *neos_js_escape (const char *in, char **esc)
{
  int nl = 0;
  int l = 0;
  unsigned char *buf = (unsigned char *)in;
  unsigned char *s;

  while (buf[l])
  {
    if (buf[l] == '/' || buf[l] == '"' || buf[l] == '\'' ||
        buf[l] == '\\' || buf[l] == '>' || buf[l] == '<' ||
        buf[l] == '&' || buf[l] == ';' || buf[l] < 32)
    {
      nl += 3;
    }
    nl++;
    l++;
  }

  s = (unsigned char *) malloc (sizeof(unsigned char) * (nl + 1));
  if (s == NULL)
    return nerr_raise (NERR_NOMEM, "Unable to allocate memory to escape %s",
        buf);

  nl = 0; l = 0;
  while (buf[l])
  {
    if (buf[l] == '/' || buf[l] == '"' || buf[l] == '\'' ||
        buf[l] == '\\' || buf[l] == '>' || buf[l] == '<' ||
        buf[l] == '&' || buf[l] == ';' || buf[l] < 32)
    {
      s[nl++] = '\\';
      s[nl++] = 'x';
      s[nl++] = "0123456789ABCDEF"[(buf[l] >> 4) & 0xF];
      s[nl++] = "0123456789ABCDEF"[buf[l] & 0xF];
      l++;
    }
    else
    {
      s[nl++] = buf[l++];
    }
  }
  s[nl] = '\0';

  *esc = (char *)s;
  return STATUS_OK;
}


NEOERR *neos_url_escape (const char *in, char **esc,
                         const char *other)
{
  int nl = 0;
  int l = 0;
  int x = 0;
  unsigned char *buf = (unsigned char *)in;
  unsigned char *uother = (unsigned char *)other;
  unsigned char *s;
  int match = 0;

  while (buf[l])
  {
    if (is_reserved_char(buf[l]))
    {
      nl += 2;
    }
    else if (uother)
    {
      x = 0;
      while (uother[x])
      {
        if (uother[x] == buf[l])
        {
          nl +=2;
          break;
        }
        x++;
      }
    }
    nl++;
    l++;
  }

  s = (unsigned char *) malloc (sizeof(unsigned char) * (nl + 1));
  if (s == NULL)
    return nerr_raise (NERR_NOMEM, "Unable to allocate memory to escape %s",
      buf);

  nl = 0; l = 0;
  while (buf[l])
  {
    match = 0;
    if (buf[l] == ' ')
    {
      s[nl++] = '+';
      l++;
    }
    else
    {
      if (is_reserved_char(buf[l]))
      {
        match = 1;
      }
      else if (uother)
      {
        x = 0;
        while (uother[x])
        {
          if (uother[x] == buf[l])
          {
            match = 1;
            break;
          }
          x++;
        }
      }
      if (match)
      {
        s[nl++] = '%';
        s[nl++] = "0123456789ABCDEF"[buf[l] / 16];
        s[nl++] = "0123456789ABCDEF"[buf[l] % 16];
        l++;
      }
      else
      {
        s[nl++] = buf[l++];
      }
    }
  }
  s[nl] = '\0';

  *esc = (char *)s;
  return STATUS_OK;
}

NEOERR *neos_html_escape (const char *src, int slen,
                          char **out)
{
  NEOERR *err = STATUS_OK;
  NEOSTRING out_s;
  int x;
  char *ptr;

  string_init(&out_s);
  err = string_append (&out_s, "");
  if (err) return nerr_pass (err);
  *out = NULL;

  x = 0;
  while (x < slen)
  {
    ptr = strpbrk(src + x, "&<>\"'\r");
    if (ptr == NULL || (ptr-src >= slen))
    {
      err = string_appendn (&out_s, src + x, slen-x);
      x = slen;
    }
    else
    {
      err = string_appendn (&out_s, src + x, (ptr - src) - x);
      if (err != STATUS_OK) break;
      x = ptr - src;
      if (src[x] == '&')
        err = string_append (&out_s, "&amp;");
      else if (src[x] == '<')
        err = string_append (&out_s, "&lt;");
      else if (src[x] == '>')
        err = string_append (&out_s, "&gt;");
      else if (src[x] == '"')
        err = string_append (&out_s, "&quot;");
      else if (src[x] == '\'')
        err = string_append (&out_s, "&#39;");
      else if (src[x] != '\r')
        err = nerr_raise (NERR_ASSERT, "src[x] == '%c'", src[x]);
      x++;
    }
    if (err != STATUS_OK) break;
  }
  if (err)
  {
    string_clear (&out_s);
    return nerr_pass (err);
  }
  *out = out_s.buf;
  return STATUS_OK;
}

char *URL_PROTOCOLS[] = {"http://", "https://", "ftp://", "mailto:"};

NEOERR *neos_url_validate (const char *in, char **esc)
{
  NEOERR *err = STATUS_OK;
  NEOSTRING out_s;
  int valid = 0;
  size_t i;
  size_t inlen;
  int num_protocols = sizeof(URL_PROTOCOLS) / sizeof(char*);
  void* slashpos;
  void* colonpos;

  inlen = strlen(in);

  /*
   * <a href="//b:80"> or <a href="a/b:80"> are allowed by browsers
   * and ":" is treated as part of the path, while
   * <a href="www.google.com:80"> is an invalid url
   * and ":" is treated as a scheme separator.
   *
   * Hence allow for ":" in the path part of a url (after /)
   */
  slashpos = memchr(in, '/', inlen);
  if (slashpos == NULL) {
    i = inlen;
  }
  else {
    i = (size_t)((char*)slashpos - in);
  }

  colonpos = memchr(in, ':', i);

  if (colonpos == NULL) {
    /* no scheme in 'in': so this is a relative url */
    valid = 1;
  }
  else {
    for (i = 0; i < num_protocols; i++)
    {
      if ((inlen >= strlen(URL_PROTOCOLS[i])) &&
          strncasecmp(in, URL_PROTOCOLS[i], strlen(URL_PROTOCOLS[i])) == 0) {
        /* 'in' starts with one of the allowed protocols */
        valid = 1;
        break;
      }

    }
  }

  if (valid)
    return neos_html_escape(in, inlen, esc);

  /* 'in' contains an unsupported scheme, replace with '#' */
  string_init(&out_s);
  err = string_append (&out_s, "#");
  if (err) return nerr_pass (err);

  *esc = out_s.buf;
  return STATUS_OK;

}

NEOERR *neos_var_escape (NEOS_ESCAPE context,
                         const char *in,
                         char **esc)
{

  /* Just dup and return if we do nothing. */
  if (context == NEOS_ESCAPE_NONE ||
      context == NEOS_ESCAPE_FUNCTION)
  {
    *esc = strdup(in);
    return STATUS_OK;
  }

  /* Now we escape based on context. This is the order of precedence:
   * url > script > style > html
   */
  if (context & NEOS_ESCAPE_URL)
    return nerr_pass(neos_url_escape(in, esc, NULL));
  else if (context & NEOS_ESCAPE_SCRIPT)
    return nerr_pass(neos_js_escape(in, esc));
  else if (context & NEOS_ESCAPE_HTML)
    return nerr_pass(neos_html_escape(in, strlen(in), esc));

  return nerr_raise(NERR_ASSERT, "unknown escape context supplied: %d",
    context);
}
