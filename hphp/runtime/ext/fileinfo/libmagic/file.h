/*
 * Copyright (c) Ian F. Darwin 1986-1995.
 * Software written by Ian F. Darwin and others;
 * maintained 1995-present by Christos Zoulas and others.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice immediately at the beginning of the file, without modification,
 *    this list of conditions, and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * file.h - definitions for file(1) program
 * @(#)$File: file.h,v 1.144 2013/02/18 15:40:59 christos Exp $
 */

#ifndef __file_h__
#define __file_h__

#include "hphp/runtime/ext/fileinfo/libmagic/compat.h"

#include "hphp/util/hphp-config.h"

#define SIZE_T_FORMAT "z"
#ifdef _MSC_VER
  #define INT64_T_FORMAT "I64"
#else
  #define INT64_T_FORMAT "ll"
#endif

#include <stdio.h>  /* Include that here, to make sure __P gets defined */
#include <errno.h>
#include <fcntl.h>  /* For open and flags */
#include <stdint.h>
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#include <sys/types.h>
#ifndef _MSC_VER
#include <sys/param.h>
#endif
/* Do this here and now, because struct stat gets re-defined on solaris */
#include <sys/stat.h>
#include <stdarg.h>

#define ENABLE_CONDITIONALS

#ifndef MAGIC
#define MAGIC "/etc/magic"
#endif

#if defined(__EMX__) || defined(_MSC_VER)
#define PATHSEP  ';'
#else
#define PATHSEP  ':'
#endif

#define private static

#if HAVE_VISIBILITY
#define public  __attribute__ ((__visibility__("default")))
#ifndef protected
#define protected __attribute__ ((__visibility__("hidden")))
#endif
#else
#define public
#ifndef protected
#define protected
#endif
#endif

#ifndef __arraycount
#define __arraycount(a) (sizeof(a) / sizeof(a[0]))
#endif

#ifndef __GNUC_PREREQ__
#ifdef __GNUC__
#define  __GNUC_PREREQ__(x, y)            \
  ((__GNUC__ == (x) && __GNUC_MINOR__ >= (y)) ||      \
   (__GNUC__ > (x)))
#else
#define  __GNUC_PREREQ__(x, y)  0
#endif
#endif

#ifndef MIN
#define  MIN(a,b)  (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define  MAX(a,b)  (((a) > (b)) ? (a) : (b))
#endif

#define OFFSET_OOB(n, o, i)    ((n) < (o) || (i) > ssize_t((n) - (o)))

#ifndef HOWMANY
# define HOWMANY (256 * 1024)  /* how much of the file to look at */
#endif
#define MAXMAGIS 8192    /* max entries in any one magic file
           or directory */
#define MAXDESC  64    /* max len of text description/MIME type */
#define MAXMIME  80    /* max len of text MIME type */
#define MAXstring 64    /* max len of "string" types */

#define MAGICNO    0xF11E041C
#define VERSIONNO  10
#define FILE_MAGICSIZE  248

#define  FILE_LOAD  0
#define FILE_CHECK  1
#define FILE_COMPILE  2
#define FILE_LIST  3

union VALUETYPE {
  uint8_t b;
  uint16_t h;
  uint32_t l;
  uint64_t q;
  uint8_t hs[2];  /* 2 bytes of a fixed-endian "short" */
  uint8_t hl[4];  /* 4 bytes of a fixed-endian "long" */
  uint8_t hq[8];  /* 8 bytes of a fixed-endian "quad" */
  char s[MAXstring];  /* the search string or regex pattern */
  unsigned char us[MAXstring];
  float f;
  double d;
};

struct magic {
  /* Word 1 */
  uint16_t cont_level;  /* level of ">" */
  uint8_t flag;
#define INDIR    0x01  /* if '(...)' appears */
#define OFFADD    0x02  /* if '>&' or '>...(&' appears */
#define INDIROFFADD  0x04  /* if '>&(' appears */
#define UNSIGNED  0x08  /* comparison is unsigned */
#define NOSPACE    0x10  /* suppress space character before output */
#define BINTEST    0x20  /* test is for a binary type (set only
           for top-level tests) */
#define TEXTTEST  0x40  /* for passing to file_softmagic */

  uint8_t factor;

  /* Word 2 */
  uint8_t reln;    /* relation (0=eq, '>'=gt, etc) */
  uint8_t vallen;    /* length of string value, if any */
  uint8_t type;    /* comparison type (FILE_*) */
  uint8_t in_type;  /* type of indirection */
#define       FILE_INVALID  0
#define       FILE_BYTE  1
#define        FILE_SHORT  2
#define        FILE_DEFAULT  3
#define        FILE_LONG  4
#define        FILE_STRING  5
#define        FILE_DATE  6
#define        FILE_BESHORT  7
#define        FILE_BELONG  8
#define        FILE_BEDATE  9
#define        FILE_LESHORT  10
#define        FILE_LELONG  11
#define        FILE_LEDATE  12
#define        FILE_PSTRING  13
#define        FILE_LDATE  14
#define        FILE_BELDATE  15
#define        FILE_LELDATE  16
#define        FILE_REGEX  17
#define        FILE_BESTRING16  18
#define        FILE_LESTRING16  19
#define        FILE_SEARCH  20
#define        FILE_MEDATE  21
#define        FILE_MELDATE  22
#define        FILE_MELONG  23
#define        FILE_QUAD  24
#define        FILE_LEQUAD  25
#define        FILE_BEQUAD  26
#define        FILE_QDATE  27
#define        FILE_LEQDATE  28
#define        FILE_BEQDATE  29
#define        FILE_QLDATE  30
#define        FILE_LEQLDATE  31
#define        FILE_BEQLDATE  32
#define        FILE_FLOAT  33
#define        FILE_BEFLOAT  34
#define        FILE_LEFLOAT  35
#define        FILE_DOUBLE  36
#define        FILE_BEDOUBLE  37
#define        FILE_LEDOUBLE  38
#define        FILE_BEID3  39
#define        FILE_LEID3  40
#define        FILE_INDIRECT  41
#define        FILE_QWDATE  42
#define        FILE_LEQWDATE  43
#define        FILE_BEQWDATE  44
#define        FILE_NAME  45
#define        FILE_USE  46
#define        FILE_NAMES_SIZE  47 /* size of array to contain all names */

#define IS_LIBMAGIC_STRING(t) \
  ((t) == FILE_STRING || \
   (t) == FILE_PSTRING || \
   (t) == FILE_BESTRING16 || \
   (t) == FILE_LESTRING16 || \
   (t) == FILE_REGEX || \
   (t) == FILE_SEARCH || \
   (t) == FILE_NAME || \
   (t) == FILE_USE || \
   (t) == FILE_DEFAULT)

#define FILE_FMT_NONE 0
#define FILE_FMT_NUM  1 /* "cduxXi" */
#define FILE_FMT_STR  2 /* "s" */
#define FILE_FMT_QUAD 3 /* "ll" */
#define FILE_FMT_FLOAT 4 /* "eEfFgG" */
#define FILE_FMT_DOUBLE 5 /* "eEfFgG" */

  /* Word 3 */
  uint8_t in_op;    /* operator for indirection */
  uint8_t mask_op;  /* operator for mask */
#ifdef ENABLE_CONDITIONALS
  uint8_t cond;    /* conditional type */
#else
  uint8_t dummy;
#endif
  uint8_t factor_op;
#define    FILE_FACTOR_OP_PLUS  '+'
#define    FILE_FACTOR_OP_MINUS  '-'
#define    FILE_FACTOR_OP_TIMES  '*'
#define    FILE_FACTOR_OP_DIV  '/'
#define    FILE_FACTOR_OP_NONE  '\0'

#define        FILE_OPS  "&|^+-*/%"
#define        FILE_OPAND  0
#define        FILE_OPOR  1
#define        FILE_OPXOR  2
#define        FILE_OPADD  3
#define        FILE_OPMINUS  4
#define        FILE_OPMULTIPLY  5
#define        FILE_OPDIVIDE  6
#define        FILE_OPMODULO  7
#define        FILE_OPS_MASK  0x07 /* mask for above ops */
#define        FILE_UNUSED_1  0x08
#define        FILE_UNUSED_2  0x10
#define        FILE_UNUSED_3  0x20
#define        FILE_OPINVERSE  0x40
#define        FILE_OPINDIRECT  0x80

#ifdef ENABLE_CONDITIONALS
#define        COND_NONE  0
#define        COND_IF    1
#define        COND_ELIF  2
#define        COND_ELSE  3
#endif /* ENABLE_CONDITIONALS */

  /* Word 4 */
  uint32_t offset;  /* offset to magic number */
  /* Word 5 */
  int32_t in_offset;  /* offset from indirection */
  /* Word 6 */
  uint32_t lineno;  /* line number in magic file */
  /* Word 7,8 */
  union {
    uint64_t _mask;  /* for use with numeric and date types */
    struct {
      uint32_t _count;  /* repeat/line count */
      uint32_t _flags;  /* modifier flags */
    } _s;    /* for use with string types */
  } _u;
#define num_mask _u._mask
#define str_range _u._s._count
#define str_flags _u._s._flags
  /* Words 9-16 */
  union VALUETYPE value;  /* either number or string */
  /* Words 17-32 */
  char desc[MAXDESC];  /* description */
  /* Words 33-52 */
  char mimetype[MAXMIME]; /* MIME type */
  /* Words 53-54 */
  char apple[8];
};

#define BIT(A)   (1 << (A))
#define STRING_COMPACT_WHITESPACE    BIT(0)
#define STRING_COMPACT_OPTIONAL_WHITESPACE  BIT(1)
#define STRING_IGNORE_LOWERCASE      BIT(2)
#define STRING_IGNORE_UPPERCASE      BIT(3)
#define REGEX_OFFSET_START      BIT(4)
#define STRING_TEXTTEST        BIT(5)
#define STRING_BINTEST        BIT(6)
#define PSTRING_1_BE        BIT(7)
#define PSTRING_1_LE        BIT(7)
#define PSTRING_2_BE        BIT(8)
#define PSTRING_2_LE        BIT(9)
#define PSTRING_4_BE        BIT(10)
#define PSTRING_4_LE        BIT(11)
#define PSTRING_LEN  \
    (PSTRING_1_BE|PSTRING_2_LE|PSTRING_2_BE|PSTRING_4_LE|PSTRING_4_BE)
#define PSTRING_LENGTH_INCLUDES_ITSELF    BIT(12)
#define STRING_TRIM        BIT(13)
#define CHAR_COMPACT_WHITESPACE      'W'
#define CHAR_COMPACT_OPTIONAL_WHITESPACE  'w'
#define CHAR_IGNORE_LOWERCASE      'c'
#define CHAR_IGNORE_UPPERCASE      'C'
#define CHAR_REGEX_OFFSET_START      's'
#define CHAR_TEXTTEST        't'
#define CHAR_TRIM        'T'
#define CHAR_BINTEST        'b'
#define CHAR_PSTRING_1_BE      'B'
#define CHAR_PSTRING_1_LE      'B'
#define CHAR_PSTRING_2_BE      'H'
#define CHAR_PSTRING_2_LE      'h'
#define CHAR_PSTRING_4_BE      'L'
#define CHAR_PSTRING_4_LE      'l'
#define CHAR_PSTRING_LENGTH_INCLUDES_ITSELF     'J'
#define STRING_IGNORE_CASE    (STRING_IGNORE_LOWERCASE|STRING_IGNORE_UPPERCASE)
#define STRING_DEFAULT_RANGE    100


/* list of magic entries */
struct mlist {
  struct magic *magic;    /* array of magic entries */
  uint32_t nmagic;      /* number of entries in array */
  void *map;      /* internal resources used by entry */
  struct mlist *next, *prev;
};

#ifdef __cplusplus
#define CAST(T, b)  static_cast<T>(b)
#define RCAST(T, b)  reinterpret_cast<T>(b)
#else
#define CAST(T, b)  (T)(b)
#define RCAST(T, b)  (T)(b)
#endif

struct level_info {
  int32_t off;
  int got_match;
#ifdef ENABLE_CONDITIONALS
  int last_match;
  int last_cond;  /* used for error checking by parse() */
#endif
};

#define MAGIC_SETS  2

struct magic_set {
  struct mlist *mlist[MAGIC_SETS];  /* list of regular entries */
  struct cont {
    size_t len;
    struct level_info *li;
  } c;
  struct out {
    char *buf;    /* Accumulation buffer */
    char *pbuf;    /* Printable buffer */
  } o;
  uint32_t offset;
  int error;
  int flags;      /* Control magic tests. */
  int event_flags;    /* Note things that happened. */
#define     EVENT_HAD_ERR    0x01
  const char *file;
  size_t line;      /* current magic line number */

  /* data for searches */
  struct {
    const char *s;    /* start of search in original source */
    size_t s_len;    /* length of search region */
    size_t offset;    /* starting offset in source: XXX - should this be off_t? */
    size_t rm_len;    /* match length */
  } search;

  /* FIXME: Make the string dynamically allocated so that e.g.
     strings matched in files can be longer than MAXstring */
  union VALUETYPE ms_value;  /* either number or string */
};

/* Type for Unicode characters */
using unichar = unsigned long;

struct stat;
#define FILE_T_LOCAL  1
#define FILE_T_WINDOWS  2
protected const char *file_fmttime(uint64_t, int, char *);
protected struct magic_set *file_ms_alloc(int);
protected void file_ms_free(struct magic_set *);
protected int file_buffer(struct magic_set *, php_stream *, const char *, const void *,
    size_t);
protected int file_fsmagic(struct magic_set *, const char *, struct stat *, php_stream *);
protected int file_pipe2file(struct magic_set *, int, const void *, size_t);
protected int file_replace(struct magic_set *, const char *, const char *);
protected int file_printf(struct magic_set *, const char *, ...);
protected int file_reset(struct magic_set *);
protected int file_tryelf(struct magic_set *, int, const unsigned char *,
    size_t);
protected int file_trycdf(struct magic_set *, int, const unsigned char *,
    size_t);
#ifdef PHP_FILEINFO_UNCOMPRESS
protected int file_zmagic(struct magic_set *, int, const char *,
    const unsigned char *, size_t);
#endif
protected int file_ascmagic(struct magic_set *, const unsigned char *, size_t,
    int);
protected int file_ascmagic_with_encoding(struct magic_set *,
    const unsigned char *, size_t, unichar *, size_t, const char *,
    const char *, int);
protected int file_encoding(struct magic_set *, const unsigned char *, size_t,
    unichar **, size_t *, const char **, const char **, const char **);
protected int file_is_tar(struct magic_set *, const unsigned char *, size_t);
protected int file_softmagic(struct magic_set *, const unsigned char *, size_t,
    size_t, int, int);
protected int file_apprentice(struct magic_set *, const char *, int);
protected int file_magicfind(struct magic_set *, const char *, struct mlist *);
protected uint64_t file_signextend(struct magic_set *, struct magic *,
    uint64_t);
protected void file_delmagic(struct magic *, int type, size_t entries);
protected void file_badread(struct magic_set *);
protected void file_badseek(struct magic_set *);
protected void file_oomem(struct magic_set *, size_t);
protected void file_error(struct magic_set *, int, const char *, ...);
protected void file_magerror(struct magic_set *, const char *, ...);
protected void file_magwarn(struct magic_set *, const char *, ...);
protected void file_showstr(FILE *, const char *, size_t);
protected size_t file_mbswidth(const char *);
protected const char *file_getbuffer(struct magic_set *);
protected ssize_t sread(int, void *, size_t, int);
protected int file_check_mem(struct magic_set *, unsigned int);
protected int file_looks_utf8(const unsigned char *, size_t, unichar *,
    size_t *);
protected size_t file_pstring_length_size(const struct magic *);
protected size_t file_pstring_get_length(const struct magic *, const char *);
protected size_t file_printedlen(const struct magic_set *ms);
#ifdef __EMX__
protected int file_os2_apptype(struct magic_set *, const char *, const void *,
    size_t);
#endif /* __EMX__ */

extern const char *file_names[];
extern const size_t file_nnames;

#ifndef HAVE_STRERROR
extern int sys_nerr;
extern const char* const sys_errlist[];
#define strerror(e) \
  (((e) >= 0 && (e) < sys_nerr) ? sys_errlist[(e)] : "Unknown error")
#endif

#ifndef HAVE_STRTOUL
#define strtoul(a, b, c)  strtol(a, b, c)
#endif

#ifndef strlcpy
size_t strlcpy(char *dst, const char *src, size_t siz);
#endif
#ifndef strlcat
size_t strlcat(char *dst, const char *src, size_t siz);
#endif
#ifndef HAVE_GETLINE
ssize_t getline(char **dst, size_t *len, FILE *fp);
ssize_t getdelim(char **dst, size_t *len, int delimiter, FILE *fp);
#endif

#if defined(HAVE_MMAP) && defined(HAVE_SYS_MMAN_H) && !defined(QUICK)
#define QUICK
#endif

#ifndef O_BINARY
#define O_BINARY  0
#endif

#ifndef __cplusplus
#if defined(__GNUC__) && (__GNUC__ >= 3)
#define FILE_RCSID(id) \
static const char rcsid[] __attribute__((__used__)) = id;
#else
#define FILE_RCSID(id) \
static const char *rcsid(const char *p) { \
  return rcsid(p = id); \
}
#endif
#else
#define FILE_RCSID(id)
#endif

#ifdef PHP_WIN32
#define FINFO_LSEEK_FUNC _lseek
#define FINFO_READ_FUNC _read
#else
#define FINFO_LSEEK_FUNC lseek
#define FINFO_READ_FUNC read
#endif

#endif /* __file_h__ */
