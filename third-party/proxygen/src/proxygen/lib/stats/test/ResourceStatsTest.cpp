/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/stats/ResourceStats.h>
#include <proxygen/lib/stats/test/MockResources.h>
#include <proxygen/lib/stats/test/PeriodicStatsTestHelper.h>

#include <folly/portability/GTest.h>

using namespace proxygen;

class ResourceStatsUnderTest : public ResourceStats {
 public:
  explicit ResourceStatsUnderTest(std::unique_ptr<MockResources> resources)
      : ResourceStats(std::move(resources)) {
  }

  // Allows callers the ability to set expectations and return values
  // appropriately.
  MockResources* getMockResources() const {
    return static_cast<MockResources*>(resources_.get());
  }
};

class ResourceStatsTest : public ::testing::Test {
 public:
  void SetUp() override {
    // Setup resourceStatsUT_ for initial use.
    auto mockResources = std::make_unique<testing::StrictMock<MockResources>>();
    mockResources->expectNextCallsToReturnValue(
        MockResources::getTestResourceData(testRDParams), 1);

    resourceStatsUT_ =
        std::make_unique<ResourceStatsUnderTest>(std::move(mockResources));
    periodicStatsUTHelper_ =
        std::make_unique<PeriodicStatsTestHelper<ResourceData>>(
            resourceStatsUT_.get());
  }

  void TearDown() override {
    resourceStatsUT_->stopRefresh();
  }

 protected:
  MockResources::GetTestResourceDataParams testRDParams;
  std::unique_ptr<ResourceStatsUnderTest> resourceStatsUT_;
  std::unique_ptr<PeriodicStatsTestHelper<ResourceData>> periodicStatsUTHelper_{
      nullptr};
};

// Verifies that the initial data instance is correctly copied to a caller.
TEST_F(ResourceStatsTest, Constructs) {
  const auto data = resourceStatsUT_->getCurrentData();

  EXPECT_EQ(data.getCpuRatioUtil(), testRDParams.cpuUtilRatio);

  EXPECT_EQ(data.getSoftIrqCpuCoreRatioUtils().size(),
            testRDParams.numCpuCores);
  for (auto const& cpuSoftIrqCoreRatioUtil :
       data.getSoftIrqCpuCoreRatioUtils()) {
    EXPECT_EQ(cpuSoftIrqCoreRatioUtil, testRDParams.cpuSoftIrqUtilRatio);
    ;
  }

  EXPECT_EQ(data.getUsedMemPct(), testRDParams.memUtilRatio * 100);

  EXPECT_EQ(data.getTcpMemRatio(), testRDParams.tcpMemUtilRatio);

  EXPECT_EQ(data.getUdpMemRatio(), testRDParams.udpMemUtilRatio);
}

// Verify that on refresh that GetCurrentData is called exactly once.
TEST_F(ResourceStatsTest, GetCurrentDataOnRefresh) {
  EXPECT_CALL(*resourceStatsUT_->getMockResources(), getCurrentData());
  periodicStatsUTHelper_->waitForRefresh({});
}
