/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) 1998-2013 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   |          Dmitry Stogov <dmitry@zend.com>                             |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#include "zend.h"
#include "zend_alloc.h"
#include "zend_globals.h"
#include "zend_operators.h"

#ifdef HAVE_SIGNAL_H
# include <signal.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef ZEND_WIN32
# include <wincrypt.h>
# include <process.h>
#endif

#ifndef ZEND_MM_HEAP_PROTECTION
# define ZEND_MM_HEAP_PROTECTION ZEND_DEBUG
#endif

#ifndef ZEND_MM_SAFE_UNLINKING
# define ZEND_MM_SAFE_UNLINKING 1
#endif

#ifndef ZEND_MM_COOKIES
# define ZEND_MM_COOKIES ZEND_DEBUG
#endif

#ifdef _WIN64
# define PTR_FMT "0x%0.16I64x"
/*
#elif sizeof(long) == 8
# define PTR_FMT "0x%0.16lx"
*/
#else
# define PTR_FMT "0x%0.8lx"
#endif

ZEND_API char *zend_strndup(const char *s, uint length)
{
  char *p;
#ifdef ZEND_SIGNALS
  TSRMLS_FETCH();
#endif

  p = (char *) malloc(length+1);
  if (UNEXPECTED(p == NULL)) {
    return p;
  }
  if (length) {
    memcpy(p, s, length);
  }
  p[length] = 0;
  return p;
}

#if defined(__GNUC__) && (defined(__native_client__) || defined(i386))

static inline size_t safe_address(size_t nmemb, size_t size, size_t offset)
{
  size_t res = nmemb;
  unsigned long overflow = 0;

  __asm__ ("mull %3\n\taddl %4,%0\n\tadcl $0,%1"
       : "=&a"(res), "=&d" (overflow)
       : "%0"(res),
         "rm"(size),
         "rm"(offset));

  if (UNEXPECTED(overflow)) {
    zend_error_noreturn(E_ERROR, "Possible integer overflow in memory allocation (%zu * %zu + %zu)", nmemb, size, offset);
    return 0;
  }
  return res;
}

#elif defined(__GNUC__) && defined(__x86_64__)

static inline size_t safe_address(size_t nmemb, size_t size, size_t offset)
{
        size_t res = nmemb;
        unsigned long overflow = 0;

#ifdef __ILP32__ /* x32 */
# define LP_SUFF "l"
#else /* amd64 */
# define LP_SUFF "q"
#endif

        __asm__ ("mul" LP_SUFF  " %3\n\t"
                 "add %4,%0\n\t"
                 "adc $0,%1"
             : "=&a"(res), "=&d" (overflow)
             : "%0"(res),
               "rm"(size),
               "rm"(offset));

#undef LP_SUFF
        if (UNEXPECTED(overflow)) {
                zend_error_noreturn(E_ERROR, "Possible integer overflow in memory allocation (%zu * %zu + %zu)", nmemb, size, offset);
                return 0;
        }
        return res;
}

#elif defined(__GNUC__) && defined(__arm__)

static inline size_t safe_address(size_t nmemb, size_t size, size_t offset)
{
        size_t res;
        unsigned long overflow;

        __asm__ ("umlal %0,%1,%2,%3"
             : "=r"(res), "=r"(overflow)
             : "r"(nmemb),
               "r"(size),
               "0"(offset),
               "1"(0));

        if (UNEXPECTED(overflow)) {
                zend_error_noreturn(E_ERROR, "Possible integer overflow in memory allocation (%zu * %zu + %zu)", nmemb, size, offset);
                return 0;
        }
        return res;
}

#elif defined(__GNUC__) && defined(__aarch64__)

static inline size_t safe_address(size_t nmemb, size_t size, size_t offset)
{
        size_t res;
        unsigned long overflow;

        __asm__ ("mul %0,%2,%3\n\tumulh %1,%2,%3\n\tadds %0,%0,%4\n\tadc %1,%1,xzr"
             : "=&r"(res), "=&r"(overflow)
             : "r"(nmemb),
               "r"(size),
               "r"(offset));

        if (UNEXPECTED(overflow)) {
                zend_error_noreturn(E_ERROR, "Possible integer overflow in memory allocation (%zu * %zu + %zu)", nmemb, size, offset);
                return 0;
        }
        return res;
}

#elif SIZEOF_SIZE_T == 4 && defined(HAVE_ZEND_LONG64)

static inline size_t safe_address(size_t nmemb, size_t size, size_t offset)
{
  zend_ulong64 res = (zend_ulong64)nmemb * (zend_ulong64)size + (zend_ulong64)offset;

  if (UNEXPECTED(res > (zend_ulong64)0xFFFFFFFFL)) {
    zend_error_noreturn(E_ERROR, "Possible integer overflow in memory allocation (%zu * %zu + %zu)", nmemb, size, offset);
    return 0;
  }
  return (size_t) res;
}

#else

static inline size_t safe_address(size_t nmemb, size_t size, size_t offset)
{
  size_t res = nmemb * size + offset;
  double _d  = (double)nmemb * (double)size + (double)offset;
  double _delta = (double)res - _d;

  if (UNEXPECTED((_d + _delta ) != _d)) {
    zend_error_noreturn(E_ERROR, "Possible integer overflow in memory allocation (%zu * %zu + %zu)", nmemb, size, offset);
    return 0;
  }
  return res;
}
#endif

ZEND_API void *_emalloc(size_t size ZEND_FILE_LINE_DC ZEND_FILE_LINE_ORIG_DC) {
  return HPHP::req::malloc(size);
}

ZEND_API void *_safe_emalloc(size_t nmemb, size_t size, size_t offset ZEND_FILE_LINE_DC ZEND_FILE_LINE_ORIG_DC) {
 return _emalloc(safe_address(nmemb, size, offset) ZEND_FILE_LINE_CC ZEND_FILE_LINE_EMPTY_CC);
}

ZEND_API void *_safe_malloc(size_t nmemb, size_t size, size_t offset) {
  return pemalloc(safe_address(nmemb, size, offset), 1);
}

ZEND_API void _efree(const zval* ptr ZEND_FILE_LINE_DC ZEND_FILE_LINE_ORIG_DC) {
  ptr->releaseMem();
}

ZEND_API void _efree(void *ptr ZEND_FILE_LINE_DC ZEND_FILE_LINE_ORIG_DC) {
  return HPHP::req::free(ptr);
}

ZEND_API void *_ecalloc(size_t nmemb, size_t size ZEND_FILE_LINE_DC ZEND_FILE_LINE_ORIG_DC) {
  return HPHP::req::calloc(nmemb, size);
}

ZEND_API void *_erealloc(void *ptr, size_t size, int allow_failure ZEND_FILE_LINE_DC ZEND_FILE_LINE_ORIG_DC) {
  return HPHP::req::realloc(ptr, size);
}

ZEND_API char *_estrndup(const char *s, unsigned int length ZEND_FILE_LINE_DC ZEND_FILE_LINE_ORIG_DC) {
  char* ret = (char*) _emalloc(length + 1 ZEND_FILE_LINE_CC ZEND_FILE_LINE_EMPTY_CC);
  memcpy(ret, s, length);
  ret[length] = '\0';
  return ret;
}

ZEND_API char *_estrdup(const char *s ZEND_FILE_LINE_DC ZEND_FILE_LINE_ORIG_DC) {
  return _estrndup(s, strlen(s) ZEND_FILE_LINE_CC ZEND_FILE_LINE_EMPTY_CC);
}
