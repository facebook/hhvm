/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>
#include <proxygen/lib/stats/BaseStats.h>

using namespace ::testing;

namespace proxygen {
class TestLazyQuantileStatWrapper : public BaseStats::LazyQuantileStatWrapper {
 public:
  explicit TestLazyQuantileStatWrapper(
      folly::StringPiece name,
      folly::Range<const facebook::fb303::ExportType*> stats =
          facebook::fb303::ExportTypeConsts::kCountAvg,
      folly::Range<const double*> quantiles =
          facebook::fb303::QuantileConsts::kP95_P99_P999,
      folly::Range<const size_t*> slidingWindowPeriods =
          facebook::fb303::SlidingWindowPeriodConsts::kOneMin)
      : BaseStats::LazyQuantileStatWrapper(
            name, stats, quantiles, slidingWindowPeriods) {
  }
  bool quantileStatCreated() {
    return statWrapper_.get() != nullptr;
  }
  // The metadata needed to create the quantile stat on-demand
  bool metadataExists() {
    return statWrapperInfo_.get() != nullptr;
  }

  size_t getNumSavedCounters() {
    return numCountersSaved_.getTcStatUnsafe()->value();
  }
};

TEST(ProxyStatsWrapperTest, LazyQuantileStatWrapper) {
  TestLazyQuantileStatWrapper wrapper("test");
  EXPECT_TRUE(wrapper.metadataExists());
  EXPECT_FALSE(wrapper.quantileStatCreated());
  wrapper.addValue(10);
  EXPECT_FALSE(wrapper.metadataExists());
  EXPECT_TRUE(wrapper.quantileStatCreated());
  wrapper.addValue(100);
  EXPECT_FALSE(wrapper.metadataExists());
  EXPECT_TRUE(wrapper.quantileStatCreated());
}

TEST(ProxyStatsWrapperTest, MultipleLazyQuantileStatWrapper) {
  TestLazyQuantileStatWrapper wrapper1("test1");
  TestLazyQuantileStatWrapper wrapper2("test2");
  EXPECT_TRUE(wrapper1.metadataExists());
  EXPECT_TRUE(wrapper2.metadataExists());
  EXPECT_FALSE(wrapper1.quantileStatCreated());
  EXPECT_FALSE(wrapper2.quantileStatCreated());
  wrapper1.addValue(10);
  wrapper2.addValue(10);
  EXPECT_FALSE(wrapper1.metadataExists());
  EXPECT_FALSE(wrapper2.metadataExists());
  EXPECT_TRUE(wrapper1.quantileStatCreated());
  EXPECT_TRUE(wrapper2.quantileStatCreated());
}

TEST(ProxyStatsWrapperTest, TestSavingsCounter) {
  TestLazyQuantileStatWrapper wrapper(
      "test1",
      facebook::fb303::ExportTypeConsts::kCountAvg,
      facebook::fb303::QuantileConsts::kP95_P99_P999,
      facebook::fb303::SlidingWindowPeriodConsts::kOneMinTenMin);
  EXPECT_EQ(wrapper.getNumSavedCounters(), 6);
  wrapper.addValue(10);
  // Add a value a second time to make sure it's only decremented on the first
  // counter bump
  wrapper.addValue(10);
  EXPECT_EQ(wrapper.getNumSavedCounters(), 0);
}

TEST(ProxyStatsWrapperTest, LazyQuantileStatWrapperConcurrency) {
  TestLazyQuantileStatWrapper wrapper("test");
  auto bumpStat = [&]() {
    for (int i = 0; i < 100; i++) {
      wrapper.addValue(10);
    }
  };

  std::vector<std::thread> threads;
  threads.reserve(10);
  for (int i = 0; i < 10; i++) {
    threads.emplace_back(bumpStat);
  }
  for (auto& thread : threads) {
    thread.join();
  }
}

} // namespace proxygen
