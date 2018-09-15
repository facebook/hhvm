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

#ifndef incl_HPHP_RUNTIME_BASE_MEMORY_USAGESTATS_H_
#define incl_HPHP_RUNTIME_BASE_MEMORY_USAGESTATS_H_

#include "hphp/util/alloc.h" // must be included before USE_JEMALLOC is used

#include <folly/CPortability.h>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Usage stats for a request, all in bytes.
 *
 * Current usage is conceptually determined by subtracting total deallocations
 * from total allocations (both are measured since the beginning of the current
 * request). In practice, the latter value is split into mm_udebt and
 * mm_uallocated, with the requirement that at all times, (mm_uallocated -
 * mm_udebt) gives the actual allocation volume. The runtime is free to
 * arbitrarily add or subtract from mm_udebt as long as it makes the same
 * adjustments to mm_uallocated.
 *
 * This allows for cheap GC trigger checks in the allocator fast path: instead
 * of adding to mm_uallocated, we subtract from mm_udebt. "Subtract and check if
 * negative" can be done with just two instructions (on x86, at least),
 * allowing us to use mm_udebt to force the allocator into a slow path after a
 * certain number of bytes are allocated.
 *
 * If jemalloc is being used, then usage and peakUsage also include bytes that
 * are reported by jemalloc's per-thread stats that are allocated outside of
 * the MemoryManager APIs (mallocSmallSize, mallocBigSize, objMalloc.)
 * totalAlloc will also be maintained, otherwise it will be 0.
 */
struct MemoryUsageStats {
  /*
   * Since the first two are allowed to wrap around arbitrarily, and
   * we only care about the difference mod 2^64, we have to make them
   * unsigned to avoid UB.
   */
  uint64_t mm_udebt;      // Allocation debt
  uint64_t mm_uallocated; // Total allocation volume for MM APIs, including debt
  int64_t  mm_freed;      // Total deallocation volume for MM APIs
  int64_t  extUsage;      // cumulative allocations via jemalloc

  int64_t  malloc_cap; // capacity of malloc'd slabs & big objects
  int64_t  mmap_cap;   // capacity of mmap'd slabs & big objects
  int64_t  peakUsage;  // how many bytes have been used at maximum
  int64_t  peakCap;    // peak bytes owned by MemoryManager (slabs and big)
  int64_t  peakIntervalUsage; // peakUsage during userland interval
  int64_t  peakIntervalCap; // peakCap during userland interval

  int64_t  totalAlloc; // how many bytes have cumulatively been allocated
                       // by the underlying allocator

  int64_t  mmap_volume; // how many bytes have cumulatively been mmap'd
                        // by the HeapImpl, not counting munmaps or madvises.

  int64_t mmAllocated() const {
    auto const diff = mm_uallocated - mm_udebt;
    assertx(diff <= std::numeric_limits<int64_t>::max());
    return diff;
  }
  int64_t mmUsage() const { return mmAllocated() - mm_freed; }
  int64_t usage() const { return mmUsage() + auxUsage(); }
  int64_t capacity() const { return mmap_cap + malloc_cap; }
  int64_t auxUsage() const { return extUsage - malloc_cap; }
};

//////////////////////////////////////////////////////////////////////

}

#endif
