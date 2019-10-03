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

#include "hphp/util/bump-mapper.h"

#include "hphp/util/assertions.h"
#include "hphp/util/hugetlb.h"
#include "hphp/util/numa.h"

#include <algorithm>
#include <atomic>
#include <mutex>

#include <folly/portability/SysMman.h>

#ifdef HAVE_NUMA
#include <numaif.h>
#endif

#if USE_JEMALLOC_EXTENT_HOOKS

namespace HPHP { namespace alloc {

bool Bump1GMapper::addMappingImpl() {
  if (m_currHugePages >= m_maxHugePages) return false;
  if (get_huge1g_info().free_hugepages <= 0) return false;

  std::lock_guard<RangeState> _(m_state);
  auto const currFrontier = m_state.low_map.load(std::memory_order_relaxed);
  if (currFrontier % size1g != 0) return false;
  auto const newFrontier = currFrontier + size1g;
  if (newFrontier > m_state.high_map.load(std::memory_order_relaxed)) {
    return false;
  }
#ifdef HAVE_NUMA
  if (numa_num_nodes > 1) {
    if (const int numAllowedNodes = __builtin_popcount(m_interleaveMask)) {
      assertx((m_interleaveMask & ~numa_node_set) == 0);
      int failCount = 0;
      // Try to map huge pages in round-robin fashion starting from m_nextNode.
      // We try on each allowed NUMA node at most once.
      do {
        auto const currNode = m_nextNode;
        m_nextNode = (currNode + 1) & numa_node_mask;
        if (!((1u << currNode) & m_interleaveMask)) {
          // Node not allowed, try next one.
          continue;
        }
        if (mmap_1g((void*)currFrontier, currNode, /* MAP_FIXED */ true)) {
          ++m_currHugePages;
          m_state.low_map.store(newFrontier, std::memory_order_release);
          return true;
        }
        if (++failCount >= numAllowedNodes) return false;
      } while (true);
    }
  }
#endif
  if (mmap_1g((void*)currFrontier, -1, /* MAP_FIXED */ true)) {
    ++m_currHugePages;
    m_state.low_map.store(newFrontier, std::memory_order_release);
    return true;
  }
  return false;
}

constexpr size_t kChunkSize = 4 * size2m;

bool Bump2MMapper::addMappingImpl() {
  if (m_currHugePages >= m_maxHugePages) return false;
  auto const freePages = get_huge2m_info().free_hugepages;
  if (freePages <= 0) return false;

  std::lock_guard<RangeState> _(m_state);
  // Recheck the mapping frontiers after grabbing the lock
  auto const currFrontier = m_state.low_map.load(std::memory_order_relaxed);
  if (currFrontier % size2m != 0) return false;
  auto nPages = std::min(m_maxHugePages - m_currHugePages, freePages);
  if (nPages <= 0) return false;
  auto const hugeSize = std::min(kChunkSize, size2m * nPages);
  auto const newFrontier = currFrontier + hugeSize;
  if (newFrontier > m_state.high_map.load(std::memory_order_relaxed)) {
    return false;
  }
  void* newPages = mmap((void*)currFrontier, hugeSize,
                        PROT_READ | PROT_WRITE,
                        MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED | MAP_HUGETLB,
                        -1, 0);
  if (newPages == MAP_FAILED) return false;
  assertx(newPages == (void*)currFrontier);    // MAP_FIXED should work
#ifdef HAVE_NUMA
  if (numa_num_nodes > 1 && m_interleaveMask) {
    unsigned long mask = m_interleaveMask;
    mbind(newPages, hugeSize, MPOL_INTERLEAVE,
          &mask, 32 /* max node */, 0 /* flag */);
  }
#endif
  // Make sure pages are faulted in.
  for (auto addr = currFrontier; addr < newFrontier; addr += size2m) {
    if (mlock(reinterpret_cast<void*>(addr), 1)) {
      // Forget it. We don't really have enough page reserved. At this moment,
      // we haven't committed to RangeState yet, so it is safe to bail out.
      munmap((void*)currFrontier, hugeSize);
      return false;
    }
  }
  m_currHugePages += hugeSize / size2m;
  m_state.low_map.store(newFrontier, std::memory_order_release);
  return true;
}


template<Direction D>
bool BumpNormalMapper<D>::addMappingImpl() {
  std::lock_guard<RangeState> _(m_state);
  auto const high = m_state.high_map.load(std::memory_order_relaxed);
  auto const low = m_state.low_map.load(std::memory_order_relaxed);
  auto const maxSize = static_cast<size_t>(high - low);
  if (maxSize == 0) return false;       // fully mapped
  auto const size = std::min(kChunkSize, maxSize);

  auto const newPageStart = (D == Direction::LowToHigh) ? low : high - size;
  assertx(newPageStart % size4k == 0);

  void* newPages = mmap((void*)newPageStart, size,
                        PROT_READ | PROT_WRITE,
                        MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED,
                        -1, 0);
  if (newPages == MAP_FAILED) return false;
  if (newPages != (void*)newPageStart) {
    assertx(false);                     // MAP_FIXED should've worked.
    munmap(newPages, size);
    return false;
  }

#ifdef HAVE_NUMA
  if (numa_num_nodes > 1 && m_interleaveMask) {
    unsigned long mask = m_interleaveMask;
    mbind(newPages, size, MPOL_INTERLEAVE,
          &mask, 32 /* max node */, 0 /* flag */);
  }
#endif
  if (D == Direction::LowToHigh) {
    m_state.low_map.store(newPageStart + size, std::memory_order_release);
  } else {
    m_state.high_map.store(newPageStart, std::memory_order_release);
  }
  return true;
}

template bool BumpNormalMapper<Direction::LowToHigh>::addMappingImpl();
template bool BumpNormalMapper<Direction::HighToLow>::addMappingImpl();

} // namespace alloc
} // namespace HPHP

#endif
