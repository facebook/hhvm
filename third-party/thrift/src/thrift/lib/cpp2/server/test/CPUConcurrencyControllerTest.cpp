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
#include <chrono>
#include <functional>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <folly/observer/Observer.h>
#include <folly/observer/SimpleObservable.h>
#include <folly/synchronization/Baton.h>

#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/cpp2/server/CPUConcurrencyController.h>
#include <thrift/lib/cpp2/server/ThriftServerConfig.h>
#include <thrift/lib/cpp2/server/test/util/MockServerConfigs.h>

using namespace std::chrono_literals;
using apache::thrift::CPUConcurrencyController;

class MockEventHandler : public CPUConcurrencyController::EventHandler {
 public:
  MOCK_METHOD(
      void,
      configUpdated,
      (std::shared_ptr<const CPUConcurrencyController::Config>),
      (override));
  MOCK_METHOD(void, onCycle, (int64_t, int64_t, int64_t), (override));
  MOCK_METHOD(void, limitIncreased, (), (override));
  MOCK_METHOD(void, limitDecreased, (), (override));
};

class CPUConcurrencyControllerTest : public ::testing::Test {
 public:
  static int64_t getCPULoad() {
    return cpuLoad_.load(std::memory_order_relaxed);
  }

 protected:
  static void setCPULoad(int64_t load) {
    cpuLoad_.store(load, std::memory_order_relaxed);
  }

  folly::observer::Observer<CPUConcurrencyController::Config>
  getConfigObserver() {
    return configObservable_.getObserver();
  }

  void setConfig(CPUConcurrencyController::Config config) {
    configObservable_.setValue(std::move(config));
  }

  CPUConcurrencyController& getCPUConcurrencyController() {
    return cpuConcurrencyController_;
  }

  apache::thrift::ThriftServerConfig& getThriftServerConfig() {
    return thriftServerConfig_;
  }

  apache::thrift::server::test::MockServerConfigs& getServerConfigs() {
    return serverConfigs_;
  }

 private:
  static std::atomic<int64_t> cpuLoad_;
  folly::observer::SimpleObservable<CPUConcurrencyController::Config>
      configObservable_;
  apache::thrift::server::test::MockServerConfigs serverConfigs_{};
  apache::thrift::ThriftServerConfig thriftServerConfig_{};
  CPUConcurrencyController cpuConcurrencyController_{
      configObservable_.getObserver(), serverConfigs_, thriftServerConfig_};
};

std::atomic<int64_t> CPUConcurrencyControllerTest::cpuLoad_ = 0;

namespace apache::thrift::detail {

THRIFT_PLUGGABLE_FUNC_SET(
    int64_t, getCPULoadCounter, std::chrono::milliseconds, CPULoadSource) {
  return CPUConcurrencyControllerTest::getCPULoad();
}

} // namespace apache::thrift::detail

// ---------------------------------------------------------------------------
// EventHandler tests
// ---------------------------------------------------------------------------

TEST_F(CPUConcurrencyControllerTest, testEventHandler) {
  using ::testing::Eq;
  using ::testing::Gt;
  folly::Baton<> baton1;
  folly::Baton<> baton2;
  folly::Baton<> baton3;
  auto eventHandler = std::make_shared<MockEventHandler>();

  ::testing::Sequence seq;
  // First cycle: EMA seeds at 0, so load=0. Early return (no limit change).
  EXPECT_CALL(*eventHandler, onCycle(Eq(100), Eq(0), Eq(0))).InSequence(seq);
  EXPECT_CALL(*eventHandler, onCycle(Eq(100), Eq(0), Eq(50)))
      .InSequence(seq)
      .WillOnce(
          ::testing::Invoke([&baton1](auto, auto, auto) { baton1.post(); }));
  EXPECT_CALL(*eventHandler, onCycle(Eq(100), Eq(0), Eq(100))).InSequence(seq);
  EXPECT_CALL(*eventHandler, limitDecreased())
      .InSequence(seq)
      .WillOnce(::testing::Invoke([&baton2]() { baton2.post(); }));
  EXPECT_CALL(*eventHandler, onCycle(Eq(95), Gt(0), Eq(50))).InSequence(seq);
  EXPECT_CALL(*eventHandler, limitIncreased())
      .InSequence(seq)
      .WillOnce(::testing::Invoke([&baton3]() { baton3.post(); }));

  getCPUConcurrencyController().setEventHandler(eventHandler);

  setCPULoad(50);

  // Start CPU-CC
  setConfig(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::ENABLED,
          .collectionSampleSize = 0,
          .concurrencyUpperBound = 100});

  // Wait for first call to complete and bump CPU > target
  ASSERT_TRUE(baton1.try_wait_for(1s));
  setCPULoad(100);

  // Wait for second call to complete, decrease CPU < target, add enough
  // requests to be within increaseDistanceRatio
  ASSERT_TRUE(baton2.try_wait_for(1s));
  setCPULoad(50);
  for (auto reqs = 90; reqs; --reqs) {
    getCPUConcurrencyController().requestStarted();
  }

  // Wait for third call to complete, disable CPU-CC
  ASSERT_TRUE(baton3.try_wait_for(1s));
  setConfig(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::DISABLED});
}

TEST_F(CPUConcurrencyControllerTest, configUpdatedCallsEventHandler) {
  auto eventHandler = std::make_shared<::testing::NiceMock<MockEventHandler>>();
  folly::Baton<> baton;

  // configUpdated should be called when setting the event handler
  EXPECT_CALL(*eventHandler, configUpdated(::testing::_))
      .WillOnce(::testing::Invoke([&baton](auto) { baton.post(); }));

  getCPUConcurrencyController().setEventHandler(eventHandler);
  ASSERT_TRUE(baton.try_wait_for(1s));
}

// ---------------------------------------------------------------------------
// getDbgInfo tests
// ---------------------------------------------------------------------------

TEST_F(CPUConcurrencyControllerTest, getDbgInfo) {
  using namespace apache::thrift;
  server::test::MockServerConfigs serverConfigs{};
  ThriftServerConfig thriftServerConfig{};
  folly::observer::SimpleObservable<CPUConcurrencyController::Config>
      configObservable;

  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::ENABLED,
          .method = CPUConcurrencyController::Method::MAX_QPS,
          .cpuTarget = 99,
          .cpuLoadSource = apache::thrift::CPULoadSource::CONTAINER_AND_HOST,
          .refreshPeriodMs = std::chrono::milliseconds(100),
          .additiveMultiplier = 0.01,
          .decreaseMultiplier = 0.02,
          .increaseDistanceRatio = 0.03,
          .bumpOnError = true,
          .refractoryPeriodMs = std::chrono::milliseconds(200),
          .initialEstimatePercentile = 0.04,
          .collectionSampleSize = 11,
          .concurrencyUpperBound = 1002,
          .concurrencyLowerBound = 1001,
          .cpuLoadSmoothingCoeff = 0.42,
      });

  CPUConcurrencyController cpuConcurrencyController{
      configObservable.getObserver(), serverConfigs, thriftServerConfig};

  auto dbgInfo = cpuConcurrencyController.getDbgInfo();

  ASSERT_EQ("ENABLED", dbgInfo.mode().value());
  ASSERT_EQ(*dbgInfo.method(), "MAX_QPS");
  ASSERT_EQ(*dbgInfo.cpuTarget(), 99);
  ASSERT_EQ(*dbgInfo.cpuLoadSource(), "CONTAINER_AND_HOST");
  ASSERT_EQ(*dbgInfo.refreshPeriodMs(), 100);
  ASSERT_EQ(*dbgInfo.additiveMultiplier(), 0.01);
  ASSERT_EQ(*dbgInfo.decreaseMultiplier(), 0.02);
  ASSERT_EQ(*dbgInfo.increaseDistanceRatio(), 0.03);
  ASSERT_TRUE(*dbgInfo.bumpOnError());
  ASSERT_EQ(*dbgInfo.refractoryPeriodMs(), 200);
  ASSERT_EQ(*dbgInfo.initialEstimatePercentile(), 0.04);
  ASSERT_EQ(*dbgInfo.collectionSampleSize(), 11);
  ASSERT_EQ(*dbgInfo.concurrencyLowerBound(), 1001);
  ASSERT_EQ(*dbgInfo.concurrencyUpperBound(), 1002);
  ASSERT_DOUBLE_EQ(*dbgInfo.cpuLoadSmoothingCoeff(), 0.42);
}

// ---------------------------------------------------------------------------
// getConcurrencyUpperBound tests
// ---------------------------------------------------------------------------

TEST_F(CPUConcurrencyControllerTest, testConcurrencyUpperBound) {
  {
    // Test concurrencyUpperBound with positive value
    setConfig(
        CPUConcurrencyController::Config{
            .mode = CPUConcurrencyController::Mode::ENABLED,
            .method = CPUConcurrencyController::Method::MAX_QPS,
            .concurrencyUpperBound = 100,
        });

    folly::observer_detail::ObserverManager::waitForAllUpdates();
    EXPECT_EQ(getCPUConcurrencyController().getConcurrencyUpperBound(), 100);
  }
  {
    setConfig(
        CPUConcurrencyController::Config{
            .mode = CPUConcurrencyController::Mode::DISABLED});

    // Test concurrencyUpperBound with kConcurrencyUpperBoundUseStaticLimit
    // MAX_REQUESTS method
    getThriftServerConfig().setMaxRequests(
        folly::observer::makeStaticObserver<std::optional<uint32_t>>(200));

    setConfig(
        CPUConcurrencyController::Config{
            .mode = CPUConcurrencyController::Mode::ENABLED,
            .method = CPUConcurrencyController::Method::MAX_REQUESTS,
            .concurrencyUpperBound =
                CPUConcurrencyController::Config::UseStaticLimit{},
        });
    folly::observer_detail::ObserverManager::waitForAllUpdates();

    EXPECT_EQ(getCPUConcurrencyController().getConcurrencyUpperBound(), 200);
  }
  {
    setConfig(
        CPUConcurrencyController::Config{
            .mode = CPUConcurrencyController::Mode::DISABLED});

    // Test concurrencyUpperBound with kConcurrencyUpperBoundUseStaticLimit
    // MAX_QPS method
    getThriftServerConfig().setMaxQps(
        folly::observer::makeStaticObserver<std::optional<uint32_t>>(300));

    setConfig(
        CPUConcurrencyController::Config{
            .mode = CPUConcurrencyController::Mode::ENABLED,
            .method = CPUConcurrencyController::Method::MAX_QPS,
            .concurrencyUpperBound =
                CPUConcurrencyController::Config::UseStaticLimit{},
        });

    folly::observer_detail::ObserverManager::waitForAllUpdates();
    EXPECT_EQ(getCPUConcurrencyController().getConcurrencyUpperBound(), 300);
  }
  {
    getThriftServerConfig().setConcurrencyLimit(
        folly::observer::makeStaticObserver<std::optional<uint32_t>>(400));

    // Test concurrencyUpperBound with kConcurrencyUpperBoundUseStaticLimit
    // CONCURRENCY_LIMIT method
    setConfig(
        CPUConcurrencyController::Config{
            .mode = CPUConcurrencyController::Mode::ENABLED,
            .method = CPUConcurrencyController::Method::CONCURRENCY_LIMIT,
            .concurrencyUpperBound =
                CPUConcurrencyController::Config::UseStaticLimit{},
        });

    folly::observer_detail::ObserverManager::waitForAllUpdates();
    EXPECT_EQ(getCPUConcurrencyController().getConcurrencyUpperBound(), 400);
  }
  {
    getThriftServerConfig().setExecutionRate(
        folly::observer::makeStaticObserver<std::optional<uint32_t>>(500));

    // Test concurrencyUpperBound with kConcurrencyUpperBoundUseStaticLimit
    // EXECUTION_RATE method
    setConfig(
        CPUConcurrencyController::Config{
            .mode = CPUConcurrencyController::Mode::ENABLED,
            .method = CPUConcurrencyController::Method::EXECUTION_RATE,
            .concurrencyUpperBound =
                CPUConcurrencyController::Config::UseStaticLimit{},
        });

    folly::observer_detail::ObserverManager::waitForAllUpdates();
    EXPECT_EQ(getCPUConcurrencyController().getConcurrencyUpperBound(), 500);
  }
  {
    setConfig(
        CPUConcurrencyController::Config{
            .mode = CPUConcurrencyController::Mode::DISABLED});

    // Test concurrencyUpperBound with negative value
    setConfig(
        CPUConcurrencyController::Config{
            .mode = CPUConcurrencyController::Mode::ENABLED,
            .method = CPUConcurrencyController::Method::MAX_QPS,
            .concurrencyUpperBound = -2,
        });

    folly::observer_detail::ObserverManager::waitForAllUpdates();
    EXPECT_EQ(getCPUConcurrencyController().getConcurrencyUpperBound(), 0);
  }
}

// ---------------------------------------------------------------------------
// requestStarted / requestShed tests
// ---------------------------------------------------------------------------

TEST_F(CPUConcurrencyControllerTest, requestStartedNoOpWhenDisabled) {
  // Controller is disabled by default. Calling requestStarted should not crash
  // and should not count requests.
  for (int i = 0; i < 100; ++i) {
    getCPUConcurrencyController().requestStarted();
  }
  EXPECT_FALSE(getCPUConcurrencyController().enabled());
}

TEST_F(CPUConcurrencyControllerTest, requestShedReturnsFalseWhenDisabled) {
  EXPECT_FALSE(getCPUConcurrencyController().requestShed());
  EXPECT_FALSE(
      getCPUConcurrencyController().requestShed(
          CPUConcurrencyController::Method::MAX_QPS));
}

TEST_F(CPUConcurrencyControllerTest, requestShedReturnsTrueWhenEnabled) {
  setConfig(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::ENABLED,
          .method = CPUConcurrencyController::Method::MAX_QPS,
          .collectionSampleSize = 0,
          .concurrencyUpperBound = 100});
  folly::observer_detail::ObserverManager::waitForAllUpdates();

  // Matching method returns true
  EXPECT_TRUE(
      getCPUConcurrencyController().requestShed(
          CPUConcurrencyController::Method::MAX_QPS));

  // nullopt method (any) returns true
  EXPECT_TRUE(getCPUConcurrencyController().requestShed());
}

TEST_F(
    CPUConcurrencyControllerTest, requestShedReturnsFalseForMismatchedMethod) {
  setConfig(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::ENABLED,
          .method = CPUConcurrencyController::Method::MAX_QPS,
          .collectionSampleSize = 0,
          .concurrencyUpperBound = 100});
  folly::observer_detail::ObserverManager::waitForAllUpdates();

  // Non-matching method returns false
  EXPECT_FALSE(
      getCPUConcurrencyController().requestShed(
          CPUConcurrencyController::Method::MAX_REQUESTS));
  EXPECT_FALSE(
      getCPUConcurrencyController().requestShed(
          CPUConcurrencyController::Method::CONCURRENCY_LIMIT));
}

// ---------------------------------------------------------------------------
// enabled() / getLoad() / isRefractoryPeriod() public getter tests
// ---------------------------------------------------------------------------

TEST_F(CPUConcurrencyControllerTest, enabledReflectsConfig) {
  EXPECT_FALSE(getCPUConcurrencyController().enabled());

  setConfig(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::ENABLED,
          .collectionSampleSize = 0,
          .concurrencyUpperBound = 100});
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  EXPECT_TRUE(getCPUConcurrencyController().enabled());

  setConfig(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::DRY_RUN,
          .collectionSampleSize = 0,
          .concurrencyUpperBound = 100});
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  EXPECT_TRUE(getCPUConcurrencyController().enabled());

  setConfig(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::DISABLED});
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  EXPECT_FALSE(getCPUConcurrencyController().enabled());
}

TEST_F(CPUConcurrencyControllerTest, getLoadReturnsCurrentCPULoad) {
  setCPULoad(42);
  EXPECT_EQ(getCPUConcurrencyController().getLoad(), 42);
}

TEST_F(CPUConcurrencyControllerTest, getLoadClampsValues) {
  setCPULoad(200);
  EXPECT_EQ(getCPUConcurrencyController().getLoad(), 100);

  setCPULoad(-50);
  EXPECT_EQ(getCPUConcurrencyController().getLoad(), 0);
}

// ---------------------------------------------------------------------------
// Custom LoadFunc tests
// ---------------------------------------------------------------------------

TEST_F(CPUConcurrencyControllerTest, customLoadFuncIsUsed) {
  using namespace apache::thrift;
  server::test::MockServerConfigs serverConfigs{};
  ThriftServerConfig thriftServerConfig{};
  folly::observer::SimpleObservable<CPUConcurrencyController::Config>
      configObservable;

  std::atomic<int64_t> customLoad{77};
  CPUConcurrencyController::LoadFunc loadFunc =
      [&customLoad](std::chrono::milliseconds, CPULoadSource) -> int64_t {
    return customLoad.load();
  };

  CPUConcurrencyController controller{
      configObservable.getObserver(),
      serverConfigs,
      thriftServerConfig,
      loadFunc};

  EXPECT_EQ(controller.getLoad(), 77);

  customLoad = 33;
  EXPECT_EQ(controller.getLoad(), 33);
}

TEST_F(CPUConcurrencyControllerTest, customLoadFuncClampsValues) {
  using namespace apache::thrift;
  server::test::MockServerConfigs serverConfigs{};
  ThriftServerConfig thriftServerConfig{};
  folly::observer::SimpleObservable<CPUConcurrencyController::Config>
      configObservable;

  CPUConcurrencyController::LoadFunc loadFunc =
      [](std::chrono::milliseconds, CPULoadSource) -> int64_t { return 150; };

  CPUConcurrencyController controller{
      configObservable.getObserver(),
      serverConfigs,
      thriftServerConfig,
      loadFunc};

  EXPECT_EQ(controller.getLoad(), 100);
}

TEST_F(CPUConcurrencyControllerTest, setLoadFuncOverridesPluggable) {
  setCPULoad(42);
  EXPECT_EQ(getCPUConcurrencyController().getLoad(), 42);

  getCPUConcurrencyController().setLoadFunc(
      [](std::chrono::milliseconds, apache::thrift::CPULoadSource) {
        return int64_t{88};
      });
  EXPECT_EQ(getCPUConcurrencyController().getLoad(), 88);
}

// ---------------------------------------------------------------------------
// Overload: limit decrease tests
// ---------------------------------------------------------------------------

TEST_F(CPUConcurrencyControllerTest, overloadDecreasesLimit) {
  using namespace apache::thrift;
  server::test::MockServerConfigs serverConfigs{};
  ThriftServerConfig thriftServerConfig{};
  folly::observer::SimpleObservable<CPUConcurrencyController::Config>
      configObservable;

  std::atomic<int64_t> load{95};
  CPUConcurrencyController::LoadFunc loadFunc =
      [&load](std::chrono::milliseconds, CPULoadSource) { return load.load(); };

  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::ENABLED,
          .method = CPUConcurrencyController::Method::MAX_QPS,
          .cpuTarget = 90,
          .decreaseMultiplier = 0.10,
          .collectionSampleSize = 0,
          .concurrencyUpperBound = 1000,
      });

  CPUConcurrencyController controller{
      configObservable.getObserver(),
      serverConfigs,
      thriftServerConfig,
      loadFunc};

  auto eventHandler = std::make_shared<::testing::NiceMock<MockEventHandler>>();
  folly::Baton<> decreased;
  int64_t limitAfterDecrease = 0;

  EXPECT_CALL(*eventHandler, limitDecreased())
      .WillOnce(::testing::Invoke([&]() {
        // limit should have decreased: 1000 - max(1000*0.10, 1) = 900
        decreased.post();
      }));
  EXPECT_CALL(*eventHandler, onCycle(::testing::_, ::testing::_, ::testing::_))
      .WillRepeatedly(::testing::Invoke([&](int64_t limit, auto, auto) {
        limitAfterDecrease = limit;
      }));

  controller.setEventHandler(eventHandler);

  ASSERT_TRUE(decreased.try_wait_for(2s));
  // The initial limit is 1000 (concurrencyUpperBound). After one overload
  // cycle with decreaseMultiplier=0.10: 1000 - 100 = 900.
  // limitAfterDecrease captures the limit passed to onCycle on the *first*
  // cycle (before it was decreased).
  EXPECT_EQ(limitAfterDecrease, 1000);

  // Disable to clean up
  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::DISABLED});
}

TEST_F(CPUConcurrencyControllerTest, decreaseClampsToConcurrencyLowerBound) {
  using namespace apache::thrift;
  server::test::MockServerConfigs serverConfigs{};
  ThriftServerConfig thriftServerConfig{};
  folly::observer::SimpleObservable<CPUConcurrencyController::Config>
      configObservable;

  CPUConcurrencyController::LoadFunc loadFunc =
      [](std::chrono::milliseconds, CPULoadSource) { return int64_t{99}; };

  // Set a high lower bound and small upper bound so that the decrease
  // clamps to the lower bound quickly.
  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::ENABLED,
          .method = CPUConcurrencyController::Method::MAX_QPS,
          .cpuTarget = 90,
          .decreaseMultiplier = 0.50,
          .collectionSampleSize = 0,
          .concurrencyUpperBound = 100,
          .concurrencyLowerBound = 80,
      });

  CPUConcurrencyController controller{
      configObservable.getObserver(),
      serverConfigs,
      thriftServerConfig,
      loadFunc};

  auto eventHandler = std::make_shared<::testing::NiceMock<MockEventHandler>>();

  // After first decrease: 100 - max(100*0.50, 1) = 50, but clamped to 80.
  // After second decrease: 80 - max(80*0.50, 1) = 40, but clamped to 80.
  // So limit should stabilize at 80.
  folly::Baton<> secondDecrease;
  int decreaseCount = 0;
  int64_t lastLimit = 0;
  EXPECT_CALL(*eventHandler, limitDecreased())
      .WillRepeatedly(::testing::Invoke([&]() {
        ++decreaseCount;
        if (decreaseCount >= 2) {
          secondDecrease.post();
        }
      }));
  EXPECT_CALL(*eventHandler, onCycle(::testing::_, ::testing::_, ::testing::_))
      .WillRepeatedly(::testing::Invoke([&](int64_t limit, auto, auto) {
        lastLimit = limit;
      }));

  controller.setEventHandler(eventHandler);

  ASSERT_TRUE(secondDecrease.try_wait_for(2s));
  // After first decrease the limit becomes max(50, 80) = 80.
  // On the second cycle, onCycle reports limit=80.
  EXPECT_EQ(lastLimit, 80);

  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::DISABLED});
}

TEST_F(CPUConcurrencyControllerTest, decreaseByAtLeastOne) {
  using namespace apache::thrift;
  server::test::MockServerConfigs serverConfigs{};
  ThriftServerConfig thriftServerConfig{};
  folly::observer::SimpleObservable<CPUConcurrencyController::Config>
      configObservable;

  CPUConcurrencyController::LoadFunc loadFunc =
      [](std::chrono::milliseconds, CPULoadSource) { return int64_t{99}; };

  // With a tiny multiplier and small limit, limit*multiplier < 1,
  // so the decrease should still be 1.
  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::ENABLED,
          .method = CPUConcurrencyController::Method::MAX_QPS,
          .cpuTarget = 90,
          .decreaseMultiplier = 0.001,
          .collectionSampleSize = 0,
          .concurrencyUpperBound = 5,
          .concurrencyLowerBound = 1,
      });

  CPUConcurrencyController controller{
      configObservable.getObserver(),
      serverConfigs,
      thriftServerConfig,
      loadFunc};

  auto eventHandler = std::make_shared<::testing::NiceMock<MockEventHandler>>();
  folly::Baton<> secondCycle;
  int64_t secondLimit = 0;
  int cycleCount = 0;

  EXPECT_CALL(*eventHandler, onCycle(::testing::_, ::testing::_, ::testing::_))
      .WillRepeatedly(::testing::Invoke([&](int64_t limit, auto, auto) {
        ++cycleCount;
        // Cycle 1: EMA seeds at 0, load=0 (no-op).
        // Cycle 2: load=99 (overload), decrease fires after onCycle.
        // Cycle 3: onCycle sees the decreased limit (4).
        if (cycleCount == 3) {
          secondLimit = limit;
          secondCycle.post();
        }
      }));
  EXPECT_CALL(*eventHandler, limitDecreased())
      .WillRepeatedly(::testing::Return());

  controller.setEventHandler(eventHandler);

  ASSERT_TRUE(secondCycle.try_wait_for(2s));
  // Initial limit = 5. After one decrease by max(5*0.001=0, 1)=1 -> limit=4.
  EXPECT_EQ(secondLimit, 4);

  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::DISABLED});
}

// ---------------------------------------------------------------------------
// Underload: limit increase tests
// ---------------------------------------------------------------------------

TEST_F(CPUConcurrencyControllerTest, increaseClampsToConcurrencyUpperBound) {
  using namespace apache::thrift;
  server::test::MockServerConfigs serverConfigs{};
  ThriftServerConfig thriftServerConfig{};
  folly::observer::SimpleObservable<CPUConcurrencyController::Config>
      configObservable;

  std::atomic<int64_t> load{50};
  CPUConcurrencyController::LoadFunc loadFunc =
      [&load](std::chrono::milliseconds, CPULoadSource) { return load.load(); };

  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::ENABLED,
          .method = CPUConcurrencyController::Method::MAX_QPS,
          .cpuTarget = 90,
          .additiveMultiplier = 0.50,
          .collectionSampleSize = 0,
          .concurrencyUpperBound = 10,
          .concurrencyLowerBound = 1,
      });

  CPUConcurrencyController controller{
      configObservable.getObserver(),
      serverConfigs,
      thriftServerConfig,
      loadFunc};

  // Generate enough requests to trigger nearExistingLimit
  auto eventHandler = std::make_shared<::testing::NiceMock<MockEventHandler>>();
  folly::Baton<> increased;
  EXPECT_CALL(*eventHandler, limitIncreased())
      .WillOnce(::testing::Invoke([&]() { increased.post(); }))
      .WillRepeatedly(::testing::Return());
  EXPECT_CALL(*eventHandler, onCycle(::testing::_, ::testing::_, ::testing::_))
      .WillRepeatedly(
          ::testing::Invoke([&controller, &load](int64_t, auto, auto) {
            // Pump requests to keep QPS high relative to the limit
            for (int i = 0; i < 100; ++i) {
              controller.requestStarted();
            }
          }));

  controller.setEventHandler(eventHandler);
  // Also pump requests from this thread
  for (int i = 0; i < 200; ++i) {
    controller.requestStarted();
  }

  ASSERT_TRUE(increased.try_wait_for(2s));

  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::DISABLED});
}

TEST_F(CPUConcurrencyControllerTest, noIncreaseWhenUsageIsZero) {
  using namespace apache::thrift;
  server::test::MockServerConfigs serverConfigs{};
  ThriftServerConfig thriftServerConfig{};
  folly::observer::SimpleObservable<CPUConcurrencyController::Config>
      configObservable;

  CPUConcurrencyController::LoadFunc loadFunc =
      [](std::chrono::milliseconds, CPULoadSource) { return int64_t{50}; };

  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::ENABLED,
          .method = CPUConcurrencyController::Method::MAX_QPS,
          .cpuTarget = 90,
          .collectionSampleSize = 0,
          .concurrencyUpperBound = 100,
      });

  CPUConcurrencyController controller{
      configObservable.getObserver(),
      serverConfigs,
      thriftServerConfig,
      loadFunc};

  auto eventHandler = std::make_shared<::testing::NiceMock<MockEventHandler>>();
  folly::Baton<> cycled;
  int cycleCount = 0;

  // Don't send any requests. Usage should be 0.
  // Limit should not increase and should not decrease (CPU < target).
  EXPECT_CALL(*eventHandler, limitIncreased()).Times(0);
  EXPECT_CALL(*eventHandler, limitDecreased()).Times(0);
  EXPECT_CALL(*eventHandler, onCycle(::testing::_, ::testing::_, ::testing::_))
      .WillRepeatedly(::testing::Invoke([&](auto, auto, auto) {
        if (++cycleCount >= 3) {
          cycled.post();
        }
      }));

  controller.setEventHandler(eventHandler);

  ASSERT_TRUE(cycled.try_wait_for(2s));

  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::DISABLED});
}

TEST_F(CPUConcurrencyControllerTest, noChangeWhenLoadIsZero) {
  using namespace apache::thrift;
  server::test::MockServerConfigs serverConfigs{};
  ThriftServerConfig thriftServerConfig{};
  folly::observer::SimpleObservable<CPUConcurrencyController::Config>
      configObservable;

  // Load <= 0 triggers the early return in the underload branch
  CPUConcurrencyController::LoadFunc loadFunc =
      [](std::chrono::milliseconds, CPULoadSource) { return int64_t{-5}; };

  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::ENABLED,
          .method = CPUConcurrencyController::Method::MAX_QPS,
          .cpuTarget = 90,
          .collectionSampleSize = 0,
          .concurrencyUpperBound = 100,
      });

  CPUConcurrencyController controller{
      configObservable.getObserver(),
      serverConfigs,
      thriftServerConfig,
      loadFunc};

  // Send requests so usage > 0
  for (int i = 0; i < 100; ++i) {
    controller.requestStarted();
  }

  auto eventHandler = std::make_shared<::testing::NiceMock<MockEventHandler>>();
  folly::Baton<> cycled;
  int cycleCount = 0;

  EXPECT_CALL(*eventHandler, limitIncreased()).Times(0);
  EXPECT_CALL(*eventHandler, limitDecreased()).Times(0);
  EXPECT_CALL(*eventHandler, onCycle(::testing::_, ::testing::_, ::testing::_))
      .WillRepeatedly(::testing::Invoke([&](auto, auto, auto) {
        if (++cycleCount >= 3) {
          cycled.post();
        }
      }));

  controller.setEventHandler(eventHandler);

  ASSERT_TRUE(cycled.try_wait_for(2s));

  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::DISABLED});
}

// ---------------------------------------------------------------------------
// bumpOnError tests
// ---------------------------------------------------------------------------

TEST_F(CPUConcurrencyControllerTest, bumpOnErrorIncreasesOnShed) {
  using namespace apache::thrift;
  server::test::MockServerConfigs serverConfigs{};
  ThriftServerConfig thriftServerConfig{};
  folly::observer::SimpleObservable<CPUConcurrencyController::Config>
      configObservable;

  std::atomic<int64_t> load{50};
  CPUConcurrencyController::LoadFunc loadFunc =
      [&load](std::chrono::milliseconds, CPULoadSource) { return load.load(); };

  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::ENABLED,
          .method = CPUConcurrencyController::Method::MAX_QPS,
          .cpuTarget = 90,
          .additiveMultiplier = 0.05,
          .bumpOnError = true,
          .collectionSampleSize = 0,
          .concurrencyUpperBound = 100,
      });

  CPUConcurrencyController controller{
      configObservable.getObserver(),
      serverConfigs,
      thriftServerConfig,
      loadFunc};

  auto eventHandler = std::make_shared<::testing::NiceMock<MockEventHandler>>();
  folly::Baton<> increased;

  EXPECT_CALL(*eventHandler, limitIncreased())
      .WillOnce(::testing::Invoke([&]() { increased.post(); }))
      .WillRepeatedly(::testing::Return());

  // With bumpOnError=true, the nearExistingLimit check is skipped.
  // Instead, it only increases when requestShed() was called recently.
  // We pump requests AND call requestShed to trigger the bump.
  EXPECT_CALL(*eventHandler, onCycle(::testing::_, ::testing::_, ::testing::_))
      .WillRepeatedly(::testing::Invoke([&controller](int64_t, auto, auto) {
        for (int i = 0; i < 100; ++i) {
          controller.requestStarted();
        }
        controller.requestShed();
      }));

  controller.setEventHandler(eventHandler);

  ASSERT_TRUE(increased.try_wait_for(2s));

  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::DISABLED});
}

TEST_F(
    CPUConcurrencyControllerTest, bumpOnErrorDoesNotIncreaseWithoutShedEvent) {
  using namespace apache::thrift;
  server::test::MockServerConfigs serverConfigs{};
  ThriftServerConfig thriftServerConfig{};
  folly::observer::SimpleObservable<CPUConcurrencyController::Config>
      configObservable;

  CPUConcurrencyController::LoadFunc loadFunc =
      [](std::chrono::milliseconds, CPULoadSource) { return int64_t{50}; };

  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::ENABLED,
          .method = CPUConcurrencyController::Method::MAX_QPS,
          .cpuTarget = 90,
          .bumpOnError = true,
          .collectionSampleSize = 0,
          .concurrencyUpperBound = 100,
      });

  CPUConcurrencyController controller{
      configObservable.getObserver(),
      serverConfigs,
      thriftServerConfig,
      loadFunc};

  // Send requests (high usage) but DON'T call requestShed()
  auto eventHandler = std::make_shared<::testing::NiceMock<MockEventHandler>>();
  folly::Baton<> cycled;
  int cycleCount = 0;

  EXPECT_CALL(*eventHandler, limitIncreased()).Times(0);
  EXPECT_CALL(*eventHandler, limitDecreased()).Times(0);
  EXPECT_CALL(*eventHandler, onCycle(::testing::_, ::testing::_, ::testing::_))
      .WillRepeatedly(
          ::testing::Invoke(
              [&controller, &cycleCount, &cycled](int64_t, auto, auto) {
                for (int i = 0; i < 100; ++i) {
                  controller.requestStarted();
                }
                if (++cycleCount >= 3) {
                  cycled.post();
                }
              }));

  controller.setEventHandler(eventHandler);

  ASSERT_TRUE(cycled.try_wait_for(2s));

  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::DISABLED});
}

// ---------------------------------------------------------------------------
// Bootstrap / stable estimate tests
// ---------------------------------------------------------------------------

TEST_F(CPUConcurrencyControllerTest, stableEstimateCollectedDuringBootstrap) {
  using namespace apache::thrift;
  server::test::MockServerConfigs serverConfigs{};
  ThriftServerConfig thriftServerConfig{};
  folly::observer::SimpleObservable<CPUConcurrencyController::Config>
      configObservable;

  CPUConcurrencyController::LoadFunc loadFunc =
      [](std::chrono::milliseconds, CPULoadSource) { return int64_t{50}; };

  // Use a small collectionSampleSize so the estimate is computed quickly
  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::ENABLED,
          .method = CPUConcurrencyController::Method::MAX_QPS,
          .cpuTarget = 90,
          .refractoryPeriodMs = 0ms,
          .initialEstimateFactor = 1.0,
          .initialEstimatePercentile = 0.5,
          .collectionSampleSize = 3,
          .concurrencyUpperBound = 1000,
          .concurrencyLowerBound = 1,
      });

  CPUConcurrencyController controller{
      configObservable.getObserver(),
      serverConfigs,
      thriftServerConfig,
      loadFunc};

  auto eventHandler = std::make_shared<::testing::NiceMock<MockEventHandler>>();
  folly::Baton<> estimateReady;

  EXPECT_CALL(*eventHandler, onCycle(::testing::_, ::testing::_, ::testing::_))
      .WillRepeatedly(
          ::testing::Invoke([&controller, &estimateReady](int64_t, auto, auto) {
            // Pump requests to ensure non-zero usage
            for (int i = 0; i < 100; ++i) {
              controller.requestStarted();
            }
            if (controller.getStableEstimate() > 0) {
              estimateReady.post();
            }
          }));
  EXPECT_CALL(*eventHandler, limitIncreased())
      .WillRepeatedly(::testing::Return());
  EXPECT_CALL(*eventHandler, limitDecreased())
      .WillRepeatedly(::testing::Return());

  controller.setEventHandler(eventHandler);

  ASSERT_TRUE(estimateReady.try_wait_for(5s));
  EXPECT_GT(controller.getStableEstimate(), 0);

  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::DISABLED});
}

TEST_F(CPUConcurrencyControllerTest, stableEstimateResetsOnConfigChange) {
  using namespace apache::thrift;
  server::test::MockServerConfigs serverConfigs{};
  ThriftServerConfig thriftServerConfig{};
  folly::observer::SimpleObservable<CPUConcurrencyController::Config>
      configObservable;

  CPUConcurrencyController::LoadFunc loadFunc =
      [](std::chrono::milliseconds, CPULoadSource) { return int64_t{50}; };

  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::ENABLED,
          .method = CPUConcurrencyController::Method::MAX_QPS,
          .cpuTarget = 90,
          .refractoryPeriodMs = 0ms,
          .collectionSampleSize = 3,
          .concurrencyUpperBound = 1000,
      });

  CPUConcurrencyController controller{
      configObservable.getObserver(),
      serverConfigs,
      thriftServerConfig,
      loadFunc};

  auto eventHandler = std::make_shared<::testing::NiceMock<MockEventHandler>>();
  folly::Baton<> estimateReady;

  EXPECT_CALL(*eventHandler, onCycle(::testing::_, ::testing::_, ::testing::_))
      .WillRepeatedly(
          ::testing::Invoke([&controller, &estimateReady](int64_t, auto, auto) {
            for (int i = 0; i < 100; ++i) {
              controller.requestStarted();
            }
            if (controller.getStableEstimate() > 0) {
              estimateReady.post();
            }
          }));
  EXPECT_CALL(*eventHandler, limitIncreased())
      .WillRepeatedly(::testing::Return());
  EXPECT_CALL(*eventHandler, limitDecreased())
      .WillRepeatedly(::testing::Return());

  controller.setEventHandler(eventHandler);

  ASSERT_TRUE(estimateReady.try_wait_for(5s));
  EXPECT_GT(controller.getStableEstimate(), 0);

  // Change config -- this should reset stable estimate
  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::DISABLED});
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  // After a brief wait for cancel() to take effect
  /* sleep override */
  std::this_thread::sleep_for(100ms);
  EXPECT_EQ(controller.getStableEstimate(), -1);
}

// ---------------------------------------------------------------------------
// Refractory period tests
// ---------------------------------------------------------------------------

TEST_F(CPUConcurrencyControllerTest, refractoryPeriodSuppressesSampling) {
  using namespace apache::thrift;
  server::test::MockServerConfigs serverConfigs{};
  ThriftServerConfig thriftServerConfig{};
  folly::observer::SimpleObservable<CPUConcurrencyController::Config>
      configObservable;

  std::atomic<int64_t> load{99};
  CPUConcurrencyController::LoadFunc loadFunc =
      [&load](std::chrono::milliseconds, CPULoadSource) { return load.load(); };

  // Long refractory period so stable estimate can't be collected after overload
  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::ENABLED,
          .method = CPUConcurrencyController::Method::MAX_QPS,
          .cpuTarget = 90,
          .refractoryPeriodMs = 10000ms,
          .collectionSampleSize = 2,
          .concurrencyUpperBound = 100,
      });

  CPUConcurrencyController controller{
      configObservable.getObserver(),
      serverConfigs,
      thriftServerConfig,
      loadFunc};

  auto eventHandler = std::make_shared<::testing::NiceMock<MockEventHandler>>();
  folly::Baton<> overloaded;

  EXPECT_CALL(*eventHandler, limitDecreased())
      .WillOnce(::testing::Invoke([&]() { overloaded.post(); }))
      .WillRepeatedly(::testing::Return());
  EXPECT_CALL(*eventHandler, onCycle(::testing::_, ::testing::_, ::testing::_))
      .WillRepeatedly(::testing::Return());

  controller.setEventHandler(eventHandler);

  // Wait for overload to register
  ASSERT_TRUE(overloaded.try_wait_for(2s));
  EXPECT_TRUE(controller.isRefractoryPeriod());

  // Now switch to underload with requests
  load = 50;
  folly::Baton<> cycled;
  int underloadCycles = 0;

  // Replace event handler expectations
  auto eventHandler2 =
      std::make_shared<::testing::NiceMock<MockEventHandler>>();
  EXPECT_CALL(*eventHandler2, onCycle(::testing::_, ::testing::_, ::testing::_))
      .WillRepeatedly(
          ::testing::Invoke(
              [&controller, &underloadCycles, &cycled](int64_t, auto, auto) {
                for (int i = 0; i < 100; ++i) {
                  controller.requestStarted();
                }
                if (++underloadCycles >= 5) {
                  cycled.post();
                }
              }));
  EXPECT_CALL(*eventHandler2, limitIncreased())
      .WillRepeatedly(::testing::Return());
  EXPECT_CALL(*eventHandler2, limitDecreased())
      .WillRepeatedly(::testing::Return());

  controller.setEventHandler(eventHandler2);

  ASSERT_TRUE(cycled.try_wait_for(2s));
  // Stable estimate should NOT have been collected because we're in
  // the refractory period (10s timeout, test runs in ~1s)
  EXPECT_EQ(controller.getStableEstimate(), -1);

  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::DISABLED});
}

// ---------------------------------------------------------------------------
// DRY_RUN mode tests
// ---------------------------------------------------------------------------

TEST_F(CPUConcurrencyControllerTest, dryRunDoesNotAffectServerLimits) {
  using namespace apache::thrift;
  server::test::MockServerConfigs serverConfigs{};
  ThriftServerConfig thriftServerConfig{};
  folly::observer::SimpleObservable<CPUConcurrencyController::Config>
      configObservable;

  std::atomic<int64_t> load{99};
  CPUConcurrencyController::LoadFunc loadFunc =
      [&load](std::chrono::milliseconds, CPULoadSource) { return load.load(); };

  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::DRY_RUN,
          .method = CPUConcurrencyController::Method::MAX_QPS,
          .cpuTarget = 90,
          .collectionSampleSize = 0,
          .concurrencyUpperBound = 100,
      });

  CPUConcurrencyController controller{
      configObservable.getObserver(),
      serverConfigs,
      thriftServerConfig,
      loadFunc};

  auto eventHandler = std::make_shared<::testing::NiceMock<MockEventHandler>>();
  folly::Baton<> decreased;

  EXPECT_CALL(*eventHandler, limitDecreased())
      .WillOnce(::testing::Invoke([&]() { decreased.post(); }))
      .WillRepeatedly(::testing::Return());
  EXPECT_CALL(*eventHandler, onCycle(::testing::_, ::testing::_, ::testing::_))
      .WillRepeatedly(::testing::Return());

  controller.setEventHandler(eventHandler);

  ASSERT_TRUE(decreased.try_wait_for(2s));

  // In DRY_RUN mode, the actual server config should NOT be modified.
  // The DRY_RUN limit tracks internally via dryRunLimit_ but doesn't
  // push to the server config observables.
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  SUCCEED() << "DRY_RUN mode ran cycles with limitDecreased without crashing";

  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::DISABLED});
}

TEST_F(CPUConcurrencyControllerTest, dryRunModeIsEnabled) {
  setConfig(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::DRY_RUN,
          .collectionSampleSize = 0,
          .concurrencyUpperBound = 100});
  folly::observer_detail::ObserverManager::waitForAllUpdates();

  // DRY_RUN is considered enabled
  EXPECT_TRUE(getCPUConcurrencyController().enabled());

  // requestStarted and requestShed should work
  getCPUConcurrencyController().requestStarted();
  EXPECT_TRUE(getCPUConcurrencyController().requestShed());
}

// ---------------------------------------------------------------------------
// MAX_REQUESTS method tests
// ---------------------------------------------------------------------------

TEST_F(
    CPUConcurrencyControllerTest, maxRequestsMethodUsesActiveRequestsForUsage) {
  using namespace apache::thrift;
  server::test::MockServerConfigs serverConfigs{};
  ThriftServerConfig thriftServerConfig{};
  folly::observer::SimpleObservable<CPUConcurrencyController::Config>
      configObservable;

  std::atomic<int64_t> load{50};
  CPUConcurrencyController::LoadFunc loadFunc =
      [&load](std::chrono::milliseconds, CPULoadSource) { return load.load(); };

  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::ENABLED,
          .method = CPUConcurrencyController::Method::MAX_REQUESTS,
          .cpuTarget = 90,
          .increaseDistanceRatio = 0.5,
          .collectionSampleSize = 0,
          .concurrencyUpperBound = 100,
      });

  CPUConcurrencyController controller{
      configObservable.getObserver(),
      serverConfigs,
      thriftServerConfig,
      loadFunc};

  // Simulate active requests via the base class method
  for (int i = 0; i < 80; ++i) {
    serverConfigs.incActiveRequests();
  }

  auto eventHandler = std::make_shared<::testing::NiceMock<MockEventHandler>>();
  folly::Baton<> cycled;
  int64_t reportedUsage = 0;

  EXPECT_CALL(*eventHandler, onCycle(::testing::_, ::testing::_, ::testing::_))
      .WillOnce(::testing::Invoke([&](int64_t, int64_t usage, int64_t) {
        reportedUsage = usage;
        cycled.post();
      }))
      .WillRepeatedly(::testing::Return());
  EXPECT_CALL(*eventHandler, limitIncreased())
      .WillRepeatedly(::testing::Return());
  EXPECT_CALL(*eventHandler, limitDecreased())
      .WillRepeatedly(::testing::Return());

  controller.setEventHandler(eventHandler);

  ASSERT_TRUE(cycled.try_wait_for(2s));
  // getLimitUsage for MAX_REQUESTS returns getActiveRequests()
  EXPECT_EQ(reportedUsage, 80);

  // Clean up active requests
  for (int i = 0; i < 80; ++i) {
    serverConfigs.decActiveRequests();
  }

  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::DISABLED});
}

// ---------------------------------------------------------------------------
// Config helper method tests
// ---------------------------------------------------------------------------

TEST_F(CPUConcurrencyControllerTest, configModeName) {
  CPUConcurrencyController::Config config;

  config.mode = CPUConcurrencyController::Mode::DISABLED;
  EXPECT_EQ(config.modeName(), "DISABLED");

  config.mode = CPUConcurrencyController::Mode::DRY_RUN;
  EXPECT_EQ(config.modeName(), "DRY_RUN");

  config.mode = CPUConcurrencyController::Mode::ENABLED;
  EXPECT_EQ(config.modeName(), "ENABLED");
}

TEST_F(CPUConcurrencyControllerTest, configMethodName) {
  CPUConcurrencyController::Config config;

  config.method = CPUConcurrencyController::Method::MAX_REQUESTS;
  EXPECT_EQ(config.methodName(), "MAX_REQUESTS");

  config.method = CPUConcurrencyController::Method::MAX_QPS;
  EXPECT_EQ(config.methodName(), "MAX_QPS");

  config.method = CPUConcurrencyController::Method::CONCURRENCY_LIMIT;
  EXPECT_EQ(config.methodName(), "CONCURRENCY_LIMIT");

  config.method = CPUConcurrencyController::Method::EXECUTION_RATE;
  EXPECT_EQ(config.methodName(), "EXECUTION_RATE");
}

TEST_F(CPUConcurrencyControllerTest, configCpuLoadSourceName) {
  CPUConcurrencyController::Config config;

  config.cpuLoadSource = apache::thrift::CPULoadSource::CONTAINER_AND_HOST;
  EXPECT_EQ(config.cpuLoadSourceName(), "CONTAINER_AND_HOST");

  config.cpuLoadSource = apache::thrift::CPULoadSource::CONTAINER_ONLY;
  EXPECT_EQ(config.cpuLoadSourceName(), "CONTAINER_ONLY");

  config.cpuLoadSource = apache::thrift::CPULoadSource::HOST_ONLY;
  EXPECT_EQ(config.cpuLoadSourceName(), "HOST_ONLY");
}

TEST_F(CPUConcurrencyControllerTest, configConcurrencyUnit) {
  CPUConcurrencyController::Config config;

  config.method = CPUConcurrencyController::Method::MAX_REQUESTS;
  EXPECT_EQ(config.concurrencyUnit(), "maxRequests");

  config.method = CPUConcurrencyController::Method::MAX_QPS;
  EXPECT_EQ(config.concurrencyUnit(), "maxQps");

  config.method = CPUConcurrencyController::Method::CONCURRENCY_LIMIT;
  EXPECT_EQ(config.concurrencyUnit(), "concurrencyLimit");

  config.method = CPUConcurrencyController::Method::EXECUTION_RATE;
  EXPECT_EQ(config.concurrencyUnit(), "executionRate");
}

TEST_F(CPUConcurrencyControllerTest, configConcurrencyUpperBoundName) {
  CPUConcurrencyController::Config config;

  config.concurrencyUpperBound = 42;
  EXPECT_EQ(config.concurrencyUpperBoundName(), "42");

  config.concurrencyUpperBound =
      CPUConcurrencyController::Config::UseStaticLimit{};
  EXPECT_EQ(config.concurrencyUpperBoundName(), "UseStaticLimit");
}

TEST_F(CPUConcurrencyControllerTest, configDescribe) {
  CPUConcurrencyController::Config config{
      .mode = CPUConcurrencyController::Mode::ENABLED,
      .method = CPUConcurrencyController::Method::MAX_QPS,
      .cpuTarget = 85,
      .refreshPeriodMs = 100ms,
      .concurrencyUpperBound = 500,
  };

  auto desc = config.describe();
  EXPECT_NE(desc.find("ENABLED"), std::string::npos);
  EXPECT_NE(desc.find("MAX_QPS"), std::string::npos);
  EXPECT_NE(desc.find("85"), std::string::npos);
  EXPECT_NE(desc.find("100"), std::string::npos);
  EXPECT_NE(desc.find("500"), std::string::npos);
}

TEST_F(CPUConcurrencyControllerTest, configEnabled) {
  CPUConcurrencyController::Config config;

  config.mode = CPUConcurrencyController::Mode::DISABLED;
  EXPECT_FALSE(config.enabled());

  config.mode = CPUConcurrencyController::Mode::DRY_RUN;
  EXPECT_TRUE(config.enabled());

  config.mode = CPUConcurrencyController::Mode::ENABLED;
  EXPECT_TRUE(config.enabled());
}

// ---------------------------------------------------------------------------
// shouldConvergeStable tests
// ---------------------------------------------------------------------------

TEST_F(CPUConcurrencyControllerTest, convergesBackToStableEstimate) {
  using namespace apache::thrift;
  server::test::MockServerConfigs serverConfigs{};
  ThriftServerConfig thriftServerConfig{};
  folly::observer::SimpleObservable<CPUConcurrencyController::Config>
      configObservable;

  std::atomic<int64_t> load{50};
  CPUConcurrencyController::LoadFunc loadFunc =
      [&load](std::chrono::milliseconds, CPULoadSource) { return load.load(); };

  // Short refractory period so we can observe convergence
  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::ENABLED,
          .method = CPUConcurrencyController::Method::MAX_QPS,
          .cpuTarget = 90,
          .additiveMultiplier = 0.05,
          .decreaseMultiplier = 0.50,
          .refractoryPeriodMs = 50ms,
          .initialEstimateFactor = 1.0,
          .initialEstimatePercentile = 0.5,
          .collectionSampleSize = 3,
          .concurrencyUpperBound = 1000,
          .concurrencyLowerBound = 1,
      });

  CPUConcurrencyController controller{
      configObservable.getObserver(),
      serverConfigs,
      thriftServerConfig,
      loadFunc};

  auto eventHandler = std::make_shared<::testing::NiceMock<MockEventHandler>>();
  folly::Baton<> estimateReady;
  folly::Baton<> overloaded;
  folly::Baton<> converging;

  enum class Phase { BOOTSTRAP, OVERLOAD, CONVERGE };
  std::atomic<Phase> phase{Phase::BOOTSTRAP};

  EXPECT_CALL(*eventHandler, onCycle(::testing::_, ::testing::_, ::testing::_))
      .WillRepeatedly(::testing::Invoke([&](int64_t, auto, auto) {
        // Pump requests in both BOOTSTRAP and CONVERGE phases so that
        // getLimitUsage() returns non-zero (avoiding the early return).
        if (phase == Phase::BOOTSTRAP || phase == Phase::CONVERGE) {
          for (int i = 0; i < 100; ++i) {
            controller.requestStarted();
          }
          if (phase == Phase::BOOTSTRAP && controller.getStableEstimate() > 0) {
            estimateReady.post();
          }
        }
      }));
  EXPECT_CALL(*eventHandler, limitIncreased())
      .WillRepeatedly(::testing::Invoke([&]() {
        if (phase == Phase::CONVERGE) {
          converging.post();
        }
      }));
  EXPECT_CALL(*eventHandler, limitDecreased())
      .WillRepeatedly(::testing::Invoke([&]() {
        if (phase == Phase::OVERLOAD) {
          overloaded.post();
        }
      }));

  controller.setEventHandler(eventHandler);

  // Phase 1: Wait for stable estimate to be collected
  ASSERT_TRUE(estimateReady.try_wait_for(5s));
  auto stableEst = controller.getStableEstimate();
  EXPECT_GT(stableEst, 0);

  // Phase 2: Trigger overload to push limit below stable estimate
  phase = Phase::OVERLOAD;
  load = 99;
  ASSERT_TRUE(overloaded.try_wait_for(2s));

  // Phase 3: Back to underload, should converge back toward stable estimate
  phase = Phase::CONVERGE;
  load = 50;
  // Wait for refractory period to expire + convergence increase
  ASSERT_TRUE(converging.try_wait_for(2s));

  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::DISABLED});
}

// ---------------------------------------------------------------------------
// EMA CPU load smoothing tests
// ---------------------------------------------------------------------------

TEST_F(CPUConcurrencyControllerTest, emaSmoothingDampensTransientSpikes) {
  // Verifies that with a low cpuLoadSmoothingCoeff, a single CPU spike
  // above cpuTarget does NOT trigger an immediate limit decrease, because
  // the EMA-smoothed load stays below the target.
  using namespace apache::thrift;
  server::test::MockServerConfigs serverConfigs{};
  ThriftServerConfig thriftServerConfig{};
  folly::observer::SimpleObservable<CPUConcurrencyController::Config>
      configObservable;

  std::atomic<int64_t> load{50};
  CPUConcurrencyController::LoadFunc loadFunc =
      [&load](std::chrono::milliseconds, CPULoadSource) { return load.load(); };

  // Use heavy smoothing (alpha=0.1) so transient spikes are dampened.
  // collectionSampleSize=0 skips bootstrap, so we start with the upper bound.
  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::ENABLED,
          .method = CPUConcurrencyController::Method::MAX_REQUESTS,
          .cpuTarget = 90,
          .refreshPeriodMs = 50ms,
          .collectionSampleSize = 0,
          .concurrencyUpperBound = 100,
          .concurrencyLowerBound = 1,
          .cpuLoadSmoothingCoeff = 0.1,
      });

  CPUConcurrencyController controller{
      configObservable.getObserver(),
      serverConfigs,
      thriftServerConfig,
      loadFunc};

  auto eventHandler = std::make_shared<::testing::NiceMock<MockEventHandler>>();

  // We'll count cycles where the limit was decreased.
  std::atomic<int> decreaseCount{0};

  EXPECT_CALL(*eventHandler, onCycle(::testing::_, ::testing::_, ::testing::_))
      .WillRepeatedly(::testing::Return());
  EXPECT_CALL(*eventHandler, limitDecreased())
      .WillRepeatedly(::testing::Invoke([&]() { decreaseCount.fetch_add(1); }));
  EXPECT_CALL(*eventHandler, limitIncreased())
      .WillRepeatedly(::testing::Return());

  controller.setEventHandler(eventHandler);

  // Simulate a transient spike: raw load jumps from 50 to 100 for one cycle,
  // then back to 50. With alpha=0.1, the EMA is seeded at 0 and ramps up:
  //   cycle 0: smoothed = 0 (seeded at zero)
  //   cycle 1: smoothed = 0.1*50 + 0.9*0 = 5
  //   ...after ~1000ms (~20 cycles): smoothed ≈ 43.9
  //   spike cycle: smoothed = 0.1*100 + 0.9*43.9 ≈ 18.6 (well below target 90)
  // So the limit should never be decreased.

  // Wait for a few cycles at load=50 to let the EMA ramp up.
  /* sleep override */
  std::this_thread::sleep_for(1000ms);
  // Spike for one cycle.
  load = 100;
  /* sleep override */
  std::this_thread::sleep_for(60ms);
  // Back to normal.
  load = 50;

  // The smoothed load should never have reached 90, so no decrease expected.
  EXPECT_EQ(decreaseCount.load(), 0);

  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::DISABLED});
}

TEST_F(CPUConcurrencyControllerTest, emaSeedAtZeroAvoidsSpikeOnFirstSample) {
  // Verifies that when the very first CPU sample is a spike (e.g., 100),
  // the EMA seed of 0 prevents false load-shedding. Previously the EMA
  // was seeded with rawLoad, so a first-sample spike would immediately
  // put smoothedLoad at 100 (>= cpuTarget 90) and trigger shedding.
  using namespace apache::thrift;
  server::test::MockServerConfigs serverConfigs{};
  ThriftServerConfig thriftServerConfig{};
  folly::observer::SimpleObservable<CPUConcurrencyController::Config>
      configObservable;

  // Start with a spike: the very first sample the controller sees is 100.
  // After one cycle it drops to normal (50).
  std::atomic<int64_t> load{100};
  CPUConcurrencyController::LoadFunc loadFunc =
      [&load](std::chrono::milliseconds, CPULoadSource) { return load.load(); };

  // Use heavy smoothing (alpha=0.1) with a low target (90).
  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::ENABLED,
          .method = CPUConcurrencyController::Method::MAX_REQUESTS,
          .cpuTarget = 90,
          .refreshPeriodMs = 50ms,
          .collectionSampleSize = 0,
          .concurrencyUpperBound = 100,
          .concurrencyLowerBound = 1,
          .cpuLoadSmoothingCoeff = 0.1,
      });

  CPUConcurrencyController controller{
      configObservable.getObserver(),
      serverConfigs,
      thriftServerConfig,
      loadFunc};

  auto eventHandler = std::make_shared<::testing::NiceMock<MockEventHandler>>();

  std::atomic<int> decreaseCount{0};
  std::atomic<int> cycleCount{0};
  folly::Baton<> enoughCycles;

  EXPECT_CALL(*eventHandler, onCycle(::testing::_, ::testing::_, ::testing::_))
      .WillRepeatedly(::testing::Invoke([&](auto, auto, auto) {
        if (++cycleCount == 5) {
          enoughCycles.post();
        }
      }));
  EXPECT_CALL(*eventHandler, limitDecreased())
      .WillRepeatedly(::testing::Invoke([&]() { decreaseCount.fetch_add(1); }));
  EXPECT_CALL(*eventHandler, limitIncreased())
      .WillRepeatedly(::testing::Return());

  controller.setEventHandler(eventHandler);

  // With the EMA seeded at 0:
  //   cycle 0: smoothed = 0 (seeded), load=0 → early return (no-op)
  //   cycle 1: smoothed = 0.1*100 + 0.9*0 = 10 (below target 90)
  // Then drop to normal load:
  /* sleep override */
  std::this_thread::sleep_for(80ms);
  load = 50;
  //   cycle 2: smoothed = 0.1*50 + 0.9*10 = 14 (below target 90)
  //   ...continues ramping up gradually, never reaching 90
  // No limit decrease should ever fire.

  ASSERT_TRUE(enoughCycles.try_wait_for(2s));

  EXPECT_EQ(decreaseCount.load(), 0);

  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::DISABLED});
}

TEST_F(CPUConcurrencyControllerTest, emaSmoothingDefaultPassesThrough) {
  // Verifies that with cpuLoadSmoothingCoeff=1.0 (default), CPU spikes
  // are NOT dampened — the raw load is used directly. A spike above
  // cpuTarget should trigger a limit decrease.
  using namespace apache::thrift;
  server::test::MockServerConfigs serverConfigs{};
  ThriftServerConfig thriftServerConfig{};
  folly::observer::SimpleObservable<CPUConcurrencyController::Config>
      configObservable;

  std::atomic<int64_t> load{50};
  CPUConcurrencyController::LoadFunc loadFunc =
      [&load](std::chrono::milliseconds, CPULoadSource) { return load.load(); };

  // Default smoothing (alpha=1.0) — raw load pass-through.
  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::ENABLED,
          .method = CPUConcurrencyController::Method::MAX_REQUESTS,
          .cpuTarget = 90,
          .refreshPeriodMs = 50ms,
          .collectionSampleSize = 0,
          .concurrencyUpperBound = 100,
          .concurrencyLowerBound = 1,
          .cpuLoadSmoothingCoeff = 1.0,
      });

  CPUConcurrencyController controller{
      configObservable.getObserver(),
      serverConfigs,
      thriftServerConfig,
      loadFunc};

  auto eventHandler = std::make_shared<::testing::NiceMock<MockEventHandler>>();
  folly::Baton<> decreased;

  EXPECT_CALL(*eventHandler, onCycle(::testing::_, ::testing::_, ::testing::_))
      .WillRepeatedly(::testing::Return());
  EXPECT_CALL(*eventHandler, limitDecreased())
      .WillOnce(::testing::Invoke([&]() { decreased.post(); }));
  EXPECT_CALL(*eventHandler, limitIncreased())
      .WillRepeatedly(::testing::Return());

  controller.setEventHandler(eventHandler);

  // Let the controller stabilize at load=50.
  /* sleep override */
  std::this_thread::sleep_for(100ms);
  // Spike above target — with alpha=1.0, this should trigger immediate
  // decrease.
  load = 99;

  ASSERT_TRUE(decreased.try_wait_for(2s));

  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::DISABLED});
}

// ---------------------------------------------------------------------------
// Dynamic config change tests
// ---------------------------------------------------------------------------

TEST_F(
    CPUConcurrencyControllerTest, configChangeResetsControllerAndReSchedules) {
  using namespace apache::thrift;
  server::test::MockServerConfigs serverConfigs{};
  ThriftServerConfig thriftServerConfig{};
  folly::observer::SimpleObservable<CPUConcurrencyController::Config>
      configObservable;

  CPUConcurrencyController::LoadFunc loadFunc =
      [](std::chrono::milliseconds, CPULoadSource) { return int64_t{50}; };

  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::ENABLED,
          .method = CPUConcurrencyController::Method::MAX_QPS,
          .cpuTarget = 90,
          .collectionSampleSize = 0,
          .concurrencyUpperBound = 100,
      });

  CPUConcurrencyController controller{
      configObservable.getObserver(),
      serverConfigs,
      thriftServerConfig,
      loadFunc};

  auto eventHandler = std::make_shared<::testing::NiceMock<MockEventHandler>>();
  folly::Baton<> firstConfig;
  folly::Baton<> secondConfig;
  std::atomic<int> configUpdateCount{0};

  EXPECT_CALL(*eventHandler, configUpdated(::testing::_))
      .WillRepeatedly(::testing::Invoke([&](auto config) {
        auto count = ++configUpdateCount;
        if (count == 1) {
          firstConfig.post();
        }
        // count == 2 is the initial setEventHandler call re-delivering,
        // count == 3 is the actual config change
        if (count >= 2 && config->cpuTarget == 80) {
          secondConfig.post();
        }
      }));
  EXPECT_CALL(*eventHandler, onCycle(::testing::_, ::testing::_, ::testing::_))
      .WillRepeatedly(::testing::Return());
  EXPECT_CALL(*eventHandler, limitIncreased())
      .WillRepeatedly(::testing::Return());
  EXPECT_CALL(*eventHandler, limitDecreased())
      .WillRepeatedly(::testing::Return());

  controller.setEventHandler(eventHandler);
  ASSERT_TRUE(firstConfig.try_wait_for(1s));

  // Change config -- should trigger configUpdated on the event handler
  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::ENABLED,
          .method = CPUConcurrencyController::Method::MAX_QPS,
          .cpuTarget = 80,
          .collectionSampleSize = 0,
          .concurrencyUpperBound = 200,
      });

  ASSERT_TRUE(secondConfig.try_wait_for(2s));

  configObservable.setValue(
      CPUConcurrencyController::Config{
          .mode = CPUConcurrencyController::Mode::DISABLED});
}
