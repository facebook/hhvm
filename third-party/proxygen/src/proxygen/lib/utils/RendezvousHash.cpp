/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/utils/RendezvousHash.h>

#include <algorithm>
#include <folly/hash/Hash.h>
#include <limits>
#include <map>
#include <math.h> /* pow */
#include <vector>

namespace proxygen {
void RendezvousHash::build(
    std::vector<std::pair<std::string, uint64_t>>& nodes) {
  for (auto it = nodes.begin(); it != nodes.end(); ++it) {
    std::string key = it->first;
    uint64_t weight = it->second;
    weights_.emplace_back(computeHash(key.c_str(), key.size()), weight);
  }
}

void RendezvousHash::buildEqualWeights(std::vector<uint64_t>& nodes) {
  for (const auto& hash : nodes) {
    weights_.emplace_back(hash, /*weight*/ 1);
  }
}

/*
 * The algorithm of RendezvousHash goes like this:
 * Assuming we have 3 clusters with names and weights:
 * ==============================================
 * | Cluster1     | Cluster2     | Cluster3     |
 * | n="ash4c07"  | n="frc1c12"  | n="prn1c11"  |
 * | w=100        | w=400        | w=500        |
 * ==============================================
 * To prepare, we calculate a hash for each cluster based on its name:
 * ==============================================
 * | Cluster1     | Cluster2     | Cluster3     |
 * | n="ash4c07"  | n="frc1c12"  | n="prn1c11"  |
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
 * | n="ash4c07"  | n="frc1c12"  | n="prn1c11"  |
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
size_t RendezvousHash::get(const uint64_t key, const size_t rank) const {
  return getNthByWeightedHash(key, rank, nullptr);
}

/*
 * Calculate Hash scaled by weight and return Top N weights.
 * */
size_t RendezvousHash::getNthByWeightedHash(
    const uint64_t key,
    const size_t rank,
    std::vector<size_t>* returnRankIds) const {
  size_t modRank = rank % weights_.size();
  // optimize if required to return element with max weight, rank ==
  // weights_.size(), keep track of the maxWeightIndex instead of populating
  // scaledWeights array.
  double maxWeight = -1;
  int maxWeightIndex = 0;

  std::vector<std::pair<double, size_t>> scaledWeights;
  if (modRank != 0) {
    scaledWeights.reserve(weights_.size());
  }
  for (size_t i = 0; i < weights_.size(); ++i) {
    const auto& entry = weights_[i];
    // combine the hash with the cluster together
    double combinedHash = computeHash(entry.first + key);
    double scaledHash =
        (double)combinedHash / std::numeric_limits<uint64_t>::max();
    double scaledWeight = 0;
    if (entry.second != 0) {
      scaledWeight = pow(scaledHash, (double)1 / entry.second);
    }
    if (modRank == 0) {
      if (scaledWeight > maxWeight) {
        maxWeight = scaledWeight;
        maxWeightIndex = i;
      }
    } else {
      scaledWeights.emplace_back(scaledWeight, i);
    }
  }

  size_t rankIndex;
  if (modRank == 0) {
    rankIndex = maxWeightIndex;
  } else {
    std::nth_element(scaledWeights.begin(),
                     scaledWeights.begin() + modRank,
                     scaledWeights.end(),
                     std::greater<std::pair<double, size_t>>());
    rankIndex = scaledWeights[modRank].second;
  }

  if (returnRankIds) {
    if (modRank == 0) {
      returnRankIds->push_back(rankIndex);
    } else {
      returnRankIds->reserve(modRank);
      for (size_t i = 0; i < modRank; i++) {
        returnRankIds->push_back(scaledWeights[i].second);
      }
    }
  }

  return rankIndex;
}

/*
 * Returns a consistent hash selection of N elements from array.
 *
 * This type of selection only obeys the probability distribution
 * when all weights are identical.
 * */
std::vector<size_t> RendezvousHash::selectNUnweighted(const uint64_t key,
                                                      const size_t rank) const {
  std::vector<size_t> selection;
  // shortcut if rank is equal or larger than array size
  if (rank >= weights_.size()) {
    selection = std::vector<size_t>(weights_.size());
    std::generate(
        selection.begin(), selection.end(), [n = 0]() mutable { return n++; });
    return selection;
  }

  getNthByWeightedHash(key, rank, &selection);
  return selection;
}

uint64_t RendezvousHash::computeHash(const char* data, size_t len) const {
  return folly::hash::fnv64_buf(data, len);
}

uint64_t RendezvousHash::computeHash(uint64_t i) const {
  return folly::hash::twang_mix64(i);
}

double RendezvousHash::getMaxErrorRate() const {
  return 0;
}

} // namespace proxygen
