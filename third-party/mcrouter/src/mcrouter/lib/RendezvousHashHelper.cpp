/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/lib/RendezvousHashHelper.h"
#include "mcrouter/lib/fbi/hash.h"

namespace facebook {
namespace memcache {

RendezvousIterator::RendezvousIterator(std::vector<ScoreAndIndex> scores)
    : queue_{std::less<ScoreAndIndex>(), std::move(scores)} {}

RendezvousIterator& RendezvousIterator::operator++() {
  if (!queue_.empty()) {
    queue_.pop();
  }

  return *this;
}

uint64_t RendezvousIterator::keyHash(folly::StringPiece key) {
  return murmur_hash_64A(key.data(), key.size(), kRendezvousExtraHashSeed);
}

} // namespace memcache
} // namespace facebook
