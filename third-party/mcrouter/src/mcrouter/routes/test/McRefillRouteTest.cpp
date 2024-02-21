/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/routes/McRefillRoute.h"
#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/lib/network/gen/MemcacheRouterInfo.h"
#include "mcrouter/lib/test/RouteHandleTestUtil.h"
#include "mcrouter/lib/test/TestRouteHandle.h"
#include "mcrouter/routes/test/RouteHandleTestBase.h"
#include "mcrouter/routes/test/RouteHandleTestUtil.h"

#include <vector>

#include <folly/Range.h>
#include <folly/String.h>
#include <folly/json/dynamic.h>
#include <gtest/gtest.h>
#include "folly/fibers/FiberManagerMap.h"
#include "folly/io/async/EventBase.h"
#include "folly/json/json.h"

namespace facebook::memcache::mcrouter {

class McRefillRouteTest : public RouteHandleTestBase<MemcacheRouterInfo> {};

TEST_F(McRefillRouteTest, create) {
  // Full TaoShardFilterRoute. Used to see if the route handle configures fine.
  constexpr folly::StringPiece kMcRefillRouteConfig = R"(
  {
    "type": "McRefillRoute",
    "primary": {"type": "NullRoute"},
    "refill": {"type": "ErrorRoute"}
  }
  )";

  auto rh = getRoute(kMcRefillRouteConfig);
  EXPECT_TRUE(rh != nullptr);
  EXPECT_EQ("McRefillRoute", rh->routeName());
}

} // namespace facebook::memcache::mcrouter
