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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <folly/experimental/observer/Observer.h>
#include <folly/experimental/observer/SimpleObservable.h>
#include <folly/synchronization/Baton.h>

#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/cpp2/server/CPUConcurrencyController.h>
#include <thrift/lib/cpp2/server/ThriftServerConfig.h>
#include <thrift/lib/cpp2/server/test/util/MockServerConfigs.h>

using namespace std::chrono_literals;
using apache::thrift::CPUConcurrencyController;

class MockEventHandler : public CPUConcurrencyController::EventHandler {
 public:
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

TEST_F(CPUConcurrencyControllerTest, testEventHandler) {
  using ::testing::Eq;
  using ::testing::Gt;
  folly::Baton<> baton1;
  folly::Baton<> baton2;
  folly::Baton<> baton3;
  auto eventHandler = std::make_shared<MockEventHandler>();

  ::testing::Sequence seq;
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
  setConfig(CPUConcurrencyController::Config{
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
  setConfig(CPUConcurrencyController::Config{
      .mode = CPUConcurrencyController::Mode::DISABLED});
}

TEST_F(CPUConcurrencyControllerTest, getDbgInfo) {
  using namespace apache::thrift;
  server::test::MockServerConfigs serverConfigs{};
  ThriftServerConfig thriftServerConfig{};
  folly::observer::SimpleObservable<CPUConcurrencyController::Config>
      configObservable;

  configObservable.setValue(CPUConcurrencyController::Config{
      .mode = CPUConcurrencyController::Mode::ENABLED,
      .method = CPUConcurrencyController::Method::TOKEN_BUCKET,
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
  });

  CPUConcurrencyController cpuConcurrencyController{
      configObservable.getObserver(), serverConfigs, thriftServerConfig};

  // Act
  auto dbgInfo = cpuConcurrencyController.getDbgInfo();

  // Assert
  ASSERT_EQ("ENABLED", dbgInfo.get_mode());
  ASSERT_EQ(*dbgInfo.method(), "TOKEN_BUCKET");
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
}

TEST_F(CPUConcurrencyControllerTest, testConcurrencyUpperBound) {
  // Test concurrencyUpperBound with positive value
  setConfig(CPUConcurrencyController::Config{
      .mode = CPUConcurrencyController::Mode::ENABLED,
      .method = CPUConcurrencyController::Method::TOKEN_BUCKET,
      .concurrencyUpperBound = 100,
  });

  folly::observer_detail::ObserverManager::waitForAllUpdates();

  EXPECT_EQ(getCPUConcurrencyController().getConcurrencyUpperBound(), 100);
  setConfig(CPUConcurrencyController::Config{
      .mode = CPUConcurrencyController::Mode::DISABLED});

  // Test concurrencyUpperBound with kConcurrencyUpperBoundUseStaticLimit
  // CONCURRENCY_LIMITS method
  getThriftServerConfig().setMaxRequests(
      folly::observer::makeStaticObserver<std::optional<uint32_t>>(200));

  setConfig(CPUConcurrencyController::Config{
      .mode = CPUConcurrencyController::Mode::ENABLED,
      .method = CPUConcurrencyController::Method::CONCURRENCY_LIMITS,
      .concurrencyUpperBound =
          CPUConcurrencyController::Config::UseStaticLimit{},
  });

  folly::observer_detail::ObserverManager::waitForAllUpdates();

  EXPECT_EQ(getCPUConcurrencyController().getConcurrencyUpperBound(), 200);
  setConfig(CPUConcurrencyController::Config{
      .mode = CPUConcurrencyController::Mode::DISABLED});

  // Test concurrencyUpperBound with kConcurrencyUpperBoundUseStaticLimit
  // TOKEN_BUCKET method
  getThriftServerConfig().setMaxQps(
      folly::observer::makeStaticObserver<std::optional<uint32_t>>(300));

  setConfig(CPUConcurrencyController::Config{
      .mode = CPUConcurrencyController::Mode::ENABLED,
      .method = CPUConcurrencyController::Method::TOKEN_BUCKET,
      .concurrencyUpperBound =
          CPUConcurrencyController::Config::UseStaticLimit{},
  });

  folly::observer_detail::ObserverManager::waitForAllUpdates();

  EXPECT_EQ(getCPUConcurrencyController().getConcurrencyUpperBound(), 300);
  setConfig(CPUConcurrencyController::Config{
      .mode = CPUConcurrencyController::Mode::DISABLED});

  // Test concurrencyUpperBound with negative value
  setConfig(CPUConcurrencyController::Config{
      .mode = CPUConcurrencyController::Mode::ENABLED,
      .method = CPUConcurrencyController::Method::TOKEN_BUCKET,
      .concurrencyUpperBound = -2,
  });

  folly::observer_detail::ObserverManager::waitForAllUpdates();

  EXPECT_EQ(getCPUConcurrencyController().getConcurrencyUpperBound(), 0);
}
