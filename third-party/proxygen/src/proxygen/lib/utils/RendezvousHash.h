/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/utils/ConsistentHash.h>
#include <string>
#include <vector>

namespace proxygen {
/*
 * Weighted Rendezvous Hash is a way to consistently route requests to
 * candidates.
 * Unlike ConsistentHash, Weighted Rendezvous Hash supports the action to
 * reduce the relative weight of a candidate while incurring minimum data
 * movement.
 */
class RendezvousHash : public ConsistentHash {
 public:
  double getMaxErrorRate() const override;

  void build(std::vector<std::pair<std::string, uint64_t>>&) override;

  size_t get(const uint64_t key, const size_t rank = 0) const override;

  std::vector<size_t> selectNUnweighted(const uint64_t key,
                                        const size_t rank) const;

 private:
  size_t getNthByWeightedHash(const uint64_t key,
                              const size_t modRank,
                              std::vector<size_t>* returnRankIds) const;
  uint64_t computeHash(const char* data, size_t len) const;

  uint64_t computeHash(uint64_t i) const;

  std::vector<std::pair<uint64_t, uint64_t>> weights_;
};

} // namespace proxygen
