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

#include <thrift/lib/cpp2/fast_thrift/bench/runner/BenchmarkRegistry.h>
#include <thrift/lib/cpp2/fast_thrift/bench/runner/ClientFactory.h>

#include <atomic>
#include <memory>
#include <vector>

#include <fmt/core.h>
#include <folly/SocketAddress.h>
#include <folly/coro/Collect.h>
#include <folly/coro/Task.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/HHWheelTimer.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/stats/Histogram.h>
#include <folly/synchronization/Latch.h>

namespace apache::thrift::fast_thrift::bench {

/**
 * Timeout callback that signals test completion by setting a boolean flag.
 * Used to terminate benchmarks after a specified duration.
 */
class TestDoneTimeout : public folly::HHWheelTimer::Callback {
 public:
  explicit TestDoneTimeout(bool& testDone) : testDone_(testDone) {}
  void timeoutExpired() noexcept override { testDone_ = true; }

 private:
  bool& testDone_;
};

struct BenchmarkStats {
  uint64_t numRequests{0};
  folly::Histogram<double> latencyHistogram;
};

/**
 * ClientRunner - manages benchmark client threads.
 */
class ClientRunner : public folly::AsyncSocket::ConnectCallback {
 public:
  explicit ClientRunner(
      const folly::SocketAddress& serverAddress,
      size_t numThreads,
      const BenchmarkEntry& bench,
      int runtimeSeconds,
      ClientFactory& factory);

  ~ClientRunner() override;

  ClientRunner(const ClientRunner&) = delete;
  ClientRunner& operator=(const ClientRunner&) = delete;
  ClientRunner(ClientRunner&&) = delete;
  ClientRunner& operator=(ClientRunner&&) = delete;

  BenchmarkStats run();
  size_t numThreads() const { return numThreads_; }

  void connectSuccess() noexcept override;
  void connectErr(const folly::AsyncSocketException& ex) noexcept override;

 private:
  class ClientThread;

  static BenchmarkStats computeStats(std::vector<double>& latencies);
  static void printStats(
      const BenchmarkStats& stats, double totalTimeMs, size_t numThreads);

  folly::SocketAddress serverAddress_;
  size_t numThreads_;
  const BenchmarkEntry& bench_;
  int runtimeSeconds_;
  folly::Latch latch_;
  std::atomic<bool> connectFailed_{false};
  ClientFactory& factory_;
  std::vector<std::unique_ptr<ClientThread>> clientThreads_;
};

} // namespace apache::thrift::fast_thrift::bench
