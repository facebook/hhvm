/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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

#include <chrono>
#include <cstdint>

namespace apache::thrift {

// This struct serves as a appendix data carrier that can store metadata about
// internal process e.g. logging info
struct ServerRequestData {
  using Clock = std::chrono::steady_clock;
  using TimePoint = Clock::time_point;
  using Duration = Clock::duration;

  TimePoint queueBegin;
  TimePoint requestExecutionBegin;
  TimePoint requestExecutionEnd;

  void setRequestExecutionBegin(TimePoint now = Clock::now());
  void setRequestExecutionEnd(TimePoint now = Clock::now());

  Duration queuedDuration() const;
  Duration requestExecutionDuration() const;

  intptr_t requestPileUserData = 0;
  intptr_t concurrencyControllerUserData = 0;
};

} // namespace apache::thrift
