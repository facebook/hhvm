/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_MEMORY_MANAGER_INL_H
#define incl_HPHP_MEMORY_MANAGER_INL_H

#include <limits>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

static_assert(
  kMaxSmartSize <= std::numeric_limits<uint32_t>::max(),
  "Size-specified smart malloc functions assume this"
);

//////////////////////////////////////////////////////////////////////

inline MemoryManager& MM() { return *MemoryManager::TlsWrapper::getNoCheck(); }

template<class T, class... Args> T* smart_new(Args&&... args) {
  auto const mem = MM().smartMallocSize(sizeof(T));
  try {
    return new (mem) T(std::forward<Args>(args)...);
  } catch (...) {
    MM().smartFreeSize(mem, sizeof(T));
    throw;
  }
}

template<class T> void smart_delete(T* t) {
  t->~T();
  MM().smartFreeSize(t, sizeof *t);
}

template<class T> T* smart_new_array(size_t count) {
  T* ret = static_cast<T*>(smart_malloc(count * sizeof(T)));
  size_t i = 0;
  try {
    for (; i < count; ++i) {
      new (&ret[i]) T();
    }
  } catch (...) {
    size_t j = i;
    while (j-- > 0) {
      ret[j].~T();
    }
    smart_free(ret);
    throw;
  }
  return ret;
}

template<class T>
void smart_delete_array(T* t, size_t count) {
  size_t i = count;
  while (i-- > 0) {
    t[i].~T();
  }
  smart_free(t);
}

//////////////////////////////////////////////////////////////////////

struct MemoryManager::MaskAlloc {
  explicit MaskAlloc(MemoryManager& mm) : m_mm(mm) {
    // capture all mallocs prior to construction
    m_mm.refreshStats();
  }
  ~MaskAlloc() {
#ifdef USE_JEMALLOC
    // exclude mallocs and frees since construction
    if (s_statsEnabled) {
      m_mm.m_prevAllocated = int64_t(*m_mm.m_allocated);
      m_mm.m_delta = int64_t(*m_mm.m_allocated) -
        int64_t(*m_mm.m_deallocated);
    }
#endif
  }

  MaskAlloc(const MaskAlloc&) = delete;
  MaskAlloc& operator=(const MaskAlloc&) = delete;

private:
  MemoryManager& m_mm;
};

//////////////////////////////////////////////////////////////////////

struct MemoryManager::SmallNode {
  size_t padbytes;  // <= kMaxSmartSize means small block
};

/*
 * Debug mode header.
 *
 * For size-untracked allocations, this sits in front of the user
 * payload for small allocations, and in front of the SweepNode in
 * big allocations.  The allocatedMagic aliases the space for the
 * FreeList::Node pointers, but should catch double frees due to
 * kAllocatedMagic.
 *
 * For size-tracked allocations, this always sits in front of
 * whatever header we're using (SmallNode or SweepNode).
 *
 * We set requestedSize to kFreedMagic when a block is not
 * allocated.
 */
struct MemoryManager::DebugHeader {
  static constexpr uintptr_t kAllocatedMagic = 0xDB6000A110C0A7EDull;
  static constexpr size_t kFreedMagic =        0x5AB07A6ED4110CEEull;

  uintptr_t allocatedMagic;
  size_t requestedSize;     // zero for size-untracked allocator
  size_t returnedCap;
  size_t padding;
};

struct MemoryManager::FreeList::Node {
  Node* next;
};

inline void* MemoryManager::FreeList::maybePop() {
  auto ret = head;
  if (LIKELY(ret != nullptr)) head = ret->next;
  return ret;
}

inline void MemoryManager::FreeList::push(void* val) {
  auto const node = static_cast<Node*>(val);
  node->next = head;
  head = node;
}

//////////////////////////////////////////////////////////////////////

template<class SizeT> ALWAYS_INLINE
SizeT MemoryManager::debugAddExtra(SizeT sz) {
  if (!debug) return sz;
  return sz + sizeof(DebugHeader);
}

template<class SizeT> ALWAYS_INLINE
SizeT MemoryManager::debugRemoveExtra(SizeT sz) {
  if (!debug) return sz;
  return sz - sizeof(DebugHeader);
}

#ifndef DEBUG

ALWAYS_INLINE void* MemoryManager::debugPostAllocate(void* p, size_t, size_t) {
  return p;
}

ALWAYS_INLINE void* MemoryManager::debugPreFree(void* p, size_t, size_t) {
  return p;
}

#endif

//////////////////////////////////////////////////////////////////////

inline uint32_t MemoryManager::smartSizeClass(uint32_t reqBytes) {
  assert(reqBytes <= kMaxSmartSize);
  auto const ret = (reqBytes + kSmartSizeMask) & ~kSmartSizeMask;
  assert(ret <= kMaxSmartSize);
  return ret;
}

inline bool MemoryManager::sweeping() {
  return !TlsWrapper::isNull() && MM().m_sweeping;
}

inline void* MemoryManager::smartMallocSize(uint32_t bytes) {
  assert(bytes > 0);
  assert(bytes <= kMaxSmartSize);

  // Note: unlike smart_malloc, we don't track internal fragmentation
  // in the usage stats when we're going through smartMallocSize.
  m_stats.usage += bytes;

  unsigned i = (bytes - 1) >> kLgSizeQuantum;
  assert(i < kNumSizes);
  void* p = m_freelists[i].maybePop();
  if (UNLIKELY(p == nullptr)) {
    p = slabAlloc(debugAddExtra(MemoryManager::smartSizeClass(bytes)));
  }
  assert(reinterpret_cast<uintptr_t>(p) % 16 == 0);

  FTRACE(1, "smartMallocSize: {} -> {}\n", bytes, p);
  return debugPostAllocate(p, bytes, bytes);
}

inline void MemoryManager::smartFreeSize(void* ptr, uint32_t bytes) {
  assert(bytes > 0);
  assert(bytes <= kMaxSmartSize);
  assert(reinterpret_cast<uintptr_t>(ptr) % 16 == 0);

  unsigned i = (bytes - 1) >> kLgSizeQuantum;
  assert(i < kNumSizes);
  m_freelists[i].push(debugPreFree(ptr, bytes, bytes));
  m_stats.usage -= bytes;

  FTRACE(1, "smartFreeSize: {} ({} bytes)\n", ptr, bytes);
}

ALWAYS_INLINE
std::pair<void*,size_t> MemoryManager::smartMallocSizeBig(size_t bytes) {
#ifdef USE_JEMALLOC
  void* ptr;
  size_t sz;
  auto const retptr = smartMallocSizeBigHelper(ptr, sz, bytes);
  FTRACE(1, "smartMallocBig: {} ({} requested, {} usable)\n",
         retptr, bytes, sz);
  return std::make_pair(retptr, sz);
#else
  m_stats.usage += bytes;
  // TODO(#2831116): we only add sizeof(SmallNode) so smartMallocBig
  // can subtract it.
  auto const ret = smartMallocBig(debugAddExtra(bytes + sizeof(SmallNode)));
  FTRACE(1, "smartMallocBig: {} ({} bytes)\n", ret, bytes);
  return std::make_pair(debugPostAllocate(ret, bytes, bytes), bytes);
#endif
}

ALWAYS_INLINE
void MemoryManager::smartFreeSizeBig(void* vp, size_t bytes) {
  m_stats.usage -= bytes;
  FTRACE(1, "smartFreeBig: {} ({} bytes)\n", vp, bytes);
  return smartFreeBig(static_cast<SweepNode*>(debugPreFree(vp, bytes, 0)) - 1);
}

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
void* MemoryManager::objMalloc(size_t size) {
  if (LIKELY(size <= kMaxSmartSize)) return smartMallocSize(size);
  return smartMallocSizeBig(size).first;
}

ALWAYS_INLINE
void MemoryManager::objFree(void* vp, size_t size) {
  if (LIKELY(size <= kMaxSmartSize)) return smartFreeSize(vp, size);
  return smartFreeSizeBig(vp, size);
}

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
void* MemoryManager::smartMallocSizeLogged(uint32_t size) {
  auto const retptr = smartMallocSize(size);
  if (memory_profiling) { logAllocation(retptr, size); }
  return retptr;
}

ALWAYS_INLINE
void MemoryManager::smartFreeSizeLogged(void* p, uint32_t size) {
  if (memory_profiling) { logDeallocation(p); }
  return smartFreeSize(p, size);
}

ALWAYS_INLINE
std::pair<void*,size_t> MemoryManager::smartMallocSizeBigLogged(size_t size) {
  auto const retptr = smartMallocSizeBig(size);
  if (memory_profiling) { logAllocation(retptr.first, size); }
  return retptr;
}

ALWAYS_INLINE
void MemoryManager::smartFreeSizeBigLogged(void* vp, size_t size) {
  if (memory_profiling) { logDeallocation(vp); }
  return smartFreeSizeBig(vp, size);
}

ALWAYS_INLINE
void* MemoryManager::objMallocLogged(size_t size) {
  auto const retptr = objMalloc(size);
  if (memory_profiling) { logAllocation(retptr, size); }
  return retptr;
}

ALWAYS_INLINE
void MemoryManager::objFreeLogged(void* vp, size_t size) {
  if (memory_profiling) { logDeallocation(vp); }
  return objFree(vp, size);
}

//////////////////////////////////////////////////////////////////////

inline int64_t MemoryManager::getAllocated() const {
#ifdef USE_JEMALLOC
  assert(m_allocated);
  return *m_allocated;
#else
  return 0;
#endif
}

inline int64_t MemoryManager::getDeallocated() const {
#ifdef USE_JEMALLOC
  assert(m_deallocated);
  return *m_deallocated;
#else
  return 0;
#endif
}

inline MemoryUsageStats& MemoryManager::getStatsNoRefresh() { return m_stats; }
inline MemoryUsageStats& MemoryManager::getStats() {
  refreshStats();
  return m_stats;
}

inline MemoryUsageStats MemoryManager::getStatsCopy() {
  MemoryUsageStats ret;
  ret = m_stats;
  refreshStatsImpl<false>(ret);
  return ret;
}

inline void MemoryManager::refreshStats() { refreshStatsImpl<true>(m_stats); }

/*
 * Refresh stats to reflect directly malloc()ed memory, and determine
 * whether the request memory limit has been exceeded.
 *
 * The stats parameter allows the updates to be applied to either
 * m_stats as in refreshStats() or to a separate MemoryUsageStats
 * struct as in getStatsSafe().
 *
 * The template variable live controls whether or not MemoryManager
 * member variables are updated and whether or not to call helper
 * methods in response to memory anomalies.
 */
template<bool live>
void MemoryManager::refreshStatsImpl(MemoryUsageStats& stats) {
#ifdef USE_JEMALLOC
  // Incrementally incorporate the difference between the previous and current
  // deltas into the memory usage statistic.  For reference, the total
  // malloced memory usage could be calculated as such, if delta0 were
  // recorded in resetStats():
  //
  //   int64 musage = delta - delta0;
  //
  // Note however, that SmartAllocator adds to m_stats.jemallocDebt
  // when it calls malloc(), so that this function can avoid
  // double-counting the malloced memory. Thus musage in the example
  // code may well substantially exceed m_stats.usage.
  if (s_statsEnabled) {
    int64_t delta = int64_t(*m_allocated) - int64_t(*m_deallocated);
    int64_t deltaAllocated = int64_t(*m_allocated) - m_prevAllocated;
    stats.usage += delta - m_delta - stats.jemallocDebt;
    stats.jemallocDebt = 0;
    stats.totalAlloc += deltaAllocated;
    if (live) {
      m_delta = delta;
      m_prevAllocated = int64_t(*m_allocated);
    }
  }
#endif
  if (stats.usage > stats.peakUsage) {
    // NOTE: the peak memory usage monotonically increases, so there cannot
    // be a second OOM exception in one request.
    assert(stats.maxBytes > 0);
    if (live && stats.peakUsage <= stats.maxBytes &&
        stats.usage > stats.maxBytes) {
      refreshStatsHelperExceeded();
    }
    // Check whether the process's active memory limit has been exceeded, and
    // if so, stop the server.
    //
    // Only check whether the total memory limit was exceeded if this request
    // is at a new high water mark.  This check could be performed regardless
    // of this request's current memory usage (because other request threads
    // could be to blame for the increased memory usage), but doing so would
    // measurably increase computation for little benefit.
#ifdef USE_JEMALLOC
    // (*m_cactive) consistency is achieved via atomic operations.  The fact
    // that we do not use an atomic operation here means that we could get a
    // stale read, but in practice that poses no problems for how we are
    // using the value.
    if (live && s_statsEnabled && *m_cactive > m_cactiveLimit) {
      refreshStatsHelperStop();
    }
#endif
    stats.peakUsage = stats.usage;
  }
}

//////////////////////////////////////////////////////////////////////

}

#endif
