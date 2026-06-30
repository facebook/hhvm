/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <unordered_map>

#include <folly/json/dynamic.h>
#include <folly/json/json.h>
#include <gtest/gtest.h>

#include "mcrouter/lib/network/gen/MemcacheRouterInfo.h"
#include "mcrouter/routes/McRouteHandleProvider.h"
#include "mcrouter/routes/test/RouteHandleTestBase.h"

namespace facebook::memcache::mcrouter {

class McRouteHandleProviderTest
    : public RouteHandleTestBase<MemcacheRouterInfo> {
 protected:
  // Builds a PoolRoute from `config` and returns the AccessPoints created for
  // `poolName`, keyed by destination port (one AccessPoint per server).
  std::unordered_map<uint16_t, std::shared_ptr<const AccessPoint>>
  makePoolAccessPoints(folly::StringPiece config, folly::StringPiece poolName) {
    rhProvider_.create(rhFactory_, "PoolRoute", folly::parseJson(config));
    auto accessPoints = rhProvider_.releaseAccessPoints();
    std::unordered_map<uint16_t, std::shared_ptr<const AccessPoint>> byPort;
    auto it = accessPoints.find(poolName.str());
    if (it != accessPoints.end()) {
      for (const auto& ap : it->second) {
        byPort.emplace(ap->getPort(), ap);
      }
    }
    return byPort;
  }
};

TEST_F(McRouteHandleProviderTest, twJobInternedAcrossServers) {
  constexpr folly::StringPiece kConfig = R"(
  {
    "pool": {
      "name": "twjob_pool",
      "servers": ["localhost:5001", "localhost:5002", "localhost:5003"],
      "protocol": "caret",
      "services": {
        "localhost:5001": {"props": {"tw_job": "tsp_usm1/fci/aggregator"}},
        "localhost:5002": {"props": {"tw_job": "tsp_usm1/fci/aggregator"}},
        "localhost:5003": {"props": {"tw_job": "tsp_uss1/fci/aggregator"}}
      }
    }
  }
  )";

  auto byPort = makePoolAccessPoints(kConfig, "twjob_pool");
  ASSERT_EQ(3, byPort.size());

  const auto& ap1 = byPort.at(5001);
  const auto& ap2 = byPort.at(5002);
  const auto& ap3 = byPort.at(5003);

  // Every server carries its tw_job.
  ASSERT_NE(nullptr, ap1->getTwJobPtr());
  ASSERT_NE(nullptr, ap2->getTwJobPtr());
  ASSERT_NE(nullptr, ap3->getTwJobPtr());
  EXPECT_EQ("tsp_usm1/fci/aggregator", *ap1->getTwJobPtr());
  EXPECT_EQ("tsp_uss1/fci/aggregator", *ap3->getTwJobPtr());

  // Servers sharing a tw_job share one interned string instance...
  EXPECT_EQ(ap1->getTwJobPtr().get(), ap2->getTwJobPtr().get());
  // ...while a distinct tw_job is a separate instance.
  EXPECT_NE(ap1->getTwJobPtr().get(), ap3->getTwJobPtr().get());
}

TEST_F(McRouteHandleProviderTest, noServicesLeavesTwJobUnset) {
  constexpr folly::StringPiece kConfig = R"(
  {
    "pool": {
      "name": "no_services_pool",
      "servers": ["localhost:5001"],
      "protocol": "caret"
    }
  }
  )";

  auto byPort = makePoolAccessPoints(kConfig, "no_services_pool");
  ASSERT_EQ(1, byPort.size());
  EXPECT_EQ(nullptr, byPort.at(5001)->getTwJobPtr());
}

} // namespace facebook::memcache::mcrouter
