/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/clock/Clock.h>
#include <folly/portability/GMock.h>

namespace fizz {
namespace test {

class MockClock : public Clock {
 public:
  MOCK_METHOD(
      std::chrono::system_clock::time_point,
      getCurrentTime,
      (),
      (const));
};

} // namespace test
} // namespace fizz
