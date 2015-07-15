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
  int64_t maxBytes;   // the max bytes allowed for a request before it is
                      // terminated for exceeding the memory limit
  int64_t usage;      // how many bytes are currently being used
#if defined(USE_JEMALLOC)
  int64_t jemallocDebt; // how many bytes of jemalloced memory have not
                      // been processed by MemoryManager::refreshStats
#endif
  int64_t alloc;      // how many bytes are currently malloc-ed in slabs
                      // by the small-size allocator APIs
  int64_t peakUsage;  // how many bytes have been used at maximum
  int64_t peakAlloc;  // how many bytes malloc-ed in slabs by the small-size
                      // APIs at maximum
  int64_t totalAlloc; // how many bytes have cumulatively been allocated
                      // by the underlying allocator
  int64_t peakIntervalUsage; // peakUsage during a userland specified interval
  int64_t peakIntervalAlloc; // peakAlloc during a userland specified interval

#ifdef USE_JEMALLOC
  void borrow(size_t amt) { jemallocDebt += amt; }
#else
  void borrow(size_t) {}
#endif
};

//////////////////////////////////////////////////////////////////////

}

#endif
