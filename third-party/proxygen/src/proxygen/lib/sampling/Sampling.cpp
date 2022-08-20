/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/sampling/Sampling.h>

#include <limits>

#include <glog/logging.h>

#include <folly/Random.h>
#include <folly/hash/Hash.h>

namespace proxygen {

Sampling::Sampling(double rate) {
  updateRate(rate);
}

Sampling::~Sampling() = default;

void Sampling::updateRate(double rate) {
  CHECK(rate >= 0.0 && rate <= 1.0);
  rate_ = rate;
  weight_ = rateToWeight(rate);
}

uint32_t Sampling::rateToWeight(double rate) {
  // we lose some decimals when the sampling fraction is very low
  uint32_t scaledRate = kErrTolerance * rate;
  // avoid division by zero
  if (scaledRate == 0) {
    return 0;
  }
  return kErrTolerance / scaledRate;
}

uint32_t Sampling::rateToKey(double rate) {
  return std::numeric_limits<uint32_t>::max() * rate;
}

uint32_t Sampling::getIntRate() const {
  return rateToKey(rate_);
}

bool Sampling::isLucky(uint32_t samplingKey) {
  return folly::Random::rand32() <= samplingKey;
}

bool Sampling::isLucky(const std::string& key) const {
  return isLucky(folly::StringPiece(key));
}

bool Sampling::isLucky(const folly::StringPiece key) const {
  // have a shortcut path for 1 (enabled) and 0 (disabled) as these are
  // gonna be the most common
  if (weight_ == 1) {
    return true;
  } else if (weight_ == 0) {
    return false;
  }
  // else compute the hash
  return folly::hash::fnv32_buf(key.data(), key.size()) < getIntRate();
}

bool Sampling::isLucky() const {
  // short path for rate = 1.0
  if (rate_ >= 1.0) {
    return true;
  }
  // roll the dice
  return folly::Random::randDouble01() < rate_;
}

} // namespace proxygen
