/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/routes/ClientCompatibilityCheckerRoute.h"
#include "mcrouter/CarbonRouterInstance.h"
#include "mcrouter/PoolFactory.h"
#include "mcrouter/lib/carbon/example/gen/HelloGoodbyeRouterInfo.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/routes/McRouteHandleProvider.h"
#include "mcrouter/routes/test/RouteHandleTestBase.h"

using namespace hellogoodbye;

namespace facebook {
namespace memcache {
namespace mcrouter {

class ClientCompatibilityCheckerRouteTest
    : public RouteHandleTestBase<HelloGoodbyeRouterInfo> {
 public:
  HelloGoodbyeRouterInfo::RouteHandlePtr getClientCompatibilityCheckerRoute(
      folly::StringPiece jsonStr) {
    McrouterOptions opts = defaultTestOptions();
    opts.config = "{ \"route\": \"NullRoute\" }";
    std::shared_ptr<ProxyRequestContextWithInfo<HelloGoodbyeRouterInfo>> ctx{
        ProxyRequestContextWithInfo<HelloGoodbyeRouterInfo>::createRecording(
            *CarbonRouterInstance<HelloGoodbyeRouterInfo>::init("test", opts)
                 ->getProxy(0),
            nullptr)};
    fiber_local<HelloGoodbyeRouterInfo>::setSharedCtx(std::move(ctx));
    return makeClientCompatibilityCheckerRoute<HelloGoodbyeRouterInfo>(
        rhFactory_, folly::parseJson(jsonStr));
  }
};

constexpr folly::StringPiece kClientCompatibilityCheckerRoute = R"(
{
  "max_supported_client_version": 10,
  "child": "NullRoute"
}
)";

TEST_F(ClientCompatibilityCheckerRouteTest, basic) {
  auto rh =
      getClientCompatibilityCheckerRoute(kClientCompatibilityCheckerRoute);
  ASSERT_TRUE(rh);

  HelloRequest req;
  HelloReply reply;

  req.clientVersion_ref() = 9;
  reply = rh->route(req);
  EXPECT_EQ(carbon::Result::NOTFOUND, *reply.result_ref());
  EXPECT_EQ("", *reply.message_ref());

  req.clientVersion_ref() = 10;
  reply = rh->route(req);
  EXPECT_EQ(carbon::Result::CLIENT_ERROR, *reply.result_ref());
  EXPECT_EQ(
      "Requests from newer client with version 10 are not supported. "
      "Maximum supported client version is 9",
      *reply.message_ref());

  req.clientVersion_ref() = 11;
  reply = rh->route(req);
  EXPECT_EQ(carbon::Result::CLIENT_ERROR, *reply.result_ref());
  EXPECT_EQ(
      "Requests from newer client with version 11 are not supported. "
      "Maximum supported client version is 9",
      *reply.message_ref());
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
