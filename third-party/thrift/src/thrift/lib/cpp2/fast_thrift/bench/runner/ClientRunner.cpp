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

#include <thrift/lib/cpp2/fast_thrift/bench/runner/ClientRunner.h>

#include <chrono>
#include <iomanip>
#include <iostream>

#include <fmt/core.h>
#include <folly/logging/xlog.h>

namespace apache::thrift::fast_thrift::bench {

class ClientRunner::ClientThread {
 public:
  ClientThread(
      const folly::SocketAddress& serverAddress,
      size_t index,
      folly::AsyncSocket::ConnectCallback* connectCb,
      const BenchmarkEntry& bench,
      int runtimeSeconds,
      ClientFactory& factory)
      : bench_(bench), runtimeSeconds_(runtimeSeconds) {
    thread_ = std::make_unique<folly::ScopedEventBaseThread>(
        fmt::format("bench-client-{}", index));
    evb_ = thread_->getEventBase();
    latencies_.reserve(100000);
    testDoneTimeout_ = std::make_unique<TestDoneTimeout>(testDone_);

    evb_->runInEventBaseThreadAndWait([&]() {
      client_ = factory.createClient(evb_, serverAddress, connectCb);
    });
  }

  ~ClientThread() { shutdown(); }

  ClientThread(const ClientThread&) = delete;
  ClientThread& operator=(const ClientThread&) = delete;
  ClientThread(ClientThread&&) = delete;
  ClientThread& operator=(ClientThread&&) = delete;

  folly::SemiFuture<folly::Unit> run() {
    evb_->timer().scheduleTimeout(
        testDoneTimeout_.get(), std::chrono::seconds(runtimeSeconds_));
    return folly::coro::co_withExecutor(evb_, runBenchmarkLoop()).start();
  }

  void shutdown() {
    if (!thread_) {
      return;
    }
    evb_->runInEventBaseThreadAndWait([this]() {
      if (testDoneTimeout_) {
        testDoneTimeout_->cancelTimeout();
      }
      if (client_) {
        client_->shutdown();
      }
    });
    thread_.reset();
  }

  const std::vector<double>& getLatencies() const { return latencies_; }

 private:
  folly::coro::Task<void> runBenchmarkLoop() {
    while (!testDone_) {
      auto start = std::chrono::high_resolution_clock::now();
      co_await bench_.fn(*client_);
      auto end = std::chrono::high_resolution_clock::now();

      auto latency_us =
          std::chrono::duration_cast<std::chrono::microseconds>(end - start)
              .count();
      latencies_.push_back(static_cast<double>(latency_us));
    }
  }

  std::unique_ptr<folly::ScopedEventBaseThread> thread_;
  folly::EventBase* evb_;
  std::unique_ptr<Client> client_;
  std::vector<double> latencies_;
  const BenchmarkEntry& bench_;
  int runtimeSeconds_;
  bool testDone_{false};
  std::unique_ptr<TestDoneTimeout> testDoneTimeout_;
};

ClientRunner::ClientRunner(
    const folly::SocketAddress& serverAddress,
    size_t numThreads,
    const BenchmarkEntry& bench,
    int runtimeSeconds,
    ClientFactory& factory)
    : serverAddress_(serverAddress),
      numThreads_(numThreads),
      bench_(bench),
      runtimeSeconds_(runtimeSeconds),
      latch_(numThreads),
      factory_(factory) {
  clientThreads_.reserve(numThreads_);
  for (size_t i = 0; i < numThreads_; ++i) {
    clientThreads_.emplace_back(
        std::make_unique<ClientThread>(
            serverAddress_, i, this, bench_, runtimeSeconds_, factory_));
  }
}

ClientRunner::~ClientRunner() {
  for (auto& clientThread : clientThreads_) {
    clientThread->shutdown();
  }
}

BenchmarkStats ClientRunner::run() {
  // Wait for all clients to connect
  latch_.wait();

  if (connectFailed_.load()) {
    return BenchmarkStats{
        .numRequests = 0,
        .latencyHistogram = folly::Histogram<double>(1.0, 0.0, 10000.0),
    };
  }

  std::cout << "=== " << bench_.name << " (" << numThreads_
            << " threads) ===" << std::endl;

  auto measureStart = std::chrono::high_resolution_clock::now();

  std::vector<folly::SemiFuture<folly::Unit>> futures;
  futures.reserve(numThreads_);
  for (auto& clientThread : clientThreads_) {
    futures.push_back(clientThread->run());
  }

  folly::collectAll(std::move(futures)).get();

  auto measureEnd = std::chrono::high_resolution_clock::now();
  double totalTimeMs = std::chrono::duration_cast<std::chrono::microseconds>(
                           measureEnd - measureStart)
                           .count() /
      1000.0;

  std::vector<double> allLatencies;
  size_t totalSize = 0;
  for (const auto& clientThread : clientThreads_) {
    totalSize += clientThread->getLatencies().size();
  }
  allLatencies.reserve(totalSize);
  for (const auto& clientThread : clientThreads_) {
    const auto& latencies = clientThread->getLatencies();
    allLatencies.insert(allLatencies.end(), latencies.begin(), latencies.end());
  }

  auto stats = computeStats(allLatencies);
  printStats(stats, totalTimeMs, numThreads_);
  return stats;
}

void ClientRunner::connectSuccess() noexcept {
  latch_.count_down();
}

void ClientRunner::connectErr(const folly::AsyncSocketException& ex) noexcept {
  XLOG(ERR) << "Connection failed: " << ex.what();
  connectFailed_.store(true);
  latch_.count_down();
}

BenchmarkStats ClientRunner::computeStats(std::vector<double>& latencies) {
  folly::Histogram<double> histogram(1.0, 0.0, 10000.0);
  for (double latency : latencies) {
    histogram.addValue(latency);
  }
  return BenchmarkStats{
      .numRequests = static_cast<uint64_t>(latencies.size()),
      .latencyHistogram = std::move(histogram),
  };
}

void ClientRunner::printStats(
    const BenchmarkStats& stats, double totalTimeMs, size_t numThreads) {
  double qps = stats.numRequests / (totalTimeMs / 1000.0);
  std::cout << "  Threads:      " << numThreads << std::endl;
  std::cout << "  QPS:          " << std::fixed << std::setprecision(0) << qps
            << std::endl;
  std::cout << "  Requests:     " << stats.numRequests << std::endl;
  std::cout << "  Latency (us):" << std::endl;
  std::cout << "    P50:    " << std::fixed << std::setprecision(2)
            << stats.latencyHistogram.getPercentileEstimate(0.50) << std::endl;
  std::cout << "    P90:    " << std::fixed << std::setprecision(2)
            << stats.latencyHistogram.getPercentileEstimate(0.90) << std::endl;
  std::cout << "    P99:    " << std::fixed << std::setprecision(2)
            << stats.latencyHistogram.getPercentileEstimate(0.99) << std::endl;
}

} // namespace apache::thrift::fast_thrift::bench
