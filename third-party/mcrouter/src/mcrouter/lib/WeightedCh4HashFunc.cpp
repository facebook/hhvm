/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "WeightedCh4HashFunc.h"

#include "mcrouter/lib/fbi/WeightedFurcHash.h"

namespace facebook {
namespace memcache {

size_t WeightedCh4HashFunc::hash(
    folly::StringPiece key,
    folly::Range<const double*> weights,
    size_t retryCount) {
  return mcrouter::weightedFurcHash(key, weights, retryCount);
}

} // namespace memcache
} // namespace facebook
