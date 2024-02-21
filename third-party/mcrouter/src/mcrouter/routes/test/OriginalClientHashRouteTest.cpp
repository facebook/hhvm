/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include <folly/json/dynamic.h>

#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/lib/test/RouteHandleTestUtil.h"
#include "mcrouter/lib/test/TestRouteHandle.h"
#include "mcrouter/routes/HashRouteFactory.h"
#include "mcrouter/routes/McrouterRouteHandle.h"
#include "mcrouter/routes/OriginalClientHashRoute.h"
#include "mcrouter/routes/test/RouteHandleTestUtil.h"

using namespace facebook::memcache;
using namespace facebook::memcache::mcrouter;

using std::make_shared;

TEST(originalClientHashRouteTest, basic) {
  std::vector<std::shared_ptr<TestHandle>> test_handles{
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "a")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "b")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "c"))};
  TestFiberManager<MemcacheRouterInfo> testfm;
  auto context = getTestContext();

  // No IP Address will return error
  testfm.run([&]() {
    fiber_local<MemcacheRouterInfo>::setSharedCtx(context);
    auto rh = createOriginalClientHashRoute<MemcacheRouterInfo>(
        get_route_handles(test_handles), 0 /* offset */);
    auto reply = rh->route(McGetRequest("0"));
    EXPECT_EQ(carbon::Result::LOCAL_ERROR, *reply.result_ref());
  });

  context->setUserIpAddress("12345");
  testfm.run([&]() {
    fiber_local<MemcacheRouterInfo>::setSharedCtx(context);
    auto rh = createOriginalClientHashRoute<MemcacheRouterInfo>(
        get_route_handles(test_handles), 0 /* offset */);
    auto reply = rh->route(McGetRequest("0"));
    EXPECT_EQ("a", carbon::valueRangeSlow(reply).str());
  });

  // Offset 1 should map to b
  testfm.run([&]() {
    fiber_local<MemcacheRouterInfo>::setSharedCtx(context);
    auto rh = createOriginalClientHashRoute<MemcacheRouterInfo>(
        get_route_handles(test_handles), 1 /* offset */);
    auto reply = rh->route(McGetRequest("0"));
    EXPECT_EQ("b", carbon::valueRangeSlow(reply).str());
  });

  // Offset 2 should map to c
  testfm.run([&]() {
    fiber_local<MemcacheRouterInfo>::setSharedCtx(context);
    auto rh = createOriginalClientHashRoute<MemcacheRouterInfo>(
        get_route_handles(test_handles), 2 /* offset */);
    auto reply = rh->route(McGetRequest("0"));
    EXPECT_EQ("c", carbon::valueRangeSlow(reply).str());
  });

  // Set IP that maps to b and set offset to 2 to test wrap around
  context->setUserIpAddress("0");
  testfm.run([&]() {
    fiber_local<MemcacheRouterInfo>::setSharedCtx(context);
    auto rh = createOriginalClientHashRoute<MemcacheRouterInfo>(
        get_route_handles(test_handles), 2 /* offset */);
    auto reply = rh->route(McGetRequest("0"));
    EXPECT_EQ("a", carbon::valueRangeSlow(reply).str());
  });
}
