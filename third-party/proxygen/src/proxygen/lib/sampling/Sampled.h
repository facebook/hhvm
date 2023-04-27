/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/sampling/Sampling.h>

namespace proxygen {

/**
 * implements the concept of a sampled object, which maintains the side effects
 * of using a sampling object. Designed to be inherited.
 */
class Sampled {

 public:
  Sampled() {
  }

  explicit Sampled(const Sampling& sampling) {
    sample(sampling);
  }

  virtual ~Sampled() {
  }

  void sample(const Sampling& sampling) {
    if (sampling.isLucky()) {
      samplingWeight_ = sampling.getWeight();
    }
  }

  uint32_t getSamplingWeight() const {
    return samplingWeight_;
  }

  bool isSampled() const {
    return samplingWeight_ > 0;
  }

 protected:
  uint32_t samplingWeight_{0};
};

} // namespace proxygen
