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
#include "hphp/runtime/base/purger.h"

#include "hphp/runtime/base/runtime-option.h"

#include "hphp/util/compilation-flags.h"
#include "hphp/util/logger.h"
#include "hphp/util/timer.h"

#include <atomic>
#include <folly/portability/SysMman.h>
#include <folly/portability/Unistd.h>

namespace HPHP {

namespace {

/*
 * Smoothstep of sigmoidal curves (https://en.wikipedia.org/wiki/Smoothstep)
 * that grow from 0 to 1 in 0 <= x <= 1.
 * smoothstep(x) = -2x^3  + 3x^2
 */
static double smoothstep(double x) {
  return x * x * (3 - 2 * x);
}

// Convert nbytes from Byte to MegaByte
static size_t mb(size_t nbytes) {
  return nbytes >> 20;
}

/*
 * Use decay() to determine how much memory we could have at the specific
 * time slot. And use a lookup table to reduce float calculation.
 */
size_t decay(size_t n, int64_t age, int64_t window) {
  return age <= 0 ? n :
         age >= window ? 0 :
         n - n * smoothstep(double(age) / window);
}

std::atomic<int> kNextid{0};
const int64_t kTime0{Timer::GetCurrentTimeMicros()};
const int64_t kPageSize{sysconf(_SC_PAGESIZE)};

}

Purger::Purger()
  : window_{RuntimeOption::EvalContiguousHeapWindowSize}
  , threshold_{RuntimeOption::EvalContiguousHeapThreshold}
  , bucket_slice_{window_ / kNumBuckets}
  , dirty_{0}
  , id_{kNextid++}
{}

void Purger::purge(char* base, char* front) {
  assert(uintptr_t(base) % kPageSize == 0);
  assert(uintptr_t(front) % kPageSize == 0);
  size_t size = front - base;
  dirty_ = std::max(size, dirty_);
  // If dirty_ exceeds threshold_, purge all the exceeded memory.
  if (dirty_ > threshold_) {
    madvise(base + threshold_, dirty_ - threshold_, MADV_DONTNEED);
    dirty_ = threshold_;
    return;
  }
  auto age = Timer::GetCurrentTimeMicros() - kTime0;
  // Update current bucket (reset to size if it expired)
  auto index = (age / bucket_slice_) % kNumBuckets;
  buckets_[index].max = age - buckets_[index].time > bucket_slice_ ? size :
                   std::max(buckets_[index].max, size);
  buckets_[index].time = age;
  // Max of decayed max heap sizes, which means the upper bound that
  // how much memory can still live in the heap.
  size_t max = 0;
  for (auto& b : buckets_) {
    max = std::max(max, decay(b.max, age - b.time, window_));
  }

  max = (max + kPageSize - 1) & ~(kPageSize - 1); // round up

  // If the dirty memory exceeds max, purge the exceeded part,
  // otherwise do nothing.
  if (dirty_ > max) {
    if (debug) {
      VLOG(1) << "purge " << id_ << ": t " << age/bucket_slice_
              << " size " << mb(size) << " max " << mb(max)
              << " dirty " << mb(dirty_) << " madvise " << mb(dirty_ - max)
              << std::endl;
    }
    madvise(base + max, dirty_ - max, MADV_DONTNEED);
    dirty_ = max;
  } else {
    if (debug) {
      VLOG(1) << "purge " << id_ << ": t " << age/bucket_slice_
              << " size " << mb(size)<< " max "<< mb(max)
              << " dirty " << mb(dirty_) << std::endl;
    }
  }
}

void Purger::flush(char* base) {
  assert(uintptr_t(base) % kPageSize == 0);
  if (debug) {
    auto age = Timer::GetCurrentTimeMicros() - kTime0;
    VLOG(1) << "flush " << id_ << ": t " << age/bucket_slice_
            << " dirty " << mb(dirty_) << std::endl;
  }
  madvise(base, dirty_, MADV_DONTNEED);
  dirty_ = 0;
}

}
