/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <assert.h>
#include <folly/Bits.h>

#include "mcrouter/lib/fbi/WeightedFurcHash.h"
#include "mcrouter/lib/fbi/hash.h"

namespace facebook {
namespace mcrouter {

/**
 * weightedFurcHash -- a consistent hash function using a binary decision tree
 * that supports weights.
 * Based on an algorithm originally by Mark Rabkin (see doc in hash.c)
 * The main idea in the weights extension (by Ori Shalev) is to use the next 32
 * bits in the murmur hash stream to check against the allowed weight range, and
 * continue with bucket selection if not accepted.
 * This improves on the current WeightedCh3 that recalculates both furc hash and
 * a Spooky hash on key variations.
 */

namespace {

/* Seed constant for MurmurHash64A selected by search for optimum diffusion
 * including recursive application.
 */
constexpr uint32_t kSeed = 4193360111ul;

/* Maximum number tries for in-range result before just returning 0. */
constexpr uint32_t kMaxFurcRetries = 32;

/* Gap in bit index per try; limits us to 2^kFurcShift shards.  Making this
 * larger will sacrifice a modest amount of performance.
 */
constexpr uint32_t kFurcShift = 23;

// Size of cache for hash values; should be > kMaxFurcRetries * (kFurcShift + 1)
constexpr uint32_t kFurcCacheSizePower = 10U;
constexpr uint32_t kFurcCacheSize = 1U << kFurcCacheSizePower;
constexpr uint32_t kFurcCacheSizeMask = kFurcCacheSize - 1U;

using HashCache = std::array<uint64_t, kFurcCacheSize>;

uint64_t murmur_rehash_64A(uint64_t k) {
  const uint64_t m = 0xc6a4a7935bd1e995ULL;
  const int r = 47;

  uint64_t h = static_cast<uint64_t>(kSeed) ^ (sizeof(uint64_t) * m);

  k *= m;
  k ^= k >> r;
  k *= m;

  h ^= k;
  h *= m;

  h ^= h >> r;
  h *= m;
  h ^= h >> r;

  return h;
}

void furcKickStartHash(folly::StringPiece key, HashCache& hash) {
  hash[0] = murmur_hash_64A(key.data(), key.size(), kSeed);
}

void furcFillCacheForOffset(HashCache& hash, uint32_t ord, uint32_t& old_ord) {
  if (old_ord >= ord) {
    return;
  }
  // Shouldn't peek too far ahead because the cache is cyclic
  assert(ord - old_ord < kFurcCacheSize / 2);
  for (auto n = old_ord + 1; n <= ord; n++) {
    hash[n & kFurcCacheSizeMask] =
        murmur_rehash_64A(hash[(n - 1) & kFurcCacheSizeMask]);
  }
  old_ord = ord;
}

/**
 * furcGetBit -- the bitstream generator
 *
 * Given a key and an index, provides a pseudorandom bit dependent on both.
 * Caches hash values.
 */
uint32_t furcGetBit(const uint32_t idx, HashCache& hash, uint32_t& old_ord) {
  uint32_t ord = idx >> 6;
  furcFillCacheForOffset(hash, ord, old_ord);
  return (hash[ord & kFurcCacheSizeMask] >> (idx & 0x3f)) & 0x1;
}

uint32_t
furcGetNext32Bits(const uint32_t idx, HashCache& hash, uint32_t& old_ord) {
  // Make sure that the next 32 bits are in cache
  furcFillCacheForOffset(hash, (idx + 32) >> 6, old_ord);
  uint32_t ord = idx >> 6;
  uint32_t bitIdx = idx & 0x3f;
  if (bitIdx <= 32) {
    return static_cast<uint32_t>(
        (hash[ord & kFurcCacheSizeMask] >> bitIdx) & 0xffffffff);
  }
  return static_cast<uint32_t>(
      ((hash[ord & kFurcCacheSizeMask] >> bitIdx) +
       (hash[(ord + 1) & kFurcCacheSizeMask] << (64 - bitIdx))) &
      0xffffffff);
}

} // namespace

uint32_t weightedFurcHash(
    folly::StringPiece key,
    folly::Range<const double*> weights,
    uint32_t maxRetries) {
  uint32_t m = weights.size();
  uint32_t num = m;
  uint32_t a;
  HashCache hash;

  if (m <= 1) {
    return 0;
  }

  uint32_t msb = folly::findLastSet(m - 1);
  uint32_t d = msb;
  uint32_t startIdx = 0;
  a = d;
  uint32_t old_ord = 0;
  uint32_t furcAttempt = 0;
  uint32_t overallAttempt = 0;
  uint32_t next32bits = 0;
  furcKickStartHash(key, hash);
  while (overallAttempt < maxRetries) {
    while (!furcGetBit(a, hash, old_ord)) {
      if (--d == startIdx) {
        num = 0;
        break;
      }
      a = d;
    }
    a += kFurcShift; // We should make progress even in the case of num == 0
    if (num != 0) { // It's not the all-zeros case where we set num = 0
      num = 1;
      for (uint32_t i = startIdx; i < d - 1; i++) {
        num = (num << 1) | furcGetBit(a, hash, old_ord);
        a += kFurcShift;
      }
      if (num >= m) {
        if (++furcAttempt == kMaxFurcRetries) { // Last attempt, default to 0
          num = 0;
        } else {
          num = m;
          continue;
        }
      }
    }
    assert(0 <= weights[num] && weights[num] <= 1.0);
    if (weights[num] == 1) {
      return num;
    }
    uint64_t weightAsInt = weights[num] * std::numeric_limits<uint32_t>::max();
    next32bits = furcGetNext32Bits(a, hash, old_ord);

    if (next32bits < weightAsInt) {
      return num;
    }

    furcAttempt = 0;
    startIdx = a;
    d = a + msb;
    a = d;
    num = m;
    overallAttempt++;
  }

  // Give up; distribute the result using the last peeked 32 bits.
  return next32bits % m;
}

} // namespace mcrouter
} // namespace facebook
