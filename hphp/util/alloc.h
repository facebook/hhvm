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
#include "hphp/util/low-ptr-def.h"

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
//   cold areans prefer addresses between 2G and 4G, to conserve space in the
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
extern int low_arena_flags;
extern int lower_arena_flags;
extern int low_cold_arena_flags;
extern int high_cold_arena_flags;
extern __thread int high_arena_flags;

void setup_local_arenas();

#if USE_JEMALLOC_EXTENT_HOOKS

// Explicit per-thread tcache for high arena.
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

void low_2m_pages(uint32_t pages);
void high_2m_pages(uint32_t pages);

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
 * Enable the numa support in hhvm.
 */
void enable_numa();
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

inline void* vm_cold_malloc(size_t size) {
#if USE_JEMALLOC_EXTENT_HOOKS
  if (!size) return nullptr;
  return mallocx(size, high_cold_arena_flags);
#else
  return malloc(size);
#endif
}

inline void vm_cold_free(void* ptr) {
#if USE_JEMALLOC_EXTENT_HOOKS
  if (ptr) dallocx(ptr, high_cold_arena_flags);
#else
  return free(ptr);
#endif
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

inline void* lower_malloc(size_t size) {
#if USE_JEMALLOC_EXTENT_HOOKS
  if (!size) return nullptr;
  return mallocx(size, lower_arena_flags);
#else
  return low_malloc(size);
#endif
}

inline void lower_free(void* ptr) {
#if USE_JEMALLOC_EXTENT_HOOKS
  if (ptr) dallocx(ptr, lower_arena_flags);
#else
  return low_free(ptr);
#endif
}

inline void* low_cold_malloc(size_t size) {
#if USE_JEMALLOC_EXTENT_HOOKS
  if (!size) return nullptr;
  return mallocx(size, low_cold_arena_flags);
#else
  return low_malloc(size);
#endif
}

inline void low_cold_free(void* ptr) {
#if USE_JEMALLOC_EXTENT_HOOKS
  if (ptr) dallocx(ptr, low_cold_arena_flags);
#else
  return low_free(ptr);
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
  template<class U>
  explicit LowAllocator(const LowAllocator<U>&) noexcept {}
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
struct LowerAllocator {
  using value_type = T;
  using pointer = T*;
  using const_pointer = const T*;

  template <class U>
  struct rebind { using other = LowerAllocator<U>; };

  LowerAllocator() noexcept {}
  template<class U>
  explicit LowerAllocator(const LowerAllocator<U>&) noexcept {}
  ~LowerAllocator() noexcept {}

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

  template<class U> bool operator==(const LowerAllocator<U>&) const {
    return true;
  }
  template<class U> bool operator!=(const LowerAllocator<U>&) const {
    return false;
  }
};

template <class T>
struct LowColdAllocator {
  using value_type = T;
  using pointer = T*;
  using const_pointer = const T*;

  template <class U>
  struct rebind { using other = LowColdAllocator<U>; };

  LowColdAllocator() noexcept {}
  template<class U>
  explicit LowColdAllocator(const LowColdAllocator<U>&) noexcept {}
  ~LowColdAllocator() noexcept {}

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

  template<class U> bool operator==(const LowColdAllocator<U>&) const {
    return true;
  }
  template<class U> bool operator!=(const LowColdAllocator<U>&) const {
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
struct VMColdAllocator {
  using value_type = T;
  using pointer = T*;
  using const_pointer = const T*;

  template <class U>
  struct rebind { using other = VMColdAllocator<U>; };

  VMColdAllocator() noexcept {}
  template<class U>
  explicit VMColdAllocator(const VMColdAllocator<U>&) noexcept {}
  ~VMColdAllocator() noexcept {}

  pointer allocate(size_t num) {
    return (pointer)vm_cold_malloc(num * sizeof(T));
  }
  void deallocate(pointer p, size_t num) {
    vm_cold_free((void*)p);
  }

  template<class U, class... Args>
  void construct(U* p, Args&&... args) {
    ::new ((void*)p) U(std::forward<Args>(args)...);
  }
  void destroy(pointer p) {
    p->~T();
  }

  template<class U> bool operator==(const VMColdAllocator<U>&) const {
    return true;
  }
  template<class U> bool operator!=(const VMColdAllocator<U>&) const {
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
