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
  kMaxSmallSize <= std::numeric_limits<uint32_t>::max(),
  "Size-specified small block alloc functions assume this"
);

//////////////////////////////////////////////////////////////////////

inline MemoryManager& MM() {
  return *MemoryManager::TlsWrapper::getNoCheck();
}

//////////////////////////////////////////////////////////////////////

namespace req {

template<class T, class... Args> T* make_raw(Args&&... args) {
  auto const mem = req::malloc(sizeof(T));
  try {
    return new (mem) T(std::forward<Args>(args)...);
  } catch (...) {
    req::free(mem);
    throw;
  }
}

template<class T> void destroy_raw(T* t) {
  t->~T();
  req::free(t);
}

template<class T> T* make_raw_array(size_t count) {
  T* ret = static_cast<T*>(req::malloc(count * sizeof(T)));
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
    req::free(ret);
    throw;
  }
  return ret;
}

template<class T>
void destroy_raw_array(T* t, size_t count) {
  size_t i = count;
  while (i-- > 0) {
    t[i].~T();
  }
  req::free(t);
}
} // namespace req

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

struct MemoryManager::SuppressOOM {
  explicit SuppressOOM(MemoryManager& mm)
      : m_mm(mm)
      , m_savedCouldOOM(mm.m_couldOOM) {
    FTRACE(1, "SuppressOOM() [couldOOM was {}]\n", m_savedCouldOOM);
    m_mm.m_couldOOM = false;
  }

  ~SuppressOOM() {
    FTRACE(1, "~SuppressOOM() [couldOOM is {}]\n", m_savedCouldOOM);
    m_mm.m_couldOOM = m_savedCouldOOM;
  }

  SuppressOOM(const SuppressOOM&) = delete;
  SuppressOOM& operator=(const SuppressOOM&) = delete;

private:
  MemoryManager& m_mm;
  bool m_savedCouldOOM;
};

//////////////////////////////////////////////////////////////////////

inline int operator<<(HeaderKind k, int bits) {
  return int(k) << bits;
}

inline void* MemoryManager::FreeList::maybePop() {
  auto ret = head;
  if (LIKELY(ret != nullptr)) head = ret->next;
  FTRACE(4, "FreeList::maybePop(): returning {}\n", ret);
  return ret;
}

inline void MemoryManager::FreeList::push(void* val, size_t size) {
  FTRACE(4, "FreeList::push({}, {}), prev head = {}\n", val, size, head);
  auto constexpr kMaxFreeSize = std::numeric_limits<uint32_t>::max();
  static_assert(kMaxSmallSize <= kMaxFreeSize, "");
  assert(size > 0 && size <= kMaxFreeSize);
  auto const node = static_cast<FreeNode*>(val);
  node->next = head;
  // The extra store to initialize a free header here is expensive.
  // Instead, initFree() initializes all free headers just before iterating
  head = node;
}

//////////////////////////////////////////////////////////////////////

inline uint32_t MemoryManager::estimateCap(uint32_t requested) {
  return requested <= kMaxSmallSize ? smallSizeClass(requested)
                                    : requested;
}

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

inline size_t MemoryManager::computeSmallSize2Index(uint32_t size) {
  uint32_t x = bsr((size<<1)-1);
  uint32_t shift = (x < kLgSizeClassesPerDoubling + kLgSmallSizeQuantum)
                   ? 0 : x - (kLgSizeClassesPerDoubling + kLgSmallSizeQuantum);
  uint32_t grp = shift << kLgSizeClassesPerDoubling;

  int32_t lgReduced = x - kLgSizeClassesPerDoubling
                      - 1; // Counteract left shift of bsr() argument.
  uint32_t lgDelta = (lgReduced < int32_t(kLgSmallSizeQuantum))
                     ? kLgSmallSizeQuantum : lgReduced;
  uint32_t deltaInverseMask = -1 << lgDelta;
  constexpr uint32_t kModMask = (1u << kLgSizeClassesPerDoubling) - 1;
  uint32_t mod = ((((size-1) & deltaInverseMask) >> lgDelta)) & kModMask;

  auto const index = grp + mod;
  assert(index < kNumSmallSizes);
  return index;
}

inline size_t MemoryManager::lookupSmallSize2Index(uint32_t size) {
  auto const index = kSmallSize2Index[(size-1) >> kLgSmallSizeQuantum];
  assert(index == computeSmallSize2Index(size));
  return index;
}

inline size_t MemoryManager::smallSize2Index(uint32_t size) {
  assert(size > 0);
  assert(size <= kMaxSmallSize);
  if (LIKELY(size <= kMaxSmallSizeLookup)) {
    return lookupSmallSize2Index(size);
  }
  return computeSmallSize2Index(size);
}

inline uint32_t MemoryManager::smallIndex2Size(size_t index) {
  return kSmallIndex2Size[index];
}

inline uint32_t MemoryManager::smallSizeClass(uint32_t reqBytes) {
  uint32_t x = bsr((reqBytes<<1)-1);
  int32_t lgReduced = x - kLgSizeClassesPerDoubling
                      - 1; // Counteract left shift of bsr() argument.
  uint32_t lgDelta = (lgReduced < int32_t(kLgSmallSizeQuantum))
                      ? kLgSmallSizeQuantum : lgReduced;
  uint32_t delta = 1u << lgDelta;
  uint32_t deltaMask = delta - 1;
  auto const ret = (reqBytes + deltaMask) & ~deltaMask;
  assert(ret <= kMaxSmallSize);
  return ret;
}

inline void* MemoryManager::mallocSmallIndex(size_t index, uint32_t bytes) {
  assert(index < kNumSmallSizes);
  assert(bytes <= kSmallIndex2Size[index]);
  m_stats.usage += bytes;

  void *p = m_freelists[index].maybePop();
  if (UNLIKELY(p == nullptr)) {
    p = mallocSmallSizeSlow(bytes, index);
  }
  assert((reinterpret_cast<uintptr_t>(p) & kSmallSizeAlignMask) == 0);
  FTRACE(3, "mallocSmallSize: {} -> {}\n", bytes, p);
  return p;
}

inline void* MemoryManager::mallocSmallSize(uint32_t bytes) {
  assert(bytes > 0);
  assert(bytes <= kMaxSmallSize);
  unsigned index = smallSize2Index(bytes);
  return mallocSmallIndex(index, bytes);
}

inline void MemoryManager::freeSmallIndex(void* ptr, size_t index,
                                          uint32_t bytes) {
  assert(index < kNumSmallSizes);
  assert((reinterpret_cast<uintptr_t>(ptr) & kSmallSizeAlignMask) == 0);
  assert(bytes <= kSmallIndex2Size[index]);

  if (UNLIKELY(m_bypassSlabAlloc)) {
    return freeBigSize(ptr, bytes);
  }

  FTRACE(3, "freeSmallSize({}, {}), freelist {}\n", ptr, bytes, index);

  m_freelists[index].push(ptr, bytes);
  m_stats.usage -= bytes;

  FTRACE(3, "freeSmallSize: {} ({} bytes)\n", ptr, bytes);
}

inline void MemoryManager::freeSmallSize(void* ptr, uint32_t bytes) {
  assert(bytes > 0);
  assert(bytes <= kMaxSmallSize);
  assert((reinterpret_cast<uintptr_t>(ptr) & kSmallSizeAlignMask) == 0);

  if (UNLIKELY(m_bypassSlabAlloc)) {
    return freeBigSize(ptr, bytes);
  }

  auto const i = smallSize2Index(bytes);
  FTRACE(3, "freeSmallSize({}, {}), freelist {}\n", ptr, bytes, i);

  m_freelists[i].push(ptr, bytes);
  assert(m_needInitFree = true); // intentional debug-only side-effect.
  m_stats.usage -= bytes;

  FTRACE(3, "freeSmallSize: {} ({} bytes)\n", ptr, bytes);
}

ALWAYS_INLINE
void MemoryManager::freeBigSize(void* vp, size_t bytes) {
  m_stats.usage -= bytes;
  // Since we account for these direct allocations in our usage and adjust for
  // them on allocation, we also need to adjust for them negatively on free.
  m_stats.borrow(-bytes);
  FTRACE(3, "freeBigSize: {} ({} bytes)\n", vp, bytes);
  m_heap.freeBig(vp);
}

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
void* MemoryManager::objMalloc(size_t size) {
  if (LIKELY(size <= kMaxSmallSize)) return mallocSmallSize(size);
  return mallocBigSize<false>(size).ptr;
}

ALWAYS_INLINE
void MemoryManager::objFree(void* vp, size_t size) {
  if (LIKELY(size <= kMaxSmallSize)) return freeSmallSize(vp, size);
  freeBigSize(vp, size);
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

inline bool MemoryManager::preAllocOOM(int64_t size) {
  if (m_couldOOM && m_stats.usage + size > m_stats.maxBytes) {
    refreshStatsHelperExceeded();
    return true;
  }
  return false;
}

inline void MemoryManager::forceOOM() {
  if (m_couldOOM) {
    refreshStatsHelperExceeded();
  }
}

inline void MemoryManager::resetExternalStats() { resetStatsImpl(false); }

//////////////////////////////////////////////////////////////////////

inline bool MemoryManager::empty() const {
  return m_heap.empty();
}

inline bool MemoryManager::contains(void *p) const {
  return m_heap.contains(p);
}

inline bool MemoryManager::checkContains(void* p) const {
  // Be conservative if the small-block allocator is disabled.
  assert(RuntimeOption::DisableSmallAllocator || contains(p));
  return true;
}

//////////////////////////////////////////////////////////////////////

inline bool MemoryManager::sweeping() {
  return !TlsWrapper::isNull() && MM().m_sweeping;
}

inline StringDataNode& MemoryManager::getStringList() {
  return m_strings;
}

//////////////////////////////////////////////////////////////////////

template <typename T>
MemoryManager::RootId MemoryManager::addRoot(req::ptr<T>&& ptr) {
  assert(ptr);
  const RootId token = ptr->getId();
  getRootMap<T>().emplace(token, std::move(ptr));
  return token;
}

template <typename T>
MemoryManager::RootId MemoryManager::addRoot(const req::ptr<T>& ptr) {
  assert(ptr);
  const RootId token = ptr->getId();
  getRootMap<T>()[token] = ptr;
  return token;
}

template <typename T>
req::ptr<T> MemoryManager::lookupRoot(RootId token) const {
  auto& handleMap = getRootMap<T>();
  auto itr = handleMap.find(token);
  return itr != handleMap.end() ? unsafe_cast_or_null<T>(itr->second) : nullptr;
}

template <typename T>
req::ptr<T> MemoryManager::removeRoot(RootId token) {
  auto& handleMap = getRootMap<T>();
  auto itr = handleMap.find(token);
  if(itr != handleMap.end()) {
    auto ptr = std::move(itr->second);
    handleMap.erase(itr);
    return unsafe_cast_or_null<T>(ptr);
  }
  return nullptr;
}

template <typename T>
bool MemoryManager::removeRoot(const req::ptr<T>& ptr) {
  return (bool)removeRoot<T>(ptr->getId());
}

template <typename T>
bool MemoryManager::removeRoot(const T* ptr) {
  return (bool)removeRoot<T>(ptr->getId());
}

template <typename F>
void MemoryManager::scanRootMaps(F& m) const {
  if (m_objectRoots) {
    for(const auto& root : *m_objectRoots) {
      scan(root.second, m);
    }
  }
  if (m_resourceRoots) {
    for(const auto& root : *m_resourceRoots) {
      scan(root.second, m);
    }
  }
}

//////////////////////////////////////////////////////////////////////

}

#endif
