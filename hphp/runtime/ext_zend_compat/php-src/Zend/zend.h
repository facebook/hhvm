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

#ifndef ZEND_H
#define ZEND_H

#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/tv-helpers.h"
#include "hphp/runtime/vm/bytecode.h"

#define ZEND_ENGINE_2

#ifdef __cplusplus
#define BEGIN_EXTERN_C() extern "C" {
#define END_EXTERN_C() }
#else
#define BEGIN_EXTERN_C()
#define END_EXTERN_C()
#endif

#define EMPTY_SWITCH_DEFAULT_CASE()

/* all HAVE_XXX test have to be after the include of zend_config above */

#include <stdio.h>

#ifdef HAVE_UNIX_H
# include <unix.h>
#endif

#ifdef HAVE_STDARG_H
# include <stdarg.h>
#endif

#ifdef HAVE_DLFCN_H
# include <dlfcn.h>
#endif

#if defined(HAVE_LIBDL)

# ifndef RTLD_LAZY
#  define RTLD_LAZY 1    /* Solaris 1, FreeBSD's (2.1.7.1 and older) */
# endif

# ifndef RTLD_GLOBAL
#  define RTLD_GLOBAL 0
# endif

# if defined(RTLD_GROUP) && defined(RTLD_WORLD) && defined(RTLD_PARENT)
#  define DL_LOAD(libname)      dlopen(libname, RTLD_LAZY | RTLD_GLOBAL | RTLD_GROUP | RTLD_WORLD | RTLD_PARENT)
# elif defined(RTLD_DEEPBIND)
#  define DL_LOAD(libname)      dlopen(libname, RTLD_LAZY | RTLD_GLOBAL | RTLD_DEEPBIND)
# else
#  define DL_LOAD(libname)      dlopen(libname, RTLD_LAZY | RTLD_GLOBAL)
# endif
# define DL_UNLOAD          dlclose
# if defined(DLSYM_NEEDS_UNDERSCORE)
#  define DL_FETCH_SYMBOL(h,s)    dlsym((h), "_" s)
# else
#  define DL_FETCH_SYMBOL      dlsym
# endif
# define DL_ERROR          dlerror
# define DL_HANDLE          void *
# define ZEND_EXTENSIONS_SUPPORT  1
#elif defined(ZEND_WIN32)
# define DL_LOAD(libname)      LoadLibrary(libname)
# define DL_FETCH_SYMBOL      GetProcAddress
# define DL_UNLOAD          FreeLibrary
# define DL_HANDLE          HMODULE
# define ZEND_EXTENSIONS_SUPPORT  1
#else
# define DL_HANDLE          void *
# define ZEND_EXTENSIONS_SUPPORT  0
#endif

#if HAVE_ALLOCA_H && !defined(_ALLOCA_H)
#  include <alloca.h>
#endif

/* AIX requires this to be the first thing in the file.  */
#ifndef __GNUC__
# ifndef HAVE_ALLOCA_H
#  ifdef _AIX
#pragma alloca
#  else
#   ifndef alloca /* predefined by HP cc +Olibcalls */
char *alloca ();
#   endif
#  endif
# endif
#endif

/* Compatibility with non-clang compilers */
#ifndef __has_attribute
# define __has_attribute(x) 0
#endif

/* GCC x.y.z supplies __GNUC__ = x and __GNUC_MINOR__ = y */
#ifdef __GNUC__
# define ZEND_GCC_VERSION (__GNUC__ * 1000 + __GNUC_MINOR__)
#else
# define ZEND_GCC_VERSION 0
#endif

#if ZEND_GCC_VERSION >= 2096
# define ZEND_ATTRIBUTE_MALLOC __attribute__ ((__malloc__))
#else
# define ZEND_ATTRIBUTE_MALLOC
#endif

#if ZEND_GCC_VERSION >= 4003 || __has_attribute(alloc_size)
# define ZEND_ATTRIBUTE_ALLOC_SIZE(X) __attribute__ ((alloc_size(X)))
# define ZEND_ATTRIBUTE_ALLOC_SIZE2(X,Y) __attribute__ ((alloc_size(X,Y)))
#else
# define ZEND_ATTRIBUTE_ALLOC_SIZE(X)
# define ZEND_ATTRIBUTE_ALLOC_SIZE2(X,Y)
#endif

#if ZEND_GCC_VERSION >= 2007
# define ZEND_ATTRIBUTE_FORMAT(type, idx, first) __attribute__ ((format(type, idx, first)))
#else
# define ZEND_ATTRIBUTE_FORMAT(type, idx, first)
#endif

#if ZEND_GCC_VERSION >= 3001 && !defined(__INTEL_COMPILER)
# define ZEND_ATTRIBUTE_PTR_FORMAT(type, idx, first) __attribute__ ((format(type, idx, first)))
#else
# define ZEND_ATTRIBUTE_PTR_FORMAT(type, idx, first)
#endif

#if ZEND_GCC_VERSION >= 3001
# define ZEND_ATTRIBUTE_DEPRECATED  __attribute__((deprecated))
#elif defined(ZEND_WIN32) && defined(_MSC_VER) && _MSC_VER >= 1300
# define ZEND_ATTRIBUTE_DEPRECATED  __declspec(deprecated)
#else
# define ZEND_ATTRIBUTE_DEPRECATED
#endif

#if defined(__GNUC__) && ZEND_GCC_VERSION >= 3004 && defined(__i386__)
# define ZEND_FASTCALL __attribute__((fastcall))
#elif defined(_MSC_VER) && defined(_M_IX86)
# define ZEND_FASTCALL __fastcall
#else
# define ZEND_FASTCALL
#endif

#if defined(__GNUC__) && ZEND_GCC_VERSION >= 3004
#else
# define __restrict__
#endif
#define restrict __restrict__

#if (HAVE_ALLOCA || (defined (__GNUC__) && __GNUC__ >= 2)) && !(defined(ZTS) && defined(ZEND_WIN32)) && !(defined(ZTS) && defined(NETWARE)) && !(defined(ZTS) && defined(HPUX)) && !defined(DARWIN)
# define ZEND_ALLOCA_MAX_SIZE (32 * 1024)
# define ALLOCA_FLAG(name) \
  zend_bool name;
# define SET_ALLOCA_FLAG(name) \
  name = 1
# define do_alloca_ex(size, limit, use_heap) \
  ((use_heap = (UNEXPECTED((size) > (limit)))) ? emalloc(size) : alloca(size))
# define do_alloca(size, use_heap) \
  do_alloca_ex(size, ZEND_ALLOCA_MAX_SIZE, use_heap)
# define free_alloca(p, use_heap) \
  do { if (UNEXPECTED(use_heap)) efree(p); } while (0)
#else
# define ALLOCA_FLAG(name)
# define SET_ALLOCA_FLAG(name)
# define do_alloca(p, use_heap)    emalloc(p)
# define free_alloca(p, use_heap)  efree(p)
#endif

#define ZEND_FILE_LINE_D
#define ZEND_FILE_LINE_DC
#define ZEND_FILE_LINE_ORIG_D
#define ZEND_FILE_LINE_ORIG_DC
#define ZEND_FILE_LINE_RELAY_C
#define ZEND_FILE_LINE_RELAY_CC
#define ZEND_FILE_LINE_C
#define ZEND_FILE_LINE_CC
#define ZEND_FILE_LINE_EMPTY_C
#define ZEND_FILE_LINE_EMPTY_CC
#define ZEND_FILE_LINE_ORIG_RELAY_C
#define ZEND_FILE_LINE_ORIG_RELAY_CC
#define ZEND_ASSERT(c)

#define ZTS_V 0

#include "zend_errors.h"
#include "zend_alloc.h"

#include "zend_types.h"

#ifdef HAVE_LIMITS_H
# include <limits.h>
#endif

#ifndef LONG_MAX
#define LONG_MAX 2147483647L
#endif

#ifndef LONG_MIN
#define LONG_MIN (- LONG_MAX - 1)
#endif

#if SIZEOF_LONG == 4
#define MAX_LENGTH_OF_LONG 11
const char long_min_digits[] = "2147483648";
#elif SIZEOF_LONG == 8
#define MAX_LENGTH_OF_LONG 20
const char long_min_digits[] = "9223372036854775808";
#else
#error "Unknown SIZEOF_LONG"
#endif

#define MAX_LENGTH_OF_DOUBLE 32

typedef enum {
  SUCCESS =  0,
  FAILURE = -1,    /* this MUST stay a negative number, or it may affect functions! */
} ZEND_RESULT_CODE;

#include "zend_hash.h"

#define INTERNAL_FUNCTION_PARAMETERS HPHP::ActRec* ar, zval* return_value, zval* this_ptr
#define INTERNAL_FUNCTION_PARAM_PASSTHRU ar, return_value, this_ptr

#if defined(__GNUC__) && __GNUC__ >= 3 && !defined(__INTEL_COMPILER) && !defined(DARWIN) && !defined(__hpux) && !defined(_AIX) && !defined(__osf__)
void zend_error_noreturn(int type, const char *format, ...) __attribute__ ((noreturn));
#else
#  define zend_error_noreturn zend_error
#endif

/*
 * zval
 */
typedef struct HPHP::Class zend_class_entry;

#include "zend_object_handlers.h"

typedef union HPHP::Value zvalue_value;

#define Z_REFCOUNT_PP(ppz)    Z_REFCOUNT_P(*(ppz))
#define Z_SET_REFCOUNT_PP(ppz, rc)  Z_SET_REFCOUNT_P(*(ppz), rc)
#define Z_ADDREF_PP(ppz)    Z_ADDREF_P(*(ppz))
#define Z_DELREF_PP(ppz)    Z_DELREF_P(*(ppz))
#define Z_ISREF_PP(ppz)      Z_ISREF_P(*(ppz))
#define Z_SET_ISREF_PP(ppz)    Z_SET_ISREF_P(*(ppz))
#define Z_UNSET_ISREF_PP(ppz)    Z_UNSET_ISREF_P(*(ppz))
#define Z_SET_ISREF_TO_PP(ppz, isref)  Z_SET_ISREF_TO_P(*(ppz), isref)

#define Z_REFCOUNT_P(pz)    zval_refcount_p(pz)
#define Z_SET_REFCOUNT_P(pz, rc)  zval_set_refcount_p(pz, rc)
#define Z_ADDREF_P(pz)      zval_addref_p(pz)
#define Z_DELREF_P(pz)      zval_delref_p(pz)
#define Z_ISREF_P(pz)      zval_isref_p(pz)
#define Z_SET_ISREF_P(pz)    zval_set_isref_p(pz)
#define Z_UNSET_ISREF_P(pz)    zval_unset_isref_p(pz)
#define Z_SET_ISREF_TO_P(pz, isref)  zval_set_isref_to_p(pz, isref)

#define Z_REFCOUNT(z)      Z_REFCOUNT_P(&(z))
#define Z_SET_REFCOUNT(z, rc)    Z_SET_REFCOUNT_P(&(z), rc)
#define Z_ADDREF(z)      Z_ADDREF_P(&(z))
#define Z_DELREF(z)      Z_DELREF_P(&(z))
#define Z_ISREF(z)      Z_ISREF_P(&(z))
#define Z_SET_ISREF(z)      Z_SET_ISREF_P(&(z))
#define Z_UNSET_ISREF(z)    Z_UNSET_ISREF_P(&(z))
#define Z_SET_ISREF_TO(z, isref)  Z_SET_ISREF_TO_P(&(z), isref)

#if ZEND_DEBUG
#define zend_always_inline inline
#define zend_never_inline
#else
#if defined(__GNUC__)
#if __GNUC__ >= 3
#define zend_always_inline inline __attribute__((always_inline))
#define zend_never_inline __attribute__((noinline))
#else
#define zend_always_inline inline
#define zend_never_inline
#endif
#elif defined(_MSC_VER)
#define zend_always_inline __forceinline
#define zend_never_inline
#else
#define zend_always_inline inline
#define zend_never_inline
#endif
#endif /* ZEND_DEBUG */

#if (defined (__GNUC__) && __GNUC__ > 2 ) && !defined(DARWIN) && !defined(__hpux) && !defined(_AIX)
# define EXPECTED(condition)   __builtin_expect(condition, 1)
# define UNEXPECTED(condition) __builtin_expect(condition, 0)
#else
# define EXPECTED(condition)   (condition)
# define UNEXPECTED(condition) (condition)
#endif

using HPHP::KindOfRefCountThreshold;
zend_always_inline zend_bool zval_isref_p(zval* pz) {
  return pz->zIsRef();
}

zend_always_inline zend_uint zval_refcount_p(zval* pz) {
  return pz->zRefcount();
}

zend_always_inline zend_uint zval_set_refcount_p(zval* pz, zend_uint rc) {
  pz->zSetRefcount(rc);
  return rc;
}

zend_always_inline zend_uint zval_addref_p(zval* pz) {
  pz->zAddRef();
  return pz->zRefcount();
}

zend_always_inline zend_uint zval_delref_p(zval* pz) {
  pz->zDelRef();
  return pz->zRefcount();
}

zend_always_inline zend_bool zval_set_isref_p(zval* pz) {
  pz->zSetIsRef();
  return true;
}

zend_always_inline zend_bool zval_unset_isref_p(zval* pz) {
  pz->zUnsetIsRef();
  return false;
}

zend_always_inline zend_bool zval_set_isref_to_p(zval* pz, zend_bool isref) {
  return isref ? zval_set_isref_p(pz) : zval_unset_isref_p(pz);
}

#include "zend_stream.h"

#undef MIN
#undef MAX
#define MAX(a, b)  (((a)>(b))?(a):(b))
#define MIN(a, b)  (((a)<(b))?(a):(b))
#define ZEND_STRL(str)    (str), (sizeof(str)-1)
#define ZEND_STRS(str)    (str), (sizeof(str))
#define ZEND_NORMALIZE_BOOL(n)      \
  ((n) ? (((n)>0) ? 1 : -1) : 0)
#define ZEND_TRUTH(x)    ((x) ? 1 : 0)
#define ZEND_LOG_XOR(a, b)    (ZEND_TRUTH(a) ^ ZEND_TRUTH(b))

/* data types */
/* All data types <= IS_BOOL have their constructor/destructors skipped */
#define IS_NULL            HPHP::KindOfNull
#define IS_LONG            HPHP::KindOfInt64
#define IS_DOUBLE          HPHP::KindOfDouble
#define IS_BOOL            HPHP::KindOfBoolean
#define IS_ARRAY           HPHP::KindOfArray
#define IS_OBJECT          HPHP::KindOfObject
#define IS_STRING          HPHP::KindOfString
#define IS_RESOURCE        HPHP::KindOfResource
#define IS_CONSTANT        8
#define IS_CONSTANT_ARRAY  9
#define IS_CALLABLE        10

ZEND_API void zend_make_printable_zval(zval *expr, zval *expr_copy, int *use_copy);

#define STR_FREE(ptr) if (ptr && !IS_INTERNED(ptr)) { efree(ptr); }
#define STR_FREE_REL(ptr) if (ptr && !IS_INTERNED(ptr)) { efree_rel(ptr); }

#define STR_EMPTY_ALLOC() estrndup("", sizeof("")-1)

#define STR_REALLOC(ptr, size) \
      ptr = (char *) erealloc(ptr, size);

BEGIN_EXTERN_C()

ZEND_API inline void zend_error(int type, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  HPHP::raise_message(static_cast<HPHP::ErrorConstants::ErrorModes>(type), format, ap);
  va_end(ap);
}

extern ZEND_API zend_class_entry *zend_standard_class_def;

END_EXTERN_C()

#define INIT_PZVAL(z) \
  Z_SET_REFCOUNT_P((z), 1); \
  Z_UNSET_ISREF_P((z));

#define INIT_ZVAL(z) ((z).zInit());

#define ALLOC_INIT_ZVAL(zp) \
  ALLOC_ZVAL(zp); \
  INIT_ZVAL(*zp);

#define MAKE_STD_ZVAL(zv) \
  ALLOC_ZVAL(zv); \
  INIT_PZVAL(zv);

#define PZVAL_IS_REF(z) Z_ISREF_P(z)

#define ZVAL_COPY_VALUE(z, v) \
  do { \
    (z)->tv()->m_data.num = (v)->tv()->m_data.num; \
    (z)->tv()->m_type = (v)->tv()->m_type; \
  } while (0)

#define INIT_PZVAL_COPY(z, v) \
  do { \
    ZVAL_COPY_VALUE(z, v); \
    Z_SET_REFCOUNT_P(z, 1); \
    Z_UNSET_ISREF_P(z); \
  } while (0)

#define SEPARATE_ZVAL(ppzv) \
  do { \
    if (Z_REFCOUNT_PP((ppzv)) > 1) { \
      zval *new_zv; \
      Z_DELREF_PP(ppzv); \
      ALLOC_ZVAL(new_zv); \
      INIT_PZVAL_COPY(new_zv, *(ppzv)); \
      *(ppzv) = new_zv; \
      zval_copy_ctor(new_zv); \
    } \
  } while (0)

#define SEPARATE_ZVAL_IF_NOT_REF(ppzv)    \
  if (!PZVAL_IS_REF(*ppzv)) {        \
    SEPARATE_ZVAL(ppzv);        \
  }

#define SEPARATE_ZVAL_TO_MAKE_IS_REF(ppzv)  \
  if (!PZVAL_IS_REF(*ppzv)) {        \
    SEPARATE_ZVAL(ppzv);        \
    Z_SET_ISREF_PP((ppzv));        \
  }

#define COPY_PZVAL_TO_ZVAL(zv, pzv)      \
  (zv) = *(pzv);              \
  if (Z_REFCOUNT_P(pzv)>1) {        \
    zval_copy_ctor(&(zv));        \
    Z_DELREF_P((pzv));          \
  } else {                \
    FREE_ZVAL(pzv);            \
  }                    \
  INIT_PZVAL(&(zv));

#define MAKE_COPY_ZVAL(ppzv, pzv)   \
  INIT_PZVAL_COPY(pzv, *(ppzv));  \
  zval_copy_ctor((pzv));

#define REPLACE_ZVAL_VALUE(ppzv_dest, pzv_src, copy) {  \
  int is_ref, refcount;            \
                        \
  SEPARATE_ZVAL_IF_NOT_REF(ppzv_dest);    \
  is_ref = Z_ISREF_PP(ppzv_dest);        \
  refcount = Z_REFCOUNT_PP(ppzv_dest);    \
  zval_dtor(*ppzv_dest);            \
  ZVAL_COPY_VALUE(*ppzv_dest, pzv_src);    \
  if (copy) {                                 \
    zval_copy_ctor(*ppzv_dest);        \
    }                                        \
  Z_SET_ISREF_TO_PP(ppzv_dest, is_ref);    \
  Z_SET_REFCOUNT_PP(ppzv_dest, refcount);    \
}

#include "zend_operators.h"
#include "zend_variables.h"

#endif /* ZEND_H */
