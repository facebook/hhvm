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

#include <thrift/conformance/stresstest/client/ClientRunner.h>

#include <folly/experimental/coro/BlockingWait.h>
#include <thrift/conformance/stresstest/util/Util.h>

DEFINE_int64(runtime_s, 10, "Runtime of test in seconds");

namespace apache {
namespace thrift {
namespace stress {

void ClientThreadMemoryStats::combine(const ClientThreadMemoryStats& other) {
  threadStart += other.threadStart;
  connectionsEstablished += other.connectionsEstablished;
  p50 += other.p50;
  p99 += other.p99;
  p100 += other.p100;
  connectionsIdle += other.connectionsIdle;
}

class TestDoneTimeout : public folly::HHWheelTimer::Callback {
 public:
  explicit TestDoneTimeout(bool& testDone) : testDone_(testDone) {}
  void timeoutExpired() noexcept override { testDone_ = true; }
  bool& testDone_;
};

class ClientThread : public folly::HHWheelTimer::Callback {
 public:
  explicit ClientThread(const ClientConfig& cfg)
      : memoryHistogram_(50, 0, 1024 * 1024 * 1024 /* 1GB */),
        testDoneTimeout_(testDone_) {
    continuous_ = cfg.continuous;
    auto* evb = thread_.getEventBase();
    // create clients in event base thread
    evb->runInEventBaseThreadAndWait([&]() {
      // capture baseline memory usage
      memoryStats_.threadStart = getThreadMemoryUsage();
      for (size_t connectionIdx = 0;
           connectionIdx < cfg.numConnectionsPerThread;
           connectionIdx++) {
        std::shared_ptr<StressTestAsyncClient> connection =
            createClient(evb, cfg.connConfig);
        for (size_t i = 0; i < cfg.numClientsPerConnection; i++) {
          clients_.emplace_back(
              std::make_unique<StressTestClient>(connection, rpcStats_));
        }
      }
      // capture memory usage after connections are established
      memoryStats_.connectionsEstablished = getThreadMemoryUsage();
      // start memory monitoring
      evb->timer().scheduleTimeout(this, std::chrono::seconds(1));
    });
  }

  ~ClientThread() {
    // destroy clients in event base thread
    thread_.getEventBase()->runInEventBaseThreadAndWait(
        [clients = std::move(clients_)]() {});
  }

  void run(const StressTestBase* test) {
    if (!continuous_) {
      thread_.getEventBase()->timer().scheduleTimeout(
          &testDoneTimeout_, std::chrono::seconds(FLAGS_runtime_s));
    }
    for (auto& client : clients_) {
      scope_.add(
          runInternal(client.get(), test).scheduleOn(thread_.getEventBase()));
    }
  }

  folly::SemiFuture<folly::Unit> stop() {
    return scope_.joinAsync()
        .semi()
        .via(thread_.getEventBase())
        .thenValue([&](auto&&) -> folly::Unit {
          // cancel memory monitoring timeout and capture residual memory usage
          cancelTimeout();
          memoryStats_.connectionsIdle = getThreadMemoryUsage();
          // record memory usage percentiles collected during test
          memoryStats_.p50 = memoryHistogram_.getPercentileEstimate(.5);
          memoryStats_.p99 = memoryHistogram_.getPercentileEstimate(.99);
          memoryStats_.p100 = memoryHistogram_.getPercentileEstimate(1.0);
          return folly::Unit{};
        });
  }

  // HHWheelTimer callback interface
  void timeoutExpired() noexcept override {
    memoryHistogram_.addValue(getThreadMemoryUsage());
    // reschedule the timeout
    thread_.getEventBase()->timer().scheduleTimeout(
        this, std::chrono::seconds(1));
  }

  const ClientRpcStats& getRpcStats() const { return rpcStats_; }
  const ClientThreadMemoryStats& getMemoryStats() const { return memoryStats_; }

  void resetStats() {
    rpcStats_.numFailure = 0;
    rpcStats_.numSuccess = 0;
  }

 private:
  folly::coro::Task<void> runInternal(
      StressTestClient* client, const StressTestBase* test) {
    while (!testDone_ && client->connectionGood()) {
      co_await test->runWorkload(client);
    }
  }

  ClientRpcStats rpcStats_;
  ClientThreadMemoryStats memoryStats_;
  folly::Histogram<size_t> memoryHistogram_;
  folly::coro::AsyncScope scope_;
  std::vector<std::unique_ptr<StressTestClient>> clients_;
  folly::ScopedEventBaseThread thread_;
  bool continuous_{false};
  bool testDone_{false};
  TestDoneTimeout testDoneTimeout_;
};

ClientRunner::ClientRunner(const ClientConfig& config)
    : continuous_(config.continuous), clientThreads_() {
  for (size_t i = 0; i < config.numClientThreads; i++) {
    clientThreads_.emplace_back(std::make_unique<ClientThread>(config));
  }
}

ClientRunner::~ClientRunner() {
  // need destructor in .cpp for ClientThread definition to be available
}

void ClientRunner::run(const StressTestBase* test) {
  CHECK(!started_) << "ClientRunner was already started";
  for (auto& clientThread : clientThreads_) {
    clientThread->run(test);
  }
  started_ = true;
}

void ClientRunner::stop() {
  CHECK(started_ && !stopped_)
      << "ClientRunner was not started or is already stopped";
  std::vector<folly::SemiFuture<folly::Unit>> stops;
  for (auto& clientThread : clientThreads_) {
    stops.push_back(clientThread->stop());
  }
  folly::collect(std::move(stops)).get();
  stopped_ = true;
}

ClientRpcStats ClientRunner::getRpcStats() const {
  CHECK(stopped_ || continuous_)
      << "ClientRunner must be stopped before accessing statistics";
  ClientRpcStats combinedStats;
  for (auto& clientThread : clientThreads_) {
    combinedStats.combine(clientThread->getRpcStats());
  }
  return combinedStats;
}

ClientThreadMemoryStats ClientRunner::getMemoryStats() const {
  CHECK(stopped_ || continuous_)
      << "ClientRunner must be stopped before accessing statistics";
  ClientThreadMemoryStats combinedStats;
  for (auto& clientThread : clientThreads_) {
    combinedStats.combine(clientThread->getMemoryStats());
  }
  return combinedStats;
}
void ClientRunner::resetStats() {
  for (auto& clientThread : clientThreads_) {
    clientThread->resetStats();
  }
}

} // namespace stress
} // namespace thrift
} // namespace apache
