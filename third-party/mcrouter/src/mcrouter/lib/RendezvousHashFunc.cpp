/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/lib/RendezvousHashFunc.h"

#include "mcrouter/lib/fbi/hash.h"

#include <cmath>

namespace facebook {
namespace memcache {

RendezvousHashFunc::RendezvousHashFunc(
    const std::vector<folly::StringPiece>& endpoints,
    const folly::dynamic& /* json */) {
  endpointHashes_.reserve(endpoints.size());
  for (const auto ap : endpoints) {
    const uint64_t hash =
        murmur_hash_64A(ap.data(), ap.size(), kRendezvousHashSeed);
    endpointHashes_.push_back(hash);
  }
}

namespace {

std::vector<RendezvousIterator::ScoreAndIndex> get_scores(
    const std::vector<uint64_t>& endpointHashes,
    folly::StringPiece key) {
  std::vector<RendezvousIterator::ScoreAndIndex> scores;

  const uint64_t keyHash = RendezvousIterator::keyHash(key);

  scores.reserve(endpointHashes.size());
  for (size_t pos = 0; pos < endpointHashes.size(); ++pos) {
    const uint64_t score = hash128to64(endpointHashes[pos], keyHash);
    scores.emplace_back(score, pos);
  }

  return scores;
}
} // namespace

RendezvousIterator RendezvousHashFunc::begin(folly::StringPiece key) const {
  return RendezvousIterator(get_scores(endpointHashes_, key));
}

} // namespace memcache
} // namespace facebook
