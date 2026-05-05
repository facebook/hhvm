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

#include <concepts>
#include <cstdint>

namespace apache::thrift::fast_thrift {

// Direction of the pipeline — determines which callback increments/decrements
// the active-request gauge.
//   Server: onRead = incoming request  (active++), onWrite = outgoing response
//   (active--) Client: onWrite = outgoing request (active++), onRead = incoming
//   response  (active--)
enum class Direction { Server, Client };

// Concept for a counter type that can be incremented and read
template <typename T>
concept CounterConcept = requires(T t, int64_t delta) {
  { t.incrementValue(delta) } noexcept -> std::same_as<void>;
  { t.value() } noexcept -> std::same_as<int64_t>;
};

// Concept for stats types used by RocketMetricsHandler
// Requires rocket-layer counters with incrementValue and value methods
template <typename T>
concept FastThriftStatsConcept = requires(T t) {
  { t.rocketInbound } -> CounterConcept;
  { t.rocketOutbound } -> CounterConcept;
  { t.rocketErrors } -> CounterConcept;
  { t.rocketActive } -> CounterConcept;
  { t.thriftInbound } -> CounterConcept;
  { t.thriftOutbound } -> CounterConcept;
  { t.thriftErrors } -> CounterConcept;
  { t.thriftActive } -> CounterConcept;
};

// No-op counter for when stats are disabled
struct NoOpCounter {
  void incrementValue(int64_t) noexcept {}
  int64_t value() const noexcept { return 0; }
};

// Stats type that disables metrics collection.
// Used as the default template parameter for FastThriftServer.
// When NoStats is used, metrics handlers are not added to the pipeline.
struct NoStats {
  NoOpCounter rocketInbound;
  NoOpCounter rocketOutbound;
  NoOpCounter rocketErrors;
  NoOpCounter rocketActive;
  NoOpCounter thriftInbound;
  NoOpCounter thriftOutbound;
  NoOpCounter thriftErrors;
  NoOpCounter thriftActive;
};

} // namespace apache::thrift::fast_thrift
