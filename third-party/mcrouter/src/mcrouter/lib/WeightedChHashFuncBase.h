/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <unordered_map>
#include <vector>

#include <folly/Range.h>

namespace folly {
struct dynamic;
} // namespace folly

namespace facebook {
namespace memcache {

class WeightedChHashFuncBase {
 public:
  /**
   * @param weights  A list of server weights.
   *                 Pool size is taken to be weights.size()
   */
  explicit WeightedChHashFuncBase(std::vector<double> weights)
      : weights_(std::move(weights)) {}

  /**
   * @param json  Json object of the following format:
   *              {
   *                "weights": [ ... ]
   *              }
   * @param n     Number of servers in the config.
   */
  WeightedChHashFuncBase(const folly::dynamic& json, size_t n)
      : weights_(parseWeights(json, n)) {}

  /**
   * @return Saved weights.
   */
  const std::vector<double>& weights() const {
    return weights_;
  }

  static std::vector<double> parseWeights(const folly::dynamic& json, size_t n);

 protected:
  static constexpr size_t kNumTries = 32;

  const std::vector<double> weights_;
};

} // namespace memcache
} // namespace facebook
