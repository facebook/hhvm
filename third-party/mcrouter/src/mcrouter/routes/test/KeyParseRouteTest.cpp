/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/routes/KeyParseRoute.h"
#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/lib/network/gen/MemcacheRouterInfo.h"
#include "mcrouter/routes/test/RouteHandleTestUtil.h"

#include <vector>

#include <folly/Range.h>
#include <folly/dynamic.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "folly/io/async/EventBase.h"
#include "folly/json.h"

namespace facebook::memcache::mcrouter {

using TestRouteHandle = std::shared_ptr<MemcacheRouteHandleIf>;

std::pair<std::vector<std::shared_ptr<TestHandle>>, TestRouteHandle>
getMockSrHandle() {
  std::vector<std::shared_ptr<TestHandle>> srHandleVec{
      std::make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "a")),
  };
  return {std::move(srHandleVec), get_route_handles(srHandleVec)[0]};
}

auto getRoutingConfig(size_t numParts, std::string delimiter = ":") {
  return fmt::format(
      R"(
  {{
    "num_routing_parts": {},
    "delimiter": "{}"
  }}
  )",
      numParts,
      delimiter);
}

TEST(KeyParseRouteTest, basicTest) {
  auto [srHandleVec, mockSrHandle] = getMockSrHandle();
  auto keyParseRouteConfig = getRoutingConfig(3);
  auto rh = createKeyParseRoute<MemcacheRouterInfo>(
      mockSrHandle, folly::parseJson(keyParseRouteConfig));
  mockFiberContext();
  rh->route(McGetRequest("get:Request:part3:part4"));
  EXPECT_FALSE(srHandleVec[0]->sawCustomRoutingkeys.empty());
  EXPECT_EQ("get:Request:part3:", srHandleVec[0]->sawCustomRoutingkeys[0]);

  rh->route(McGetRequest("get:Request:part3:"));
  EXPECT_EQ("get:Request:part3:", srHandleVec[0]->sawCustomRoutingkeys[1]);

  rh->route(McGetRequest("get:Request:part3"));
  EXPECT_EQ("get:Request:part3", srHandleVec[0]->sawCustomRoutingkeys[2]);

  rh->route(McGetRequest("get:Request"));
  EXPECT_EQ("get:Request", srHandleVec[0]->sawCustomRoutingkeys[3]);

  rh->route(McGetRequest("getTest"));
  EXPECT_EQ("getTest", srHandleVec[0]->sawCustomRoutingkeys[4]);

  rh->route(McGetRequest("getTest::asff:"));
  EXPECT_EQ("getTest::asff:", srHandleVec[0]->sawCustomRoutingkeys[5]);

  rh->route(McGetRequest("getTest:::asff:"));
  EXPECT_EQ("getTest:::", srHandleVec[0]->sawCustomRoutingkeys[6]);

  rh->route(McGetRequest("::::::"));
  EXPECT_EQ(":::", srHandleVec[0]->sawCustomRoutingkeys[7]);
}

TEST(KeyParseRouteTest, testParsing) {
  EXPECT_THAT(
      [&]() {
        parseKeyParseRouteSettings(folly::parseJson(getRoutingConfig(0)));
      },
      testing::ThrowsMessage<std::logic_error>(
          testing::HasSubstr("should be in range")));

  EXPECT_THAT(
      [&]() {
        parseKeyParseRouteSettings(folly::parseJson(getRoutingConfig(3, "::")));
      },
      testing::ThrowsMessage<std::logic_error>(
          testing::HasSubstr("KeyParseRoute: missing single char delimiter")));
}

TEST(KeyParseRouteTest, keyParseWithBucketRoute) {
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
  auto keyParseRouteConfig = getRoutingConfig(3);
  auto json = folly::parseJson(keyParseRouteConfig);

  auto keyParseRh = createKeyParseRoute<MemcacheRouterInfo>(rh, json);
  ASSERT_TRUE(keyParseRh);
  mockFiberContext();
  keyParseRh->route(McGetRequest("cache:is:key:to:scaling")); // bucketId == 80
  keyParseRh->route(McGetRequest("cache:is:key:to:success")); // bucketId == 80
  auto proxy = &fiber_local<MemcacheRouterInfo>::getSharedCtx()->proxy();
  EXPECT_EQ(2, proxy->stats().getValue(bucketized_routing_stat));
  EXPECT_EQ(2, srHandleVec[0]->sawBucketIds.size());
  EXPECT_EQ(80, srHandleVec[0]->sawBucketIds[0]);
  EXPECT_EQ(80, srHandleVec[0]->sawBucketIds[1]);
}

} // namespace facebook::memcache::mcrouter
