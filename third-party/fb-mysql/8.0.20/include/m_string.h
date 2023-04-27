/*
   Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef M_STRING_INCLUDED
#define M_STRING_INCLUDED

/**
  @file include/m_string.h
*/

#include <float.h>
#include <limits.h>
#include <stdbool.h>  // IWYU pragma: keep
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>

#include "decimal.h"
#include "lex_string.h"
#include "my_config.h"
#include "my_inttypes.h"
#include "my_macros.h"

/**
  Definition of the null string (a null pointer of type char *),
  used in some of our string handling code. New code should use
  nullptr instead.
*/
#define NullS (char *)0

/*
  my_str_malloc(), my_str_realloc() and my_str_free() are assigned to
  implementations in strings/alloc.cc, but can be overridden in
  the calling program.
 */
extern void *(*my_str_malloc)(size_t);
extern void *(*my_str_realloc)(void *, size_t);
extern void (*my_str_free)(void *);

/* Declared in int2str.cc. */
extern const char _dig_vec_upper[];
extern const char _dig_vec_lower[];

/* Prototypes for string functions */

extern char *strmake(char *dst, const char *src, size_t length);
extern char *strcont(char *src, const char *set);
extern char *strxmov(char *dst, const char *src, ...);
extern char *strxnmov(char *dst, size_t len, const char *src, ...);

/*
  bchange(dst, old_length, src, new_length, tot_length)
  replaces old_length characters at dst to new_length characters from
  src in a buffer with tot_length bytes.
*/
static inline void bchange(uchar *dst, size_t old_length, const uchar *src,
                           size_t new_length, size_t tot_length) {
  memmove(dst + new_length, dst + old_length, tot_length - old_length);
  memcpy(dst, src, new_length);
}

/*
  strend(s) returns a character pointer to the NUL which ends s.  That
  is,  strend(s)-s  ==  strlen(s). This is useful for adding things at
  the end of strings.  It is redundant, because  strchr(s,'\0')  could
  be used instead, but this is clearer and faster.
*/
static inline const char *strend(const char *s) {
  while (*s++)
    ;
  return s - 1;
}

static inline char *strend(char *s) {
  while (*s++)
    ;
  return s - 1;
}

/*
  strcend(s, c) returns a pointer to the  first  place  in  s where  c
  occurs,  or a pointer to the end-null of s if c does not occur in s.
*/
static inline const char *strcend(const char *s, char c) {
  for (;;) {
    if (*s == c) return s;
    if (!*s++) return s - 1;
  }
}

/*
  strfill(dest, len, fill) makes a string of fill-characters. The result
  string is of length == len. The des+len character is allways set to NULL.
  strfill() returns pointer to dest+len;
*/
static inline char *strfill(char *s, size_t len, char fill) {
  while (len--) *s++ = fill;
  *(s) = '\0';
  return (s);
}

/*
  my_stpmov(dst, src) moves all the  characters  of  src  (including  the
  closing NUL) to dst, and returns a pointer to the new closing NUL in
  dst.	 The similar UNIX routine strcpy returns the old value of dst,
  which I have never found useful.  my_stpmov(my_stpmov(dst,a),b) moves a//b
  into dst, which seems useful.
*/
static inline char *my_stpmov(char *dst, const char *src) {
  while ((*dst++ = *src++))
    ;
  return dst - 1;
}

/*
  my_stpnmov(dst,src,length) moves length characters, or until end, of src to
  dst and appends a closing NUL to dst if src is shorter than length.
  The result is a pointer to the first NUL in dst, or is dst+n if dst was
  truncated.
*/
static inline char *my_stpnmov(char *dst, const char *src, size_t n) {
  while (n-- != 0) {
    if (!(*dst++ = *src++)) return (char *)dst - 1;
  }
  return dst;
}

/**
   Copy a string from src to dst until (and including) terminating null byte.

   @param dst   Destination
   @param src   Source

   @note src and dst cannot overlap.
         Use my_stpmov() if src and dst overlaps.

   @note Unsafe, consider using my_stpnpy() instead.

   @return pointer to terminating null byte.
*/
static inline char *my_stpcpy(char *dst, const char *src) {
#if defined(HAVE_BUILTIN_STPCPY)
  /*
    If __builtin_stpcpy() is available, use it instead of stpcpy(), since GCC in
    some situations is able to transform __builtin_stpcpy() into more efficient
    strcpy() or memcpy() calls. It does not perform these transformations for a
    plain call to stpcpy() when the compiler runs in strict mode. See GCC bug
    82429.
  */
  return __builtin_stpcpy(dst, src);
#elif defined(HAVE_STPCPY)
  return stpcpy(dst, src);
#else
  /* Fallback to implementation supporting overlap. */
  return my_stpmov(dst, src);
#endif
}

/**
   Copy fixed-size string from src to dst.

   @param dst   Destination
   @param src   Source
   @param n     Maximum number of characters to copy.

   @note src and dst cannot overlap
         Use my_stpnmov() if src and dst overlaps.

   @return pointer to terminating null byte.
*/
static inline char *my_stpncpy(char *dst, const char *src, size_t n) {
#if defined(HAVE_STPNCPY)
  return stpncpy(dst, src, n);
#else
  /* Fallback to implementation supporting overlap. */
  return my_stpnmov(dst, src, n);
#endif
}

static inline longlong my_strtoll(const char *nptr, char **endptr, int base) {
#if defined _WIN32
  return _strtoi64(nptr, endptr, base);
#else
  return strtoll(nptr, endptr, base);
#endif
}

static inline ulonglong my_strtoull(const char *nptr, char **endptr, int base) {
#if defined _WIN32
  return _strtoui64(nptr, endptr, base);
#else
  return strtoull(nptr, endptr, base);
#endif
}

static inline char *my_strtok_r(char *str, const char *delim, char **saveptr) {
#if defined _WIN32
  return strtok_s(str, delim, saveptr);
#else
  return strtok_r(str, delim, saveptr);
#endif
}

/* native_ rather than my_ since my_strcasecmp already exists */
static inline int native_strcasecmp(const char *s1, const char *s2) {
#if defined _WIN32
  return _stricmp(s1, s2);
#else
  return strcasecmp(s1, s2);
#endif
}

/* native_ rather than my_ for consistency with native_strcasecmp */
static inline int native_strncasecmp(const char *s1, const char *s2, size_t n) {
#if defined _WIN32
  return _strnicmp(s1, s2, n);
#else
  return strncasecmp(s1, s2, n);
#endif
}

/*
  is_prefix(s, t) returns 1 if s starts with t.
  A empty t is always a prefix.
*/
static inline int is_prefix(const char *s, const char *t) {
  while (*t)
    if (*s++ != *t++) return 0;
  return 1; /* WRONG */
}

/* Conversion routines */
typedef enum { MY_GCVT_ARG_FLOAT, MY_GCVT_ARG_DOUBLE } my_gcvt_arg_type;

double my_strtod(const char *str, const char **end, int *error);
double my_atof(const char *nptr);
size_t my_fcvt(double x, int precision, char *to, bool *error);
size_t my_fcvt_compact(double x, char *to, bool *error);
size_t my_gcvt(double x, my_gcvt_arg_type type, int width, char *to,
               bool *error);

/*
  The longest string my_fcvt can return is 311 + "precision" bytes.
  Here we assume that we never call my_fcvt() with precision >=
  DECIMAL_NOT_SPECIFIED
  (+ 1 byte for the terminating '\0').
*/
static constexpr int FLOATING_POINT_BUFFER{311 + DECIMAL_NOT_SPECIFIED};

/*
  We want to use the 'e' format in some cases even if we have enough space
  for the 'f' one just to mimic sprintf("%.15g") behavior for large integers,
  and to improve it for numbers < 10^(-4).
  That is, for |x| < 1 we require |x| >= 10^(-15), and for |x| > 1 we require
  it to be integer and be <= 10^DBL_DIG for the 'f' format to be used.
  We don't lose precision, but make cases like "1e200" or "0.00001" look nicer.
*/
#define MAX_DECPT_FOR_F_FORMAT DBL_DIG

/*
  The maximum possible field width for my_gcvt() conversion.
  (DBL_DIG + 2) significant digits + sign + "." + ("e-NNN" or
  MAX_DECPT_FOR_F_FORMAT zeros for cases when |x|<1 and the 'f' format is used).
*/
#define MY_GCVT_MAX_FIELD_WIDTH \
  (DBL_DIG + 4 + std::max(5, MAX_DECPT_FOR_F_FORMAT))

extern bool fast_integer_to_string;
const char *str2int(const char *src, int radix, long lower, long upper,
                    long *val);
longlong my_strtoll10(const char *nptr, const char **endptr, int *error);
char *ll2str(int64_t val, char *dst, int radix, bool upcase);
char *longlong10_to_str(int64_t val, char *dst, int radix);

inline char *longlong2str(int64_t val, char *dst, int radix) {
  return ll2str(val, dst, radix, true);
}

/*
  This function saves a longlong value in a buffer and returns the pointer to
  the buffer.
*/
static inline char *llstr(longlong value, char *buff) {
  longlong10_to_str(value, buff, -10);
  return buff;
}

static inline char *ullstr(longlong value, char *buff) {
  longlong10_to_str(value, buff, 10);
  return buff;
}

#define STRING_WITH_LEN(X) (X), ((sizeof(X) - 1))

/**
  Skip trailing space (ASCII spaces only).

  @return New end of the string.
*/
static inline const uchar *skip_trailing_space(const uchar *ptr, size_t len) {
  const uchar *end = ptr + len;
  while (end - ptr >= 8) {
    uint64_t chunk;
    memcpy(&chunk, end - 8, sizeof(chunk));
    if (chunk != 0x2020202020202020ULL) break;
    end -= 8;
  }
  while (end > ptr && end[-1] == 0x20) end--;
  return (end);
}

/*
  Format a double (representing number of bytes) into a human-readable string.

  @param buf     Buffer used for printing
  @param buf_len Length of buffer
  @param dbl_val Value to be formatted

  @note
    Sample output format: 42 1K 234M 2G
    If we exceed ULLONG_MAX YiB we give up, and convert to "+INF".

  @todo Consider writing KiB GiB etc, since we use 1024 rather than 1000
 */
static inline void human_readable_num_bytes(char *buf, int buf_len,
                                            double dbl_val) {
  const char size[] = {'\0', 'K', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y'};
  unsigned int i;
  for (i = 0; dbl_val > 1024 && i < sizeof(size) - 1; i++) dbl_val /= 1024;
  const char mult = size[i];
  // ULLONG_MAX (18446744073709551615) should be enough for most ...
  // static_cast<double>(ULLONG_MAX) is equal 18446744073709551616.0
  if (dbl_val >= static_cast<double>(ULLONG_MAX))
    snprintf(buf, buf_len, "+INF");
  else
    snprintf(buf, buf_len, "%llu%c", (unsigned long long)dbl_val, mult);
}

static inline void lex_string_set(LEX_STRING *lex_str, char *c_str) {
  lex_str->str = c_str;
  lex_str->length = strlen(c_str);
}

static inline void lex_cstring_set(LEX_CSTRING *lex_str, const char *c_str) {
  lex_str->str = c_str;
  lex_str->length = strlen(c_str);
}

#endif  // M_STRING_INCLUDED
