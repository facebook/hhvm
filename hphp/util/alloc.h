/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include <folly/portability/PThread.h>

#include "hphp/util/assertions.h"
#include "hphp/util/exception.h"

#if defined(FOLLY_SANITIZE_ADDRESS) || defined(FOLLY_SANITIZE_THREAD)
// ASan is less precise than valgrind so we'll need a superset of those tweaks
# define VALGRIND
// TODO: (t2869817) ASan doesn't play well with jemalloc
# ifdef USE_JEMALLOC
#  undef USE_JEMALLOC
# endif
#endif

#ifdef USE_TCMALLOC
#include <gperftools/malloc_extension.h>
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
# if (JEMALLOC_VERSION_MAJOR == 4) && defined(__linux__)
#  define USE_JEMALLOC_CHUNK_HOOKS 1
# elif (JEMALLOC_VERSION_MAJOR == 5) && defined(__linux__)
#  define USE_JEMALLOC_EXTENT_HOOKS 1
# endif
// Enable with either chunk_hooks or extent_hooks
# if defined(USE_JEMALLOC_CHUNK_HOOKS) || defined(USE_JEMALLOC_EXTENT_HOOKS)
#  define USE_JEMALLOC_CUSTOM_HOOKS 1
# endif
# if (JEMALLOC_VERSION_MAJOR > 4)
#  define JEMALLOC_NEW_ARENA_CMD "arenas.create"
# else
#  define JEMALLOC_NEW_ARENA_CMD "arenas.extend"
# endif
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

constexpr bool use_jemalloc =
#ifdef USE_JEMALLOC
  true
#else
  false
#endif
  ;

// ASAN modifies the generated code in ways that cause abnormally high C++
// stack usage.
constexpr size_t kStackSizeMinimum =
#ifdef FOLLY_SANITIZE_ADDRESS
  16 << 20;
#else
  8 << 20;
#endif

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
  return MALLOCX_ARENA(low_arena) | MALLOCX_TCACHE_NONE;
#else
  return MALLOCX_ARENA(low_arena);
#endif
}

#ifdef MALLOCX_TCACHE_NONE
inline constexpr int low_dallocx_flags() {
  // Bypass the implicit tcache for this deallocation.
  return MALLOCX_TCACHE_NONE;
}
#else
inline int low_dallocx_flags() {
  // Prior to the introduction of MALLOCX_TCACHE_NONE, explicitly specifying
  // MALLOCX_ARENA(a) caused jemalloc to bypass tcache.
  return MALLOCX_ARENA(low_arena);
}
#endif


#ifdef USE_JEMALLOC_CUSTOM_HOOKS
extern unsigned low_huge1g_arena;
extern unsigned high_huge1g_arena;

// Explicit per-thread tcache for the huge arenas.
extern __thread int high_huge1g_tcache;

inline int low_mallocx_huge1g_flags() {
  // MALLOCX_TCACHE_NONE is introduced earlier than the chunk hook API
  return MALLOCX_ARENA(low_huge1g_arena) | MALLOCX_TCACHE_NONE;
}

inline constexpr int low_dallocx_huge1g_flags() {
  return MALLOCX_TCACHE_NONE;
}

inline int mallocx_huge1g_flags() {
  return MALLOCX_ARENA(high_huge1g_arena) | MALLOCX_TCACHE(high_huge1g_tcache);
}

inline int dallocx_huge1g_flags() {
  return MALLOCX_TCACHE(high_huge1g_tcache);
}

// Functions to manipulate tcaches for huge arenas
void thread_huge_tcache_create();       // tcache.create
void thread_huge_tcache_flush();        // tcache.flush
void thread_huge_tcache_destroy();      // tcache.destroy

#endif

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

inline void* low_malloc_data(size_t size) {
#ifndef USE_JEMALLOC_CUSTOM_HOOKS
  return low_malloc(size);
#else
  extern void* low_malloc_huge1g_impl(size_t);
  return low_malloc_huge1g_impl(size);
#endif
}

inline void low_free_data(void* ptr) {
#ifndef USE_JEMALLOC_CUSTOM_HOOKS
  low_free(ptr);
#else
  if (ptr) dallocx(ptr, low_dallocx_huge1g_flags());
#endif
}

inline void* malloc_huge(size_t size) {
#ifndef USE_JEMALLOC_CUSTOM_HOOKS
  return malloc(size);
#else
  extern void* malloc_huge1g_impl(size_t);
  return malloc_huge1g_impl(size);
#endif
}

inline void free_huge(void* ptr) {
#ifndef USE_JEMALLOC_CUSTOM_HOOKS
  free(ptr);
#else
  if (ptr) dallocx(ptr, dallocx_huge1g_flags());
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
 * Free all unused memory back to system. On error, returns false and, if
 * not null, sets an error message in *errStr.
 */
bool purge_all(std::string* errStr = nullptr);

/**
 * Like scoped_ptr, but calls free() on destruct
 */
struct ScopedMem {
 private:
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
 * Set the thread affinity, and the jemalloc arena for the current
 * thread.
 * Also initializes s_numaNode
 */
void set_numa_binding(int node);

/*
 * mallctl wrappers.
 */

/*
 * Call mallctl, reading/writing values of type <T> if out/in are non-null,
 * respectively.  Assert/log on error, depending on errOk.
 */
template <typename T>
int mallctlHelper(const char *cmd, T* out, T* in, bool errOk) {
#ifdef USE_JEMALLOC
  assert(mallctl != nullptr);
  size_t outLen = sizeof(T);
  int err = mallctl(cmd,
                    out, out ? &outLen : nullptr,
                    in, in ? sizeof(T) : 0);
  assert(err != 0 || outLen == sizeof(T));
#else
  int err = ENOENT;
#endif
  if (err != 0) {
    if (!errOk) {
      std::string errStr =
        folly::format("mallctl {}: {} ({})", cmd, strerror(err), err).str();
      // Do not use Logger here because JEMallocInitializer() calls this
      // function and JEMallocInitializer has the highest constructor priority.
      // The static variables in Logger are not initialized yet.
      fprintf(stderr, "%s\n", errStr.c_str());
    }
    always_assert(errOk || err == 0);
  }
  return err;
}

template <typename T>
int mallctlReadWrite(const char *cmd, T* out, T in, bool errOk=false) {
  return mallctlHelper(cmd, out, &in, errOk);
}

template <typename T>
int mallctlRead(const char* cmd, T* out, bool errOk=false) {
  return mallctlHelper(cmd, out, static_cast<T*>(nullptr), errOk);
}

template <typename T>
int mallctlWrite(const char* cmd, T in, bool errOk=false) {
  return mallctlHelper(cmd, static_cast<T*>(nullptr), &in, errOk);
}

int mallctlCall(const char* cmd, bool errOk=false);

/*
 * jemalloc pprof utility functions.
 */
int jemalloc_pprof_enable();
int jemalloc_pprof_disable();
int jemalloc_pprof_dump(const std::string& prefix, bool force);

template <class T>
struct LowAllocator {
  typedef T              value_type;
  typedef T*             pointer;
  typedef const T*       const_pointer;
  typedef T&             reference;
  typedef const T&       const_reference;
  typedef std::size_t    size_type;
  typedef std::ptrdiff_t difference_type;

  template <class U>
  struct rebind { using other = LowAllocator<U>; };

  pointer address(reference value) {
    return &value;
  }
  const_pointer address(const_reference value) const {
    return &value;
  }

  LowAllocator() noexcept {}
  template<class U> LowAllocator(const LowAllocator<U>&) noexcept {}
  ~LowAllocator() noexcept {}

  size_type max_size() const {
    return std::numeric_limits<std::size_t>::max() / sizeof(T);
  }

  pointer allocate(size_type num, const void* = nullptr) {
    pointer ret = (pointer)low_malloc_data(num * sizeof(T));
    return ret;
  }

  template<class U, class... Args>
  void construct(U* p, Args&&... args) {
    ::new ((void*)p) U(std::forward<Args>(args)...);
  }

  void destroy(pointer p) {
    p->~T();
  }

  void deallocate(pointer p, size_type /*num*/) { low_free_data((void*)p); }

  template<class U> bool operator==(const LowAllocator<U>&) const {
    return true;
  }

  template<class U> bool operator!=(const LowAllocator<U>&) const {
    return false;
  }
};

template <class T>
struct HugeAllocator {
  using value_type = T;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  template <class U>
  struct rebind { using other = HugeAllocator<U>; };

  pointer address(reference value) {
    return &value;
  }
  const_pointer address(const_reference value) const {
    return &value;
  }

  HugeAllocator() noexcept {}
  template<class U> explicit HugeAllocator(const HugeAllocator<U>&) noexcept {}
  ~HugeAllocator() noexcept {}

  size_type max_size() const {
    return std::numeric_limits<std::size_t>::max() / sizeof(T);
  }

  pointer allocate(size_type num, const void* = nullptr) {
    pointer ret = (pointer)malloc_huge(num * sizeof(T));
    return ret;
  }

  template<class U, class... Args>
  void construct(U* p, Args&&... args) {
    ::new ((void*)p) U(std::forward<Args>(args)...);
  }

  void destroy(pointer p) {
    p->~T();
  }

  void deallocate(pointer p, size_type /*num*/) { free_huge((void*)p); }

  template<class U> bool operator==(const HugeAllocator<U>&) const {
    return true;
  }

  template<class U> bool operator!=(const HugeAllocator<U>&) const {
    return false;
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_UTIL_ALLOC_H_
