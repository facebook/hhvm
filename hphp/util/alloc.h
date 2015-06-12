/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_UTIL_ALLOC_H_
#define incl_HPHP_UTIL_ALLOC_H_

#include <stdint.h>
#include <cassert>
#include <atomic>

#include <folly/Portability.h>

#include "hphp/util/exception.h"

#ifdef FOLLY_SANITIZE_ADDRESS
// ASan is less precise than valgrind so we'll need a superset of those tweaks
# define VALGRIND
// TODO: (t2869817) ASan doesn't play well with jemalloc
# ifdef USE_JEMALLOC
#  undef USE_JEMALLOC
# endif
#endif

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
# undef MALLOCX_LG_ALIGN
# undef MALLOCX_ZERO
# include <jemalloc/jemalloc.h>
#endif

#include "hphp/util/maphuge.h"

extern "C" {
#ifdef USE_TCMALLOC
#define MallocExtensionInstance _ZN15MallocExtension8instanceEv
  MallocExtension* MallocExtensionInstance() __attribute__((__weak__));
#endif

#ifdef USE_JEMALLOC

  int mallctl(const char *name, void *oldp, size_t *oldlenp, void *newp,
              size_t newlen) __attribute__((__weak__));
  int mallctlnametomib(const char *name, size_t* mibp, size_t*miblenp)
              __attribute__((__weak__));
  int mallctlbymib(const size_t* mibp, size_t miblen, void *oldp,
              size_t *oldlenp, void *newp, size_t newlen) __attribute__((__weak__));
  void malloc_stats_print(void (*write_cb)(void *, const char *),
                          void *cbopaque, const char *opts)
    __attribute__((__weak__));
#endif
}

enum class NotNull {};

/*
 * The placement-new provided by the standard library is required by the
 * C++ specification to perform a null check because it is marked with noexcept
 * or throw() depending on the compiler version. This override of placement
 * new doesn't use either of these, so it is allowed to omit the null check.
 */
inline void* operator new(size_t, NotNull, void* location) {
  assert(location);
  return location;
}

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const bool use_jemalloc =
#ifdef USE_JEMALLOC
  true
#else
  false
#endif
  ;

struct OutOfMemoryException : Exception {
  explicit OutOfMemoryException(size_t size)
    : Exception("Unable to allocate %zu bytes of memory", size) {}
  EXCEPTION_COMMON_IMPL(OutOfMemoryException);
};

///////////////////////////////////////////////////////////////////////////////

#ifdef USE_JEMALLOC
extern unsigned low_arena;
extern std::atomic<int> low_huge_pages;

inline int low_mallocx_flags() {
  // Allocate from low_arena, and bypass the implicit tcache to assure that the
  // result actually comes from low_arena.
#ifdef MALLOCX_TCACHE_NONE
  return MALLOCX_ARENA(low_arena)|MALLOCX_TCACHE_NONE;
#else
  return MALLOCX_ARENA(low_arena);
#endif
}

inline int low_dallocx_flags() {
#ifdef MALLOCX_TCACHE_NONE
  // Bypass the implicit tcache for this deallocation.
  return MALLOCX_TCACHE_NONE;
#else
  // Prior to the introduction of MALLOCX_TCACHE_NONE, explicitly specifying
  // MALLOCX_ARENA(a) caused jemalloc to bypass tcache.
  return MALLOCX_ARENA(low_arena);
#endif
}
#endif

inline void* low_malloc(size_t size) {
#ifndef USE_JEMALLOC
  return malloc(size);
#else
  extern void* low_malloc_impl(size_t size);
  return low_malloc_impl(size);
#endif
}

inline void low_free(void* ptr) {
#ifndef USE_JEMALLOC
  free(ptr);
#else
  if (ptr) dallocx(ptr, low_dallocx_flags());
#endif
}

inline void low_malloc_huge_pages(int pages) {
#ifdef USE_JEMALLOC
  low_huge_pages = pages;
#endif
}

void low_malloc_skip_huge(void* start, void* end);

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
    assert(!m_ptr);
    m_ptr = ptr;
    return *this;
  }
 private:
  void* m_ptr;
};

extern __thread uintptr_t s_stackLimit;
extern __thread size_t s_stackSize;
void init_stack_limits(pthread_attr_t* attr);

extern const size_t s_pageSize;

/*
 * The numa node this thread is bound to
 */
extern __thread int32_t s_numaNode;
/*
 * enable the numa support in hhvm,
 * and determine whether threads should default to using
 * local memory.
 */
void enable_numa(bool local);
/*
 * Determine the node that the next thread should run on.
 */
int next_numa_node();
/*
 * Set the thread affinity, and the jemalloc arena for the current
 * thread.
 * Also initializes s_numaNode
 */
void set_numa_binding(int node);
/*
 * The number of numa nodes in the system
 */
int num_numa_nodes();
/*
 * Enable numa interleaving for the specified address range
 */
void numa_interleave(void* start, size_t size);
/*
 * Allocate the specified address range on the local node
 */
void numa_local(void* start, size_t size);
/*
 * Allocate the specified address range on the given node
 */
void numa_bind_to(void* start, size_t size, int node);

#ifdef USE_JEMALLOC

/**
 * jemalloc pprof utility functions.
 */
int jemalloc_pprof_enable();
int jemalloc_pprof_disable();
int jemalloc_pprof_dump(const std::string& prefix, bool force);

#endif // USE_JEMALLOC

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_UTIL_ALLOC_H_
