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

using namespace proxygen;

/**
 * Test Resources implementation that allows us to control what resource data is
 * returned to ResourceStats
 */
class MockResources : public Resources {
 public:
  MOCK_METHOD(ResourceData, getCurrentData, ());

  void expectNextCallsToReturnValue(const ResourceData& data, uint64_t times) {
    EXPECT_CALL(*this, getCurrentData())
        .Times(times)
        .WillRepeatedly(testing::Return(data));
  }

  void expectAllCallsToReturnValue(const ResourceData& data) {
    EXPECT_CALL(*this, getCurrentData()).WillRepeatedly(testing::Return(data));
  }

  struct GetTestResourceDataParams {
    double cpuUtilRatio{0.5};
    double cpuSoftIrqUtilRatio{0.5};
    uint64_t numCpuCores{1};
    double memUtilRatio{0.5};
    double tcpMemUtilRatio{0.5};
    double udpMemUtilRatio{0.5};
    std::chrono::milliseconds lastUpdateTime{-1};
  };

  // Helper method to get a ResourceData instance formed using the built-in
  // test params.
  static ResourceData getTestResourceData(
      const GetTestResourceDataParams& params) {
    ResourceData data;

    data.setCpuStats(
        params.cpuUtilRatio,
        std::vector<double>(params.numCpuCores, params.cpuUtilRatio),
        80,
        params.cpuSoftIrqUtilRatio,
        std::vector<double>(params.numCpuCores, params.cpuSoftIrqUtilRatio));
    uint64_t totalMemBytes = 100;
    data.setMemStats(
        folly::to<uint64_t>(round(params.memUtilRatio * totalMemBytes)),
        folly::to<uint64_t>(round(params.memUtilRatio * totalMemBytes)),
        totalMemBytes);
    uint64_t totalNetMemBytes = 100;
    data.setTcpMemStats(
        folly::to<uint64_t>(round(params.tcpMemUtilRatio * totalNetMemBytes)),
        25,
        75,
        totalNetMemBytes);
    data.setUdpMemStats(
        folly::to<uint64_t>(round(params.udpMemUtilRatio * totalNetMemBytes)),
        25,
        75,
        totalNetMemBytes);

    if (params.lastUpdateTime < std::chrono::milliseconds(0)) {
      data.refreshLastUpdateTime();
    } else {
      data.setLastUpdateTime(params.lastUpdateTime);
    }

    return data;
  }
};
