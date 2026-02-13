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

#include <folly/coro/BlockingWait.h>
#include <folly/io/async/AsyncIoUringSocketFactory.h>
#include <thrift/conformance/stresstest/common/TimeoutCallbacks.h>
#include <thrift/conformance/stresstest/util/Util.h>
#include <thrift/lib/cpp2/transport/rocket/client/RocketClient.h>

#include <folly/experimental/io/IoUringBackend.h>

#include <thrift/conformance/stresstest/util/IoUringUtil.h>

namespace apache::thrift::stress {

void ClientThreadMemoryStats::combine(const ClientThreadMemoryStats& other) {
  threadStart += other.threadStart;
  connectionsEstablished += other.connectionsEstablished;
  p50 += other.p50;
  p99 += other.p99;
  p100 += other.p100;
  connectionsIdle += other.connectionsIdle;
}

std::unique_ptr<folly::EventBaseBackendBase> getIOUringBackend() {
#if FOLLY_HAS_LIBURING
  return std::make_unique<folly::IoUringBackend>(getIoUringOptions());
#else
  LOG(FATAL) << "IoUring not supported";
#endif
}

std::unique_ptr<folly::EventBaseBackendBase> getDefaultBackend() {
  return folly::EventBase::getDefaultBackend();
}

class ClientThread : public folly::HHWheelTimer::Callback {
 public:
  explicit ClientThread(
      const ClientConfig& cfg,
      size_t index,
      BaseLoadGenerator& loadGenerator,
      bool useLoadGenerator)
      : memoryHistogram_(50, 0, 1024 * 1024 * 1024 /* 1GB */),
        continuous_(cfg.continuous),
        numRunsPerClient_(cfg.numRunsPerClient),
        useLoadGenerator_(useLoadGenerator),
        loadGenerator_(loadGenerator) {
    auto ebm = folly::EventBaseManager::get();
    warmupDoneTimeout_ = std::make_unique<WarmupDoneTimeout>(rpcStats_);
    testDoneTimeout_ = std::make_unique<TestDoneTimeout>(testDone_);
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

  ~ClientThread() override {
    // destroy clients in event base thread
    thread_->getEventBase()->runInEventBaseThreadAndWait(
        [clients = std::move(clients_),
         warmupDoneTimeout = std::move(warmupDoneTimeout_),
         testDoneTimeout = std::move(testDoneTimeout_)]() {});
  }

  void checkIsContinuous() {
    if (!continuous_ && numRunsPerClient_ == 0) {
      thread_->getEventBase()->timer().scheduleTimeout(
          testDoneTimeout_.get(),
          std::chrono::seconds(FLAGS_warmup_s + FLAGS_runtime_s));
    }
  }

  void warmup() {
    if (FLAGS_warmup_s >= 0) {
      thread_->getEventBase()->timer().scheduleTimeout(
          warmupDoneTimeout_.get(), std::chrono::seconds(FLAGS_warmup_s));
    }
  }

  void concurrentRun(const StressTestBase* test) {
    for (auto& client : clients_) {
      scope_.add(co_withExecutor(
          thread_->getEventBase(), runConcurrentInternal(client.get(), test)));
    }
  }

  void loadGeneratedRun(const StressTestBase* test) {
    for (auto& client : clients_) {
      scope_.add(co_withExecutor(
          thread_->getEventBase(),
          runLoadGeneratorInternal(client.get(), test)));
    }
  }

  folly::SemiFuture<folly::Unit> run(const StressTestBase* test) {
    checkIsContinuous();
    warmup();

    if (useLoadGenerator_) {
      LOG(INFO) << "starting load generator run";
      loadGeneratedRun(test);
    } else {
      LOG(INFO) << "starting concurrent run";
      concurrentRun(test);
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

  folly::EventBase* getEventBase() { return thread_->getEventBase(); }

  void addClient(std::unique_ptr<StressTestClient> client) {
    clients_.push_back(std::move(client));
  }

  void bindSocketsForZcRx(const folly::IPAddress& destAddr, uint16_t destPort) {
    for (auto& client : clients_) {
      auto* socketTransport =
          client->getTransport()
              ->getUnderlyingTransport<folly::AsyncSocketTransport>();
      if (socketTransport == nullptr) {
        LOG(ERROR) << "Failed to get AsyncSocketTransport for client: "
                   << client->getTransport()->getLocalAddress();
        continue;
      }
      bool bindRes = folly::AsyncIoUringSocketFactory::bindSocketForZcRx(
          *socketTransport, destAddr, destPort);
      if (!bindRes) {
        LOG(ERROR) << "Failed to find src port to bind with for dest addr: "
                   << destAddr << ", dest port: " << destPort
                   << ", and client: "
                   << client->getTransport()->getLocalAddress();
      }
    }
  }

  std::vector<std::unique_ptr<StressTestClient>> removeClients(
      std::unordered_map<int, ClientThread*>& threads) {
    std::unordered_map<int, folly::EventBase*> evbs;
    for (const auto& [id, th] : threads) {
      evbs.emplace(id, th->getEventBase());
    }

    std::vector<std::unique_ptr<StressTestClient>> removedClients;
    auto end =
        std::remove_if(clients_.begin(), clients_.end(), [&](auto& client) {
          if (client->reattach(evbs)) {
            removedClients.push_back(std::move(client));
            return true;
          }
          return false;
        });
    clients_.erase(end, clients_.end());

    return removedClients;
  }

 private:
  folly::coro::Task<void> runConcurrentInternal(
      StressTestClient* client, const StressTestBase* test) {
    size_t count = 0;
    while (!testDone_ && client->connectionGood()) {
      co_await test->runWorkload(client);
      count++;
      if (numRunsPerClient_ > 0 && count >= numRunsPerClient_) {
        break;
      }
    }
  }

  folly::coro::Task<void> runLoadGeneratorInternal(
      StressTestClient* client, const StressTestBase* test) {
    auto signals = loadGenerator_.getRequestCount();
    while (auto s = co_await signals.next()) {
      if (testDone_) {
        break;
      }

      auto v = *s;
      while (v-- > 0) {
        test->runWorkload(client).semi().via(thread_->getEventBase());
      }
    }
  }

  ClientRpcStats rpcStats_;
  ClientThreadMemoryStats memoryStats_;
  folly::Histogram<size_t> memoryHistogram_;
  folly::coro::AsyncScope scope_;
  std::vector<std::unique_ptr<StressTestClient>> clients_;
  std::unique_ptr<folly::ScopedEventBaseThread> thread_;
  bool continuous_{false};
  uint64_t numRunsPerClient_{0};
  bool useLoadGenerator_{false};
  bool testDone_{false};
  std::unique_ptr<TestDoneTimeout> testDoneTimeout_;
  std::unique_ptr<WarmupDoneTimeout> warmupDoneTimeout_;
  BaseLoadGenerator& loadGenerator_;
};

ClientRunner::ClientRunner(const ClientConfig& config)
    : config_(config),
      useLoadGenerator_(config.useLoadGenerator),
      latch_(config.numClientThreads * config.numConnectionsPerThread),
      clientThreads_() {
  rocket::THRIFT_FLAG_SET_MOCK(
      rocket_enable_frame_relative_alignment,
      config.enableRocketFrameRelativeAlignment);
  THRIFT_FLAG_SET_MOCK(
      rocket_client_set_eor_flag, config.enableRocketFrameRelativeAlignment);
  auto configCopy = config;
  configCopy.connConfig.connectCb = this;
  auto targetQpsPerClient = config.targetQps / config.numClientThreads;
  for (size_t i = 0; i < config.numClientThreads; i++) {
    loadGenerator_.emplace_back(
        std::make_unique<PoissonLoadGenerator>(
            targetQpsPerClient, config.gen_load_interval));
    clientThreads_.emplace_back(
        std::make_unique<ClientThread>(
            configCopy, i, *loadGenerator_[i], config.useLoadGenerator));
  }
}

ClientRunner::~ClientRunner() {
  // need destructor in .cpp for ClientThread definition to be available
}

void ClientRunner::run(const StressTestBase* test) {
  CHECK(!started_) << "ClientRunner was already started";
  std::vector<folly::SemiFuture<folly::Unit>> starts;

  latch_.wait();

  if (config_.connConfig.ioUringZcrx && !config_.connConfig.srcPortBind) {
    for (auto& thread : clientThreads_) {
      auto napiId = thread->getEventBase()->getBackend()->getNapiId();
      napiToThreads_.emplace(napiId, thread.get());
    }

    std::vector<std::unique_ptr<StressTestClient>> shuffleClients;
    for (auto& thread : clientThreads_) {
      auto clients = thread->removeClients(napiToThreads_);
      shuffleClients.insert(
          shuffleClients.end(),
          std::make_move_iterator(clients.begin()),
          std::make_move_iterator(clients.end()));
    }

    for (auto& client : shuffleClients) {
      auto id = client->getTransport()->getNapiId();
      auto it = napiToThreads_.find(id);
      if (it == napiToThreads_.end()) {
        LOG(FATAL) << "Could not find EVB with NAPI ID: " << id;
      }
      it->second->addClient(std::move(client));
    }
  }

  for (auto& clientThread : clientThreads_) {
    starts.push_back(clientThread->run(test));
  }

  if (useLoadGenerator_) {
    for (const auto& generator : loadGenerator_) {
      generator->start();
    }
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

void ClientRunner::connectSuccess() noexcept {
  latch_.count_down();
}

void ClientRunner::connectErr(const folly::AsyncSocketException& ex) noexcept {
  LOG(FATAL) << "Socket connection failed: " << ex.what();
}

} // namespace apache::thrift::stress
