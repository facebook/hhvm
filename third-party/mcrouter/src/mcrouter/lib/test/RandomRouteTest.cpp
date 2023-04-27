/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "mcrouter/lib/McResUtil.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/lib/routes/RandomRoute.h"
#include "mcrouter/lib/test/RouteHandleTestUtil.h"
#include "mcrouter/lib/test/TestRouteHandle.h"

using namespace facebook::memcache;

using std::make_shared;
using std::vector;

using TestHandle = TestHandleImpl<TestRouteHandleIf>;

TEST(randomRouteTest, success) {
  vector<std::shared_ptr<TestHandle>> test_handles{
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "a")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "b")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "c")),
  };

  TestRouteHandle<RandomRoute<TestRouteHandleIf>> rh(
      get_route_handles(test_handles));

  auto reply = rh.route(McGetRequest("0"));
  EXPECT_TRUE(isHitResult(*reply.result_ref()));
}

TEST(randomRouteTest, cover) {
  vector<std::shared_ptr<TestHandle>> test_handles{
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "a")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::NOTFOUND, "b")),
  };

  TestRouteHandle<RandomRoute<TestRouteHandleIf>> rh(
      get_route_handles(test_handles));

  int hit = 0, miss = 0;
  const int rounds = 32;
  for (int i = 0; i < rounds; i++) {
    auto reply = rh.route(McGetRequest("0"));
    hit += ((isHitResult(*reply.result_ref())) ? 1 : 0);
    miss += ((isMissResult(*reply.result_ref())) ? 1 : 0);
  }

  EXPECT_GT(hit, 0);
  EXPECT_GT(miss, 0);
  EXPECT_EQ(hit + miss, rounds);
}

TEST(randomRouteTest, fail) {
  vector<std::shared_ptr<TestHandle>> test_handles{
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::TIMEOUT, "a")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::NOTFOUND, "b")),
      make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::REMOTE_ERROR, "c")),
  };

  TestRouteHandle<RandomRoute<TestRouteHandleIf>> rh(
      get_route_handles(test_handles));

  auto reply = rh.route(McGetRequest("0"));

  EXPECT_TRUE(!isHitResult(*reply.result_ref()));
}
