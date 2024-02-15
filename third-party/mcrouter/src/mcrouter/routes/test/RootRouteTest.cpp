/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <vector>

#include <folly/Range.h>
#include <gtest/gtest.h>

#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/lib/network/gen/MemcacheRouterInfo.h"
#include "mcrouter/routes/test/RouteHandleTestUtil.h"

namespace facebook::memcache::mcrouter {

class RootRouteTest : public ::testing::Test {
  using PrefixSelector = PrefixSelectorRoute<MemcacheRouteHandleIf>;

 public:
  RootRouteTest() {
    srHandleVec1.push_back(std::make_shared<TestHandle>(
        GetRouteTestData(carbon::Result::FOUND, "a")));
    srHandleVec2.push_back(std::make_shared<TestHandle>(
        GetRouteTestData(carbon::Result::FOUND, "b")));
    srHandleVec3.push_back(std::make_shared<TestHandle>(
        GetRouteTestData(carbon::Result::FOUND, "c")));
    mockSrHandle1 = get_route_handles(srHandleVec1)[0];
    mockSrHandle2 = get_route_handles(srHandleVec2)[0];
    mockSrHandle3 = get_route_handles(srHandleVec3)[0];
    srHandleVecs.push_back(srHandleVec1[0]);
    srHandleVecs.push_back(srHandleVec2[0]);
    srHandleVecs.push_back(srHandleVec3[0]);

    PrefixSelector selector1;
    selector1.wildcard = mockSrHandle1;
    auto ptr1 = std::make_shared<PrefixSelector>(selector1);
    PrefixSelector selector2;
    selector2.wildcard = mockSrHandle2;
    auto ptr2 = std::make_shared<PrefixSelector>(selector2);
    PrefixSelector selector3;
    selector3.wildcard = mockSrHandle3;
    auto ptr3 = std::make_shared<PrefixSelector>(selector3);

    routeSelectors[prefix1] = ptr1;
    routeSelectors[prefix2] = ptr2;
    routeSelectors[prefix3] = ptr3;
  }

  auto& getRouteSelectors() {
    return routeSelectors;
  }

  std::vector<std::shared_ptr<MemcacheRouteHandleIf>> getMockSrHandles() {
    return {mockSrHandle1, mockSrHandle2, mockSrHandle3};
  }

  std::shared_ptr<TestHandle>& getTestHandle(size_t index) {
    return srHandleVecs[index];
  }

 private:
  std::string prefix1 = "/././";
  std::string prefix2 = "/virginia/c/";
  std::string prefix3 = "/georgia/d/";
  std::vector<std::shared_ptr<TestHandle>> srHandleVec1;
  std::vector<std::shared_ptr<TestHandle>> srHandleVec2;
  std::vector<std::shared_ptr<TestHandle>> srHandleVec3;
  std::vector<std::shared_ptr<TestHandle>> srHandleVecs;
  std::shared_ptr<MemcacheRouteHandleIf> mockSrHandle1;
  std::shared_ptr<MemcacheRouteHandleIf> mockSrHandle2;
  std::shared_ptr<MemcacheRouteHandleIf> mockSrHandle3;
  RouteSelectorMap<typename MemcacheRouterInfo::RouteHandleIf> routeSelectors;
};

constexpr RootRouteRolloutOpts defaultOpts;

TEST_F(RootRouteTest, NoRoutingPrefixGetRoutesToDefault) {
  mockFiberContext();
  auto proxy = &fiber_local<MemcacheRouterInfo>::getSharedCtx()->proxy();
  RootRoute<MemcacheRouterInfo> rr{*proxy, getRouteSelectors(), defaultOpts};
  auto reply = rr.route(McGetRequest("getReq"));
  EXPECT_FALSE(getTestHandle(0)->saw_keys.empty());
  EXPECT_TRUE(getTestHandle(1)->saw_keys.empty());
  EXPECT_TRUE(getTestHandle(2)->saw_keys.empty());

  EXPECT_EQ(getTestHandle(0)->saw_keys.size(), 1);

  EXPECT_EQ(getTestHandle(0)->saw_keys[0], "getReq");

  EXPECT_TRUE(getTestHandle(0)->distributionRegionInFiber.empty());
  EXPECT_TRUE(getTestHandle(1)->distributionRegionInFiber.empty());
  EXPECT_TRUE(getTestHandle(2)->distributionRegionInFiber.empty());
}

TEST_F(RootRouteTest, BroadcastPrefixGetRoutesToAll) {
  mockFiberContext();
  auto proxy = &fiber_local<MemcacheRouterInfo>::getSharedCtx()->proxy();
  RootRoute<MemcacheRouterInfo> rr{*proxy, getRouteSelectors(), defaultOpts};
  TestFiberManager<MemcacheRouterInfo> fm;
  fm.runAll({[&]() { rr.route(McGetRequest("/*/*/getReq")); }});
  EXPECT_FALSE(getTestHandle(0)->saw_keys.empty());
  EXPECT_FALSE(getTestHandle(1)->saw_keys.empty());
  EXPECT_FALSE(getTestHandle(2)->saw_keys.empty());

  EXPECT_EQ(getTestHandle(0)->saw_keys.size(), 1);
  EXPECT_EQ(getTestHandle(1)->saw_keys.size(), 1);
  EXPECT_EQ(getTestHandle(2)->saw_keys.size(), 1);

  EXPECT_EQ(getTestHandle(0)->saw_keys[0], "/*/*/getReq");
  EXPECT_EQ(getTestHandle(1)->saw_keys[0], "/*/*/getReq");
  EXPECT_EQ(getTestHandle(2)->saw_keys[0], "/*/*/getReq");

  EXPECT_TRUE(getTestHandle(0)->distributionRegionInFiber.empty());
  EXPECT_TRUE(getTestHandle(1)->distributionRegionInFiber.empty());
  EXPECT_TRUE(getTestHandle(2)->distributionRegionInFiber.empty());
}

TEST_F(RootRouteTest, DirectedCrossRegionPrefixGetRoutesToOne) {
  mockFiberContext();
  auto proxy = &fiber_local<MemcacheRouterInfo>::getSharedCtx()->proxy();
  RootRoute<MemcacheRouterInfo> rr{*proxy, getRouteSelectors(), defaultOpts};
  TestFiberManager<MemcacheRouterInfo> fm;
  fm.runAll({[&]() { rr.route(McGetRequest("/virginia/c/getReq")); }});
  EXPECT_TRUE(getTestHandle(0)->saw_keys.empty());
  EXPECT_FALSE(getTestHandle(1)->saw_keys.empty());
  EXPECT_TRUE(getTestHandle(2)->saw_keys.empty());

  EXPECT_EQ(getTestHandle(1)->saw_keys.size(), 1);

  EXPECT_EQ(getTestHandle(1)->saw_keys[0], "/virginia/c/getReq");

  EXPECT_TRUE(getTestHandle(0)->distributionRegionInFiber.empty());
  EXPECT_TRUE(getTestHandle(1)->distributionRegionInFiber.empty());
  EXPECT_TRUE(getTestHandle(2)->distributionRegionInFiber.empty());
}

TEST_F(RootRouteTest, NoRoutingPrefixSetRoutesToDefault) {
  mockFiberContext();
  auto proxy = &fiber_local<MemcacheRouterInfo>::getSharedCtx()->proxy();
  RootRoute<MemcacheRouterInfo> rr{*proxy, getRouteSelectors(), defaultOpts};
  auto reply = rr.route(McSetRequest("setReq"));
  EXPECT_FALSE(getTestHandle(0)->saw_keys.empty());
  EXPECT_TRUE(getTestHandle(1)->saw_keys.empty());
  EXPECT_TRUE(getTestHandle(2)->saw_keys.empty());

  EXPECT_EQ(getTestHandle(0)->saw_keys.size(), 1);

  EXPECT_EQ(getTestHandle(0)->saw_keys[0], "setReq");

  EXPECT_TRUE(getTestHandle(0)->distributionRegionInFiber.empty());
  EXPECT_TRUE(getTestHandle(1)->distributionRegionInFiber.empty());
  EXPECT_TRUE(getTestHandle(2)->distributionRegionInFiber.empty());
}

TEST_F(RootRouteTest, BroadcastPrefixSetRoutesToAll) {
  mockFiberContext();
  auto proxy = &fiber_local<MemcacheRouterInfo>::getSharedCtx()->proxy();
  RootRoute<MemcacheRouterInfo> rr{*proxy, getRouteSelectors(), defaultOpts};
  TestFiberManager<MemcacheRouterInfo> fm;
  fm.runAll({[&]() { rr.route(McSetRequest("/*/*/setReq")); }});
  EXPECT_FALSE(getTestHandle(0)->saw_keys.empty());
  EXPECT_FALSE(getTestHandle(1)->saw_keys.empty());
  EXPECT_FALSE(getTestHandle(2)->saw_keys.empty());

  EXPECT_EQ(getTestHandle(0)->saw_keys.size(), 1);
  EXPECT_EQ(getTestHandle(1)->saw_keys.size(), 1);
  EXPECT_EQ(getTestHandle(2)->saw_keys.size(), 1);

  EXPECT_EQ(getTestHandle(0)->saw_keys[0], "/*/*/setReq");
  EXPECT_EQ(getTestHandle(1)->saw_keys[0], "/*/*/setReq");
  EXPECT_EQ(getTestHandle(2)->saw_keys[0], "/*/*/setReq");

  EXPECT_TRUE(getTestHandle(0)->distributionRegionInFiber.empty());
  EXPECT_TRUE(getTestHandle(1)->distributionRegionInFiber.empty());
  EXPECT_TRUE(getTestHandle(2)->distributionRegionInFiber.empty());
}

TEST_F(RootRouteTest, DirectedCrossRegionPrefixSetRoutesToOne) {
  mockFiberContext();
  auto proxy = &fiber_local<MemcacheRouterInfo>::getSharedCtx()->proxy();
  RootRoute<MemcacheRouterInfo> rr{*proxy, getRouteSelectors(), defaultOpts};
  TestFiberManager<MemcacheRouterInfo> fm;
  fm.runAll({[&]() { rr.route(McSetRequest("/virginia/c/setReq")); }});
  EXPECT_TRUE(getTestHandle(0)->saw_keys.empty());
  EXPECT_FALSE(getTestHandle(1)->saw_keys.empty());
  EXPECT_TRUE(getTestHandle(2)->saw_keys.empty());

  EXPECT_EQ(getTestHandle(1)->saw_keys.size(), 1);

  EXPECT_EQ(getTestHandle(1)->saw_keys[0], "/virginia/c/setReq");

  EXPECT_TRUE(getTestHandle(0)->distributionRegionInFiber.empty());
  EXPECT_TRUE(getTestHandle(1)->distributionRegionInFiber.empty());
  EXPECT_TRUE(getTestHandle(2)->distributionRegionInFiber.empty());
}

TEST_F(RootRouteTest, NoRoutingPrefixDeleteRoutesToDefault) {
  mockFiberContext();
  auto proxy = &fiber_local<MemcacheRouterInfo>::getSharedCtx()->proxy();
  RootRoute<MemcacheRouterInfo> rr{*proxy, getRouteSelectors(), defaultOpts};
  auto reply = rr.route(McDeleteRequest("delReq"));
  EXPECT_FALSE(getTestHandle(0)->saw_keys.empty());
  EXPECT_TRUE(getTestHandle(1)->saw_keys.empty());
  EXPECT_TRUE(getTestHandle(2)->saw_keys.empty());

  EXPECT_EQ(getTestHandle(0)->saw_keys.size(), 1);

  EXPECT_EQ(getTestHandle(0)->saw_keys[0], "delReq");

  EXPECT_TRUE(getTestHandle(0)->distributionRegionInFiber.empty());
  EXPECT_TRUE(getTestHandle(1)->distributionRegionInFiber.empty());
  EXPECT_TRUE(getTestHandle(2)->distributionRegionInFiber.empty());
}

TEST_F(RootRouteTest, BroadcastPrefixDeleteRoutesToAll) {
  mockFiberContext();
  auto proxy = &fiber_local<MemcacheRouterInfo>::getSharedCtx()->proxy();
  RootRoute<MemcacheRouterInfo> rr{*proxy, getRouteSelectors(), defaultOpts};
  TestFiberManager<MemcacheRouterInfo> fm;
  fm.runAll({[&]() { rr.route(McDeleteRequest("/*/*/delReq")); }});
  EXPECT_FALSE(getTestHandle(0)->saw_keys.empty());
  EXPECT_FALSE(getTestHandle(1)->saw_keys.empty());
  EXPECT_FALSE(getTestHandle(2)->saw_keys.empty());

  EXPECT_EQ(getTestHandle(0)->saw_keys.size(), 1);
  EXPECT_EQ(getTestHandle(1)->saw_keys.size(), 1);
  EXPECT_EQ(getTestHandle(2)->saw_keys.size(), 1);

  EXPECT_EQ(getTestHandle(0)->saw_keys[0], "/*/*/delReq");
  EXPECT_EQ(getTestHandle(1)->saw_keys[0], "/*/*/delReq");
  EXPECT_EQ(getTestHandle(2)->saw_keys[0], "/*/*/delReq");

  EXPECT_TRUE(getTestHandle(0)->distributionRegionInFiber.empty());
  EXPECT_TRUE(getTestHandle(1)->distributionRegionInFiber.empty());
  EXPECT_TRUE(getTestHandle(2)->distributionRegionInFiber.empty());
}

TEST_F(RootRouteTest, DirectedCrossRegionPrefixDeleteRoutesToOne) {
  mockFiberContext();
  auto proxy = &fiber_local<MemcacheRouterInfo>::getSharedCtx()->proxy();
  RootRoute<MemcacheRouterInfo> rr{*proxy, getRouteSelectors(), defaultOpts};
  TestFiberManager<MemcacheRouterInfo> fm;
  fm.runAll({[&]() { rr.route(McDeleteRequest("/virginia/c/delReq")); }});
  EXPECT_TRUE(getTestHandle(0)->saw_keys.empty());
  EXPECT_FALSE(getTestHandle(1)->saw_keys.empty());
  EXPECT_TRUE(getTestHandle(2)->saw_keys.empty());

  EXPECT_EQ(getTestHandle(1)->saw_keys.size(), 1);

  EXPECT_EQ(getTestHandle(1)->saw_keys[0], "/virginia/c/delReq");

  EXPECT_TRUE(getTestHandle(0)->distributionRegionInFiber.empty());
  EXPECT_TRUE(getTestHandle(1)->distributionRegionInFiber.empty());
  EXPECT_TRUE(getTestHandle(2)->distributionRegionInFiber.empty());
}

TEST_F(RootRouteTest, NoRoutingPrefixDeleteWithDistributionOnRoutesToDefault) {
  mockFiberContext();
  auto proxy = &fiber_local<MemcacheRouterInfo>::getSharedCtx()->proxy();
  RootRoute<MemcacheRouterInfo> rr{
      *proxy,
      getRouteSelectors(),
      RootRouteRolloutOpts{
          .enableDeleteDistribution = true,
          .enableCrossRegionDeleteRpc = true,
      }};
  auto reply = rr.route(McDeleteRequest("delReq"));
  EXPECT_FALSE(getTestHandle(0)->saw_keys.empty());
  EXPECT_TRUE(getTestHandle(1)->saw_keys.empty());
  EXPECT_TRUE(getTestHandle(2)->saw_keys.empty());

  EXPECT_EQ(getTestHandle(0)->saw_keys.size(), 1);

  EXPECT_EQ(getTestHandle(0)->saw_keys[0], "delReq");

  EXPECT_TRUE(getTestHandle(0)->distributionRegionInFiber.empty());
  EXPECT_TRUE(getTestHandle(1)->distributionRegionInFiber.empty());
  EXPECT_TRUE(getTestHandle(2)->distributionRegionInFiber.empty());
}

TEST_F(RootRouteTest, BroadcastPrefixDeleteWithDistributionOnRoutesToAll) {
  mockFiberContext();
  auto proxy = &fiber_local<MemcacheRouterInfo>::getSharedCtx()->proxy();
  RootRoute<MemcacheRouterInfo> rr{
      *proxy,
      getRouteSelectors(),
      RootRouteRolloutOpts{
          .enableDeleteDistribution = true,
          .enableCrossRegionDeleteRpc = true,
      }};
  TestFiberManager<MemcacheRouterInfo> fm;
  fm.runAll({[&]() { rr.route(McDeleteRequest("/*/*/delReq")); }});
  EXPECT_FALSE(getTestHandle(0)->saw_keys.empty());
  EXPECT_FALSE(getTestHandle(1)->saw_keys.empty());
  EXPECT_FALSE(getTestHandle(2)->saw_keys.empty());

  EXPECT_EQ(getTestHandle(0)->saw_keys.size(), 1);
  EXPECT_EQ(getTestHandle(1)->saw_keys.size(), 1);
  EXPECT_EQ(getTestHandle(2)->saw_keys.size(), 1);

  EXPECT_EQ(getTestHandle(0)->saw_keys[0], "/*/*/delReq");
  EXPECT_EQ(getTestHandle(1)->saw_keys[0], "/*/*/delReq");
  EXPECT_EQ(getTestHandle(2)->saw_keys[0], "/*/*/delReq");

  EXPECT_EQ(getTestHandle(0)->distributionRegionInFiber.size(), 1);
  EXPECT_EQ(getTestHandle(0)->distributionRegionInFiber[0], "");
  EXPECT_TRUE(getTestHandle(1)->distributionRegionInFiber.empty());
  EXPECT_TRUE(getTestHandle(2)->distributionRegionInFiber.empty());
}

TEST_F(
    RootRouteTest,
    DirectedCrossRegionPrefixDeleteWithDistributionOnRoutesToTwo) {
  mockFiberContext();
  auto proxy = &fiber_local<MemcacheRouterInfo>::getSharedCtx()->proxy();
  RootRoute<MemcacheRouterInfo> rr{
      *proxy,
      getRouteSelectors(),
      RootRouteRolloutOpts{
          .enableDeleteDistribution = true,
          .enableCrossRegionDeleteRpc = true,
      }};
  TestFiberManager<MemcacheRouterInfo> fm;
  fm.runAll({[&]() { rr.route(McDeleteRequest("/virginia/c/delReq")); }});
  EXPECT_FALSE(getTestHandle(0)->saw_keys.empty());
  EXPECT_FALSE(getTestHandle(1)->saw_keys.empty());
  EXPECT_TRUE(getTestHandle(2)->saw_keys.empty());

  EXPECT_EQ(getTestHandle(0)->saw_keys.size(), 1);
  EXPECT_EQ(getTestHandle(1)->saw_keys.size(), 1);

  EXPECT_EQ(getTestHandle(0)->saw_keys[0], "/virginia/c/delReq");
  EXPECT_EQ(getTestHandle(1)->saw_keys[0], "/virginia/c/delReq");
  EXPECT_EQ(getTestHandle(0)->distributionRegionInFiber.size(), 1);
  EXPECT_EQ(getTestHandle(0)->distributionRegionInFiber[0], "virginia");
  EXPECT_TRUE(getTestHandle(1)->distributionRegionInFiber.empty());
  EXPECT_TRUE(getTestHandle(2)->distributionRegionInFiber.empty());
}

TEST_F(
    RootRouteTest,
    BroadcastPrefixDeleteWithDistributionOnRcpOffRoutesToLocal) {
  mockFiberContext();
  auto proxy = &fiber_local<MemcacheRouterInfo>::getSharedCtx()->proxy();
  RootRoute<MemcacheRouterInfo> rr{
      *proxy,
      getRouteSelectors(),
      RootRouteRolloutOpts{
          .enableDeleteDistribution = true,
          .enableCrossRegionDeleteRpc = false,
      }};
  TestFiberManager<MemcacheRouterInfo> fm;
  fm.runAll({[&]() { rr.route(McDeleteRequest("/*/*/delReq")); }});
  EXPECT_FALSE(getTestHandle(0)->saw_keys.empty());
  EXPECT_TRUE(getTestHandle(1)->saw_keys.empty());
  EXPECT_TRUE(getTestHandle(2)->saw_keys.empty());

  EXPECT_EQ(getTestHandle(0)->saw_keys.size(), 1);

  EXPECT_EQ(getTestHandle(0)->saw_keys[0], "/*/*/delReq");
  EXPECT_EQ(getTestHandle(0)->distributionRegionInFiber.size(), 1);
  EXPECT_EQ(getTestHandle(0)->distributionRegionInFiber[0], "");
  EXPECT_TRUE(getTestHandle(1)->distributionRegionInFiber.empty());
  EXPECT_TRUE(getTestHandle(2)->distributionRegionInFiber.empty());
}

TEST_F(
    RootRouteTest,
    DirectedCrossRegionPrefixDeleteWithDistributionOnRpcOffRoutesToLocal) {
  mockFiberContext();
  auto proxy = &fiber_local<MemcacheRouterInfo>::getSharedCtx()->proxy();
  RootRoute<MemcacheRouterInfo> rr{
      *proxy,
      getRouteSelectors(),
      RootRouteRolloutOpts{
          .enableDeleteDistribution = true,
          .enableCrossRegionDeleteRpc = false,
      }};
  TestFiberManager<MemcacheRouterInfo> fm;
  fm.runAll({[&]() { rr.route(McDeleteRequest("/virginia/c/delReq")); }});
  EXPECT_FALSE(getTestHandle(0)->saw_keys.empty());
  EXPECT_TRUE(getTestHandle(1)->saw_keys.empty());
  EXPECT_TRUE(getTestHandle(2)->saw_keys.empty());

  EXPECT_EQ(getTestHandle(0)->saw_keys.size(), 1);

  EXPECT_EQ(getTestHandle(0)->saw_keys[0], "/virginia/c/delReq");

  EXPECT_EQ(getTestHandle(0)->distributionRegionInFiber.size(), 1);
  EXPECT_EQ(getTestHandle(0)->distributionRegionInFiber[0], "virginia");
  EXPECT_TRUE(getTestHandle(1)->distributionRegionInFiber.empty());
  EXPECT_TRUE(getTestHandle(2)->distributionRegionInFiber.empty());
}

} // namespace facebook::memcache::mcrouter
