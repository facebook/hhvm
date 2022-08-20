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

#include <thrift/lib/cpp/util/PausableTimer.h>

#include <stdint.h>

#include <folly/portability/SysTime.h>

namespace apache {
namespace thrift {
namespace util {

PausableTimer::PausableTimer(int timeLimit) {
  isTimeLimitFinite_ = timeLimit > 0;
  timeLimit_.tv_sec = static_cast<int>(timeLimit / 1000);
  timeLimit_.tv_usec = static_cast<int>((timeLimit % 1000) * 1000);
  reset();
}

void PausableTimer::reset() {
  if (isTimeLimitFinite_) {
    totalTimed_ = timeval{};
    lastRunningTime_ = timeval{};
    paused_ = true;
  }
}

void PausableTimer::start() {
  if (isTimeLimitFinite_) {
    if (paused_) {
      gettimeofday(&startTime_, nullptr);
      paused_ = false;
    }
  }
}

void PausableTimer::stop() {
  if (isTimeLimitFinite_) {
    if (!paused_) {
      struct timeval end;
      gettimeofday(&end, nullptr);
      timersub(&end, &startTime_, &lastRunningTime_);
      timeradd(&lastRunningTime_, &totalTimed_, &totalTimed_);
      paused_ = true;
    }
  }
}

bool PausableTimer::hasExceededTimeLimit() {
  if (isTimeLimitFinite_) {
    return timercmp(&totalTimed_, &timeLimit_, >);
  }

  return false;
}

bool PausableTimer::didLastRunningTimeExceedLimit(
    uint64_t limit_in_microseconds) {
  if (limit_in_microseconds == 0) {
    return false;
  }

  return 1000 * 1000 * lastRunningTime_.tv_sec +
      (uint64_t)lastRunningTime_.tv_usec >
      limit_in_microseconds;
}

} // namespace util
} // namespace thrift
} // namespace apache
