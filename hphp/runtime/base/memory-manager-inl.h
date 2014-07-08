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

#ifndef incl_HPHP_MEMORY_MANAGER_INL_H
#define incl_HPHP_MEMORY_MANAGER_INL_H

#include <limits>
#include <utility>

#include "hphp/util/compilation-flags.h"

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
    FTRACE(1, "MaskAlloc()\n");
    m_mm.refreshStats();
  }
  ~MaskAlloc() {
    FTRACE(1, "~MaskAlloc()\n");
#ifdef USE_JEMALLOC
    // exclude mallocs and frees since construction
    if (s_statsEnabled) {
      FTRACE(1, "old: prev alloc: {}\nprev dealloc: {}\n",
        m_mm.m_prevAllocated, m_mm.m_prevDeallocated);

      m_mm.m_prevAllocated = *m_mm.m_allocated;
      m_mm.m_prevDeallocated = *m_mm.m_deallocated;

      FTRACE(1, "new: prev alloc: {}\nprev dealloc: {}\n\n",
        m_mm.m_prevAllocated, m_mm.m_prevDeallocated);
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
  return sz + kDebugExtraSize;
}

template<class SizeT> ALWAYS_INLINE
SizeT MemoryManager::debugRemoveExtra(SizeT sz) {
  if (!debug) return sz;
  return sz - kDebugExtraSize;
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

inline uint32_t MemoryManager::bsr(uint32_t x) {
#if defined(__i386__) || defined(__x86_64__)
  uint32_t ret;
  __asm__ ("bsr %1, %0"
           : "=r"(ret) // Outputs.
           : "r"(x)    // Inputs.
           );
  return ret;
#else
  // Equivalent (but incompletely strength-reduced by gcc):
  return 31 - __builtin_clz(x);
#endif
}

inline uint8_t MemoryManager::smartSize2IndexCompute(uint32_t size) {
  uint32_t x = bsr((size<<1)-1);
  uint32_t shift = (x < kLgSizeClassesPerDoubling + kLgSmartSizeQuantum)
                   ? 0 : x - (kLgSizeClassesPerDoubling + kLgSmartSizeQuantum);
  uint32_t grp = shift << kLgSizeClassesPerDoubling;

  int32_t lgReduced = x - kLgSizeClassesPerDoubling
                      - 1; // Counteract left shift of bsr() argument.
  uint32_t lgDelta = (lgReduced < int32_t(kLgSmartSizeQuantum))
                     ? kLgSmartSizeQuantum : lgReduced;
  uint32_t deltaInverseMask = -1 << lgDelta;
  constexpr uint32_t kModMask = (1u << kLgSizeClassesPerDoubling) - 1;
  uint32_t mod = ((((size-1) & deltaInverseMask) >> lgDelta)) & kModMask;

  auto const index = grp + mod;
  assert(index < kNumSmartSizes);
  return index;
}

inline uint8_t MemoryManager::smartSize2IndexLookup(uint32_t size) {
  uint8_t index = kSmartSize2Index[(size-1) >> kLgSmartSizeQuantum];
  assert(index == smartSize2IndexCompute(size));
  return index;
}

inline uint8_t MemoryManager::smartSize2Index(uint32_t size) {
  assert(size > 0);
  assert(size <= kMaxSmartSize + kDebugExtraSize);
  if (LIKELY(size <= kMaxSmartSizeLookup)) {
    return smartSize2IndexLookup(size);
  }
  return smartSize2IndexCompute(size);
}

inline uint32_t MemoryManager::smartSizeClass(uint32_t reqBytes) {
  uint32_t x = bsr((reqBytes<<1)-1);
  int32_t lgReduced = x - kLgSizeClassesPerDoubling
                      - 1; // Counteract left shift of bsr() argument.
  uint32_t lgDelta = (lgReduced < int32_t(kLgSmartSizeQuantum))
                      ? kLgSmartSizeQuantum : lgReduced;
  uint32_t delta = 1u << lgDelta;
  uint32_t deltaMask = delta - 1;
  auto const ret = (reqBytes + deltaMask) & ~deltaMask;
  assert(ret <= kMaxSmartSize + kDebugExtraSize);
  if (kDebugExtraSize != 0) {
    return std::min(ret, uint32_t(kMaxSmartSize));
  }
  return ret;
}

inline bool MemoryManager::sweeping() {
  return !TlsWrapper::isNull() && MM().m_sweeping;
}

inline void* MemoryManager::smartMallocSize(uint32_t bytes) {
  assert(bytes > 0);
  assert(bytes <= kMaxSmartSize + kDebugExtraSize);

  // Note: unlike smart_malloc, we don't track internal fragmentation
  // in the usage stats when we're going through smartMallocSize.
  m_stats.usage += bytes;

  unsigned i = smartSize2Index(bytes);
  void* p = m_freelists[i].maybePop();
  if (UNLIKELY(p == nullptr)) {
    p = slabAlloc(debugAddExtra(MemoryManager::smartSizeClass(bytes)), i);
  }
  assert((reinterpret_cast<uintptr_t>(p) & kSmartSizeAlignMask) == 0);

  FTRACE(3, "smartMallocSize: {} -> {}\n", bytes, p);
  return debugPostAllocate(p, bytes, bytes);
}

inline void MemoryManager::smartFreeSize(void* ptr, uint32_t bytes) {
  assert(bytes > 0);
  assert(bytes <= kMaxSmartSize + kDebugExtraSize);
  assert((reinterpret_cast<uintptr_t>(ptr) & kSmartSizeAlignMask) == 0);

  unsigned i = smartSize2Index(bytes);
  m_freelists[i].push(debugPreFree(ptr, bytes, bytes));
  m_stats.usage -= bytes;

  FTRACE(3, "smartFreeSize: {} ({} bytes)\n", ptr, bytes);
}

template<bool callerSavesActualSize>
ALWAYS_INLINE
std::pair<void*,size_t> MemoryManager::smartMallocSizeBig(size_t bytes) {
#ifdef USE_JEMALLOC
  void* ptr;
  size_t sz;
  auto const retptr =
    smartMallocSizeBigHelper<callerSavesActualSize>(ptr, sz, bytes);
  FTRACE(3, "smartMallocBig: {} ({} requested, {} usable)\n",
         retptr, bytes, sz);
  return std::make_pair(retptr, sz);
#else
  m_stats.usage += bytes;
  auto const ret = smartMallocBig(debugAddExtra(bytes));
  FTRACE(3, "smartMallocBig: {} ({} bytes)\n", ret, bytes);
  return std::make_pair(debugPostAllocate(ret, bytes, bytes), bytes);
#endif
}

ALWAYS_INLINE
void MemoryManager::smartFreeSizeBig(void* vp, size_t bytes) {
  m_stats.usage -= bytes;
  // Since we account for these direct allocations in our usage and adjust for
  // them on allocation, we also need to adjust for them negatively on free.
  JEMALLOC_STATS_ADJUST(&m_stats, -bytes);
  FTRACE(3, "smartFreeBig: {} ({} bytes)\n", vp, bytes);
  return smartFreeBig(static_cast<SweepNode*>(debugPreFree(vp, bytes, 0)) - 1);
}

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
void* MemoryManager::objMalloc(size_t size) {
  if (LIKELY(size <= kMaxSmartSize)) return smartMallocSize(size);
  return smartMallocSizeBig<false>(size).first;
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

template<bool callerSavesActualSize>
ALWAYS_INLINE
std::pair<void*,size_t> MemoryManager::smartMallocSizeBigLogged(size_t size) {
  auto const retptr = smartMallocSizeBig<callerSavesActualSize>(size);
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

inline bool MemoryManager::startStatsInterval() {
  auto ret = !m_statsIntervalActive;
  refreshStats();
  // For the reasons stated below in refreshStatsImpl, usage can potentially be
  // negative. Make sure that doesn't occur here.
  m_stats.peakIntervalUsage = std::max<int64_t>(0, m_stats.usage);
  m_stats.peakIntervalAlloc = m_stats.alloc;
  assert(m_stats.peakIntervalAlloc >= 0);
  m_statsIntervalActive = true;
  return ret;
}

inline bool MemoryManager::stopStatsInterval() {
  auto ret = m_statsIntervalActive;
  m_statsIntervalActive = false;
  m_stats.peakIntervalUsage = 0;
  m_stats.peakIntervalAlloc = 0;
  return ret;
}

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
  // recorded in resetStatsImpl():
  //
  //   int64 musage = delta - delta0;
  //
  // Note however, that SmartAllocator adds to m_stats.jemallocDebt
  // when it calls malloc(), so that this function can avoid
  // double-counting the malloced memory. Thus musage in the example
  // code may well substantially exceed m_stats.usage.
  if (m_enableStatsSync) {
    uint64_t jeDeallocated = *m_deallocated;
    uint64_t jeAllocated = *m_allocated;

    // We can't currently handle wrapping so make sure this isn't happening.
    assert(jeAllocated >= 0 &&
           jeAllocated <= std::numeric_limits<int64_t>::max());
    assert(jeDeallocated >= 0 &&
           jeDeallocated <= std::numeric_limits<int64_t>::max());

    // Since these deltas potentially include memory allocated from another
    // thread but deallocated on this one, it is possible for these nubmers to
    // go negative.
    int64_t jeDeltaAllocated =
      int64_t(jeAllocated) - int64_t(jeDeallocated);
    int64_t mmDeltaAllocated =
      int64_t(m_prevAllocated) - int64_t(m_prevDeallocated);

    // This is the delta between the current and the previous jemalloc reading.
    int64_t jeMMDeltaAllocated =
      int64_t(jeAllocated) - int64_t(m_prevAllocated);

    FTRACE(1, "Before stats sync:\n");
    FTRACE(1, "je alloc:\ncurrent: {}\nprevious: {}\ndelta with MM: {}\n",
      jeAllocated, m_prevAllocated, jeAllocated - m_prevAllocated);
    FTRACE(1, "je dealloc:\ncurrent: {}\nprevious: {}\ndelta with MM: {}\n",
      jeDeallocated, m_prevDeallocated, jeDeallocated - m_prevDeallocated);
    FTRACE(1, "je delta:\ncurrent: {}\nprevious: {}\n",
      jeDeltaAllocated, mmDeltaAllocated);
    FTRACE(1, "usage: {}\ntotal (je) alloc: {}\nje debt: {}\n",
      stats.usage, stats.totalAlloc, stats.jemallocDebt);

    // Subtract the old jemalloc adjustment (delta0) and add the current one
    // (delta) to arrive at the new combined usage number.
    stats.usage += jeDeltaAllocated - mmDeltaAllocated;
    // Remove the "debt" accrued from allocating the slabs so we don't double
    // count the slab-based allocations.
    stats.usage -= stats.jemallocDebt;

    stats.jemallocDebt = 0;
    // We need to do the calculation instead of just setting it to jeAllocated
    // because of the MaskAlloc capability.
    stats.totalAlloc += jeMMDeltaAllocated;
    if (live) {
      m_prevAllocated = jeAllocated;
      m_prevDeallocated = jeDeallocated;
    }

    FTRACE(1, "After stats sync:\n");
    FTRACE(1, "usage: {}\ntotal (je) alloc: {}\n\n",
      stats.usage, stats.totalAlloc);
  }
#endif
  if (stats.usage > stats.peakUsage) {
    // NOTE: the peak memory usage monotonically increases, so there cannot
    // be a second OOM exception in one request.
    assert(stats.maxBytes > 0);
    if (live && m_couldOOM && stats.usage > stats.maxBytes) {
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
  if (live && m_statsIntervalActive) {
    if (stats.usage > stats.peakIntervalUsage) {
      stats.peakIntervalUsage = stats.usage;
    }
    if (stats.alloc > stats.peakIntervalAlloc) {
      stats.peakIntervalAlloc = stats.alloc;
    }
  }
}

inline void MemoryManager::resetCouldOOM() { m_couldOOM = true; }

inline void MemoryManager::resetExternalStats() { resetStatsImpl(false); }

//////////////////////////////////////////////////////////////////////

}

#endif
