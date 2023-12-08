/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Conv.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <proxygen/facebook/lib/statistics/ARLResourceStats.h>
#include <proxygen/lib/stats/ResourceData.h>

#include <proxygen/lib/stats/test/MockResources.h>

namespace proxygen {

// MockARLResourceStats used by AdaptiveRateLimiterUnderTest.
// Tests in this class should not actually need to interact with this object.
// Tests should overide what the shed algo returns to customize behavior.
class MockARLResourceStats : public ARLResourceStats {
 public:
  explicit MockARLResourceStats(std::unique_ptr<MockResources> resources)
      : ARLResourceStats(std::move(resources)) {
  }

  MOCK_METHOD(const ARLResourceData&, getCurrentData, (), (const));
  MOCK_METHOD(const ARLResourceData&, getPreviousData, (), (const));

  // Tests in this class should not care what value is returned from
  // getCurrentData() as it is the shed algo that consumes it.  So we
  // only need to make sure its called the number of times we expect.
  void expectGetCurrentDataToBeCalled(int times) {
    if (times > 0) {
      EXPECT_CALL(*this, getCurrentData())
          .Times(times)
          .WillRepeatedly(testing::ReturnRef(dummyARLResourceData_));
    } else {
      EXPECT_CALL(*this, getCurrentData()).Times(times);
    }
  }

  void expectGetPreviousDataToReturnValue(const ARLResourceData& data) {
    EXPECT_CALL(*this, getPreviousData()).WillOnce(testing::ReturnRef(data));
  }

 private:
  // Dummy ARLResourceData that only exists so expectGetCurrentDataToBeCalled()
  // can return a valid reference.
  ARLResourceData dummyARLResourceData_;
};

} // namespace proxygen
