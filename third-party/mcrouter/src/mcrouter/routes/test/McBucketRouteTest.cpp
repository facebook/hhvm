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
#include "mcrouter/routes/test/RouteHandleTestUtil.h"

#include <vector>

#include <gtest/gtest.h>

#include <folly/Range.h>
#include <folly/dynamic.h>
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
    "bucketize_until": 29,
    "bucketization_keyspace": "testRegion:testPool"
  }
  )";

  auto rh =
      makeMcBucketRoute(mockSrHandle, folly::parseJson(kMcBucketRouteConfig));
  ASSERT_TRUE(rh);
  mockFiberContext();
  rh->route(McGetRequest("getReq")); // bucketId == 28
  auto proxy = &fiber_local<MemcacheRouterInfo>::getSharedCtx()->proxy();
  EXPECT_EQ(1, proxy->stats().getValue(bucketized_routing_stat));
  EXPECT_FALSE(srHandleVec[0]->sawBucketIds.empty());
  EXPECT_EQ(28, srHandleVec[0]->sawBucketIds[0]);
}

TEST(McBucketRouteTest, bucketIdShouldNotPropagate) {
  std::vector<std::shared_ptr<TestHandle>> srHandleVec{
      std::make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "a")),
  };
  auto mockSrHandle = get_route_handles(srHandleVec)[0];

  constexpr folly::StringPiece kMcBucketRouteConfig = R"(
  {
    "bucketize": true,
    "total_buckets": 100,
    "bucketize_until": 27,
    "bucketization_keyspace": "testRegion:testPool"
  }
  )";

  auto rh =
      makeMcBucketRoute(mockSrHandle, folly::parseJson(kMcBucketRouteConfig));
  ASSERT_TRUE(rh);
  mockFiberContext();
  rh->route(McGetRequest("getReq")); // bucketId == 28
  auto proxy = &fiber_local<MemcacheRouterInfo>::getSharedCtx()->proxy();
  EXPECT_EQ(0, proxy->stats().getValue(bucketized_routing_stat));
  EXPECT_TRUE(srHandleVec[0]->sawBucketIds.empty());
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
    "bucketize_until": 29,
    "bucketization_keyspace": "testRegion:testPool"
  }
  )";

  auto rh =
      makeMcBucketRoute(mockSrHandle, folly::parseJson(kMcBucketRouteConfig));
  mockFiberContext();
  RouteHandleTraverser<MemcacheRouterInfo::RouteHandleIf> t{};
  rh->traverse(McGetRequest("getReq"), t); // bucketId == 28
  EXPECT_FALSE(srHandleVec[0]->sawBucketIds.empty());
  EXPECT_EQ(28, srHandleVec[0]->sawBucketIds[0]);
}

TEST(McBucketRouteTest, bucketIdShouldNotPropagateInTraverse) {
  std::vector<std::shared_ptr<TestHandle>> srHandleVec{
      std::make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "a")),
  };
  auto mockSrHandle = get_route_handles(srHandleVec)[0];

  constexpr folly::StringPiece kMcBucketRouteConfig = R"(
  {
    "bucketize": true,
    "total_buckets": 100,
    "bucketize_until": 28,
    "bucketization_keyspace": "testRegion:testPool"
  }
  )";

  auto rh =
      makeMcBucketRoute(mockSrHandle, folly::parseJson(kMcBucketRouteConfig));
  mockFiberContext();
  RouteHandleTraverser<MemcacheRouterInfo::RouteHandleIf> t{};
  rh->traverse(McGetRequest("getReq"), t); // bucketId == 28
  EXPECT_TRUE(srHandleVec[0]->sawBucketIds.empty());
}

} // namespace facebook::memcache::mcrouter
