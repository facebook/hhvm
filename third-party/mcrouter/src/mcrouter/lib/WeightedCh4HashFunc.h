/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/lib/HashFunctionType.h"
#include "mcrouter/lib/WeightedChHashFuncBase.h"

namespace facebook {
namespace memcache {

/**
 * A weighted CH4 hash function.
 *
 * Each server is assigned a weight between 0.0 and 1.0 inclusive.
 * Unlike WeightedCh3HashFunc, this hashing function calls the new
 * weightedFurcHash with the provided weights, so there is no need to handle
 * the weight filtering in this level. This way, we save the computation of two
 * different hashes (furc + spooky), and we don't do retries with key variations
 * like in WeightedCh3Func.
 *
 * NOTES BELOW ARE COPIED FROM WeightedCh3HashFunc, ALSO APPLY HERE:
 *
 * Note that if all weights are 1.0, the algorithm returns the same indices
 * as simply CH3(key, n).
 *
 * The algorithm is consistent both with respect to individual weights and
 * mostly consistent wrt n. i.e. reducing any single weight slightly will
 * only spread out a small fraction of the load from that server to all other
 * servers, but changing the number of servers may involve some spillover.
 * Consistency is a function of how far the weights are from 1, with all weights
 * at 1 being perfectly consistent
 *
 * NOTE: The algorithm gives up after given no. of retries and returns the index
 * of the last retry. If the weights are too skewed or if there are too many
 * zeros in the vector then the algorithm can fail.

 * The probability of the algorithm returning in single attempt is equal to the
 * avg. weight (SUM(weights) / COUNT(weights)). If the avg. weight is not close
 * to 1 then retries are useful and one may need more retries if the avg. weight
 * is too low. For instance is avg. weight is 0.25, then each iteration of the
 * algorithm will fail with probability (1 - 0.25) and it requires upto 16
 * retries to bring the chances of failure below 1%.
 */

class WeightedCh4HashFunc : public WeightedChHashFuncBase {
 public:
  /**
   * @param weights  A list of server weights.
   *                 Pool size is taken to be weights.size()
   */
  explicit WeightedCh4HashFunc(std::vector<double> weights)
      : WeightedChHashFuncBase(std::move(weights)) {}

  /**
   * @param json  Json object of the following format:
   *              {
   *                "weights": [ ... ]
   *              }
   * @param n     Number of servers in the config.
   */
  WeightedCh4HashFunc(const folly::dynamic& json, size_t n)
      : WeightedChHashFuncBase(json, n) {}

  size_t operator()(folly::StringPiece key) const {
    return hash(key, weights_);
  }

  static const char* type() {
    return "WeightedCh4";
  }

  static HashFunctionType typeId() {
    return HashFunctionType::WeightedCh4;
  }

 private:
  static size_t hash(
      folly::StringPiece key,
      folly::Range<const double*> weights,
      size_t retryCount = kNumTries);
};

} // namespace memcache
} // namespace facebook
