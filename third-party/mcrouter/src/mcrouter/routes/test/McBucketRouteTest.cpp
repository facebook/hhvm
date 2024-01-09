/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/routes/McBucketRoute.h"
#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/lib/network/gen/MemcacheRouterInfo.h"
#include "mcrouter/routes/RoutingUtils.h"
#include "mcrouter/routes/test/RouteHandleTestUtil.h"

#include <vector>

#include <folly/Range.h>
#include <folly/String.h>
#include <folly/dynamic.h>
#include <gtest/gtest.h>
#include "folly/fibers/FiberManagerMap.h"
#include "folly/io/async/EventBase.h"
#include "folly/json.h"

namespace facebook::memcache::mcrouter {

TEST(McBucketRouteTest, bucketIdShouldPropagate) {
  std::vector<std::shared_ptr<TestHandle>> srHandleVec{
      std::make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "a")),
  };
  auto mockSrHandle = get_route_handles(srHandleVec)[0];

  constexpr folly::StringPiece kMcBucketRouteConfig = R"(
  {
    "bucketize": true,
    "total_buckets": 100,
    "bucketization_keyspace": "testRegion:testPool"
  }
  )";

  auto rh = makeMcBucketRoute<MemcacheRouterInfo>(
      mockSrHandle, folly::parseJson(kMcBucketRouteConfig));
  ASSERT_TRUE(rh);
  mockFiberContext();
  rh->route(McGetRequest("getReq")); // bucketId == 28
  auto proxy = &fiber_local<MemcacheRouterInfo>::getSharedCtx()->proxy();
  EXPECT_EQ(1, proxy->stats().getValue(bucketized_routing_stat));
  EXPECT_FALSE(srHandleVec[0]->sawBucketIds.empty());
  EXPECT_EQ(28, srHandleVec[0]->sawBucketIds[0]);
}

TEST(McBucketRouteTest, bucketIdShouldPropagateInTraverse) {
  std::vector<std::shared_ptr<TestHandle>> srHandleVec{
      std::make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "a")),
  };
  auto mockSrHandle = get_route_handles(srHandleVec)[0];

  constexpr folly::StringPiece kMcBucketRouteConfig = R"(
  {
    "bucketize": true,
    "total_buckets": 100,
    "bucketization_keyspace": "testRegion:testPool"
  }
  )";

  auto rh = makeMcBucketRoute<MemcacheRouterInfo>(
      mockSrHandle, folly::parseJson(kMcBucketRouteConfig));
  mockFiberContext();
  RouteHandleTraverser<MemcacheRouterInfo::RouteHandleIf> t{};
  rh->traverse(McGetRequest("getReq"), t); // bucketId == 28
  EXPECT_FALSE(srHandleVec[0]->sawBucketIds.empty());
  EXPECT_EQ(28, srHandleVec[0]->sawBucketIds[0]);
}

TEST(McBucketRouteTest, checkParams) {
  std::vector<std::shared_ptr<TestHandle>> srHandleVec{
      std::make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "a")),
  };
  auto mockSrHandle = get_route_handles(srHandleVec)[0];
  std::string_view total = "100";
  std::string_view keyspace = "testReg:testPool";

  std::string kMcBucketRouteConfig = folly::sformat(
      R"(
  {{
    "bucketize": true,
    "total_buckets": {},
    "bucketization_keyspace": "{}"
  }}
  )",
      total,
      keyspace);

  auto rh = makeMcBucketRoute<MemcacheRouterInfo>(
      mockSrHandle, folly::parseJson(kMcBucketRouteConfig));
  mockFiberContext();
  auto name = rh->routeName();
  std::vector<std::string> params;
  folly::split('|', name, params, true);
  EXPECT_TRUE(params.size() == 4);
  EXPECT_EQ(params[0], "bucketize");
  EXPECT_EQ(params[1], folly::to<std::string>("total_buckets=", total));
  EXPECT_EQ(params[2], folly::to<std::string>("salt="));
  EXPECT_EQ(
      params[3], folly::to<std::string>("bucketization_keyspace=", keyspace));
}

TEST(McBucketRouteTest, recordBucketizationData) {
  std::vector<std::shared_ptr<TestHandle>> srHandleVec{
      std::make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "a")),
  };
  auto mockSrHandle = get_route_handles(srHandleVec)[0];

  constexpr folly::StringPiece kMcBucketRouteConfig = R"(
  {
    "bucketize": true,
    "total_buckets": 100,
    "bucketization_keyspace": "testRegionBucketKeyspace"
  }
  )";

  auto rh = makeMcBucketRoute<MemcacheRouterInfo>(
      mockSrHandle, folly::parseJson(kMcBucketRouteConfig));
  ASSERT_TRUE(rh);
  mockFiberContext();
  auto keyBucketPairs = std::vector<std::pair<std::string, std::string>>();
  setRecordBucketDataContext(keyBucketPairs);

  rh->route(McGetRequest("getReq")); // bucketId == 28
  // the request won't get to SRRoute
  EXPECT_TRUE(srHandleVec[0]->sawBucketIds.empty());
  EXPECT_TRUE(keyBucketPairs.size() == 1);
  EXPECT_EQ("getReq", keyBucketPairs[0].first);
  EXPECT_EQ("28", keyBucketPairs[0].second);
}

TEST(McBucketRouteTest, GetRoutingKey) {
  auto req = McGetRequest("getReq");

  EXPECT_EQ(getRoutingKey(req), "getReq");
  EXPECT_EQ(getRoutingKey(req, "someSalt"), "getReqsomeSalt");
  EXPECT_EQ(getRoutingKey(req, std::string()), "getReq");
}

} // namespace facebook::memcache::mcrouter
