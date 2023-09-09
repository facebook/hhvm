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

#include "hphp/util/address-range.h"
#include "hphp/util/alloc-defs.h"
#include "hphp/util/assertions.h"
#include "hphp/util/exception.h"
#include "hphp/util/jemalloc-util.h"
#include "hphp/util/low-ptr-def.h"
#include "hphp/util/read-only-arena.h"
#include "hphp/util/slab-manager.h"

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

struct OutOfMemoryException : Exception {
  explicit OutOfMemoryException(size_t size)
    : Exception("Unable to allocate %zu bytes of memory", size) {}
  EXCEPTION_COMMON_IMPL(OutOfMemoryException);
};

///////////////////////////////////////////////////////////////////////////////


#ifdef USE_JEMALLOC

// When jemalloc 5 and above is used, we use the extent hooks to create the
// following arenas, to gain detailed control over address space, huge page
// mapping, and data layout.
//
// - low arena, lower arena, and low cold arena try to give addresses that fit
//   in 32 bits. Use lower arena when 31-bit address is preferred, and when we
//   want to make full use of the huge pages there (if present). low and low
//   cold arenas prefer addresses between 2G and 4G, to conserve space in the
//   lower range. These are just preferences, all these arenas are able to use
//   spare space in the 1G to 4G region, when the preferred range is used up. In
//   LOWPTR builds, running out of space in any of the low arenas will cause a
//   crash (we hope).
//
// - high arena and high cold arena span addresses from 4G to kHighArenaMaxAddr.
//   It is currently used for some VM metadata and APC (the table, and all
//   uncounted data). high_cold_arena can be used for global cold data. We don't
//   expect to run out of memory in the high arenas.
//
// - local arena only exists in some threads, mostly for data that is not
//   accessed by other threads. In some threads, local arena is 0, and the
//   automatic arena is used in that case.
//
// A cold arena shares an address range with its hotter counterparts, but
// tries to give separte address ranges. This is done by allocating from higher
// address downwards, while the hotter ones go from lower address upwards.
//
// Some prior experiments showed that high_arena needs tcache, due to spikiness
// in APC-related memory allocation and deallocation behaviors. Other arenas
// shouldn't need tcache.
//
// With earlier jemalloc versions, only the lower arena exists (using dss), and
// low arena and low cold arena alias to lower arena. Allocations in the high
// arenas are served using default malloc(), and no assumption about the
// resulting address range can be made.

extern unsigned low_arena;
extern unsigned lower_arena;
extern unsigned low_cold_arena;
extern unsigned high_arena;
extern unsigned high_cold_arena;
extern __thread unsigned local_arena;

extern int low_arena_flags;
extern int lower_arena_flags;
extern int low_cold_arena_flags;
extern int high_cold_arena_flags;
extern int high_arena_flags;
extern __thread int local_arena_flags;

struct PageSpec {
  unsigned n1GPages{0};
  unsigned n2MPages{0};
};

void setup_local_arenas(PageSpec, unsigned slabs);
unsigned get_local_arena(uint32_t node);
SlabManager* get_local_slab_manager(uint32_t node);
void shutdown_slab_managers();

void setup_arena0(PageSpec);

#if USE_JEMALLOC_EXTENT_HOOKS

// Explicit per-thread tcache for high arena.
extern __thread int high_arena_tcache;

/* Set up extent hooks to use 1g pages for jemalloc metadata. */
void setup_jemalloc_metadata_extent_hook(bool enable, bool enable_numa_arena,
                                         size_t reserved);

// Functions to run upon thread creation/flush/exit.
void arenas_thread_init();
void arenas_thread_flush();
void arenas_thread_exit();

#endif // USE_JEMALLOC_EXTENT_HOOKS

#endif // USE_JEMALLOC

/**
 * Get the number of bytes held by the slab managers, but are free for request
 * use.
 *
 * The value is calculated using relaxed atomic adds and subs, and may become
 * negative at moments due to the unpredictable memory ordering.
 */
ssize_t get_free_slab_bytes();

void low_2m_pages(uint32_t pages);
void high_2m_pages(uint32_t pages);

void set_cold_file_dir(const char* dir);
void enable_high_cold_file();

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
 * Get the number of bytes that could be purged via `purge_all()`.
 * JEMalloc holds pages in three states:
 *   - active: In use by the application
 *   - dirty: Held by JEMalloc for future allocations
 *   - muzzy: madvise(FREE) but not madvised(DONTNEED), so mapping may still
 *            exist, but kernel could reclaim if necessary
 * By default pages spend 10s in dirty state after being freed up, and then
 * move to muzzy state for an additional 10s prior to being
 * `madvise(DONTNEED)`.  This function reports the number of bytes that are in
 * the dirty state.  These are bytes unusable by the kernel, but also unused by
 * the application.  A force purge will make JEMalloc `madvise(DONTNEED)` these
 * pages immediately.
 */
ssize_t purgeable_bytes();

/**
 * Instruct the kernel to free parts of the unused stack back to the system.
 * Like flush_thread_caches, this is called when the thread has been idle
 * and predicted to continue to be idle for a while.
 */
void flush_thread_stack();

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
 * Set the thread affinity, and the jemalloc arena for the current
 * thread.
 * Also initializes s_numaNode
 */
void set_numa_binding(int node);
/*
 * Allocate on a specific NUMA node, with alignment requirement.
 */
void* mallocx_on_node(size_t size, int node, size_t align);

///////////////////////////////////////////////////////////////////////////////

// Helpers (malloc, free, sized_free) to allocate/deallocate on a specific arena
// given flags. When not using event hooks, fallback version is used. `fallback`
// can be empty, in which case generic malloc/free will be used when not using
// extent hooks. These functions will crash with 0-sized alloc/deallocs.
#if USE_JEMALLOC_EXTENT_HOOKS
#define DEF_ALLOC_FUNCS(prefix, flag, fallback)                 \
  inline void* prefix##_malloc(size_t size) {                   \
    assert(size != 0);                                          \
    return mallocx(size, flag);                                 \
  }                                                             \
  inline void prefix##_free(void* ptr) {                        \
    assert(ptr != nullptr);                                     \
    return dallocx(ptr, flag);                                  \
  }                                                             \
  inline void* prefix##_realloc(void* ptr, size_t size) {       \
    assert(size != 0);                                          \
    return rallocx(ptr, size, flag);                            \
  }                                                             \
  inline void prefix##_sized_free(void* ptr, size_t size) {     \
    assert(ptr != nullptr);                                     \
    assert(sallocx(ptr, flag) == nallocx(size, flag));          \
    return sdallocx(ptr, size, flag);                           \
  }
#else
#define DEF_ALLOC_FUNCS(prefix, flag, fallback)                 \
  inline void* prefix##_malloc(size_t size) {                   \
    return fallback##malloc(size);                              \
  }                                                             \
  inline void prefix##_free(void* ptr) {                        \
    return fallback##free(ptr);                                 \
  }                                                             \
  inline void* prefix##_realloc(void* ptr, size_t size) {       \
    assert(size != 0);                                          \
    return fallback##realloc(ptr, size);                        \
  }                                                             \
  inline void prefix##_sized_free(void* ptr, size_t size) {     \
    return fallback##free(ptr);                                 \
  }
#endif

#if USE_JEMALLOC_EXTENT_HOOKS
#define HIGH_ARENA_FLAGS (high_arena_flags | MALLOCX_TCACHE(high_arena_tcache))
#else
#define HIGH_ARENA_FLAGS 0
#endif

DEF_ALLOC_FUNCS(vm, HIGH_ARENA_FLAGS, )
DEF_ALLOC_FUNCS(vm_cold, high_cold_arena_flags, )

// Allocations that are guaranteed to live below kUncountedMaxAddr when
// USE_JEMALLOC_EXTENT_HOOKS. This provides a new way to check for countedness
// for arrays and strings.
DEF_ALLOC_FUNCS(uncounted, HIGH_ARENA_FLAGS, )

// Allocations for the APC but do not necessarily live below kUncountedMaxAddr,
// e.g., APCObject, or the hash table. Currently they live below
// kUncountedMaxAddr anyway, but this may change later.
DEF_ALLOC_FUNCS(apc, HIGH_ARENA_FLAGS, )

// Thread-local allocations that are not accessed outside the thread.
DEF_ALLOC_FUNCS(local, local_arena_flags, )

// Low arena is always present when jemalloc is used, even when arena hooks are
// not used.
inline void* low_malloc(size_t size) {
#ifndef USE_JEMALLOC
  return malloc(size);
#else
  assert(size);
  return mallocx(size, low_arena_flags);
#endif
}

inline void low_free(void* ptr) {
#ifndef USE_JEMALLOC
  free(ptr);
#else
  assert(ptr);
  dallocx(ptr, low_arena_flags);
#endif
}

inline void* low_realloc(void* ptr, size_t size) {
#ifndef USE_JEMALLOC
  return realloc(ptr, size);
#else
  assert(ptr);
  return rallocx(ptr, size, low_arena_flags);
#endif
}

inline void low_sized_free(void* ptr, size_t size) {
#ifndef USE_JEMALLOC
  free(ptr);
#else
  assert(ptr);
  sdallocx(ptr, size, low_arena_flags);
#endif
}

// lower arena and low_cold arena alias low arena when extent hooks are not
// used.
DEF_ALLOC_FUNCS(lower, lower_arena_flags, low_)
DEF_ALLOC_FUNCS(low_cold, low_cold_arena_flags, low_)

#undef DEF_ALLOC_FUNCS

// General purpose adaptor that wraps allocation and sized deallocation function
// into an allocator that works with STL-stype containers.
template <void* (*AF)(size_t), void (*DF)(void*, size_t), typename T>
struct WrapAllocator {
  using value_type = T;
  using reference = T&;
  using const_reference = const T&;
  using pointer = T*;
  using const_pointer = const T*;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  template <class U> struct rebind { using other = WrapAllocator<AF, DF, U>; };

  WrapAllocator() noexcept {}
  template<class U>
  explicit WrapAllocator(const WrapAllocator<AF, DF, U>&) noexcept {}
  ~WrapAllocator() noexcept {}

  pointer allocate(size_t num) {
    if (num == 0) return nullptr;
    return (pointer)AF(num * sizeof(T));
  }
  void deallocate(pointer p, size_t num) {
    if (p == nullptr) return;
    DF((void*)p, num * sizeof(T));
  }
  template<class U, class... Args>
  void construct(U* p, Args&&... args) {
    ::new ((void*)p) U(std::forward<Args>(args)...);
  }
  void destroy(pointer p) {
    p->~T();
  }
  template<class U> bool operator==(const WrapAllocator<AF, DF, U>&) const {
    return true;
  }
  template<class U> bool operator!=(const WrapAllocator<AF, DF, U>&) const {
    return false;
  }
};

template<typename T> using LowAllocator =
  WrapAllocator<low_malloc, low_sized_free, T>;
template<typename T> using LowerAllocator =
  WrapAllocator<lower_malloc, lower_sized_free, T>;
template<typename T> using LowColdAllocator =
  WrapAllocator<low_cold_malloc, low_cold_sized_free, T>;
template<typename T> using VMAllocator =
  WrapAllocator<vm_malloc, vm_sized_free, T>;
template<typename T> using VMColdAllocator =
  WrapAllocator<vm_cold_malloc, vm_cold_sized_free, T>;
template<typename T> using APCAllocator =
  WrapAllocator<apc_malloc, apc_sized_free, T>;
template<typename T> using LocalAllocator =
  WrapAllocator<local_malloc, local_sized_free, T>;

// Per-thread buffer for global data, using a bump allocator.
using TLStaticArena = ReadOnlyArena<LowerAllocator<char>, true, 8>;
extern __thread TLStaticArena* tl_static_arena;
extern bool s_enable_static_arena;

inline void* static_alloc(size_t size) {
  if (tl_static_arena) return tl_static_arena->allocate(size);
  return lower_malloc(size);
}

// This can only free the memory allocated using static_alloc(), immediately
// after allocation, and it must happen in the same thread where allocation
// happens.
inline void static_try_free(void* ptr, size_t size) {
  if (tl_static_arena) return tl_static_arena->deallocate(ptr, size);
  return lower_sized_free(ptr, size);
}

template<typename T, typename... Args>
inline LowPtr<T> static_new(Args&&... args) {
  void* p = static_alloc(sizeof(T));
  return new (p) T(std::forward<Args>(args)...);
}

using SwappableReadonlyArena = ReadOnlyArena<VMColdAllocator<char>, false, 8>;
void setup_swappable_readonly_arena(uint32_t chunk_size);
SwappableReadonlyArena* get_swappable_readonly_arena();

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_UTIL_ALLOC_H_
