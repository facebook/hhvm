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

#include <thrift/conformance/stresstest/client/ClientRunnerStats.h>

namespace apache::thrift::stress {

/**
 * Container for aggregated stress test statistics.
 * Combines memory usage metrics and RPC performance metrics.
 */
struct StressTestStats {
  /**
   * Logs formatted statistics including:
   * - Total requests (success/failure counts)
   * - Average QPS (Queries Per Second)
   * - Latency percentiles (P50, P99, P100) in microseconds
   * - Memory stats at different lifecycle stages
   */
  void log() const;

  int64_t runtimeSeconds;
  ClientThreadMemoryStats memoryStats;
  ClientRpcStats rpcStats;
};

} // namespace apache::thrift::stress
