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

#ifndef incl_HPHP_RUNTIME_BASE_MEMORY_USAGESTATS_H_
#define incl_HPHP_RUNTIME_BASE_MEMORY_USAGESTATS_H_

#include "hphp/util/alloc.h" // must be included before USE_JEMALLOC is used

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Usage stats, all in bytes.
 */
struct MemoryUsageStats {
  int64_t maxBytes;   // what's request's max bytes allowed
  int64_t usage;      // how many bytes are currently being used
#if defined(USE_JEMALLOC)
  int64_t jemallocDebt; // how many bytes of jemalloced memory have not
                      // been processed by MemoryManager::refreshStats
  int64_t alloc;      // how many bytes are currently malloc-ed
#else
  union {
    int64_t jemallocDebt; // unused
    int64_t alloc;    // how many bytes are currently malloc-ed
  };
#endif
  int64_t peakUsage;  // how many bytes have been dispensed at maximum
  int64_t peakAlloc;  // how many bytes malloc-ed at maximum
  int64_t totalAlloc; // how many bytes allocated, in total.
};

#define JEMALLOC_STATS_ADJUST(stats, amt)       \
  ((void)(use_jemalloc && ((stats)->jemallocDebt += (amt))))

//////////////////////////////////////////////////////////////////////

}

#endif
