/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "WeightedCh3HashFunc.h"

#include <folly/dynamic.h>
#include <folly/hash/SpookyHashV2.h>

#include "mcrouter/lib/fbi/cpp/util.h"
#include "mcrouter/lib/fbi/hash.h"

namespace facebook {
namespace memcache {

size_t WeightedCh3HashFunc::hash(
    folly::StringPiece key,
    folly::Range<const double*> weights,
    size_t retryCount) {
  constexpr uint32_t kHashSeed = 0xface2014;

  auto n = weights.size();
  checkLogic(n && n <= furc_maximum_pool_size(), "Invalid pool size: {}", n);
  size_t salt = 0;
  size_t index = 0;
  std::string saltedKey;
  const auto originalKey = key;
  for (size_t i = 0; i < retryCount; ++i) {
    index = furc_hash(key.data(), key.size(), n);

    /* Use 32-bit hash, but store in 64-bit ints so that
       we don't have to deal with overflows */
    uint64_t p =
        folly::hash::SpookyHashV2::Hash32(key.data(), key.size(), kHashSeed);
    assert(0 <= weights[index] && weights[index] <= 1.0);
    uint64_t w = weights[index] * std::numeric_limits<uint32_t>::max();

    /* Rehash only if p is out of range */
    if (FOLLY_LIKELY(p < w)) {
      return index;
    }

    /* Change the key to rehash */
    if (saltedKey.empty()) {
      /* Reserve room for a few extra digits */
      saltedKey.reserve(originalKey.size() + 2);
      saltedKey = originalKey;
    }

    auto s = salt++;
    saltedKey.resize(originalKey.size());
    do {
      saltedKey.push_back(char(s % 10) + '0');
      s /= 10;
    } while (s > 0);

    key = saltedKey;
  }

  return index;
}

} // namespace memcache
} // namespace facebook
