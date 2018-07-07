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
#include "hphp/util/maphuge.h"
#include "hphp/util/numa.h"

#include <folly/portability/SysMman.h>

#ifdef HAVE_NUMA
#include <numaif.h>
#endif

namespace HPHP { namespace alloc {

BumpAllocState::BumpAllocState(uintptr_t base, size_t maxCap, LockPolicy p)
  : m_base(base)
  , m_maxCapacity(maxCap)
  , m_lockPolicy(p) {
  auto ret = mmap((void*)base, maxCap, PROT_NONE,
                  MAP_ANONYMOUS | MAP_PRIVATE | MAP_NORESERVE,
                  -1, 0);
  if (ret != (void*)base) {
    char msg[128];
    if (ret == MAP_FAILED) {
      std::snprintf(msg, sizeof(msg),
                    "failed to reserve address range 0x%" PRIxPTR
                    " to 0x%" PRIxPTR ", errno = %d",
                    base, base + maxCap, errno);
    } else {
      munmap(ret, maxCap);
      std::snprintf(msg, sizeof(msg),
                    "failed to reserve address range 0x%" PRIxPTR
                    " to 0x%" PRIxPTR ", got 0x%p instead",
                    base, base + maxCap, ret);
    }
    throw std::runtime_error{msg};
  }
}

void BumpMapper::append(BumpMapper* m) {
  // add things one by one so that we can easily avoid loops in the chain.
  assert(!m->m_fallback);
  assert(m != this);
  if (m_fallback) {
    return m_fallback->append(m);
  }
  m_fallback = m;
}

bool Bump1GMapper::addMappingImpl(BumpAllocState& state, size_t /*newSize*/) {
  // Check quota and alignment before mmap
  if (m_currNumPages >= m_maxNumPages) {
    m_failed = true;
    return false;
  }
  auto const currFrontier = state.frontier();
  if (currFrontier % size1g != 0) return false;

  HugePageInfo info = get_huge1g_info();
  if (info.nr_hugepages == num_1g_pages()) {
    // The current process has grabbed all reserved 1G huge pages.  Fail all
    // subsequent efforts (because we don't release the huge pages before
    // shutting down, and we don't dynamically add 1G pages to the system).
    m_failed = true;
    return false;
  }
  if (info.free_hugepages <= 0) {
    // We have not obtained all the 1G pages we intended to, but someone else is
    // holding the page now.  This would probably make the page unusable to this
    // arena, even after the other process releases the 1G page because we need
    // 1G alignment in the frontier.
    return false;
  }

  auto const newPageStart = currFrontier;

#ifdef HAVE_NUMA
  assert((m_interleaveMask & ~numa_node_set) == 0);
  if (const int numAllowedNodes = __builtin_popcount(m_interleaveMask)) {
    int failCount = 0;
    // Try to map 1G pages in round-robin fashion starting from m_nextNode.  We
    // try on each allowed NUMA node at most once.
    do {
      auto const currNode = m_nextNode;
      m_nextNode = (currNode + 1) & numa_node_mask;
      if (!((1u << currNode) & m_interleaveMask)) {
        // Node not allowed, try next one.
        continue;
      }
      if (mmap_1g(reinterpret_cast<void*>(newPageStart),
                  currNode, /* MAP_FIXED */true)) {
        state.m_currCapacity += size1g;
        m_failed = (++m_currNumPages >= m_maxNumPages);
        return true;
      }
      if (++failCount >= numAllowedNodes) return false;
    } while (true);
  }
#endif
  // This covers cases when HAVE_NUMA is defined, and when `m_interleaveMask` is
  // set to 0 (on single-socket machines).
  if (mmap_1g(reinterpret_cast<void*>(newPageStart), -1,
              /* MAP_FIXED */ true)) {
    ++m_currNumPages;
    state.m_currCapacity += size1g;
    return true;
  }
  return false;
}

constexpr size_t kChunkSize = 4 * size2m;

bool Bump4KMapper::addMappingImpl(BumpAllocState& state, size_t /*newSize*/) {
  auto const currFrontier = state.frontier();
  if (currFrontier % size4k != 0) return false;
  void* newPageStart = reinterpret_cast<void*>(currFrontier);
  void* newPages = mmap(newPageStart, kChunkSize,
                        PROT_READ | PROT_WRITE,
                        MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED,
                        -1, 0);
  if (newPages == MAP_FAILED) return false;
  if (newPages != newPageStart) {
    // failed to get desired address range
    munmap(newPages, kChunkSize);
    return false;
  }
  state.m_currCapacity += kChunkSize;

#ifdef HAVE_NUMA
  if (m_interleaveMask) {
    unsigned long mask = m_interleaveMask;
    mbind(newPages, kChunkSize, MPOL_INTERLEAVE,
          &mask, 32 /* max node */, 0 /* flag */);
  }
#endif
  return true;
}

bool Bump2MMapper::addMappingImpl(BumpAllocState& state, size_t newSize) {
  // Check quota and alignment before trying to map.  Note that m_maxNumPages
  // may be initialized later for this mapper, so don't fail permanently before
  // it is initialized.
  if (m_currNumPages >= m_maxNumPages) {
    if (m_maxNumPages) {
      m_failed = true;
    }
    return false;
  }
  auto newPageBase = state.frontier();
  // Add some 4K pages before madvise()
  if (!Bump4KMapper::addMappingImpl(state, newSize)) {
    return false;
  }
  auto const nHugePages =
    std::min(static_cast<unsigned>(kChunkSize / size2m),
             m_maxNumPages - m_currNumPages);
  hintHuge(reinterpret_cast<void*>(newPageBase), nHugePages * size2m);
  m_currNumPages += nHugePages;
  return true;
}

} // namespace alloc
} // namespace HPHP
