/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/lib/test/RouteHandleTestUtil.h"
#include "mcrouter/lib/test/TestRouteHandle.h"
#include "mcrouter/routes/WarmUpRoute.h"

using namespace facebook::memcache;
using namespace facebook::memcache::mcrouter;

using std::make_shared;
using std::string;
using std::vector;

using TestHandle = TestHandleImpl<TestRouteHandleIf>;

TEST(warmUpRouteTest, warmUp) {
  vector<std::shared_ptr<TestHandle>> test_handles{
      make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "a"),
          UpdateRouteTestData(carbon::Result::STORED),
          DeleteRouteTestData(carbon::Result::DELETED)),
      make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "b"),
          UpdateRouteTestData(carbon::Result::STORED),
          DeleteRouteTestData(carbon::Result::NOTFOUND)),
      make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::NOTFOUND, ""),
          UpdateRouteTestData(carbon::Result::NOTSTORED),
          DeleteRouteTestData(carbon::Result::NOTFOUND)),
  };
  auto route_handles = get_route_handles(test_handles);

  TestFiberManager fm;

  fm.run([&]() {
    TestRouteHandle<WarmUpRoute<TestRouteHandleIf>> rh(
        route_handles[0], route_handles[1], 1);

    auto reply_get = rh.route(McGetRequest("key_get"));
    EXPECT_EQ("b", carbon::valueRangeSlow(reply_get).str());
    EXPECT_NE(vector<string>{"key_get"}, test_handles[0]->saw_keys);
    EXPECT_EQ(vector<string>{"key_get"}, test_handles[1]->saw_keys);
    (test_handles[0]->saw_keys).clear();
    (test_handles[1]->saw_keys).clear();

    auto reply_del = rh.route(McDeleteRequest("key_del"));
    EXPECT_EQ(carbon::Result::NOTFOUND, *reply_del.result_ref());
    EXPECT_NE(vector<string>{"key_del"}, test_handles[0]->saw_keys);
    EXPECT_EQ(vector<string>{"key_del"}, test_handles[1]->saw_keys);
  });
  fm.run([&]() {
    TestRouteHandle<WarmUpRoute<TestRouteHandleIf>> rh(
        route_handles[0], route_handles[2], 1);

    auto reply_get = rh.route(McGetRequest("key_get"));
    EXPECT_EQ("a", carbon::valueRangeSlow(reply_get).str());
    EXPECT_EQ(vector<string>{"key_get"}, test_handles[0]->saw_keys);
    EXPECT_EQ(vector<string>{"key_get"}, test_handles[2]->saw_keys);
  });
  fm.run([&]() {
    EXPECT_EQ((vector<uint32_t>{0, 1}), test_handles[2]->sawExptimes);
    (test_handles[0]->saw_keys).clear();
    (test_handles[2]->saw_keys).clear();
    EXPECT_EQ(
        (vector<std::string>{"get", "add"}), test_handles[2]->sawOperations);
  });
  fm.run([&]() {
    TestRouteHandle<WarmUpRoute<TestRouteHandleIf>> rh(
        route_handles[0], route_handles[2], 1);

    auto reply_del = rh.route(McDeleteRequest("key_del"));
    EXPECT_EQ(carbon::Result::NOTFOUND, *reply_del.result_ref());
    EXPECT_NE(vector<string>{"key_del"}, test_handles[0]->saw_keys);
    EXPECT_EQ(vector<string>{"key_del"}, test_handles[2]->saw_keys);
  });
}
