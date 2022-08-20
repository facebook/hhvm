/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/stats/PeriodicStats.h>
#include <proxygen/lib/stats/test/PeriodicStatsTestHelper.h>

#include <chrono>
#include <folly/portability/GTest.h>
#include <folly/synchronization/Baton.h>

using namespace proxygen;

// Test container for use as PeriodicStats template type in tests.
class PeriodicStatsData {
 public:
  std::chrono::milliseconds getLastUpdateTime() const {
    return time_;
  }

  void setLastUpdateTime(
      std::chrono::milliseconds updateTime =
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch())) {
    time_ = updateTime;
  }

 private:
  std::chrono::milliseconds time_{0};
};

// Test instance of PeriodicStats used in subsequent tests.
class PeriodicStatsUnderTest : public PeriodicStats<PeriodicStatsData> {
 public:
  PeriodicStatsUnderTest(PeriodicStatsData* initialData)
      : PeriodicStats<PeriodicStatsData>(initialData) {
  }

  PeriodicStatsData& getMutableData() {
    return *data_.load();
  }

 protected:
  // Subclass implementation.  We could have opted to mock this method but
  // technically this interface could be private and we specically only care
  // about validating and verifying the public interface of this class.
  PeriodicStatsData* getNewData() const override {
    auto* data = new PeriodicStatsData();
    // As this is a test context, we really only need the timestamp to be new
    // and not actually accurate.
    data->setLastUpdateTime(data_.load()->getLastUpdateTime() +
                            std::chrono::milliseconds(1));
    return data;
  }
};

class PeriodicStatsTest : public ::testing::Test {
 public:
  void SetUp() override {
    auto* data = new PeriodicStatsData();
    data->setLastUpdateTime();
    initialUpdateTime_ = data->getLastUpdateTime();

    periodStatsUT_ = std::make_unique<PeriodicStatsUnderTest>(data);
    periodicStatsUTHelper_ =
        std::make_unique<PeriodicStatsTestHelper<PeriodicStatsData>>(
            periodStatsUT_.get());
  }

  void TearDown() override {
    periodStatsUT_->stopRefresh();
  }

 protected:
  std::chrono::milliseconds initialUpdateTime_{0};
  std::unique_ptr<PeriodicStatsUnderTest> periodStatsUT_{nullptr};
  std::unique_ptr<PeriodicStatsTestHelper<PeriodicStatsData>>
      periodicStatsUTHelper_{nullptr};
};

// Tests Below

TEST_F(PeriodicStatsTest, GetCurrentData) {
  const auto data = periodStatsUT_->getCurrentData();
  EXPECT_EQ(data.getLastUpdateTime(), initialUpdateTime_);
}

TEST_F(PeriodicStatsTest, GetCurrentDataRefreshes) {
  // Get the current data and copy it for comparison later.
  // This is required as getCurrentData returns a reference.
  // Also verify that getting it again returns the same data.
  const auto data = periodStatsUT_->getCurrentData();
  auto copiedData = data;
  EXPECT_EQ(copiedData.getLastUpdateTime(),
            periodStatsUT_->getCurrentData().getLastUpdateTime());

  // Now mutate the cached data directly and expect to get this mutated
  // version back, even if the refresh thread has not been started.
  periodStatsUT_->getMutableData().setLastUpdateTime(
      periodStatsUT_->getMutableData().getLastUpdateTime() +
      std::chrono::milliseconds(1));
  const auto refreshedData = periodStatsUT_->getCurrentData();
  auto copiedRefreshedData = refreshedData;
  EXPECT_GT(copiedRefreshedData.getLastUpdateTime(),
            copiedData.getLastUpdateTime());
  EXPECT_EQ(copiedRefreshedData.getLastUpdateTime(),
            periodStatsUT_->getCurrentData().getLastUpdateTime());

  // Now set a refresh period to something longer than the rest of the test
  // duration but with an initial delay of 1 ms so that the data refreshes
  // once but never again for the duration of the test.
  periodicStatsUTHelper_->waitForRefresh({});

  // Now we simply verify that we indeed refreshed once, but that we don't
  // for the remainder of this test.
  const auto newData = periodStatsUT_->getCurrentData();
  auto copiedNewData = newData;
  EXPECT_GT(copiedNewData.getLastUpdateTime(),
            copiedRefreshedData.getLastUpdateTime());
  periodicStatsUTHelper_->waitForRefresh(
      {.startRefresh = false, .waitResult = false});
  EXPECT_EQ(periodStatsUT_->getCurrentData().getLastUpdateTime(),
            copiedNewData.getLastUpdateTime());

  periodStatsUT_->stopRefresh();
}
