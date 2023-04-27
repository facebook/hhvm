/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/routes/LatencyInjectionRoute.h"
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

class LatencyInjectionRouteTest
    : public RouteHandleTestBase<HelloGoodbyeRouterInfo> {
 public:
  HelloGoodbyeRouterInfo::RouteHandlePtr getLatencyInjectionRoute(
      folly::StringPiece jsonStr) {
    McrouterOptions opts = defaultTestOptions();
    opts.config = "{ \"route\": \"NullRoute\" }";
    std::shared_ptr<ProxyRequestContextWithInfo<HelloGoodbyeRouterInfo>> ctx{
        ProxyRequestContextWithInfo<HelloGoodbyeRouterInfo>::createRecording(
            *CarbonRouterInstance<HelloGoodbyeRouterInfo>::init("test", opts)
                 ->getProxy(0),
            nullptr)};
    fiber_local<HelloGoodbyeRouterInfo>::setSharedCtx(std::move(ctx));
    return makeLatencyInjectionRoute<HelloGoodbyeRouterInfo>(
        rhFactory_, folly::parseJson(jsonStr));
  }

  void testCreate(folly::StringPiece config) {
    auto rh = getLatencyInjectionRoute(config);
    ASSERT_TRUE(rh);
    EXPECT_TRUE(rh->routeName().find("latency-injection") == 0);
  }
};

constexpr folly::StringPiece kLatencyInjectionRouteRequestPayload = R"(
{
  "request_payload_latency": true,
  "child": "NullRoute"
}
)";

TEST_F(LatencyInjectionRouteTest, create) {
  testCreate(kLatencyInjectionRouteRequestPayload);
}

TEST_F(LatencyInjectionRouteTest, routeRequestPayload) {
  auto rh = getLatencyInjectionRoute(kLatencyInjectionRouteRequestPayload);
  ASSERT_TRUE(rh);

  HelloRequest req;
  HelloReply reply;

  req.beforeLatencyUs_ref() = 1000000;
  req.afterLatencyUs_ref() = 1000000;
  const auto before_ms = getCurrentTimeInMs();
  reply = rh->route(req);
  const auto elapsed =
      std::chrono::milliseconds(getCurrentTimeInMs() - before_ms);
  EXPECT_EQ(carbon::Result::NOTFOUND, *reply.result_ref());
  EXPECT_GE(elapsed.count(), 1999);
}

constexpr folly::StringPiece kLatencyInjectionRouteIgnore = R"(
{
  "request_payload_latency": true,
  "max_request_latency_us": 100,
  "child": "NullRoute"
}
)";
TEST_F(LatencyInjectionRouteTest, routeRequestPayloadIgnore) {
  auto rh = getLatencyInjectionRoute(kLatencyInjectionRouteIgnore);
  ASSERT_TRUE(rh);

  HelloRequest req;
  HelloReply reply;

  req.beforeLatencyUs_ref() = 1000000000;
  req.afterLatencyUs_ref() = 1000000000;
  const auto before_ms = getCurrentTimeInMs();
  reply = rh->route(req);
  const auto elapsed =
      std::chrono::milliseconds(getCurrentTimeInMs() - before_ms);
  EXPECT_EQ(carbon::Result::NOTFOUND, *reply.result_ref());
  EXPECT_LE(elapsed.count(), 20000);
}

constexpr folly::StringPiece kLatencyInjectionRouteBefore = R"(
{
  "before_latency_ms": 1000,
  "child": "NullRoute"
}
)";
TEST_F(LatencyInjectionRouteTest, routeBefore) {
  auto rh = getLatencyInjectionRoute(kLatencyInjectionRouteBefore);
  ASSERT_TRUE(rh);

  HelloRequest req;
  HelloReply reply;

  const auto before_ms = getCurrentTimeInMs();
  reply = rh->route(req);
  const auto elapsed =
      std::chrono::milliseconds(getCurrentTimeInMs() - before_ms);
  EXPECT_EQ(carbon::Result::NOTFOUND, *reply.result_ref());
  EXPECT_GE(elapsed.count(), 999);
}

constexpr folly::StringPiece kLatencyInjectionRouteAfter = R"(
{
  "after_latency_ms": 1000,
  "child": "NullRoute"
}
)";
TEST_F(LatencyInjectionRouteTest, routeAfter) {
  auto rh = getLatencyInjectionRoute(kLatencyInjectionRouteAfter);
  ASSERT_TRUE(rh);

  HelloRequest req;
  HelloReply reply;

  const auto before_ms = getCurrentTimeInMs();
  reply = rh->route(req);
  const auto elapsed =
      std::chrono::milliseconds(getCurrentTimeInMs() - before_ms);
  EXPECT_EQ(carbon::Result::NOTFOUND, *reply.result_ref());
  EXPECT_GE(elapsed.count(), 999);
}

constexpr folly::StringPiece kLatencyInjectionRouteTotal = R"(
{
  "total_latency_ms": 1000,
  "child": "NullRoute"
}
)";
TEST_F(LatencyInjectionRouteTest, routeTotal) {
  auto rh = getLatencyInjectionRoute(kLatencyInjectionRouteTotal);
  ASSERT_TRUE(rh);

  HelloRequest req;
  HelloReply reply;

  const auto before_ms = getCurrentTimeInMs();
  reply = rh->route(req);
  const auto elapsed =
      std::chrono::milliseconds(getCurrentTimeInMs() - before_ms);
  EXPECT_EQ(carbon::Result::NOTFOUND, *reply.result_ref());
  EXPECT_GE(elapsed.count(), 999);
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
