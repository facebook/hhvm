/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/lib/test/RouteHandleTestUtil.h"
#include "mcrouter/lib/test/TestRouteHandle.h"
#include "mcrouter/routes/MissFailoverRoute.h"

using namespace facebook::memcache;
using namespace facebook::memcache::mcrouter;

using std::make_shared;
using std::vector;

using TestHandle = TestHandleImpl<TestRouteHandleIf>;

TEST(missMissFailoverRouteTest, success) {
  vector<std::shared_ptr<TestHandle>> test_handles{
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "a")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "b")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "c"))};

  TestRouteHandle<MissFailoverRoute<TestRouterInfo>> rh(
      get_route_handles(test_handles));

  auto reply = rh.route(McGetRequest("0"));
  EXPECT_EQ("a", carbon::valueRangeSlow(reply).str());
}

TEST(missMissFailoverRouteTest, once) {
  vector<std::shared_ptr<TestHandle>> test_handles{
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::NOTFOUND, "a")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "b")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "c"))};

  TestRouteHandle<MissFailoverRoute<TestRouterInfo>> rh(
      get_route_handles(test_handles));

  auto reply = rh.route(McGetRequest("0"));
  EXPECT_EQ("b", carbon::valueRangeSlow(reply).str());
}

TEST(missMissFailoverRouteTest, twice) {
  vector<std::shared_ptr<TestHandle>> test_handles{
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::NOTFOUND, "a")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::TIMEOUT, "b")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "c"))};

  TestRouteHandle<MissFailoverRoute<TestRouterInfo>> rh(
      get_route_handles(test_handles));

  auto reply = rh.route(McGetRequest("0"));
  EXPECT_EQ("c", carbon::valueRangeSlow(reply).str());
}

TEST(missMissFailoverRouteTest, fail) {
  vector<std::shared_ptr<TestHandle>> test_handles{
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::NOTFOUND, "a")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::TIMEOUT, "b")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::TIMEOUT, "c"))};

  TestRouteHandle<MissFailoverRoute<TestRouterInfo>> rh(
      get_route_handles(test_handles));

  auto reply = rh.route(McGetRequest("0"));

  // Should get the last reply
  EXPECT_EQ("c", carbon::valueRangeSlow(reply).str());
  EXPECT_EQ(carbon::Result::TIMEOUT, *reply.result_ref());
}

TEST(missMissFailoverRouteTest, bestOnError1) {
  vector<std::shared_ptr<TestHandle>> test_handles{
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::NOTFOUND, "a")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::TIMEOUT, "b")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::TIMEOUT, "c"))};

  TestRouteHandle<MissFailoverRoute<TestRouterInfo>> rh(
      get_route_handles(test_handles), true);

  auto reply = rh.route(McGetRequest("0"));

  // Should return the first and the only healthy reply
  EXPECT_EQ("a", carbon::valueRangeSlow(reply).str());
  EXPECT_EQ(carbon::Result::NOTFOUND, *reply.result_ref());
}

TEST(missMissFailoverRouteTest, bestOnError2) {
  vector<std::shared_ptr<TestHandle>> test_handles{
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::TIMEOUT, "a")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::NOTFOUND, "b")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::TIMEOUT, "c"))};

  TestRouteHandle<MissFailoverRoute<TestRouterInfo>> rh(
      get_route_handles(test_handles), true);

  auto reply = rh.route(McGetRequest("0"));

  // Should return the only failover-healthy reply
  EXPECT_EQ("b", carbon::valueRangeSlow(reply).str());
  EXPECT_EQ(carbon::Result::NOTFOUND, *reply.result_ref());
}

TEST(missMissFailoverRouteTest, bestOnError3) {
  vector<std::shared_ptr<TestHandle>> test_handles{
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::TIMEOUT, "a")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::NOTFOUND, "b")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::NOTFOUND, "c"))};

  TestRouteHandle<MissFailoverRoute<TestRouterInfo>> rh(
      get_route_handles(test_handles), true);

  auto reply = rh.route(McGetRequest("0"));

  // Should get the LAST healthy reply
  EXPECT_EQ("c", carbon::valueRangeSlow(reply).str());
  EXPECT_EQ(carbon::Result::NOTFOUND, *reply.result_ref());
}

TEST(missMissFailoverRouteTest, nonGetLike) {
  vector<std::shared_ptr<TestHandle>> test_handles{
      make_shared<TestHandle>(UpdateRouteTestData(carbon::Result::NOTSTORED)),
      make_shared<TestHandle>(UpdateRouteTestData(carbon::Result::STORED)),
      make_shared<TestHandle>(UpdateRouteTestData(carbon::Result::STORED))};

  TestRouteHandle<MissFailoverRoute<TestRouterInfo>> rh(
      get_route_handles(test_handles));

  McSetRequest req("0");
  req.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "a");
  auto reply = rh.route(std::move(req));

  EXPECT_EQ(carbon::Result::NOTSTORED, *reply.result_ref());
  // only first handle sees the key
  EXPECT_EQ(vector<std::string>{"0"}, test_handles[0]->saw_keys);
  EXPECT_TRUE(test_handles[1]->saw_keys.empty());
  EXPECT_TRUE(test_handles[2]->saw_keys.empty());
}
