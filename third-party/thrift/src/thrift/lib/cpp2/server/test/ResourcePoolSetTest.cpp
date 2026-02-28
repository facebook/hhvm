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

#include <gtest/gtest.h>

#include <atomic>
#include <thread>

#include <fmt/core.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/synchronization/Latch.h>
#include <thrift/lib/cpp2/server/ParallelConcurrencyController.h>
#include <thrift/lib/cpp2/server/ResourcePool.h>
#include <thrift/lib/cpp2/server/ResourcePoolHandle.h>
#include <thrift/lib/cpp2/server/RoundRobinRequestPile.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/test/util/TestHandler.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace ::testing;

namespace apache::thrift {
TEST(ResourcePoolSetTest, testDefaultPoolsOverride_overrideSync_expectCrash) {
  ResourcePoolSet set;

  set.setResourcePool(
      ResourcePoolHandle::defaultSync(), nullptr, nullptr, nullptr);
  EXPECT_THROW(
      set.setResourcePool(
          ResourcePoolHandle::defaultSync(), nullptr, nullptr, nullptr),
      std::invalid_argument);
}

TEST(ResourcePoolSetTest, testDefaultPoolsOverride_overrideAsync_expectCrash) {
  ResourcePoolSet set;

  set.setResourcePool(
      ResourcePoolHandle::defaultAsync(), nullptr, nullptr, nullptr);
  EXPECT_THROW(
      set.setResourcePool(
          ResourcePoolHandle::defaultAsync(), nullptr, nullptr, nullptr),
      std::invalid_argument);
}

TEST(
    ResourcePoolSetTest,
    testThriftServer_whenResourcePoolsSuppliedButNotAsyncPool_expectDefaultSyncAndAsyncPoolsCreated) {
  auto handler = std::make_shared<TestHandler>();
  auto server = std::make_shared<ScopedServerInterfaceThread>(
      handler, "::1", 0, [](ThriftServer& server) {
        // Set up thrift server with 2 RPs none of them is "default"
        {
          server.requireResourcePools();

          auto requestPile = std::make_unique<RoundRobinRequestPile>(
              RoundRobinRequestPile::Options());
          auto executor = std::make_shared<folly::CPUThreadPoolExecutor>(1);
          auto concurrencyController =
              std::make_unique<ParallelConcurrencyController>(
                  *requestPile, *executor);
          server.resourcePoolSet().addResourcePool(
              "first",
              std::move(requestPile),
              executor,
              std::move(concurrencyController),
              concurrency::PRIORITY::IMPORTANT);
        }
        {
          auto requestPile = std::make_unique<RoundRobinRequestPile>(
              RoundRobinRequestPile::Options());
          auto executor = std::make_shared<folly::CPUThreadPoolExecutor>(1);
          auto concurrencyController =
              std::make_unique<ParallelConcurrencyController>(
                  *requestPile, *executor);
          server.resourcePoolSet().addResourcePool(
              "second",
              std::move(requestPile),
              executor,
              std::move(concurrencyController),
              concurrency::PRIORITY::IMPORTANT);
        }
      });
  EXPECT_TRUE(server->getThriftServer().resourcePoolSet().hasResourcePool(
      ResourcePoolHandle::defaultAsync()));
  EXPECT_TRUE(server->getThriftServer().resourcePoolSet().hasResourcePool(
      ResourcePoolHandle::defaultSync()));
}

TEST(
    ResourcePoolSetTest,
    ConcurrentAddResourcePoolAndWorkerCount_ThreadSafetyTest) {
  // This test reproduces the TSAN race condition
  // where workerCount() reads resourcePools_ without holding the mutex
  // while addResourcePool() modifies the vector concurrently.
  //
  // The race occurs when:
  // 1. Thread 1 calls addResourcePool() which modifies resourcePools_ under
  // mutex
  // 2. Thread 2 calls workerCount() which reads resourcePools_ WITHOUT mutex
  //    (workerCount only checks `if (!locked_)` which is not synchronized with
  //    addResourcePool's mutex acquisition)
  //
  // In the original crash, ThreadManagerCounterRecorder was started before
  // the server was fully set up, leading to concurrent addResourcePool() calls
  // from DebugConnectionHandler while workerCount() was being called.

  ResourcePoolSet set;

  std::atomic<bool> stopFlag{false};
  std::atomic<bool> locked{false};
  folly::Latch startLatch{3};

  // Thread 1: Adds resource pools - this is the writer
  // Simulates DebugConnectionHandler adding resource pools during setup
  // Note: We add resource pools with nullptr components to avoid lifetime
  // issues with the ConcurrencyController holding raw references to moved
  // objects.
  std::thread writerThread([&]() {
    startLatch.count_down();
    startLatch.wait();
    for (int i = 0; i < 50 && !stopFlag.load(std::memory_order_acquire); ++i) {
      try {
        // Use nullptr for concurrencyController to avoid dangling reference
        // issues. The race condition is in accessing resourcePools_ vector, not
        // the contents
        auto executor = std::make_shared<folly::CPUThreadPoolExecutor>(1);
        set.addResourcePool(
            fmt::format("pool_{}", i),
            nullptr, // requestPile
            executor,
            nullptr, // concurrencyController
            std::nullopt,
            false);
      } catch (const std::logic_error&) {
        // Expected after lock() is called
        break;
      }
    }
  });

  // Thread 2: Repeatedly calls workerCount() - this is the reader
  // Simulates ThreadManagerCounterRecorder::recordCounters()
  std::thread readerThread([&]() {
    startLatch.count_down();
    startLatch.wait();
    while (!stopFlag.load(std::memory_order_acquire)) {
      // workerCount() reads resourcePools_ without holding mutex
      // This can race with addResourcePool() in the writer thread
      [[maybe_unused]] auto count = set.workerCount();
      [[maybe_unused]] auto idleCount = set.idleWorkerCount();
    }
  });

  // Thread 3: Locks the set after some delay
  // Simulates ThriftServer::setup() calling lock()
  std::thread lockerThread([&]() {
    startLatch.count_down();
    startLatch.wait();
    // Small delay to allow some concurrent operations
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    set.lock();
    locked.store(true, std::memory_order_release);
    // Let the test run a bit more after locking
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    stopFlag.store(true, std::memory_order_release);
  });

  writerThread.join();
  readerThread.join();
  lockerThread.join();

  // Verify the set is in a valid state
  EXPECT_TRUE(locked.load());
}
} // namespace apache::thrift
