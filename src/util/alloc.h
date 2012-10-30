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

#include <stdint.h>

#include "exception.h"

#ifdef USE_TCMALLOC
#include <google/malloc_extension.h>
#endif

#ifndef USE_JEMALLOC
# ifdef __FreeBSD__
#  include "stdlib.h"
#  include "malloc_np.h"
# else
#  include "malloc.h"
# endif
#else
# include <jemalloc/jemalloc.h>
# ifndef ALLOCM_ARENA
#  define ALLOCM_ARENA(a) 0
# endif
#endif

extern "C" {
#ifdef USE_TCMALLOC
#define MallocExtensionInstance _ZN15MallocExtension8instanceEv
  MallocExtension* MallocExtensionInstance() __attribute__((weak));
#endif

#ifdef USE_JEMALLOC
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
  EXCEPTION_COMMON_IMPL(OutOfMemoryException);
};

namespace Util {
///////////////////////////////////////////////////////////////////////////////

#ifdef USE_JEMALLOC
extern unsigned low_arena;
#endif

inline void* low_malloc(size_t size) {
#ifndef USE_JEMALLOC
  return malloc(size);
#else
  void* ptr = NULL;
  allocm(&ptr, NULL, size, ALLOCM_ARENA(low_arena));
  return ptr;
#endif
}

inline void low_free(void* ptr) {
#ifndef USE_JEMALLOC
  free(ptr);
#else
  dallocm(ptr, ALLOCM_ARENA(low_arena));
#endif
}

/**
 * Safe memory allocation.
 */
inline void* safe_malloc(size_t size) {
  void* p = malloc(size);
  if (!p) throw OutOfMemoryException(size);
  return p;
}

inline void* safe_calloc(size_t count, size_t size) {
  void* p = calloc(count, size);
  if (!p) throw OutOfMemoryException(size);
  return p;
}

inline void* safe_realloc(void* ptr, size_t size) {
  ptr = realloc(ptr, size);
  if (!ptr && size > 0) throw OutOfMemoryException(size);
  return ptr;
}

inline void safe_free(void* ptr) {
  return free(ptr);
}

/**
 * Instruct low level memory allocator to free memory back to system. Called
 * when thread's been idle and predicted to continue to be idle for a while.
 */
void flush_thread_caches();

/**
 * Instruct the kernel to free parts of the unused stack back to the system.
 * Like flush_thread_caches, this is called when the thread has been idle
 * and predicted to continue to be idle for a while.
 */
void flush_thread_stack();

/**
 * Like scoped_ptr, but calls free() on destruct
 */
class ScopedMem {
  ScopedMem(const ScopedMem&); // disable copying
  ScopedMem& operator=(const ScopedMem&);
 public:
  ScopedMem() : m_ptr(0) {}
  explicit ScopedMem(void* ptr) : m_ptr(ptr) {}
  ~ScopedMem() { free(m_ptr); }
  ScopedMem& operator=(void* ptr) {
    ASSERT(!m_ptr);
    m_ptr = ptr;
    return *this;
  }
 private:
  void* m_ptr;
};

extern __thread uintptr_t s_stackLimit;
extern __thread size_t s_stackSize;
///////////////////////////////////////////////////////////////////////////////
}}

#endif // __HPHP_UTIL_ALLOC_H__
