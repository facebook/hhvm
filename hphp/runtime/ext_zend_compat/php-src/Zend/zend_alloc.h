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
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef ZEND_ALLOC_H
#define ZEND_ALLOC_H

#include <stdio.h>

#include "../TSRM/TSRM.h" // nolint
#include "zend.h"

#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/complex-types.h"

/**
 * Some times a zval is passed to efree() and friends, but unfortunately
 * zvals (i.e. RefDatas) are not allocated using smart_malloc but rather
 * they are allocated using their own allocator, so we have a different
 * overload of _efree() to handle freeing these correctly.
 */
inline void _efree(const HPHP::RefData* ptr) {
  auto p = const_cast<HPHP::RefData*>(ptr);
  p->tv()->m_type = HPHP::KindOfNull;
  p->release();
}

BEGIN_EXTERN_C()

ZEND_API inline void *_emalloc(size_t size ZEND_FILE_LINE_DC ZEND_FILE_LINE_ORIG_DC) {
  return HPHP::smart_malloc(size);
}
ZEND_API inline void *_safe_emalloc(size_t nmemb, size_t size, size_t offset ZEND_FILE_LINE_DC ZEND_FILE_LINE_ORIG_DC) {
  return _emalloc(nmemb * size + offset);
}
ZEND_API inline void *_safe_malloc(size_t nmemb, size_t size, size_t offset) {
  return _emalloc(nmemb * size + offset);
}
ZEND_API inline void _efree(const void *ptr ZEND_FILE_LINE_DC ZEND_FILE_LINE_ORIG_DC) {
  HPHP::smart_free(const_cast<void*>(ptr));
}
ZEND_API inline void *_ecalloc(size_t nmemb, size_t size ZEND_FILE_LINE_DC ZEND_FILE_LINE_ORIG_DC) {
  return HPHP::smart_calloc(nmemb, size);
}
ZEND_API inline void *_erealloc(void *ptr, size_t size, int allow_failure ZEND_FILE_LINE_DC ZEND_FILE_LINE_ORIG_DC) {
  return HPHP::smart_realloc(ptr, size);
}
ZEND_API inline char *_estrndup(const char *s, unsigned int length ZEND_FILE_LINE_DC ZEND_FILE_LINE_ORIG_DC) {
  char* ret = (char*) _emalloc(length + 1);
  memcpy(ret, s, length);
  ret[length] = '\0';
  return ret;
}
ZEND_API inline char *_estrdup(const char *s ZEND_FILE_LINE_DC ZEND_FILE_LINE_ORIG_DC) {
  return _estrndup(s, strlen(s));
}

/* Standard wrapper macros */
#define emalloc(size)            _emalloc((size) ZEND_FILE_LINE_CC ZEND_FILE_LINE_EMPTY_CC)
#define safe_emalloc(nmemb, size, offset)  _safe_emalloc((nmemb), (size), (offset) ZEND_FILE_LINE_CC ZEND_FILE_LINE_EMPTY_CC)
#define efree(ptr)              _efree((ptr) ZEND_FILE_LINE_CC ZEND_FILE_LINE_EMPTY_CC)
#define ecalloc(nmemb, size)        _ecalloc((nmemb), (size) ZEND_FILE_LINE_CC ZEND_FILE_LINE_EMPTY_CC)
#define erealloc(ptr, size)          _erealloc((ptr), (size), 0 ZEND_FILE_LINE_CC ZEND_FILE_LINE_EMPTY_CC)
#define safe_erealloc(ptr, nmemb, size, offset)  _safe_erealloc((ptr), (nmemb), (size), (offset) ZEND_FILE_LINE_CC ZEND_FILE_LINE_EMPTY_CC)
#define erealloc_recoverable(ptr, size)    _erealloc((ptr), (size), 1 ZEND_FILE_LINE_CC ZEND_FILE_LINE_EMPTY_CC)
#define estrdup(s)              _estrdup((s) ZEND_FILE_LINE_CC ZEND_FILE_LINE_EMPTY_CC)
#define estrndup(s, length)          _estrndup((s), (length) ZEND_FILE_LINE_CC ZEND_FILE_LINE_EMPTY_CC)
#define zend_mem_block_size(ptr)      _zend_mem_block_size((ptr) TSRMLS_CC ZEND_FILE_LINE_CC ZEND_FILE_LINE_EMPTY_CC)

/* Relay wrapper macros */
#define emalloc_rel(size)            _emalloc((size) ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_CC)
#define safe_emalloc_rel(nmemb, size, offset)  _safe_emalloc((nmemb), (size), (offset) ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_CC)
#define efree_rel(ptr)              _efree((ptr) ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_CC)
#define ecalloc_rel(nmemb, size)        _ecalloc((nmemb), (size) ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_CC)
#define erealloc_rel(ptr, size)          _erealloc((ptr), (size), 0 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_CC)
#define erealloc_recoverable_rel(ptr, size)    _erealloc((ptr), (size), 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_CC)
#define safe_erealloc_rel(ptr, nmemb, size, offset)  _safe_erealloc((ptr), (nmemb), (size), (offset) ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_CC)
#define estrdup_rel(s)              _estrdup((s) ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_CC)
#define estrndup_rel(s, length)          _estrndup((s), (length) ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_CC)
#define zend_mem_block_size_rel(ptr)      _zend_mem_block_size((ptr) TSRMLS_CC ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_CC)

inline void * __zend_malloc(size_t len)
{
  void *tmp = malloc(len);
  if (tmp) {
    return tmp;
  }
  fprintf(stderr, "Out of memory\n");
  exit(1);
}

inline void * __zend_calloc(size_t nmemb, size_t len)
{
  void *tmp = _safe_malloc(nmemb, len, 0);
  memset(tmp, 0, nmemb * len);
  return tmp;
}

inline void * __zend_realloc(void *p, size_t len)
{
  p = realloc(p, len);
  if (p) {
    return p;
  }
  fprintf(stderr, "Out of memory\n");
  exit(1);
}


/* Selective persistent/non persistent allocation macros */
#define pemalloc(size, persistent) ((persistent)?__zend_malloc(size):emalloc(size))
#define safe_pemalloc(nmemb, size, offset, persistent)  ((persistent)?_safe_malloc(nmemb, size, offset):safe_emalloc(nmemb, size, offset))
#define pefree(ptr, persistent)  ((persistent)?free(ptr):efree(ptr))
#define pecalloc(nmemb, size, persistent) ((persistent)?__zend_calloc((nmemb), (size)):ecalloc((nmemb), (size)))
#define perealloc(ptr, size, persistent) ((persistent)?__zend_realloc((ptr), (size)):erealloc((ptr), (size)))
#define safe_perealloc(ptr, nmemb, size, offset, persistent)  ((persistent)?_safe_realloc((ptr), (nmemb), (size), (offset)):safe_erealloc((ptr), (nmemb), (size), (offset)))
#define perealloc_recoverable(ptr, size, persistent) ((persistent)?__zend_realloc((ptr), (size)):erealloc_recoverable((ptr), (size)))
#define pestrdup(s, persistent) ((persistent)?strdup(s):estrdup(s))
#define pestrndup(s, length, persistent) ((persistent)?zend_strndup((s),(length)):estrndup((s),(length)))

#define pemalloc_rel(size, persistent) ((persistent)?__zend_malloc(size):emalloc_rel(size))
#define pefree_rel(ptr, persistent)  ((persistent)?free(ptr):efree_rel(ptr))
#define pecalloc_rel(nmemb, size, persistent) ((persistent)?__zend_calloc((nmemb), (size)):ecalloc_rel((nmemb), (size)))
#define perealloc_rel(ptr, size, persistent) ((persistent)?__zend_realloc((ptr), (size)):erealloc_rel((ptr), (size)))
#define perealloc_recoverable_rel(ptr, size, persistent) ((persistent)?__zend_realloc((ptr), (size)):erealloc_recoverable_rel((ptr), (size)))
#define pestrdup_rel(s, persistent) ((persistent)?strdup(s):estrdup_rel(s))

#define safe_estrdup(ptr)  ((ptr)?(estrdup(ptr)):STR_EMPTY_ALLOC())
#define safe_estrndup(ptr, len) ((ptr)?(estrndup((ptr), (len))):STR_EMPTY_ALLOC())

END_EXTERN_C()

/* fast cache for zval's */
#define ALLOC_ZVAL(z)  \
  (z) = HPHP::RefData::Make(HPHP::KindOfNull, 0)

#define FREE_ZVAL(z)  \
  do { \
    (z)->tv()->m_type = KindOfNull; \
    (z)->release(); \
  } while (0)

#define ALLOC_ZVAL_REL(z)  \
  (z) = HPHP::RefData::Make(HPHP::KindOfNull, 0)

#define FREE_ZVAL_REL(z)  \
  do { \
    (z)->tv()->m_type = KindOfNull; \
    (z)->release(); \
  } while (0)

/* fast cache for HashTables */
#define ALLOC_HASHTABLE(ht)                               \
  (ht) = [&]{ auto ret = HPHP::HphpArray::MakeReserve(0); \
              ret->setRefCount(0);                        \
              return ret; }()

// TODO Should we try to free the hashtable here? At present it
// gets freed by zend_hash_destroy()
#define FREE_HASHTABLE(ht)

#define ALLOC_HASHTABLE_REL(ht)  \
  (ht) = HashTable::Make(0);

// TODO Should we try to free the hashtable here? At present it
// gets freed by zend_hash_destroy()
#define FREE_HASHTABLE_REL(ht)

#endif
