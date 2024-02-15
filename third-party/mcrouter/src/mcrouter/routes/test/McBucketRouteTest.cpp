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
  EXPECT_EQ(
      params[2], folly::to<std::string>("bucketization_keyspace=", keyspace));
  EXPECT_EQ(params[3], "prefix_map_enabled=false");
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

TEST(McBucketRouteTest, bucketIdWithColocation) {
  std::vector<std::shared_ptr<TestHandle>> srHandleVec{
      std::make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "a")),
  };
  auto mockSrHandle = get_route_handles(srHandleVec)[0];

  constexpr folly::StringPiece kMcBucketRouteConfig = R"(
  {
    "bucketize": true,
    "total_buckets": 1000000,
    "total_buckets_by_prefix": {
      "asdf": 1,
      "qwert": 20,
      "zxcv": 25
    },
    "bucketization_keyspace": "testRegion:testPool"
  }
  )";

  auto rh = makeMcBucketRoute<MemcacheRouterInfo>(
      mockSrHandle, folly::parseJson(kMcBucketRouteConfig));
  ASSERT_TRUE(rh);
  mockFiberContext();
  rh->route(McGetRequest("getReq")); // not colocated, bucketId == 755248
  EXPECT_FALSE(srHandleVec[0]->sawBucketIds.empty());
  EXPECT_EQ(755248, srHandleVec[0]->sawBucketIds[0]);
  rh->route(McGetRequest("asdf:getReq")); // colocated, bucketId == 304678
  EXPECT_EQ(304678, srHandleVec[0]->sawBucketIds[1]);
  rh->route(McGetRequest("asdf:getReq2")); // colocated, bucketId == 304678
  EXPECT_EQ(304678, srHandleVec[0]->sawBucketIds[2]);
  rh->route(McGetRequest("asdf:getReq3")); // colocated, bucketId == 304678
  EXPECT_EQ(304678, srHandleVec[0]->sawBucketIds[3]);
  rh->route(
      McGetRequest("asdfgetReqanything")); // colocated, bucketId == 304678
  EXPECT_EQ(304678, srHandleVec[0]->sawBucketIds[4]);
  rh->route(McGetRequest("asd:getReq")); // not colocated, bucketId == 973109
  EXPECT_EQ(973109, srHandleVec[0]->sawBucketIds[5]);
  rh->route(McGetRequest("qwert:getReq")); // colocated, bucketId == 454705
  EXPECT_EQ(454705, srHandleVec[0]->sawBucketIds[6]);
  rh->route(McGetRequest("zxcvgetReq")); // colocated, bucketId == 97898
  EXPECT_EQ(97898, srHandleVec[0]->sawBucketIds[7]);
  rh->route(McGetRequest("zxcv")); // colocated, bucketId == 418469
  EXPECT_EQ(418469, srHandleVec[0]->sawBucketIds[8]);
  rh->route(McGetRequest("zxc")); // not colocated, bucketId == 839734
  EXPECT_EQ(839734, srHandleVec[0]->sawBucketIds[9]);
}

} // namespace facebook::memcache::mcrouter
