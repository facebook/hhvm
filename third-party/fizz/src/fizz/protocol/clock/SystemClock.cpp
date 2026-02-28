/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/protocol/clock/SystemClock.h>

namespace fizz {

std::chrono::system_clock::time_point SystemClock::getCurrentTime() const {
  return std::chrono::system_clock::now();
}

} // namespace fizz
