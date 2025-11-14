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

#include <folly/coro/Task.h>
#include <folly/stats/Histogram.h>
#include <thrift/conformance/stresstest/if/gen-cpp2/StressTest.h>

namespace apache::thrift::stress {

class ClientThread;

struct ClientRpcStats {
  ClientRpcStats();
  void combine(const ClientRpcStats& other);

  folly::Histogram<double> latencyHistogram;
  uint64_t numSuccess{0};
  uint64_t numFailure{0};
};

/**
 * Wrapper around the generated client to transparently collect statistics of
 * requests being sent
 */
class StressTestClient {
 public:
  explicit StressTestClient(ClientRpcStats& stats) : stats_(stats) {}
  virtual ~StressTestClient() {}

  virtual folly::coro::Task<void> co_ping() = 0;
  virtual folly::coro::Task<std::string> co_echo(const std::string&) = 0;
  virtual folly::coro::Task<std::string> co_echoEb(const std::string&) = 0;
  virtual folly::coro::Task<void> co_requestResponseEb(const BasicRequest&) = 0;
  virtual folly::coro::Task<void> co_requestResponseTm(const BasicRequest&) = 0;
  virtual folly::coro::Task<void> co_streamTm(const StreamRequest&) = 0;
  virtual folly::coro::Task<void> co_sinkTm(const StreamRequest&) = 0;
  virtual folly::coro::Task<double> co_calculateSquares(int32_t) = 0;
  virtual folly::coro::Task<void> co_alignedRequestResponseEb(
      RpcOptions&, AlignedResponse&, const AlignedRequest&) = 0;
  virtual folly::coro::Task<void> co_alignedRequestResponseTm(
      RpcOptions&, AlignedResponse&, const AlignedRequest&) = 0;

  virtual folly::AsyncTransport* getTransport() = 0;
  virtual bool reattach(std::unordered_map<int, folly::EventBase*>& evbs) = 0;
  bool connectionGood() const { return connectionGood_; }

 protected:
  ClientRpcStats& stats_;
  bool connectionGood_{true};
};

/**
 * Concrete class to collect statistics of requests sent from a Thrift/SR client
 */
class ThriftStressTestClient : public StressTestClient {
 public:
  explicit ThriftStressTestClient(
      std::shared_ptr<StressTestAsyncClient> client,
      ClientRpcStats& stats,
      bool enableChecksum = false)
      : StressTestClient(stats),
        client_(std::move(client)),
        enableChecksum_(enableChecksum) {}

  folly::coro::Task<void> co_ping() override;

  folly::coro::Task<std::string> co_echo(const std::string&) override;

  folly::coro::Task<std::string> co_echoEb(const std::string&) override;

  folly::coro::Task<void> co_requestResponseEb(const BasicRequest&) override;

  folly::coro::Task<void> co_requestResponseTm(const BasicRequest&) override;

  folly::coro::Task<void> co_streamTm(const StreamRequest&) override;

  folly::coro::Task<void> co_sinkTm(const StreamRequest&) override;

  folly::coro::Task<double> co_calculateSquares(int32_t) override;

  folly::coro::Task<void> co_alignedRequestResponseEb(
      RpcOptions&, AlignedResponse&, const AlignedRequest&) override;

  folly::coro::Task<void> co_alignedRequestResponseTm(
      RpcOptions&, AlignedResponse&, const AlignedRequest&) override;

  folly::AsyncTransport* getTransport() override;
  bool reattach(std::unordered_map<int, folly::EventBase*>& evbs) override;

 private:
  template <class Fn>
  folly::coro::Task<void> timedExecute(Fn&& fn);

  std::shared_ptr<StressTestAsyncClient> client_;

  const bool enableChecksum_;
};

} // namespace apache::thrift::stress
