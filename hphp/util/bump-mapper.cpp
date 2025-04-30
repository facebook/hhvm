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
#include "hphp/util/maphuge.h"
#include "hphp/util/hugetlb.h"
#include "hphp/util/numa.h"

#include <algorithm>
#include <atomic>

#include <folly/portability/SysMman.h>

#ifdef HAVE_NUMA
#include <numaif.h>
#endif
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#if USE_JEMALLOC_EXTENT_HOOKS

namespace HPHP {

bool g_useTHPUponHugeTLBFailure = false;

namespace alloc {

bool BumpSinglePageMapper::addMappingImpl() {
  if (m_currHugePages >= m_maxHugePages) return false;

  auto _ = m_state.lock();
  auto const currFrontier = m_state.low_map.load(std::memory_order_acquire);
  if (currFrontier % pagesize() != 0) return false;
  auto const newFrontier = currFrontier + pagesize();
  if (newFrontier > m_state.high_map.load(std::memory_order_acquire)) {
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
        if (addPage((void*)currFrontier, currNode)) {
          ++m_currHugePages;
          m_state.low_map.store(newFrontier, std::memory_order_release);
          return true;
        }
        if (++failCount >= numAllowedNodes) return false;
      } while (true);
    }
  }
#endif
  if (addPage((void*)currFrontier, -1)) {
    ++m_currHugePages;
    m_state.low_map.store(newFrontier, std::memory_order_release);
    return true;
  }
  return false;
}


void* Bump1GMapper::addPage(void* addr, int node) {
  return mmap_1g(addr, node, /* map_fixed */ true);
}


size_t BumpTHPMapper::pagesize() const {
  return THPPageSize();
}

void* BumpTHPMapper::addPage(void* addr, int node) {
#ifdef HAVE_NUMA
  SavedNumaPolicy numaPolicy;
  if (node >= 0 && numa_num_nodes > 1) {
    numaPolicy.save();
    unsigned long singleNodeMask = 1ul << node;
    set_mempolicy(MPOL_BIND, &singleNodeMask, sizeof(singleNodeMask));
  }
#endif
  auto const size = pagesize();
  void* ret = mmap(addr, size, PROT_READ | PROT_WRITE,
                   MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0);
  if (ret == MAP_FAILED) {
    return nullptr;
  }
  if (ret != addr) {
    munmap(ret, size);
    return nullptr;
  }
  madvise(ret, size, MADV_HUGEPAGE);  // let's hope kernel can get what we want.
  return ret;
}


constexpr size_t kChunkSize = 4 * size2m;

bool Bump2MMapper::addMappingImpl() {
  if (m_currHugePages >= m_maxHugePages) return false;
  const uint32_t freePages =
    g_useTHPUponHugeTLBFailure ? (kChunkSize / size2m)
                               : get_huge2m_info().free_hugepages;
  if (freePages <= 0) return false;

  auto _ = m_state.lock();
  // Recheck the mapping frontiers after grabbing the lock
  auto const currFrontier = m_state.low_map.load(std::memory_order_acquire);

  if (currFrontier % size2m != 0) return false;
  auto nPages = std::min(m_maxHugePages - m_currHugePages, freePages);
  if (nPages <= 0) return false;
  auto const hugeSize = std::min(kChunkSize, size2m * nPages);
  auto const newFrontier = currFrontier + hugeSize;
  if (newFrontier > m_state.high_map.load(std::memory_order_acquire)) {
    return false;
  }
  void* newPages = mmap((void*)currFrontier, hugeSize,
                        PROT_READ | PROT_WRITE,
                        MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED | MAP_HUGETLB,
                        -1, 0);
  if (newPages == MAP_FAILED) {
    if (!g_useTHPUponHugeTLBFailure) return false;
    if (THPPageSize() != size2m) return false;
    // Use transparent hugepages instead.
    newPages = mmap((void*)currFrontier, hugeSize,
                    PROT_READ | PROT_WRITE,
                    MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED,
                    -1, 0);
    if (newPages == MAP_FAILED) return false;
    assertx(newPages == (void*)currFrontier);
    madvise(newPages, hugeSize, MADV_HUGEPAGE);
  }
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
  auto _ = m_state.lock();
  auto const high = m_state.high_map.load(std::memory_order_acquire);
  auto const low = m_state.low_map.load(std::memory_order_acquire);
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

bool BumpFileMapper::setDirectory(const char* dir) {
  auto const len = strlen(dir);
  if (len >= sizeof(m_dirName)) {
    return false;
  }
  memcpy(m_dirName, dir, len + 1);
  return true;
}

bool BumpFileMapper::addMappingImpl() {
  auto _ = m_state.lock();
  if (m_fd) return false;               // already initialized
  if (!m_dirName[0]) return false;      // setDirectory() not done successfully
  // Create a temporary file and map it in upon the first request.
  m_fd = open(m_dirName,
              O_TMPFILE | O_DIRECTORY | O_RDWR | O_CLOEXEC,
              S_IRUSR | S_IWUSR);
  if (m_fd == -1) {
    return false;
  }
  if (ftruncate(m_fd, m_state.capacity())) {
    return false;
  }
  auto const addr = mmap(reinterpret_cast<void*>(m_state.low()),
                         m_state.capacity(),
                         PROT_READ | PROT_WRITE,
                         MAP_FIXED | MAP_SHARED,
                         m_fd, 0);
  if (addr == (void*)-1) {
    return false;
  }
  m_state.low_map.store(m_state.high(), std::memory_order_release);
  return true;
}

std::atomic_bool BumpEmergencyMapper::s_emergencyFlag{false};

bool BumpEmergencyMapper::addMappingImpl() {
  auto _ = m_state.lock();
  auto low = m_state.low();
  auto const high = m_state.high();
  if (low == high) return false; // another thread added this range already.
  mprotect(reinterpret_cast<void*>(low), high - low,
           PROT_READ | PROT_WRITE);
  m_state.low_map.store(high, std::memory_order_release);
  s_emergencyFlag.store(true, std::memory_order_release);
  if (m_exit) m_exit();
  return true;
}

template bool BumpNormalMapper<Direction::LowToHigh>::addMappingImpl();
template bool BumpNormalMapper<Direction::HighToLow>::addMappingImpl();

} // namespace alloc
} // namespace HPHP

#endif
