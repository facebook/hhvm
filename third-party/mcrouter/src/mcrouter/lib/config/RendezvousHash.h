/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdexcept>
#include <utility>
#include <vector>

#include <folly/Range.h>

namespace facebook {
namespace memcache {

class RendezvousHash {
 public:
  /**
   * @param Iterator  input iterator that iterates over pairs
   *                  { <string convertible to folly::StringPiece>, double }
   * @throws invalid_argument if weight is less than zero or
   *                          sum of all weights is zero.
   */
  template <class Iterator>
  RendezvousHash(Iterator begin, Iterator end) {
    nodes_.reserve(std::distance(begin, end));
    for (auto it = begin; it != end; ++it) {
      if (it->second < 0.0) {
        throw std::invalid_argument("Weight should be greater than 0");
      }
      nodes_.emplace_back(computeHash(it->first), it->second);
    }
    normalizeWeights();
  }

  /**
   * Get node id for given key.
   * @return 0 <= id < number of nodes. If nodes are empty, returns 0.
   */
  size_t get(uint64_t key) const;

 private:
  uint64_t computeHash(folly::StringPiece data) const;

  uint64_t computeHash(uint64_t i) const;

  // { node hash, node weight }
  std::vector<std::pair<uint64_t, double>> nodes_;

  /**
   * @throws invalid_argument if sum of weights is too small (< eps)
   */
  void normalizeWeights();
};
} // namespace memcache
} // namespace facebook
