/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef __HPHP_UTIL_ALLOC_H__
#define __HPHP_UTIL_ALLOC_H__

#include "exception.h"

#ifndef NO_TCMALLOC
#include <google/malloc_extension.h>
#endif

#ifndef NO_JEMALLOC
#include <jemalloc/jemalloc.h>
#endif

extern "C" {
#ifndef NO_TCMALLOC
#define MallocExtensionInstance _ZN15MallocExtension8instanceEv
  MallocExtension* MallocExtensionInstance() __attribute__((weak));
#endif

#ifndef NO_JEMALLOC
  int mallctl(const char *name, void *oldp, size_t *oldlenp, void *newp,
              size_t newlen) __attribute__((weak));
  int mallctlnametomib(const char *name, size_t* mibp, size_t*miblenp)
              __attribute__((weak));
  int mallctlbymib(const size_t* mibp, size_t miblen, void *oldp,
              size_t *oldlenp, void *newp, size_t newlen) __attribute__((weak));
  void malloc_stats_print(void (*write_cb)(void *, const char *),
                          void *cbopaque, const char *opts)
    __attribute__((weak));
#endif
}

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class OutOfMemoryException : public Exception {
public:
  OutOfMemoryException(size_t size)
      : Exception("Unable to allocate %zu bytes of memory", size) {}
  virtual ~OutOfMemoryException() throw() {}
};

namespace Util {
///////////////////////////////////////////////////////////////////////////////

/**
 * Safe memory allocation.
 */
void* safe_malloc(size_t size);
void* safe_realloc(void* ptr, size_t size);
void  safe_free(void* ptr);

/**
 * Instruct low level memory allocator to free memory back to system. Called
 * when thread's been idle and predicted to continue to be idle for a while.
 */
void flush_thread_caches();

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __HPHP_UTIL_ALLOC_H__
