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
# if (JEMALLOC_VERSION_MAJOR >= 5) && defined(USE_LOWPTR) && \
     defined(__linux__) && !defined(USE_JEMALLOC_EXTENT_HOOKS)
#  define USE_JEMALLOC_EXTENT_HOOKS 1
#  if (JEMALLOC_VERSION_MAJOR > 5) || (JEMALLOC_VERSION_MINOR >= 1)
#   define JEMALLOC_METADATA_1G_PAGES 1
#  endif
# endif
# if (JEMALLOC_VERSION_MAJOR > 4)
#  define JEMALLOC_NEW_ARENA_CMD "arenas.create"
# else
#  define JEMALLOC_NEW_ARENA_CMD "arenas.extend"
# endif
#endif

extern "C" {
#ifdef USE_TCMALLOC
#define MallocExtensionInstance _ZN15MallocExtension8instanceEv
  MallocExtension* MallocExtensionInstance() __attribute__((__weak__));
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

// When this is true, all static/uncounted strings/arrays have addresses lower
// than kUncountedMaxAddr, and all counted HeapObjects have higher addresses.
constexpr bool use_addr_to_check_counted =
#if USE_JEMALLOC_EXTENT_HOOKS && defined(USE_ADDR_CHECK_COUNTED)
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

// Address ranges for the managed arenas.  Low arena is in [1G, 4G), and high
// arena in [4G, kUncountedMaxAddr) at most.  LOW_PTR builds won't work if low
// arena overflows.  High arena overflow would result in a crash, so we size it
// large enough to make sure we run out of memory before it overflows.  These
// constants are only meaningful when use_addr_to_check_counted is true (which
// currently depends on USE_JEMALLOC_EXTENT_HOOKS).  We make them available for
// all modes to avoid having ifdefs everywhere.
constexpr unsigned kUncountedMaxShift = 38;
constexpr uintptr_t kLowArenaMinAddr = 1ull << 30;
constexpr uintptr_t kLowArenaMaxAddr = 1ull << 32;
constexpr uintptr_t kUncountedMaxAddr = 1ull << kUncountedMaxShift;
constexpr uintptr_t kHighArenaMaxAddr = kUncountedMaxAddr;
constexpr size_t kLowArenaMaxCap = 3ull << 30;
constexpr size_t kHighArenaMaxCap = kHighArenaMaxAddr - kLowArenaMaxAddr;

#ifdef USE_JEMALLOC

// Low arena uses ManagedArena if extent hooks are used, otherwise it is using
// DSS.  It should always be available for supported versions of jemalloc.  High
// arena is 0 if extent hook API isn't used, but mallocx/dallocx could use 0 as
// flags and behave similarly to malloc/free.  Low arena doesn't use tcache, but
// we need tcache for the high arena, so the flags are thread-local.
extern unsigned low_arena;
extern unsigned high_arena;
extern int low_arena_flags;
extern __thread int high_arena_flags;

#if !USE_JEMALLOC_EXTENT_HOOKS

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

// Explicit per-thread tcache for the huge arenas.
extern __thread int high_arena_tcache;

/* Set up extent hooks to use 1g pages for jemalloc metadata. */
void setup_jemalloc_metadata_extent_hook(bool enable, bool enable_numa_arena,
                                         size_t reserved);

// Functions to manipulate tcaches for the high arena
void high_arena_tcache_create();        // tcache.create
void high_arena_tcache_flush();         // tcache.flush
void high_arena_tcache_destroy();       // tcache.destroy

#endif // USE_JEMALLOC_EXTENT_HOOKS

#endif // USE_JEMALLOC

void low_malloc_huge_pages(int pages);

inline void* malloc_huge_internal(size_t size) {
#if !USE_JEMALLOC_EXTENT_HOOKS
  return malloc(size);
#else
  assert(size);
  return mallocx(size, high_arena_flags);
#endif
}

inline void free_huge_internal(void* ptr) {
#if !USE_JEMALLOC_EXTENT_HOOKS
  free(ptr);
#else
  assert(ptr);
  dallocx(ptr, high_arena_flags);
#endif
}

inline void sized_free_huge_internal(void* ptr, size_t size) {
#if !USE_JEMALLOC_EXTENT_HOOKS
  free(ptr);
#else
  assert(ptr);
  assert(sallocx(ptr, high_arena_flags) == nallocx(size, high_arena_flags));
  sdallocx(ptr, size, high_arena_flags);
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

/*
 * The numa node this thread is bound to
 */
extern __thread int32_t s_numaNode;
/*
 * The optional preallocated space collocated with thread stack.
 */
extern __thread MemBlock s_tlSpace;
/*
 * The part of thread stack and s_tlSpace that lives on huge pages.  It could be
 * empty if huge page isn't used for this thread.
 */
extern __thread MemBlock s_hugeRange;

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
template <typename T, bool ErrOK>
int mallctlHelper(const char *cmd, T* out, T* in) {
#ifdef USE_JEMALLOC
  size_t outLen = sizeof(T);
  int err = mallctl(cmd,
                    out, out ? &outLen : nullptr,
                    in, in ? sizeof(T) : 0);
  assert(err != 0 || outLen == sizeof(T));
#else
  int err = ENOENT;
#endif
  if (!ErrOK && err != 0) {
    char msg[128];
    snprintf(msg, sizeof(msg), "mallctl %s failed with error %d", cmd, err);
    throw std::runtime_error{msg};
  }
  return err;
}

template <typename T, bool ErrOK = false>
int mallctlReadWrite(const char *cmd, T* out, T in) {
  return mallctlHelper<T, ErrOK>(cmd, out, &in);
}

template <typename T, bool ErrOK = false>
int mallctlRead(const char* cmd, T* out) {
  return mallctlHelper<T, ErrOK>(cmd, out, static_cast<T*>(nullptr));
}

template <typename T, bool ErrOK = false>
int mallctlWrite(const char* cmd, T in) {
  return mallctlHelper<T, ErrOK>(cmd, static_cast<T*>(nullptr), &in);
}

template <bool ErrOK = false> int mallctlCall(const char* cmd) {
  // Use <unsigned> rather than <void> to avoid sizeof(void).
  return mallctlHelper<unsigned, ErrOK>(cmd, nullptr, nullptr);
}

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

inline void vm_sized_free(void* ptr, size_t size) {
  return sized_free_huge_internal(ptr, size);
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

inline void uncounted_sized_free(void* ptr, size_t size) {
  return sized_free_huge_internal(ptr, size);
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

inline void apc_sized_free(void* ptr, size_t size) {
  return sized_free_huge_internal(ptr, size);
}

inline void* low_malloc(size_t size) {
#ifndef USE_JEMALLOC
  return malloc(size);
#else
  if (!size) return nullptr;
  auto ptr = mallocx(size, low_arena_flags);
#ifndef USE_LOWPTR
  // low_malloc isn't required to return 32-bit addresses, but we still want to
  // make sure it is below kUncountedMaxAddr, when ManagedArena is used.
  if (!ptr) {
    return uncounted_malloc(size);
  }
#endif
  return ptr;
#endif
}

inline void low_free(void* ptr) {
#ifndef USE_JEMALLOC
  free(ptr);
#else
  if (ptr) dallocx(ptr, low_arena_flags);
#endif
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

  pointer allocate(size_t num) {
    if (!num) return nullptr;
    return (pointer)vm_malloc(num * sizeof(T));
  }
  void deallocate(pointer p, size_t num) {
    if (!p) return;
    vm_sized_free((void*)p, num * sizeof(T));
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
    if (!num) return nullptr;
    return (pointer)apc_malloc(num * sizeof(T));
  }
  void deallocate(pointer p, size_t num) {
    if (!p) return;
    apc_sized_free((void*)p, num * sizeof(T));
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
