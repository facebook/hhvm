/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>
#include <vector>

namespace proxygen {

class ConsistentHash {
 public:
  virtual ~ConsistentHash() {
  }

  /**
   * build() builds the hashing pool based on a vector of nodes with their keys
   * and weights.
   *
   * The bevahior of calling build multiple times is undefined.
   *
   * build() is not thread safe with get(), documented below.
   */
  virtual void build(std::vector<std::pair<std::string, uint64_t>> &) = 0;

  /**
   * get(key, N) finds the node ranked N in the consistent hashing space
   * for the given key.
   *
   * The returning value is the node's index in the input vector of build().
   */
  virtual size_t get(const uint64_t key, const size_t rank = 0) const = 0;

  /**
   * get max error rate the current hashing space
   *
   */
  virtual double getMaxErrorRate() const = 0;
};
} // namespace proxygen
