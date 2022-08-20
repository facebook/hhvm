/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <memory>
#include <vector>

#include <fizz/server/ReplayCache.h>

#include <folly/io/async/AsyncTimeout.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/ScopedEventBaseThread.h>

namespace fizz {
namespace server {

class SlidingBloomReplayCache : public ReplayCache,
                                private folly::AsyncTimeout {
 public:
  // CellType is the actual data type holding the buckets for a cell. The
  // maximumum number of buckets corresponds to the bit size of the cell.
  // You can use one of uint8_t, uint16_t, uint32_t, or uint64_t.
  using CellType = uint64_t;
  using HashFunction = std::function<CellType(const unsigned char*, size_t)>;
  /*
   * Create a time bucketed bloom filter with following parameters:
   * - ttlInSeconds: TTL for each checked attempt in seconds.
   * - requestsPerSecond: Estimated amount of requests to be able to handle
   *   without exceeding the acceptable false positive rate.
   * - acceptableFPR: Acceptable rate of false positive for given TTL + RPS
   * - evb: EventBase to run the clearing function on.
   */
  SlidingBloomReplayCache(
      int64_t ttlInSeconds,
      size_t requestsPerSecond,
      double acceptableFPR,
      folly::EventBase* evb);
  ~SlidingBloomReplayCache() override;

  // Note: Do not use outside of a test environmemt.
  // When using, these functions must be called from the
  // event base thread (if it's not null).
  void set(folly::ByteRange query);
  bool test(folly::ByteRange query) const;
  bool testAndSet(folly::ByteRange query);

  folly::SemiFuture<ReplayCacheResult> check(
      std::unique_ptr<folly::IOBuf> query) override;

 private:
  void clearBucket(size_t bucket);
  void clear();
  void timeoutExpired() noexcept override;

  std::chrono::milliseconds bucketWidthInMs_;
  size_t bitSize_;

  size_t currentBucket_;

  // bit array as a buffer
  std::unique_ptr<CellType[]> bitBuf_;

  std::vector<HashFunction> hashers_;

  // Used for clearing and checking the cache.
  folly::Executor::KeepAlive<folly::EventBase> executor_;
};
} // namespace server
} // namespace fizz
