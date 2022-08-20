/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/routes/BlackholeRoute.h"
#include "mcrouter/CarbonRouterInstance.h"
#include "mcrouter/PoolFactory.h"
#include "mcrouter/lib/carbon/example/gen/HelloGoodbyeRouterInfo.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/options.h"
#include "mcrouter/routes/McRouteHandleProvider.h"
#include "mcrouter/routes/test/RouteHandleTestBase.h"

using namespace hellogoodbye;

namespace facebook {
namespace memcache {
namespace mcrouter {

class BlackholeRouteTest : public RouteHandleTestBase<HelloGoodbyeRouterInfo> {
 public:
  HelloGoodbyeRouterInfo::RouteHandlePtr getBlackholeRoute(
      folly::StringPiece jsonStr) {
    return makeBlackholeRoute<HelloGoodbyeRouterInfo>(
        rhFactory_, folly::parseJson(jsonStr));
  }

  void testCreate(folly::StringPiece config) {
    auto rh = getBlackholeRoute(config);
    ASSERT_TRUE(rh);
    EXPECT_EQ("blackhole", rh->routeName());
  }
};

constexpr folly::StringPiece kBlackholeConfigWithNoChild = R"(
{
  "policies": {
    "shardId": [
      {
        "op": "equals",
        "value": 1234
      }
    ]
  },
  "default": "ErrorRoute"
}
)";

TEST_F(BlackholeRouteTest, create) {
  testCreate(kBlackholeConfigWithNoChild);
}

TEST_F(BlackholeRouteTest, route) {
  auto rh = getBlackholeRoute(kBlackholeConfigWithNoChild);
  ASSERT_TRUE(rh);

  GoodbyeRequest req;
  GoodbyeReply reply;

  req.shardId_ref() = 1234;
  reply = rh->route(req);
  EXPECT_EQ(carbon::Result::NOTFOUND, *reply.result_ref());

  req.shardId_ref() = 2345;
  reply = rh->route(req);
  EXPECT_EQ(carbon::Result::LOCAL_ERROR, *reply.result_ref());
}

constexpr folly::StringPiece kBlackholeConfigWithChild = R"(
{
  "policies": {
    "shardId": [
      {
        "op": "not_equals",
        "value": 5678
      }
    ]
  },
  "default": "NullRoute",
  "blackhole_child": "ErrorRoute"
}
)";

TEST_F(BlackholeRouteTest, createWithChild) {
  testCreate(kBlackholeConfigWithChild);
}

TEST_F(BlackholeRouteTest, routeWithChild) {
  auto rh = getBlackholeRoute(kBlackholeConfigWithChild);
  ASSERT_TRUE(rh);

  GoodbyeRequest req;
  GoodbyeReply reply;

  req.shardId_ref() = 1234;
  reply = rh->route(req);
  EXPECT_EQ(carbon::Result::LOCAL_ERROR, *reply.result_ref());

  req.shardId_ref() = 5678;
  reply = rh->route(req);
  EXPECT_EQ(carbon::Result::NOTFOUND, *reply.result_ref());
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
