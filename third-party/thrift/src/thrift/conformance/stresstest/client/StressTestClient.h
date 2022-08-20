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

#include <folly/experimental/coro/Task.h>
#include <folly/stats/Histogram.h>
#include <thrift/conformance/stresstest/if/gen-cpp2/StressTest.h>

namespace apache {
namespace thrift {
namespace stress {

class ClientThread;

struct ClientRpcStats {
  ClientRpcStats();
  void combine(const ClientRpcStats& other);

  folly::Histogram<double> latencyHistogram;
  uint64_t numSuccess{0};
  uint64_t numFailure{0};
};

/**
 * Wrapper around the generated StressTestAsyncClient to transparently collect
 * statistics of requests being sent
 */
class StressTestClient {
 public:
  explicit StressTestClient(
      std::shared_ptr<StressTestAsyncClient> client, ClientRpcStats& stats)
      : client_(std::move(client)), stats_(stats) {}

  folly::coro::Task<void> co_ping();

  folly::coro::Task<void> co_echo(const std::string& x);

  folly::coro::Task<void> co_requestResponseEb(const BasicRequest& req);

  folly::coro::Task<void> co_requestResponseTm(const BasicRequest& req);

  folly::coro::Task<void> co_streamTm(const StreamRequest& req);

  folly::coro::Task<void> co_sinkTm(const StreamRequest& req);

  bool connectionGood() const { return connectionGood_; }

 private:
  template <class Fn>
  folly::coro::Task<void> timedExecute(Fn&& fn);

  std::shared_ptr<StressTestAsyncClient> client_;
  ClientRpcStats& stats_;
  bool connectionGood_{true};
};

} // namespace stress
} // namespace thrift
} // namespace apache
