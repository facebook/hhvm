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

#include <atomic>

#include <gtest/gtest.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/server/ParallelConcurrencyController.h>
#include <thrift/lib/cpp2/server/ResourcePoolHandle.h>
#include <thrift/lib/cpp2/server/RoundRobinRequestPile.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/e2e/gen-cpp2/TestResourcePoolService.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

namespace apache::thrift {

namespace {

using ServiceHandler_ = ServiceHandler<detail::test::TestResourcePoolService>;
using ServiceClient_ = Client<detail::test::TestResourcePoolService>;

/**
 * Test fixture for resource pool routing E2E tests.
 *
 * Provides startServer with an optional server config lambda so tests
 * can configure resource pools before the server starts.
 */
class ResourcePoolE2ETest : public ::testing::Test {
 protected:
  using ServerConfigCb = ScopedServerInterfaceThread::ServerConfigCb;

  void startServer(
      std::shared_ptr<AsyncProcessorFactory> handler,
      ServerConfigCb configCb = {}) {
    server_ = std::make_unique<ScopedServerInterfaceThread>(
        std::move(handler), std::move(configCb));
  }

  std::unique_ptr<ServiceClient_> makeClient() {
    return server_->newClient<ServiceClient_>(
        nullptr, RocketClientChannel::newChannel);
  }

  ThriftServer& getServer() { return server_->getThriftServer(); }

  std::unique_ptr<ScopedServerInterfaceThread> server_;
};

// Handler that blocks the FIRST echo() call on an atomic flag.
// Subsequent echo() calls pass through without blocking.
struct BlockableHandler : public ServiceHandler_ {
  explicit BlockableHandler(folly::Baton<>& blocked, std::atomic<bool>& proceed)
      : blocked_(blocked), proceed_(proceed) {}

  folly::coro::Task<std::unique_ptr<std::string>> co_echo(
      std::unique_ptr<std::string> message) override {
    if (!alreadyBlocked_.exchange(true)) {
      blocked_.post();
      while (!proceed_.load()) {
        co_await folly::coro::co_reschedule_on_current_executor;
      }
    }
    co_return std::move(message);
  }

  folly::coro::Task<void> co_voidMethod() override { co_return; }

  folly::coro::Task<std::unique_ptr<std::string>> co_slowEcho(
      std::unique_ptr<std::string> message) override {
    co_return std::move(message);
  }

  folly::Baton<>& blocked_;
  std::atomic<bool>& proceed_;
  std::atomic<bool> alreadyBlocked_{false};
};

// Handler that records which thread executed each method.
struct TrackingHandler : public ServiceHandler_ {
  folly::coro::Task<std::unique_ptr<std::string>> co_echo(
      std::unique_ptr<std::string> message) override {
    echoThreadId.store(std::this_thread::get_id());
    co_return std::move(message);
  }

  folly::coro::Task<void> co_voidMethod() override {
    voidThreadId.store(std::this_thread::get_id());
    co_return;
  }

  folly::coro::Task<std::unique_ptr<std::string>> co_slowEcho(
      std::unique_ptr<std::string> message) override {
    slowEchoThreadId.store(std::this_thread::get_id());
    co_return std::move(message);
  }

  std::atomic<std::thread::id> echoThreadId{};
  std::atomic<std::thread::id> voidThreadId{};
  std::atomic<std::thread::id> slowEchoThreadId{};
};

/**
 * Wrapper around an AsyncProcessorFactory that overrides
 * selectResourcePool() to route specific methods to custom pools.
 */
class RoutingProcessorFactory : public AsyncProcessorFactory {
 public:
  RoutingProcessorFactory(
      std::shared_ptr<AsyncProcessorFactory> inner,
      std::function<SelectPoolResult(const ServerRequest&)> selector)
      : inner_(std::move(inner)), selector_(std::move(selector)) {}

  std::unique_ptr<AsyncProcessor> getProcessor() override {
    return inner_->getProcessor();
  }

  CreateMethodMetadataResult createMethodMetadata() override {
    return inner_->createMethodMetadata();
  }

  std::vector<ServiceHandlerBase*> getServiceHandlers() override {
    return inner_->getServiceHandlers();
  }

  SelectPoolResult selectResourcePool(
      const ServerRequest& request) const override {
    return selector_(request);
  }

 private:
  std::shared_ptr<AsyncProcessorFactory> inner_;
  std::function<SelectPoolResult(const ServerRequest&)> selector_;
};

} // namespace

// ==================== Basic routing ====================

TEST_F(ResourcePoolE2ETest, DefaultPoolHandlesAllRequests) {
  struct Handler : public ServiceHandler_ {
    folly::coro::Task<std::unique_ptr<std::string>> co_echo(
        std::unique_ptr<std::string> message) override {
      co_return std::move(message);
    }
  };

  startServer(std::make_shared<Handler>());
  auto client = makeClient();

  auto result = client->semifuture_echo("hello").get();
  EXPECT_EQ(result, "hello");

  auto& pools = getServer().resourcePoolSet();
  EXPECT_EQ(pools.numQueued(), 0);
}

// ==================== Concurrency limits ====================

TEST_F(ResourcePoolE2ETest, ConcurrencyLimitEnforcedOnDefaultPool) {
  folly::Baton<> handlerBlocked;
  std::atomic<bool> proceed{false};
  auto handler = std::make_shared<BlockableHandler>(handlerBlocked, proceed);

  startServer(handler);
  auto& pools = getServer().resourcePoolSet();
  auto& asyncPool = pools.resourcePool(ResourcePoolHandle::defaultAsync());
  auto& cc = dynamic_cast<ParallelConcurrencyController&>(
      asyncPool.concurrencyController().value().get());
  cc.setExecutionLimitRequests(1);

  auto client = makeClient();

  // First request: should start executing (fills the single slot)
  auto future1 = client->semifuture_echo("first");
  EXPECT_TRUE(handlerBlocked.try_wait_for(std::chrono::seconds(5)));

  // Second request: should queue because the slot is full
  auto future2 = client->semifuture_echo("second");
  // Give it a moment to arrive at the server
  /* sleep override */ std::this_thread::sleep_for(
      std::chrono::milliseconds(100));
  EXPECT_GE(pools.numQueued(), 1);

  // Unblock first request
  proceed.store(true);
  auto result1 = std::move(future1).get();
  EXPECT_EQ(result1, "first");

  // Second request should now complete
  auto result2 = std::move(future2).get();
  EXPECT_EQ(result2, "second");
}

// ==================== Custom pool routing ====================

TEST_F(ResourcePoolE2ETest, CustomPoolRoutedBySelectResourcePool) {
  folly::Baton<> echoBlocked;
  std::atomic<bool> echoProceed{false};
  auto handler = std::make_shared<BlockableHandler>(echoBlocked, echoProceed);

  // We'll store the custom pool handle here for the routing function.
  std::optional<ResourcePoolHandle> echoPoolHandle;

  auto factory = std::make_shared<RoutingProcessorFactory>(
      handler, [&echoPoolHandle](const ServerRequest& req) -> SelectPoolResult {
        if (echoPoolHandle && req.requestContext()->getMethodName() == "echo") {
          return std::cref(*echoPoolHandle);
        }
        return std::monostate{};
      });

  startServer(factory, [&echoPoolHandle](ThriftServer& server) {
    auto pile = std::make_unique<RoundRobinRequestPile>(
        RoundRobinRequestPile::Options());
    auto executor = std::make_shared<folly::CPUThreadPoolExecutor>(1);
    auto cc = std::make_unique<ParallelConcurrencyController>(*pile, *executor);
    echoPoolHandle = server.resourcePoolSet().addResourcePool(
        "EchoPool", std::move(pile), executor, std::move(cc));
  });

  auto client = makeClient();

  // echo() goes to EchoPool, blocks there
  auto echoFuture = client->semifuture_echo("routed");
  EXPECT_TRUE(echoBlocked.try_wait_for(std::chrono::seconds(5)));

  // voidMethod() goes to default pool, should complete despite EchoPool blocked
  client->semifuture_voidMethod().get();

  // Unblock echo
  echoProceed.store(true);
  auto result = std::move(echoFuture).get();
  EXPECT_EQ(result, "routed");
}

// ==================== Priority-based routing ====================

TEST_F(ResourcePoolE2ETest, PriorityBasedRouting) {
  // With PRIORITY_QUEUE and 1 worker thread, higher priority requests
  // should execute before lower priority ones.
  folly::Baton<> handlerBlocked;
  std::atomic<bool> proceed{false};
  std::vector<std::string> executionOrder;
  std::mutex orderMutex;

  struct OrderTrackingHandler : public ServiceHandler_ {
    OrderTrackingHandler(
        folly::Baton<>& blocked,
        std::atomic<bool>& proceed,
        std::vector<std::string>& order,
        std::mutex& mu)
        : blocked_(blocked), proceed_(proceed), order_(order), mu_(mu) {}

    folly::coro::Task<std::unique_ptr<std::string>> co_echo(
        std::unique_ptr<std::string> message) override {
      if (*message == "blocker") {
        blocked_.post();
        while (!proceed_.load()) {
          co_await folly::coro::co_reschedule_on_current_executor;
        }
      } else {
        std::lock_guard lock(mu_);
        order_.push_back(*message);
      }
      co_return std::move(message);
    }

    folly::Baton<>& blocked_;
    std::atomic<bool>& proceed_;
    std::vector<std::string>& order_;
    std::mutex& mu_;
  };

  auto handler = std::make_shared<OrderTrackingHandler>(
      handlerBlocked, proceed, executionOrder, orderMutex);

  startServer(handler, [](ThriftServer& server) {
    server.setThreadManagerType(
        ThriftServer::ThreadManagerType::PRIORITY_QUEUE);
    server.setNumCPUWorkerThreads(1);
  });

  auto client = makeClient();

  // Block the single worker thread
  auto blockerFuture = client->semifuture_echo("blocker");
  EXPECT_TRUE(handlerBlocked.try_wait_for(std::chrono::seconds(5)));

  // Send low-priority request first, then high-priority
  RpcOptions lowOpts;
  lowOpts.setPriority(concurrency::PRIORITY::BEST_EFFORT);
  auto lowFuture = client->semifuture_echo(lowOpts, "low");

  RpcOptions highOpts;
  highOpts.setPriority(concurrency::PRIORITY::HIGH);
  auto highFuture = client->semifuture_echo(highOpts, "high");

  // Give requests time to arrive and queue
  /* sleep override */ std::this_thread::sleep_for(
      std::chrono::milliseconds(100));

  // Unblock
  proceed.store(true);
  std::move(blockerFuture).get();
  std::move(highFuture).get();
  std::move(lowFuture).get();

  // High priority should have executed before low
  ASSERT_EQ(executionOrder.size(), 2);
  EXPECT_EQ(executionOrder[0], "high");
  EXPECT_EQ(executionOrder[1], "low");
}

// ==================== Pool isolation ====================

TEST_F(ResourcePoolE2ETest, PoolExhaustionDoesNotAffectOtherPools) {
  folly::Baton<> echoBlocked;
  std::atomic<bool> echoProceed{false};

  struct Handler : public ServiceHandler_ {
    Handler(folly::Baton<>& blocked, std::atomic<bool>& proceed)
        : blocked_(blocked), proceed_(proceed) {}

    folly::coro::Task<std::unique_ptr<std::string>> co_echo(
        std::unique_ptr<std::string> message) override {
      if (!alreadyBlocked_.exchange(true)) {
        blocked_.post();
        while (!proceed_.load()) {
          co_await folly::coro::co_reschedule_on_current_executor;
        }
      }
      co_return std::move(message);
    }

    folly::coro::Task<std::unique_ptr<std::string>> co_slowEcho(
        std::unique_ptr<std::string> message) override {
      co_return std::move(message);
    }

    folly::Baton<>& blocked_;
    std::atomic<bool>& proceed_;
    std::atomic<bool> alreadyBlocked_{false};
  };

  auto handler = std::make_shared<Handler>(echoBlocked, echoProceed);
  std::optional<ResourcePoolHandle> echoPoolHandle;
  std::optional<ResourcePoolHandle> slowEchoPoolHandle;

  auto factory = std::make_shared<RoutingProcessorFactory>(
      handler,
      [&echoPoolHandle,
       &slowEchoPoolHandle](const ServerRequest& req) -> SelectPoolResult {
        auto method = req.requestContext()->getMethodName();
        if (echoPoolHandle && method == "echo") {
          return std::cref(*echoPoolHandle);
        }
        if (slowEchoPoolHandle && method == "slowEcho") {
          return std::cref(*slowEchoPoolHandle);
        }
        return std::monostate{};
      });

  startServer(
      factory, [&echoPoolHandle, &slowEchoPoolHandle](ThriftServer& server) {
        // PoolA for echo: 1 executor thread, concurrency limit 1
        {
          auto pile = std::make_unique<RoundRobinRequestPile>(
              RoundRobinRequestPile::Options());
          auto executor = std::make_shared<folly::CPUThreadPoolExecutor>(1);
          auto cc =
              std::make_unique<ParallelConcurrencyController>(*pile, *executor);
          cc->setExecutionLimitRequests(1);
          echoPoolHandle = server.resourcePoolSet().addResourcePool(
              "EchoPool", std::move(pile), executor, std::move(cc));
        }

        // PoolB for slowEcho: separate executor
        {
          auto pile = std::make_unique<RoundRobinRequestPile>(
              RoundRobinRequestPile::Options());
          auto executor = std::make_shared<folly::CPUThreadPoolExecutor>(1);
          auto cc =
              std::make_unique<ParallelConcurrencyController>(*pile, *executor);
          slowEchoPoolHandle = server.resourcePoolSet().addResourcePool(
              "SlowEchoPool", std::move(pile), executor, std::move(cc));
        }
      });

  auto client = makeClient();

  // Block EchoPool completely
  auto echoFuture = client->semifuture_echo("blocked");
  EXPECT_TRUE(echoBlocked.try_wait_for(std::chrono::seconds(5)));

  // slowEcho uses SlowEchoPool -- should complete despite EchoPool blocked
  auto result = client->semifuture_slowEcho("works").get();
  EXPECT_EQ(result, "works");

  // Unblock echo
  echoProceed.store(true);
  EXPECT_EQ(std::move(echoFuture).get(), "blocked");
}

// ==================== Dynamic concurrency limit ====================

TEST_F(ResourcePoolE2ETest, DynamicConcurrencyLimitChange) {
  struct Handler : public ServiceHandler_ {
    folly::coro::Task<std::unique_ptr<std::string>> co_echo(
        std::unique_ptr<std::string> message) override {
      co_return std::move(message);
    }
  };

  startServer(std::make_shared<Handler>());
  auto& pools = getServer().resourcePoolSet();
  auto& asyncPool = pools.resourcePool(ResourcePoolHandle::defaultAsync());
  auto& cc = dynamic_cast<ParallelConcurrencyController&>(
      asyncPool.concurrencyController().value().get());

  auto client = makeClient();

  // Set limit to 0 -- all requests should queue
  cc.setExecutionLimitRequests(0);
  auto future = client->semifuture_echo("queued");
  /* sleep override */ std::this_thread::sleep_for(
      std::chrono::milliseconds(100));
  EXPECT_GE(pools.numQueued(), 1);

  // Increase limit -- request should complete
  cc.setExecutionLimitRequests(1);
  auto result = std::move(future).get();
  EXPECT_EQ(result, "queued");
  EXPECT_EQ(pools.numQueued(), 0);
}

} // namespace apache::thrift
