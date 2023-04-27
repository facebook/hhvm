/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>

#include <folly/container/F14Map.h>
#include <proxygen/lib/sampling/Sampling.h>

namespace proxygen {

class MultiSampled {
 public:
  MultiSampled() {
  }

  virtual ~MultiSampled() {
  }

  void sample(const std::string& tag, const Sampling& sampling) {
    if (sampling.isLucky()) {
      weights_[tag] = sampling.getWeight();
    }
  }

  int getSamplingWeight(const std::string& tag) const {
    auto it = weights_.find(tag);
    if (it == weights_.end()) {
      return 0;
    }
    return it->second;
  }

  bool isSampled() const {
    // we are sampled if we have at least a non-zero weight
    return weights_.size() > 0;
  }

  bool isSampled(const std::string& tag) const {
    return getSamplingWeight(tag) > 0;
  }

 private:
  folly::F14FastMap<std::string, int> weights_;
};

} // namespace proxygen
