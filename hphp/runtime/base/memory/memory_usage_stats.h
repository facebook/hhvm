/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef incl_RUNTIME_BASE_MEMORY_USAGE_STATS_H_
#define incl_RUNTIME_BASE_MEMORY_USAGE_STATS_H_

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/**
 * Usage stats, all in bytes.
 */
struct MemoryUsageStats {
  int64 maxBytes;   // what's request's max bytes allowed
  int64 usage;      // how many bytes are currently being used
#if defined(USE_JEMALLOC)
  int64 jemallocDebt; // how many bytes of jemalloced memory have not
                      // been processed by MemoryManager::refreshStats
  int64 alloc;      // how many bytes are currently malloc-ed
#else
  union {
    int64 jemallocDebt; // unused
    int64 alloc;    // how many bytes are currently malloc-ed
  };
#endif
  int64 peakUsage;  // how many bytes have been dispensed at maximum
  int64 peakAlloc;  // how many bytes malloc-ed at maximum
  int64 totalAlloc; // how many bytes allocated, in total.
};

#define JEMALLOC_STATS_ADJUST(stats, amt) \
  if (hhvm && use_jemalloc) {             \
    (stats)->jemallocDebt += (amt);       \
  } else if (use_jemalloc) {              \
    (stats)->usage -= (amt);              \
  }

//////////////////////////////////////////////////////////////////////

}

#endif
