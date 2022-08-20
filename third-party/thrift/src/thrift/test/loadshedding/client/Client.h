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

#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/stats/Histogram.h>
#include <thrift/test/loadshedding/if/gen-cpp2/BackendService.h>

#include <stdint.h>
#include <atomic>

namespace apache {
namespace thrift {
namespace test {

/**
 * Stats class use by the client to keep track of any request/response related
 * statistics (including latency, count of success/errors).
 */
class Stats {
 public:
  using SyncHistogram = folly::Synchronized<folly::Histogram<uint64_t>>;

  Stats() {}

  void addValue(std::chrono::milliseconds latency) {
    latencyHistogram_.wlock()->addValue(latency.count());
  }

  double getPercentileEstimate(double percentile) {
    return latencyHistogram_.rlock()->getPercentileEstimate(percentile);
  }

  void clear() {
    responseCount = 0;
    success = 0;
    errors = 0;
    latencyHistogram_.wlock()->clear();
  }

  std::atomic<uint64_t> responseCount{0};
  std::atomic<uint64_t> success{0};
  std::atomic<uint64_t> errors{0};

 private:
  SyncHistogram latencyHistogram_{folly::Histogram<uint64_t>{1, 0, 30'000}};
};

/**
 * Client that send a sustained traffic to a server following a Poisson
 * distribution for a specific amount of time.
 */
class Client {
 public:
  Client(const std::string& addr, int port);
  ~Client();

  void runSynchronously(double rps, std::chrono::duration<double> duration);

  uint64_t getResponseCount() const { return stats_.responseCount; }

  uint64_t getSuccess() const { return stats_.success; }

  uint64_t getError() const { return stats_.errors; }

  double getLatencyPercentile(double p) {
    return stats_.getPercentileEstimate(p);
  }

  void clearStats() { stats_.clear(); }

 private:
  folly::ScopedEventBaseThread scopedEventBaseThread_{"ClientIO"};
  std::unique_ptr<facebook::thrift::test::BackendServiceAsyncClient> client_;
  Stats stats_;
};

} // namespace test
} // namespace thrift
} // namespace apache
