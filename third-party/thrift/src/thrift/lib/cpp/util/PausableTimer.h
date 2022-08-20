/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <stdint.h>
#include <sys/types.h>

#include <folly/portability/SysTime.h>

namespace apache {
namespace thrift {
namespace util {

class PausableTimer {
 public:
  explicit PausableTimer(int timeLimit = 0);

  void reset();
  void start();
  void stop();

  // isTimeLimitFinite_ && totalTimed_ > timeLimit_
  bool hasExceededTimeLimit();

  // limit_in_microseconds == 0 implies there is no time limit to
  // exceed, i.e. that this method will return false.
  bool didLastRunningTimeExceedLimit(uint64_t limit_in_microseconds);

 private:
  bool isTimeLimitFinite_;
  bool paused_;
  struct timeval timeLimit_;

  // time at which timer went from paused to running (not paused)
  // state
  struct timeval startTime_;

  // time timer has run, i.e. not been paused, since last reset
  struct timeval totalTimed_;

  // time elapsed between latest pause and the start before it
  struct timeval lastRunningTime_;
};

} // namespace util
} // namespace thrift
} // namespace apache
