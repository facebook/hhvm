/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include <string>

#include <folly/Function.h>
#include <folly/Range.h>

namespace proxygen {

class Sampling {
 public:
  static const uint32_t kErrTolerance = 1000000;

  explicit Sampling(double rate = 1.0);
  virtual ~Sampling();

  static uint32_t rateToWeight(double rate);

  static uint32_t rateToKey(double rate);

  static bool isLucky(uint32_t samplingKey);

  bool isLucky() const;

  bool isLucky(const std::string& key) const;
  bool isLucky(folly::StringPiece key) const;

  uint32_t getWeight() const {
    return weight_;
  }

  void updateRate(double rate);

  uint32_t getIntRate() const;

  void runSampled(folly::FunctionRef<void()> func) {
    if (isLucky()) {
      func();
    }
  }

 private:
  double rate_{0.0};
  uint32_t weight_{0};
};

} // namespace proxygen
