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

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Usage stats for a request, all in bytes.
 *
 * If jemalloc is being used, then usage and peakUsage also include bytes that
 * are reported by jemalloc's per-thread stats that are allocated outside of
 * the MemoryManager APIs (mallocSmallSize, mallocBigSize, objMalloc.)
 * totalAlloc will also be maintained, otherwise it will be 0.
 */
struct MemoryUsageStats {
  int64_t mmUsage;    // bytes currently in use via MM apis
  int64_t threadUsage;// bytes currently in use via jemalloc apis

  int64_t capacity;   // sum of slabs & big objects (MM's capacity)
  int64_t limit;      // the max bytes allowed for a request before it is
                      // terminated for exceeding the memory limit
  int64_t peakUsage;  // how many bytes have been used at maximum
  int64_t peakCap;    // peak bytes owned by MemoryManager (slabs and big)
  int64_t peakIntervalUsage; // peakUsage during userland interval
  int64_t peakIntervalCap; // peakCap during userland interval

  int64_t totalAlloc; // how many bytes have cumulatively been allocated
                      // by the underlying allocator

  /*
   * Current malloc usage for this thread, minus the large objects
   * and slabs owned by MemoryManager.
   */
  int64_t auxUsage() const { return threadUsage - capacity; }

  /*
   * Current usage for this thread as the sum of MemoryManager usage
   * (allocated but not yet freed) plus direct jemalloc usage
   * (allocated-deallocated) that bypasses MemoryManager.
   */
  int64_t usage() const { return mmUsage + auxUsage(); }

  friend struct MemoryManager;
};

//////////////////////////////////////////////////////////////////////

}

#endif
