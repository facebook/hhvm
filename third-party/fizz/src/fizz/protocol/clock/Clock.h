/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>

namespace fizz {

// Simple clock abstraction to facilitate testing.
class Clock {
 public:
  virtual ~Clock() = default;
  virtual std::chrono::system_clock::time_point getCurrentTime() const = 0;
};

} // namespace fizz
