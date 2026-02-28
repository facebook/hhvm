/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/SpinLock.h>
#include <folly/concurrency/CacheLocality.h>

namespace facebook::memcache {

struct CacheClientCounters {
  size_t fetchCount{0};
  size_t fetchKeyBytes{0};
  size_t fetchValueBytes{0};
  size_t updateCount{0};
  size_t updateKeyBytes{0};
  size_t updateValueBytes{0};
  size_t invalidateCount{0};
  size_t invalidateKeyBytes{0};

  CacheClientCounters& operator+=(const CacheClientCounters& other) {
    fetchCount += other.fetchCount;
    fetchKeyBytes += other.fetchKeyBytes;
    fetchValueBytes += other.fetchValueBytes;
    updateCount += other.updateCount;
    updateKeyBytes += other.updateKeyBytes;
    updateValueBytes += other.updateValueBytes;
    invalidateCount += other.invalidateCount;
    invalidateKeyBytes += other.invalidateKeyBytes;

    return *this;
  }
};

class CacheClientStats {
 public:
  CacheClientCounters getCounters() const noexcept {
    std::unique_lock g(lock_);
    return counters_;
  }

  void recordFetchRequest(size_t keyBytes, size_t replyValueBytes) noexcept {
    std::unique_lock g(lock_);
    counters_.fetchCount++;
    counters_.fetchKeyBytes += keyBytes;
    counters_.fetchValueBytes += replyValueBytes;
  }

  void recordUpdateRequest(size_t keyBytes, size_t valueBytes) noexcept {
    std::unique_lock g(lock_);
    counters_.updateCount++;
    counters_.updateKeyBytes += keyBytes;
    counters_.updateValueBytes += valueBytes;
  }

  void recordInvalidateRequest(size_t keyBytes) noexcept {
    std::unique_lock g(lock_);
    counters_.invalidateCount++;
    counters_.invalidateKeyBytes += keyBytes;
  }

 private:
  alignas(folly::hardware_destructive_interference_size) mutable folly::SpinLock
      lock_;
  CacheClientCounters counters_;
};
} // namespace facebook::memcache
