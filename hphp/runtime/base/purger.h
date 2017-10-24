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

#ifndef incl_HPHP_PURGER_H_
#define incl_HPHP_PURGER_H_

#include <array>
#include <cstddef>
#include <cstdint>

namespace HPHP {

/*
 * Purger implements decay-based purging of unused dirty pages using a similar
 * algorithm to jemalloc. we maintain a number of buckets, representing time
 * slices over the decay time window. Each bucket tracks the max heap size
 * observed in that time slice.
 *
 * Each time purge() is called, we apply the decay function to the heap size for
 * each slice, to arrive at the number of unused dirty pages we would like to
 * retain, then purge the rest by calling madvise(MADV_DONTNEED).
 *
 * There are a few tuning parameters in Purger:
 * kNumBuckets: 16,     the number of time slices over the decay time window
 * RuntimeOption::ContiguousHeapThreshold: (bytes) max retained dirty memory
 *   anything over this is eagerly purged. default 128MB.
 * RuntimeOption::ContiguousHeapWindowSize: (ns) decay time window. default 5s.
 */

struct Purger {
  Purger();

  /*
   * Purge the memory from base to front, according to the decay curve
   */
  void purge(char* base, char* front);

  /*
   * Purge all the dirty pages starting from base
   */
  void flush(char* base);

private:
  static const int64_t kNumBuckets = 16;
  struct Bucket {
    int64_t time; // most recent timestamp for this bucket
    size_t max;   // max heap size in this bucket time slice
  };

  // Time window size for purge size calculation. The amount of memory that
  // is allowed to be alive will be set to the maximum memory amount of the
  // latest time window according to the decay curve.
  int64_t const window_;

  // the dirty_ memory threshold
  size_t const threshold_;
  int64_t const bucket_slice_;
  size_t dirty_; // bytes of dirty memory
  std::array<Bucket,kNumBuckets> buckets_; // buckets array for time window
  int const id_;
};

}

#endif
