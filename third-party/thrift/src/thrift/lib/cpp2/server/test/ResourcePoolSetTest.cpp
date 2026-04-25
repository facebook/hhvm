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

struct AsyncPoolComponents {
  std::unique_ptr<RoundRobinRequestPile> requestPile;
  std::shared_ptr<folly::CPUThreadPoolExecutor> executor;
  std::unique_ptr<ParallelConcurrencyController> concurrencyController;

  explicit AsyncPoolComponents(int numThreads = 1)
      : requestPile(
            std::make_unique<RoundRobinRequestPile>(
                RoundRobinRequestPile::Options())),
        executor(std::make_shared<folly::CPUThreadPoolExecutor>(numThreads)),
        concurrencyController(
            std::make_unique<ParallelConcurrencyController>(
                *requestPile, *executor)) {}
};

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

// Phase 2: Lock Semantics

TEST(ResourcePoolSetTest, LockSemantics_setResourcePoolAfterLock_throws) {
  ResourcePoolSet set;
  set.setResourcePool(
      ResourcePoolHandle::defaultAsync(), nullptr, nullptr, nullptr);
  set.lock();
  EXPECT_THROW(
      set.setResourcePool(
          ResourcePoolHandle::defaultSync(), nullptr, nullptr, nullptr),
      std::logic_error);
}

TEST(ResourcePoolSetTest, LockSemantics_addResourcePoolAfterLock_throws) {
  ResourcePoolSet set;
  set.setResourcePool(
      ResourcePoolHandle::defaultAsync(), nullptr, nullptr, nullptr);
  set.lock();
  EXPECT_THROW(
      set.addResourcePool("extra", nullptr, nullptr, nullptr),
      std::logic_error);
}

// Phase 3: Pool Lookup

TEST(
    ResourcePoolSetTest,
    FindResourcePool_existingDefaultSync_returnsDefaultSyncHandle) {
  ResourcePoolSet set;
  set.setResourcePool(
      ResourcePoolHandle::defaultSync(), nullptr, nullptr, nullptr);
  set.lock();

  auto handle = set.findResourcePool("DefaultSync");
  ASSERT_TRUE(handle.has_value());
  EXPECT_EQ(handle->index(), ResourcePoolHandle::kDefaultSyncIndex);
}

TEST(
    ResourcePoolSetTest,
    FindResourcePool_existingDefaultAsync_returnsDefaultAsyncHandle) {
  ResourcePoolSet set;
  AsyncPoolComponents pool;
  set.setResourcePool(
      ResourcePoolHandle::defaultAsync(),
      std::move(pool.requestPile),
      pool.executor,
      std::move(pool.concurrencyController));
  set.lock();

  auto handle = set.findResourcePool("DefaultAsync");
  ASSERT_TRUE(handle.has_value());
  EXPECT_EQ(handle->index(), ResourcePoolHandle::kDefaultAsyncIndex);
}

TEST(ResourcePoolSetTest, FindResourcePool_customPool_returnsMakeHandle) {
  ResourcePoolSet set;
  set.setResourcePool(
      ResourcePoolHandle::defaultAsync(), nullptr, nullptr, nullptr);
  AsyncPoolComponents pool;
  set.addResourcePool(
      "MyCustomPool",
      std::move(pool.requestPile),
      pool.executor,
      std::move(pool.concurrencyController));
  set.lock();

  auto handle = set.findResourcePool("MyCustomPool");
  ASSERT_TRUE(handle.has_value());
  EXPECT_GT(handle->index(), ResourcePoolHandle::kMaxReservedIndex);
}

TEST(ResourcePoolSetTest, FindResourcePool_nonExistent_returnsNullopt) {
  ResourcePoolSet set;
  set.setResourcePool(
      ResourcePoolHandle::defaultAsync(), nullptr, nullptr, nullptr);
  set.lock();

  EXPECT_FALSE(set.findResourcePool("DoesNotExist").has_value());
}

TEST(ResourcePoolSetTest, HasResourcePool_outOfRange_returnsFalse) {
  ResourcePoolSet set;
  set.setResourcePool(
      ResourcePoolHandle::defaultAsync(), nullptr, nullptr, nullptr);
  set.lock();

  auto farHandle = ResourcePoolHandle::makeHandle("far", 999);
  EXPECT_FALSE(set.hasResourcePool(farHandle));
}

// Phase 4: Pool Access & Component Accessors

TEST(ResourcePoolSetTest, ResourcePoolAccess_asyncPoolComponents_allPresent) {
  ResourcePoolSet set;
  AsyncPoolComponents pool;
  set.setResourcePool(
      ResourcePoolHandle::defaultAsync(),
      std::move(pool.requestPile),
      pool.executor,
      std::move(pool.concurrencyController));
  set.lock();

  auto& rp = set.resourcePool(ResourcePoolHandle::defaultAsync());
  EXPECT_TRUE(rp.requestPile().has_value());
  EXPECT_TRUE(rp.executor().has_value());
  EXPECT_TRUE(rp.concurrencyController().has_value());
  EXPECT_EQ(rp.name(), "DefaultAsync");
}

TEST(ResourcePoolSetTest, ResourcePoolAccess_syncPoolComponents_allAbsent) {
  ResourcePoolSet set;
  set.setResourcePool(
      ResourcePoolHandle::defaultSync(), nullptr, nullptr, nullptr);
  set.lock();

  auto& rp = set.resourcePool(ResourcePoolHandle::defaultSync());
  EXPECT_FALSE(rp.requestPile().has_value());
  EXPECT_FALSE(rp.executor().has_value());
  EXPECT_FALSE(rp.concurrencyController().has_value());
  EXPECT_EQ(rp.name(), "DefaultSync");
}

TEST(
    ResourcePoolSetTest, AddResourcePool_handleCorrectness_indexAboveReserved) {
  ResourcePoolSet set;
  set.setResourcePool(
      ResourcePoolHandle::defaultAsync(), nullptr, nullptr, nullptr);

  AsyncPoolComponents pool1;
  auto h1 = set.addResourcePool(
      "first",
      std::move(pool1.requestPile),
      pool1.executor,
      std::move(pool1.concurrencyController));

  AsyncPoolComponents pool2;
  auto h2 = set.addResourcePool(
      "second",
      std::move(pool2.requestPile),
      pool2.executor,
      std::move(pool2.concurrencyController));

  EXPECT_EQ(h1.index(), ResourcePoolHandle::kMaxReservedIndex + 1);
  EXPECT_EQ(h2.index(), ResourcePoolHandle::kMaxReservedIndex + 2);
}

// Phase 5: Statistics

TEST(ResourcePoolSetTest, Statistics_beforeLock_allReturnZero) {
  ResourcePoolSet set;
  AsyncPoolComponents pool;
  set.setResourcePool(
      ResourcePoolHandle::defaultAsync(),
      std::move(pool.requestPile),
      pool.executor,
      std::move(pool.concurrencyController));

  EXPECT_EQ(set.numQueued(), 0);
  EXPECT_EQ(set.numInExecution(), 0);
  EXPECT_EQ(set.numPendingDeque(), 0);
  EXPECT_EQ(set.workerCount(), 0);
  EXPECT_EQ(set.idleWorkerCount(), 0);
}

TEST(ResourcePoolSetTest, Statistics_afterLock_workerAndIdleCounts) {
  ResourcePoolSet set;
  AsyncPoolComponents pool(4);
  set.setResourcePool(
      ResourcePoolHandle::defaultAsync(),
      std::move(pool.requestPile),
      pool.executor,
      std::move(pool.concurrencyController));
  set.lock();

  EXPECT_EQ(set.workerCount(), 4);
  EXPECT_EQ(set.idleWorkerCount(), 4);
}

// Phase 6: Priority Mapping
// These tests exercise resourcePoolByPriority_deprecated() which is the only
// public API to verify calculatePriorityMapping() — the non-deprecated internal
// logic that maps concurrency::PRIORITY values to resource pools.

TEST(
    ResourcePoolSetTest,
    PriorityMapping_defaultAsyncOnly_allPrioritiesRouteToAsync) {
  ResourcePoolSet set;
  AsyncPoolComponents pool;
  set.setResourcePool(
      ResourcePoolHandle::defaultAsync(),
      std::move(pool.requestPile),
      pool.executor,
      std::move(pool.concurrencyController));
  set.lock();

  auto& asyncPool = set.resourcePool(ResourcePoolHandle::defaultAsync());
  for (int p = 0; p < concurrency::N_PRIORITIES; ++p) {
    auto priority = static_cast<concurrency::PRIORITY>(p);
// Suppress -Wdeprecated-declarations: this deprecated method is the only
// public API that exposes the priority→pool mapping for verification.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    auto& routed = set.resourcePoolByPriority_deprecated(priority);
#pragma clang diagnostic pop
    EXPECT_EQ(&routed, &asyncPool)
        << "priority " << p << " should route to defaultAsync";
  }
}

TEST(ResourcePoolSetTest, PriorityMapping_customPriorityHint_routesCorrectly) {
  ResourcePoolSet set;
  AsyncPoolComponents defaultPool;
  set.setResourcePool(
      ResourcePoolHandle::defaultAsync(),
      std::move(defaultPool.requestPile),
      defaultPool.executor,
      std::move(defaultPool.concurrencyController));

  AsyncPoolComponents importantPool;
  auto importantHandle = set.addResourcePool(
      "ImportantPool",
      std::move(importantPool.requestPile),
      importantPool.executor,
      std::move(importantPool.concurrencyController),
      concurrency::PRIORITY::IMPORTANT);
  set.lock();

  auto& impPool = set.resourcePool(importantHandle);

// Suppress -Wdeprecated-declarations: see comment on the test above.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
  EXPECT_EQ(
      &set.resourcePoolByPriority_deprecated(concurrency::IMPORTANT), &impPool);
  EXPECT_EQ(
      &set.resourcePoolByPriority_deprecated(concurrency::HIGH), &impPool);
  EXPECT_EQ(
      &set.resourcePoolByPriority_deprecated(concurrency::HIGH_IMPORTANT),
      &impPool);

  auto& asyncPool = set.resourcePool(ResourcePoolHandle::defaultAsync());
  EXPECT_EQ(
      &set.resourcePoolByPriority_deprecated(concurrency::NORMAL), &asyncPool);
  EXPECT_EQ(
      &set.resourcePoolByPriority_deprecated(concurrency::BEST_EFFORT),
      &asyncPool);
#pragma clang diagnostic pop
}

// Phase 7: Empty, Size, Describe & Debug

TEST(ResourcePoolSetTest, EmptyAndSize_freshSet_emptyAndZero) {
  ResourcePoolSet set;
  EXPECT_TRUE(set.empty());
  EXPECT_EQ(set.size(), 0);
}

TEST(
    ResourcePoolSetTest, EmptyAndSize_afterAddingPools_notEmptyAndCorrectSize) {
  ResourcePoolSet set;
  set.setResourcePool(
      ResourcePoolHandle::defaultSync(), nullptr, nullptr, nullptr);
  AsyncPoolComponents asyncPool;
  set.setResourcePool(
      ResourcePoolHandle::defaultAsync(),
      std::move(asyncPool.requestPile),
      asyncPool.executor,
      std::move(asyncPool.concurrencyController));
  AsyncPoolComponents customPool;
  set.addResourcePool(
      "custom",
      std::move(customPool.requestPile),
      customPool.executor,
      std::move(customPool.concurrencyController));

  EXPECT_FALSE(set.empty());
  EXPECT_EQ(set.size(), 3);
}

TEST(ResourcePoolSetTest, Describe_afterLock_returnsNonEmptyString) {
  ResourcePoolSet set;
  AsyncPoolComponents pool;
  set.setResourcePool(
      ResourcePoolHandle::defaultAsync(),
      std::move(pool.requestPile),
      pool.executor,
      std::move(pool.concurrencyController));
  set.lock();

  auto desc = set.describe();
  EXPECT_NE(desc.find("ResourcePoolSet"), std::string::npos);
}

TEST(ResourcePoolSetTest, PoolsDescriptions_matchesSize) {
  ResourcePoolSet set;
  set.setResourcePool(
      ResourcePoolHandle::defaultSync(), nullptr, nullptr, nullptr);
  AsyncPoolComponents asyncPool;
  set.setResourcePool(
      ResourcePoolHandle::defaultAsync(),
      std::move(asyncPool.requestPile),
      asyncPool.executor,
      std::move(asyncPool.concurrencyController));
  set.lock();

  auto descriptions = set.poolsDescriptions();
  EXPECT_EQ(descriptions.size(), set.size());
}

// Phase 8: ForEachResourcePool

TEST(ResourcePoolSetTest, ForEachResourcePool_iteratesAllPools) {
  ResourcePoolSet set;
  set.setResourcePool(
      ResourcePoolHandle::defaultSync(), nullptr, nullptr, nullptr);
  AsyncPoolComponents asyncPool;
  set.setResourcePool(
      ResourcePoolHandle::defaultAsync(),
      std::move(asyncPool.requestPile),
      asyncPool.executor,
      std::move(asyncPool.concurrencyController));
  AsyncPoolComponents custom1;
  set.addResourcePool(
      "custom1",
      std::move(custom1.requestPile),
      custom1.executor,
      std::move(custom1.concurrencyController));
  AsyncPoolComponents custom2;
  set.addResourcePool(
      "custom2",
      std::move(custom2.requestPile),
      custom2.executor,
      std::move(custom2.concurrencyController));
  set.lock();

  std::size_t count = 0;
  set.forEachResourcePool([&](const ResourcePool* pool) {
    if (pool) {
      ++count;
    }
  });
  EXPECT_EQ(count, 4);
}

} // namespace apache::thrift
