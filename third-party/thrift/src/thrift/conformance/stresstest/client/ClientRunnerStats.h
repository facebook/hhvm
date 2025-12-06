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

#include <folly/stats/Histogram.h>

namespace apache::thrift::stress {

struct ClientRpcStats {
  ClientRpcStats();
  void combine(const ClientRpcStats& other);

  folly::Histogram<double> latencyHistogram;
  uint64_t numSuccess{0};
  uint64_t numFailure{0};
};

struct ClientThreadMemoryStats {
  void combine(const ClientThreadMemoryStats& other);
  size_t threadStart{0};
  size_t connectionsEstablished{0};
  size_t p50{0};
  size_t p99{0};
  size_t p100{0};
  size_t connectionsIdle{0};
};

} // namespace apache::thrift::stress
