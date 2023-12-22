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
#include <optional>
#include <utility>

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

  // If this request was stored in RoundRobinRequestPile we want to record the
  // bucket this request was assigned to so that we can later use this for
  // logging and counters.
  using RoundRobinRequestPileBucket = std::pair<size_t, size_t>;
  std::optional<RoundRobinRequestPileBucket> bucket;
};

} // namespace apache::thrift
