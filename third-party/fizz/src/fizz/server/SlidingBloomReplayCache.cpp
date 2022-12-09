/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/server/SlidingBloomReplayCache.h>

#include <sys/types.h>

#include <fstream>

#include <folly/Conv.h>
#include <folly/hash/Hash.h>
#include <folly/io/IOBuf.h>
#include <folly/portability/Unistd.h>

#include <fizz/crypto/RandomGenerator.h>

#include <folly/Range.h>
#include <cmath>

using namespace folly::hash;

namespace fizz {
namespace server {
const static int kBitsPerByte = 0x8;
const static unsigned int kBucketCount = 12;
const static unsigned int kHashCount = 4;

// You can only have as many buckets as you have bits in your cell
static_assert(
    kBucketCount <= sizeof(SlidingBloomReplayCache::CellType) * kBitsPerByte,
    "Bucket count greater than cell bit count");

/*
 * This is largely tasked with calculating the appropriate bitSize to use given
 * the parameters.
 *
 * For a given Bloom Filter, its false positive probability can be expressed as
 *
 * p = (1 - e^(-kn/m))^k
 * k = number of hash functions
 * n = number of items in the filter
 * m = size of the filter (bitSize)
 *
 * n can be expanded to rps * ttl / buckets (the TTL is split into buckets, each
 * handling ttl/bucket seconds)
 *
 * Solving for m:
 * m = (k * rps * ttl) / (buckets * ln(1 - p^(1/k))
 */

SlidingBloomReplayCache::SlidingBloomReplayCache(
    int64_t ttlInSecs,
    size_t requestsPerSecond,
    double acceptableFPR,
    folly::EventBase* evb)
    : folly::AsyncTimeout(evb),
      executor_{folly::Executor::getKeepAliveToken(evb)} {
  if (acceptableFPR <= 0.0 || acceptableFPR >= 1.0) {
    throw std::runtime_error("false positive rate must lie between 0 and 1");
  }

  // Do all calculations with doubles.
  double ttlDouble = ttlInSecs;
  double rpsDouble = requestsPerSecond;
  double hashCountDouble = kHashCount;
  double bucketCountDouble = kBucketCount;
  double dividend = -hashCountDouble * rpsDouble * ttlDouble;
  double root = pow(acceptableFPR, 1.0 / hashCountDouble);
  double divisor = bucketCountDouble * log(1.0 - root);
  bitSize_ = std::ceil(dividend / divisor);
  VLOG(8) << "Initializing with bitSize = " << bitSize_;

  bucketWidthInMs_ =
      std::chrono::milliseconds(((ttlInSecs * 1000) / kBucketCount) + 1);

  // Reset bit buffer
  bitBuf_.reset(new CellType[bitSize_]());

  // Initialize current bucket
  currentBucket_ = 0;

  // Set up hashers
  for (unsigned int i = 0; i < kHashCount; i++) {
    hashers_.emplace_back(
        [seed = RandomNumGenerator<uint64_t>().generateRandom()](
            const unsigned char* buf, size_t len) -> uint64_t {
          return SpookyHashV2::Hash64((const void*)buf, len, seed);
        });
  }

  // Schedule reaping function (if evb given)
  if (executor_) {
    folly::via(
        executor_, [this]() { scheduleTimeout(bucketWidthInMs_.count()); });
  } else {
    VLOG(8) << "Started replay cache without reaping";
  }
}

SlidingBloomReplayCache::~SlidingBloomReplayCache() {
  if (executor_) {
    folly::via(executor_, [this]() { cancelTimeout(); }).get();
  }
}

void SlidingBloomReplayCache::set(folly::ByteRange query) {
  if (executor_) {
    executor_->dcheckIsInEventBaseThread();
  }

  CellType mask = (static_cast<CellType>(1)) << currentBucket_;

  for (auto& hasher : hashers_) {
    size_t idx = hasher(query.data(), query.size()) % bitSize_;

    bitBuf_[idx] |= mask;
  }
}

bool SlidingBloomReplayCache::test(folly::ByteRange query) const {
  if (executor_) {
    executor_->dcheckIsInEventBaseThread();
  }

  CellType ret(std::numeric_limits<CellType>::max());

  for (auto& hasher : hashers_) {
    size_t idx = hasher(query.data(), query.size()) % bitSize_;

    ret &= bitBuf_[idx];
  }

  return (ret != 0);
}

bool SlidingBloomReplayCache::testAndSet(folly::ByteRange query) {
  if (executor_) {
    executor_->dcheckIsInEventBaseThread();
  }

  CellType ret(std::numeric_limits<CellType>::max());
  CellType mask = (static_cast<CellType>(1)) << currentBucket_;

  for (auto& hasher : hashers_) {
    size_t idx = hasher(query.data(), query.size()) % bitSize_;

    ret &= bitBuf_[idx];
    bitBuf_[idx] |= mask;
  }

  return (ret != 0);
}

folly::SemiFuture<ReplayCacheResult> SlidingBloomReplayCache::check(
    std::unique_ptr<folly::IOBuf> query) {
  return folly::via(
             executor_,
             [this, queryBuffer = std::move(query)]() {
               const auto result = testAndSet(queryBuffer->coalesce())
                   ? ReplayCacheResult::MaybeReplay
                   : ReplayCacheResult::NotReplay;
               return result;
             })
      .semi();
}

void SlidingBloomReplayCache::clearBucket(size_t bucket) {
  VLOG(8) << "Clearing bit " << bucket << ", current bucket is "
          << currentBucket_;

  CellType mask = ~((static_cast<CellType>(1)) << bucket);
  for (size_t i = 0; i < bitSize_; ++i) {
    bitBuf_[i] &= mask;
  }
}

void SlidingBloomReplayCache::clear() {
  // Clear the soon to be occupied bucket
  clearBucket((currentBucket_ + 1) % kBucketCount);

  // Increment current bucket
  currentBucket_ = (currentBucket_ + 1) % kBucketCount;
}

void SlidingBloomReplayCache::timeoutExpired() noexcept {
  clear();
  scheduleTimeout(bucketWidthInMs_.count());
}
} // namespace server
}; // namespace fizz
