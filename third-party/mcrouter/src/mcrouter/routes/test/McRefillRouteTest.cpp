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

class McRefillRouteTest : public RouteHandleTestBase<MemcacheRouterInfo> {
 public:
  McRefillRouteTest() {
    primaryTestHandle = std::make_shared<TestHandle>(
        GetRouteTestData(carbon::Result::NOTFOUND, "a"));
    refillTestHandle = std::make_shared<TestHandle>(
        GetRouteTestData(carbon::Result::FOUND, "a"));
    primary = primaryTestHandle->rh;
    refill = refillTestHandle->rh;
  }

  std::shared_ptr<MemcacheRouteHandleIf> primary;
  std::shared_ptr<TestHandle> primaryTestHandle;
  std::shared_ptr<TestHandle> refillTestHandle;
  std::shared_ptr<MemcacheRouteHandleIf> refill;
};

TEST_F(McRefillRouteTest, create) {
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

TEST_F(McRefillRouteTest, testRefill) {
  mockFiberContext();
  McRefillRoute<MemcacheRouterInfo> rh(primary, refill);
  auto getReq = McGetRequest("key");
  std::string clientId = "iamaclient";
  getReq.setClientIdentifier(clientId);
  TestFiberManager<MemcacheRouterInfo> fm;
  fm.runAll({[&]() { rh.route(getReq); }});
  EXPECT_EQ(primaryTestHandle->sawOperations.size(), 2);
  EXPECT_EQ(primaryTestHandle->sawOperations[0], McGetRequest::name);
  EXPECT_EQ(primaryTestHandle->sawClientIdentifiers[0], clientId);
  EXPECT_EQ(primaryTestHandle->sawOperations[1], McSetRequest::name);
  EXPECT_EQ(primaryTestHandle->sawClientIdentifiers[1], clientId);

  EXPECT_EQ(refillTestHandle->sawOperations.size(), 2);
  EXPECT_EQ(refillTestHandle->sawOperations[0], McGetRequest::name);
  EXPECT_EQ(refillTestHandle->sawClientIdentifiers[0], clientId);
  EXPECT_EQ(refillTestHandle->sawOperations[1], McMetagetRequest::name);
  EXPECT_EQ(refillTestHandle->sawClientIdentifiers[1], clientId);
}

} // namespace facebook::memcache::mcrouter
