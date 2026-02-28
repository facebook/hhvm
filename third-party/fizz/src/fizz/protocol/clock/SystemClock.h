/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/clock/Clock.h>

namespace fizz {

class SystemClock : public Clock {
  std::chrono::system_clock::time_point getCurrentTime() const override;
};

} // namespace fizz
