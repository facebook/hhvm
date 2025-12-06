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

#include <thrift/conformance/stresstest/common/StressTestStats.h>

#include <fmt/core.h>
#include <glog/logging.h>

namespace apache::thrift::stress {

void StressTestStats::log() const {
  LOG(INFO) << fmt::format(
      "Total requests:        {} ({} succeeded, {} failed)",
      (rpcStats.numSuccess + rpcStats.numFailure),
      rpcStats.numSuccess,
      rpcStats.numFailure);
  LOG(INFO) << fmt::format(
      "Average QPS:           {:.2f}",
      (static_cast<double>(rpcStats.numSuccess) / runtimeSeconds));
  LOG(INFO) << fmt::format(
      "Request latency:       P50={:.2f}us, P99={:.2f}us, P100={:.2f}us",
      rpcStats.latencyHistogram.getPercentileEstimate(.5),
      rpcStats.latencyHistogram.getPercentileEstimate(.99),
      rpcStats.latencyHistogram.getPercentileEstimate(1.0));
  LOG(INFO) << "Allocated memory stats:";
  LOG(INFO) << fmt::format(
      "  Before test:         {} bytes", memoryStats.threadStart);
  LOG(INFO) << fmt::format(
      "  Clients connected:   {} bytes", memoryStats.connectionsEstablished);
  LOG(INFO) << fmt::format(
      "  During test:         P50={} bytes, P99={} bytes, P100={} bytes",
      memoryStats.p50,
      memoryStats.p99,
      memoryStats.p100);
  LOG(INFO) << fmt::format(
      "  Clients idle:        {} bytes", memoryStats.connectionsIdle);
}

} // namespace apache::thrift::stress
