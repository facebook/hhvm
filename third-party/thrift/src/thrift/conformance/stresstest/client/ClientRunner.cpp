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

#include <folly/experimental/io/IoUringBackend.h>

#include <thrift/conformance/stresstest/util/IoUringUtil.h>

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

class WarmupDoneTimeout : public folly::HHWheelTimer::Callback {
 public:
  explicit WarmupDoneTimeout(ClientRpcStats& stats) : stats_(stats) {}
  void timeoutExpired() noexcept override {
    stats_.numSuccess = 0;
    stats_.numFailure = 0;
    stats_.latencyHistogram.clear();
  }
  ClientRpcStats& stats_;
};

std::unique_ptr<folly::EventBaseBackendBase> getIOUringBackend() {
  return std::make_unique<folly::IoUringBackend>(getIoUringOptions());
}

std::unique_ptr<folly::EventBaseBackendBase> getDefaultBackend() {
  return folly::EventBase::getDefaultBackend();
}

class ClientThread : public folly::HHWheelTimer::Callback {
 public:
  explicit ClientThread(const ClientConfig& cfg, size_t index)
      : memoryHistogram_(50, 0, 1024 * 1024 * 1024 /* 1GB */),
        testDoneTimeout_(testDone_),
        warmupDoneTimeout_(rpcStats_) {
    continuous_ = cfg.continuous;

    auto ebm = folly::EventBaseManager::get();
    auto factoryFunction = cfg.connConfig.ioUring
        ? folly::EventBaseBackendBase::FactoryFunc(getIOUringBackend)
        : folly::EventBaseBackendBase::FactoryFunc(getDefaultBackend);
    auto threadName = fmt::format("client-runner-thread-{}", index);
    thread_ = std::make_unique<folly::ScopedEventBaseThread>(
        folly::EventBase::Options().setBackendFactory(factoryFunction),
        ebm,
        threadName);
    auto* evb = thread_->getEventBase();
    // create clients in event base thread
    evb->runInEventBaseThreadAndWait([&]() {
      // capture baseline memory usage
      memoryStats_.threadStart = getThreadMemoryUsage();
      clients_ = createClients(evb, cfg, rpcStats_);
      // capture memory usage after connections are established
      memoryStats_.connectionsEstablished = getThreadMemoryUsage();
      // start memory monitoring
      evb->timer().scheduleTimeout(this, std::chrono::seconds(1));
    });
  }

  ~ClientThread() {
    // destroy clients in event base thread
    thread_->getEventBase()->runInEventBaseThreadAndWait(
        [clients = std::move(clients_)]() {});
  }

  folly::SemiFuture<folly::Unit> run(const StressTestBase* test) {
    if (!continuous_) {
      thread_->getEventBase()->timer().scheduleTimeout(
          &testDoneTimeout_,
          std::chrono::seconds(FLAGS_warmup_s + FLAGS_runtime_s));
    }
    if (FLAGS_warmup_s >= 0) {
      thread_->getEventBase()->timer().scheduleTimeout(
          &warmupDoneTimeout_, std::chrono::seconds(FLAGS_warmup_s));
    }
    for (auto& client : clients_) {
      scope_.add(
          runInternal(client.get(), test).scheduleOn(thread_->getEventBase()));
    }
    return scope_.joinAsync()
        .semi()
        .via(thread_->getEventBase())
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
    thread_->getEventBase()->timer().scheduleTimeout(
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
  std::unique_ptr<folly::ScopedEventBaseThread> thread_;
  bool continuous_{false};
  bool testDone_{false};
  TestDoneTimeout testDoneTimeout_;
  WarmupDoneTimeout warmupDoneTimeout_;
};

ClientRunner::ClientRunner(const ClientConfig& config)
    : continuous_(config.continuous), clientThreads_() {
  for (size_t i = 0; i < config.numClientThreads; i++) {
    clientThreads_.emplace_back(std::make_unique<ClientThread>(config, i));
  }
}

ClientRunner::~ClientRunner() {
  // need destructor in .cpp for ClientThread definition to be available
}

void ClientRunner::run(const StressTestBase* test) {
  CHECK(!started_) << "ClientRunner was already started";
  std::vector<folly::SemiFuture<folly::Unit>> starts;
  for (auto& clientThread : clientThreads_) {
    starts.push_back(clientThread->run(test));
  }
  folly::collect(std::move(starts)).get();
  started_ = true;
}

ClientRpcStats ClientRunner::getRpcStats() const {
  ClientRpcStats combinedStats;
  for (auto& clientThread : clientThreads_) {
    combinedStats.combine(clientThread->getRpcStats());
  }
  return combinedStats;
}

ClientThreadMemoryStats ClientRunner::getMemoryStats() const {
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
