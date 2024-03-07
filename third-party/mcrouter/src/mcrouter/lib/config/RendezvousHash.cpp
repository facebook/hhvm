/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "RendezvousHash.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include <folly/hash/Hash.h>

namespace facebook {
namespace memcache {

/*
 * The algorithm of RendezvousHash goes like this:
 * Assuming we have 3 clusters with names and weights:
 * ==============================================
 * | Cluster1     | Cluster2     | Cluster3     |
 * | n="cluster1" | n="cluster2" | n="cluster3" |
 * | w=100        | w=400        | w=500        |
 * ==============================================
 * To prepare, we calculate a hash for each cluster based on its name:
 * ==============================================
 * | Cluster1     | Cluster2     | Cluster3     |
 * | n="cluster1" | n="cluster2" | n="cluster3" |
 * | w = 100      | w=400        | w=500        |
 * | h = hash(n)  | h = hash(n)  | h = hash(n)  |
 * ==============================================
 *
 * When a key comes, we have to decide which cluster we want to assign it to:
 * E.g., k = 10240
 *
 * For each cluster, we calculate a combined hash with the sum of key
 * and the cluster's hash:
 *
 *
 *                             ==============================================
 *                             | Cluster1     | Cluster2     | Cluster3     |
 *                                               ...
 *        k                    | h=hash(n)    | h = hash(n)  | h=hash(n)    |
 *        |                    ==============================================
 *        |                                          |
 *        +-------------+----------------------------+
 *                      |
 *                  ch=hash(h + k)
 *                      |
 *                      v
 * ==============================================
 * | Cluster1     | Cluster2     | Cluster3     |
 * | n="cluster1" | n="cluster2" | n="cluster3" |
 * | w=100        | w=400        | w=500        |
 * | h=hash(n)    | h = hash(n)  | h=hash(n)    |
 * |ch=hash(h+k)  |ch = hash(h+k)|ch=hash(h+k)  |
 * ==============================================
 *
 * ch is now a random variable from 0 to max_int that follows
 * uniform distribution,
 * we need to scale it to a r.v. * from 0 to 1 by dividing it with max_int:
 *
 * scaledHash = ch / max_int
 *
 * ==============================================
 * | Cluster1     | Cluster2     | Cluster3     |
 *                    ....
 * |ch=hash(h+k)  |ch = hash(h+k)|ch=hash(h+k)  |
 * ==============================================
 *                      |
 *                    sh=ch/max_int
 *                      |
 *                      v
 * ==============================================
 * | Cluster1     | Cluster2     | Cluster3     |
 *                    ....
 * |ch=hash(h+k)  |ch = hash(h+k)|ch=hash(h+k)  |
 * |sh=ch/max_int |sh=ch/max_int |sh=ch/max_int |
 * ==============================================
 *
 * We also need to respect the weights, we have to scale it again with
 * a function of its weight:
 *
 * ==============================================
 * | Cluster1     | Cluster2     | Cluster3     |
 *                    ....
 * |sh=ch/max_int |sh=ch/max_int |sh=ch/max_int |
 * ==============================================
 *                      |
 *                      |
 *               sw = pow(sh, 1/w)
 *                      |
 *                      V
 * ==============================================
 * | Cluster1     | Cluster2     | Cluster3     |
 *                    ....
 * |sh=ch/max_int |sh=ch/max_int |sh=ch/max_int |
 * |sw=pow(sh,1/w)|sw=pow(sh,1/w)|sw=pow(sh,1/w)|
 * ==============================================
 *
 * We now calculate who has the largest sw, that is the cluster that we are
 * going to map k into:
 * ==============================================
 * | Cluster1     | Cluster2     | Cluster3     |
 *                    ....
 * |sw=pow(sh,1/w)|sw=pow(sh,1/w)|sw=pow(sh,1/w)|
 * ==============================================
 *                      |
 *                     max(sw)
 *                      |
 *                      V
 *                   Cluster
 *
 */
size_t RendezvousHash::get(uint64_t key) const {
  double maxWeight = -1.0;
  size_t maxWeightId = 0;
  for (size_t i = 0; i < nodes_.size(); ++i) {
    auto& it = nodes_[i];
    // combine the hash with the cluster together
    const double combinedHash = computeHash(it.first + key);
    // Note that double(UINT64_MAX) cannot be exactly represented as a float. We
    // ignore the small inaccuracy here.
    const double scaledHash = combinedHash /
        static_cast<double>(std::numeric_limits<uint64_t>::max());
    const double scaledWeight =
        it.second > std::numeric_limits<double>::epsilon()
        ? std::pow(scaledHash, 1.0 / it.second)
        : 0.0;
    if (scaledWeight > maxWeight) {
      maxWeightId = i;
      maxWeight = scaledWeight;
    }
  }
  return maxWeightId;
}

void RendezvousHash::normalizeWeights() {
  double sum{0.0};
  for (const auto& it : nodes_) {
    sum += it.second;
  }
  if (sum < std::numeric_limits<double>::epsilon()) {
    throw std::invalid_argument("Sum of weights is 0");
  }
  for (auto& it : nodes_) {
    it.second /= sum;
  }
}

uint64_t RendezvousHash::computeHash(folly::StringPiece key) const {
  return folly::hash::fnv64_buf(key.data(), key.size());
}

uint64_t RendezvousHash::computeHash(uint64_t i) const {
  return folly::hash::twang_mix64(i);
}
} // namespace memcache
} // namespace facebook
