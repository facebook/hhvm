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

#include <array>
#include <atomic>

#include <stdint.h>

#include <folly/CPortability.h>
#include <folly/Portability.h>
#include <folly/portability/PThread.h>

#include "hphp/util/assertions.h"
#include "hphp/util/exception.h"

#if FOLLY_SANITIZE
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
# if (JEMALLOC_VERSION_MAJOR == 5) && defined(__linux__)
#  define USE_JEMALLOC_EXTENT_HOOKS 1
#  ifdef FACEBOOK
// Requires customizable extent hooks on arena 0 (will ship in jemalloc 5.1)
#   define USE_JEMALLOC_METADATA_1G_PAGES 1
#  endif
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
#if FOLLY_SANITIZE
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

#if !USE_JEMALLOC_EXTENT_HOOKS
extern unsigned dss_arena;
extern std::atomic<int> low_huge_pages;

#else // USE_JEMALLOC_EXTENT_HOOKS

#ifndef MAX_MANAGED_ARENA_COUNT
#define MAX_MANAGED_ARENA_COUNT 4
#endif
static_assert(MAX_MANAGED_ARENA_COUNT >= 1, "");
// All ManagedArena's represented as an array of pair<id, pointer>.  Each
// pointer can be casted to the underlying ExtentAllocator/Arena. We use this
// to access the state of ExtentAllocators in extent hooks.  An id of zero
// indicates an empty entry.
using ArenaArray = std::array<std::pair<unsigned, void*>,
                              MAX_MANAGED_ARENA_COUNT>;
extern ArenaArray g_arenas;
template<typename T> inline T* GetByArenaId(unsigned id) {
  for (auto i : g_arenas) {
    if (i.first == id) {
      return static_cast<T*>(i.second);
    }
  }
  return nullptr;
}

extern unsigned low_arena;
extern unsigned high_arena;

// Address ranges for the managed arenas.  Low arena is in [1G, 4G), and high
// arena in [4G, 64G) at most.  Both grows down and can be smaller.  But things
// won't work well if either overflows.
constexpr uintptr_t kLowArenaMaxAddr = 4ull << 30;
constexpr uintptr_t kUncountedMaxAddr = 64ull << 30;
constexpr uintptr_t kHighArenaMaxAddr = kUncountedMaxAddr;
constexpr size_t kLowArenaMaxCap = 3ull << 30;
constexpr size_t kHighArenaMaxCap = kUncountedMaxAddr - kLowArenaMaxAddr;

// Explicit per-thread tcache for the huge arenas.
extern __thread int high_arena_tcache;

inline int mallocx_huge_flags() {
  return MALLOCX_ARENA(high_arena) | MALLOCX_TCACHE(high_arena_tcache);
}

inline int dallocx_huge_flags() {
  return MALLOCX_TCACHE(high_arena_tcache);
}

/* Set up extent hooks to use 1g pages for jemalloc metadata. */
void setup_jemalloc_metadata_extent_hook(bool enable, bool enable_numa_arena,
                                         size_t reserved);

// Functions to manipulate tcaches for the high arena
void high_arena_tcache_create();        // tcache.create
void high_arena_tcache_flush();         // tcache.flush
void high_arena_tcache_destroy();       // tcache.destroy

#endif // USE_JEMALLOC_EXTENT_HOOKS

inline int low_mallocx_flags() {
  // Allocate from low_arena if extend hooks are available, otherwise allocate
  // from dss_arena.  Bypass the implicit tcache to assure that the result
  // actually comes from the desired arena.
#ifdef USE_JEMALLOC_EXTENT_HOOKS
  return MALLOCX_ARENA(low_arena) | MALLOCX_TCACHE_NONE;
#elif defined(MALLOCX_TCACHE_NONE)
  return MALLOCX_ARENA(dss_arena) | MALLOCX_TCACHE_NONE;
#else
  return MALLOCX_ARENA(dss_arena);
#endif
}

inline int low_dallocx_flags() {
#ifdef MALLOCX_TCACHE_NONE
  return MALLOCX_TCACHE_NONE;
#else
  return MALLOCX_ARENA(dss_arena);
#endif
}

#endif // USE_JEMALLOC

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

void low_malloc_huge_pages(int pages);

inline void* malloc_huge_internal(size_t size) {
#if !USE_JEMALLOC_EXTENT_HOOKS
  return malloc(size);
#else
  extern void* malloc_huge_impl(size_t);
  return malloc_huge_impl(size);
#endif
}

inline void free_huge_internal(void* ptr) {
#ifndef USE_JEMALLOC_EXTENT_HOOKS
  free(ptr);
#else
  if (LIKELY(reinterpret_cast<uintptr_t>(ptr) < kHighArenaMaxAddr)) {
    if (ptr) dallocx(ptr, dallocx_huge_flags());
  } else {
    // Not from the high 1G arena
    free(ptr);
  }
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

inline void* safe_aligned_alloc(size_t align, size_t size) {
  auto p = aligned_alloc(align, size);
  if (!p) throw OutOfMemoryException(size);
  return p;
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

// POD type for tracking arbitrary memory ranges
template<class T> struct MemRange {
  T ptr;
  size_t size; // bytes
};

using MemBlock = MemRange<void*>;

extern __thread uintptr_t s_stackLimit;
extern __thread size_t s_stackSize;
void init_stack_limits(pthread_attr_t* attr);

extern const size_t s_pageSize;
extern unsigned s_hugeStackSizeKb;     // RuntimeOption "Server.HugeStackSizeKb"

/*
 * The numa node this thread is bound to
 */
extern __thread int32_t s_numaNode;
/*
 * The optional preallocated first slab
 */
extern __thread MemBlock s_firstSlab;
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
 * Allocate on a specific NUMA node, with alignment requirement.
 */
void* mallocx_on_node(size_t size, int node, size_t align);

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
      always_assert(false);
    }
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


// For allocation of VM data.
inline void* vm_malloc(size_t size) {
  return malloc_huge_internal(size);
}

inline void vm_free(void* ptr) {
  return free_huge_internal(ptr);
}

// Allocations that are guaranteed to live below kUncountedMaxAddr when
// USE_JEMALLOC_EXTENT_HOOKS.  This provides a new way to check for countedness
// for arrays and strings.
inline void* uncounted_malloc(size_t size) {
  return malloc_huge_internal(size);
}

inline void uncounted_free(void* ptr) {
  return free_huge_internal(ptr);
}

// Allocations for the APC but do not necessarily live below kUncountedMaxAddr,
// e.g., APCObject, or the hash table.  Currently they live below
// kUncountedMaxAddr anyway, but this may change later.
inline void* apc_malloc(size_t size) {
  return malloc_huge_internal(size);
}

inline void apc_free(void* ptr) {
  return free_huge_internal(ptr);
}

template <class T>
struct LowAllocator {
  using value_type = T;
  using pointer = T*;
  using const_pointer = const T*;

  template <class U>
  struct rebind { using other = LowAllocator<U>; };

  LowAllocator() noexcept {}
  template<class U> LowAllocator(const LowAllocator<U>&) noexcept {}
  ~LowAllocator() noexcept {}

  pointer allocate(size_t num) {
    return (pointer)low_malloc(num * sizeof(T));
  }
  void deallocate(pointer p, size_t /*num*/) {
    low_free((void*)p);
  }

  template<class U, class... Args>
  void construct(U* p, Args&&... args) {
    ::new ((void*)p) U(std::forward<Args>(args)...);
  }
  void destroy(pointer p) {
    p->~T();
  }

  template<class U> bool operator==(const LowAllocator<U>&) const {
    return true;
  }
  template<class U> bool operator!=(const LowAllocator<U>&) const {
    return false;
  }
};

template <class T>
struct VMAllocator {
  using value_type = T;
  using pointer = T*;
  using const_pointer = const T*;

  template <class U>
  struct rebind { using other = VMAllocator<U>; };

  VMAllocator() noexcept {}
  template<class U> explicit VMAllocator(const VMAllocator<U>&) noexcept {}
  ~VMAllocator() noexcept {}

  pointer allocate(size_t num, const void* = nullptr) {
    pointer ret = (pointer)vm_malloc(num * sizeof(T));
    return ret;
  }
  void deallocate(pointer p, size_t /*num*/) {
    vm_free((void*)p);
  }

  template<class U, class... Args>
  void construct(U* p, Args&&... args) {
    ::new ((void*)p) U(std::forward<Args>(args)...);
  }
  void destroy(pointer p) {
    p->~T();
  }

  template<class U> bool operator==(const VMAllocator<U>&) const {
    return true;
  }
  template<class U> bool operator!=(const VMAllocator<U>&) const {
    return false;
  }
};

template <class T>
struct APCAllocator {
  using value_type = T;
  using pointer = T*;
  using const_pointer = const T*;

  template <class U>
  struct rebind { using other = APCAllocator<U>; };

  APCAllocator() noexcept {}
  template<class U> explicit APCAllocator(const APCAllocator<U>&) noexcept {}
  ~APCAllocator() noexcept {}

  pointer allocate(size_t num, const void* = nullptr) {
    return (pointer)apc_malloc(num * sizeof(T));
  }
  void deallocate(pointer p, size_t /*num*/) {
    apc_free((void*)p);
  }

  template<class U, class... Args>
  void construct(U* p, Args&&... args) {
    ::new ((void*)p) U(std::forward<Args>(args)...);
  }
  void destroy(pointer p) {
    p->~T();
  }

  template<class U> bool operator==(const APCAllocator<U>&) const {
    return true;
  }
  template<class U> bool operator!=(const APCAllocator<U>&) const {
    return false;
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_UTIL_ALLOC_H_
