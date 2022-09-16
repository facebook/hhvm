/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdint.h>

namespace watchman {

// This is the Hash128to64 function from Google's cityhash (available
// under the MIT License).  We use it to reduce multiple 64 bit hashes
// into a single hash.
inline uint64_t hash_128_to_64(const uint64_t upper, const uint64_t lower) {
  // Murmur-inspired hashing.
  const uint64_t kMul = 0x9ddfea08eb382d69ULL;
  uint64_t a = (lower ^ upper) * kMul;
  a ^= (a >> 47);
  uint64_t b = (upper ^ a) * kMul;
  b ^= (b >> 47);
  b *= kMul;
  return b;
}

inline constexpr uint64_t hash_combine(std::initializer_list<uint64_t> hashes) {
  const uint64_t* b = hashes.begin();
  const uint64_t* e = hashes.end();
  if (b == e) {
    return 0;
  } else {
    uint64_t rv = *b++;
    while (b != e) {
      rv = hash_128_to_64(rv, *b++);
    }
    return rv;
  }
}

} // namespace watchman
