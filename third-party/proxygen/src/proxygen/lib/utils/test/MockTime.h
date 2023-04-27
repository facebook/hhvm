/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <glog/logging.h>
#include <proxygen/lib/utils/Time.h>

namespace proxygen {

template <typename ClockType = std::chrono::steady_clock>
class MockTimeUtilGeneric : public TimeUtilGeneric<ClockType> {
 public:
  void advance(std::chrono::milliseconds ms) {
    t_ += ms;
  }

  void setCurrentTime(std::chrono::time_point<ClockType> t) {
    CHECK(t.time_since_epoch() > t_.time_since_epoch())
        << "Time can not move backwards";
    t_ = t;
  }

  void verifyAndClear() {
  }

  std::chrono::time_point<ClockType> now() const override {
    return t_;
  }

 private:
  std::chrono::time_point<ClockType> t_;
};

using MockTimeUtil = MockTimeUtilGeneric<>;

} // namespace proxygen
