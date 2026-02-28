// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#pragma once

#include <folly/Random.h>
#include <folly/random/xoshiro256pp.h>

namespace HPHP {

// Returns a thread_local folly::xoshiro256pp_64. The default folly
// ThreadLocalPRNG wraps a 32-bit xoshiro256pp, but some random algorithms
// (e.g. oneIn()) are faster when using 64 bits of entropy. Prefer passing
// this to folly::Random methods.
inline folly::xoshiro256pp_64& threadLocalRng64() {
  thread_local auto tl_rng = folly::Random::create<folly::xoshiro256pp_64>();
  return tl_rng;
}

} // namespace HPHP
