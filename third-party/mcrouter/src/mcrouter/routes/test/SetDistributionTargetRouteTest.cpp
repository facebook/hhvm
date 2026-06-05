/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/lib/config/RouteHandleBuilder.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/lib/network/gen/MemcacheRouterInfo.h"
#include "mcrouter/routes/SetDistributionTargetRoute.h"
#include "mcrouter/routes/test/RouteHandleTestUtil.h"

namespace facebook::memcache::mcrouter {

namespace {
// Builds a SetDistributionTargetRoute over a single recording TestHandle.
// TestHandle records, during its own route(), the distribution target region
// observed on the fiber (in `distributionRegionInFiber`) -- which is exactly
// what we use to assert what the handle stamped.
template <class Request>
void runThrough(
    const std::string& targetRegion,
    Request req,
    std::vector<std::shared_ptr<TestHandle>>& handleVec,
    bool stampUpstream = false,
    const std::string& upstreamRegion = "") {
  handleVec = {std::make_shared<TestHandle>(
      GetRouteTestData(carbon::Result::FOUND, "a"))};
  auto child = get_route_handles(handleVec)[0];
  auto rh =
      makeRouteHandleWithInfo<MemcacheRouterInfo, SetDistributionTargetRoute>(
          child, targetRegion);

  mockFiberContext();
  TestFiberManager<MemcacheRouterInfo> fm;
  fm.runAll({[&]() {
    if (stampUpstream) {
      fiber_local<MemcacheRouterInfo>::runWithLocals([&]() {
        fiber_local<MemcacheRouterInfo>::setDistributionTargetRegion(
            upstreamRegion);
        rh->route(req);
      });
    } else {
      rh->route(req);
      // After route() returns, the handle's runWithLocals scope must have
      // restored the fiber-local -- no leak past the call.
      EXPECT_FALSE(
          fiber_local<MemcacheRouterInfo>::getDistributionTargetRegion()
              .has_value());
    }
  }});
}
} // namespace

// (c) Empty routing prefix -> the handle stamps the configured broadcast
// target ("") for the downstream to consume, and the child observes it.
TEST(SetDistributionTargetRouteTest, emptyPrefixSetStampsBroadcast) {
  std::vector<std::shared_ptr<TestHandle>> handleVec;
  runThrough("", McSetRequest("setkey"), handleVec);

  ASSERT_EQ(handleVec[0]->saw_keys.size(), 1);
  EXPECT_EQ(handleVec[0]->saw_keys[0], "setkey");
  ASSERT_EQ(handleVec[0]->distributionRegionInFiber.size(), 1);
  EXPECT_EQ(handleVec[0]->distributionRegionInFiber[0], "");
}

// Stamping is operation-agnostic: an empty-prefix DELETE is stamped too.
TEST(SetDistributionTargetRouteTest, emptyPrefixDeleteStampsBroadcast) {
  std::vector<std::shared_ptr<TestHandle>> handleVec;
  runThrough("", McDeleteRequest("delkey"), handleVec);

  ASSERT_EQ(handleVec[0]->saw_keys.size(), 1);
  EXPECT_EQ(handleVec[0]->saw_keys[0], "delkey");
  ASSERT_EQ(handleVec[0]->distributionRegionInFiber.size(), 1);
  EXPECT_EQ(handleVec[0]->distributionRegionInFiber[0], "");
}

// (c) A non-empty configured target_region with an empty-prefix request stamps
// that specific (directed) region. Not used by the current ads config (which
// always configures ""), but locks down the documented directed mode.
TEST(SetDistributionTargetRouteTest, emptyPrefixDirectedTargetStamps) {
  std::vector<std::shared_ptr<TestHandle>> handleVec;
  runThrough("georgia", McSetRequest("setkey"), handleVec);

  ASSERT_EQ(handleVec[0]->saw_keys.size(), 1);
  ASSERT_EQ(handleVec[0]->distributionRegionInFiber.size(), 1);
  EXPECT_EQ(handleVec[0]->distributionRegionInFiber[0], "georgia");
}

// (b) Directed cross-region prefix with no upstream stamp -> the handle must
// NOT stamp (no converting a directed request into a broadcast). The child is
// still reached, but observes no distribution target.
TEST(SetDistributionTargetRouteTest, directedPrefixNoStampNotBroadcast) {
  std::vector<std::shared_ptr<TestHandle>> handleVec;
  runThrough("", McSetRequest("/georgia/default/setkey"), handleVec);

  ASSERT_EQ(handleVec[0]->saw_keys.size(), 1);
  EXPECT_EQ(handleVec[0]->saw_keys[0], "/georgia/default/setkey");
  EXPECT_TRUE(handleVec[0]->distributionRegionInFiber.empty());
}

// (b) Broadcast prefix (/*/*/) with no upstream stamp -> also left for
// RootRoute to govern; the handle does not (re-)stamp.
TEST(SetDistributionTargetRouteTest, broadcastPrefixNoStampLeftToRoot) {
  std::vector<std::shared_ptr<TestHandle>> handleVec;
  runThrough("", McSetRequest("/*/*/setkey"), handleVec);

  ASSERT_EQ(handleVec[0]->saw_keys.size(), 1);
  EXPECT_EQ(handleVec[0]->saw_keys[0], "/*/*/setkey");
  EXPECT_TRUE(handleVec[0]->distributionRegionInFiber.empty());
}

// (a) A directed prefix WITH an upstream stamp (as RootRoute would set for a
// directed cross-region SET when set-distribution is enabled) -> the handle
// must defer to the existing target, not overwrite it with "".
TEST(SetDistributionTargetRouteTest, alreadyStampedNotOverwritten) {
  std::vector<std::shared_ptr<TestHandle>> handleVec;
  runThrough(
      "",
      McSetRequest("/georgia/default/setkey"),
      handleVec,
      /*stampUpstream=*/true,
      /*upstreamRegion=*/"georgia");

  ASSERT_EQ(handleVec[0]->saw_keys.size(), 1);
  ASSERT_EQ(handleVec[0]->distributionRegionInFiber.size(), 1);
  EXPECT_EQ(handleVec[0]->distributionRegionInFiber[0], "georgia");
}

} // namespace facebook::memcache::mcrouter
