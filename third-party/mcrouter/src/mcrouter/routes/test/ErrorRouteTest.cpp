/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include "mcrouter/lib/carbon/example/gen/HelloGoodbyeRouterInfo.h"
#include "mcrouter/lib/test/RouteHandleTestUtil.h"
#include "mcrouter/lib/test/TestRouteHandle.h"
#include "mcrouter/routes/ErrorRoute.h"
#include "mcrouter/routes/test/RouteHandleTestBase.h"

using namespace hellogoodbye;
using namespace facebook::memcache;
using namespace facebook::memcache::mcrouter;

constexpr folly::StringPiece kErrorRouteConfigWithBusyReply = R"(
{
  "result": "mc_res_busy"
}
)";

constexpr folly::StringPiece kErrorRouteConfigWithoutLogging = R"(
{
  "enable_logging": false
}
)";
constexpr folly::StringPiece kErrorRouteConfigWithUnknownReply = R"(
{
  "result": "this_is_a_bad_name"
}
)";
constexpr folly::StringPiece kErrorRouteConfigWithAllParams = R"(
{
  "response": "custom msg",
  "enable_logging": true,
  "result": "mc_res_local_error"
}
)";
constexpr folly::StringPiece kErrorRouteConfigWithInvalidReply = R"(
{
  "result": "mc_res_notfound"
}
)";

class ErrorRouteTest : public RouteHandleTestBase<HelloGoodbyeRouterInfo> {
 public:
  HelloGoodbyeRouterInfo::RouteHandlePtr getErrorRoute(
      folly::StringPiece jsonStr) {
    return makeErrorRoute<HelloGoodbyeRouterInfo>(
        rhFactory_, folly::parseJson(jsonStr));
  }
};

TEST_F(ErrorRouteTest, createWithoutLogging) {
  auto rh = getErrorRoute(kErrorRouteConfigWithoutLogging);
  EXPECT_EQ("error|no-log|mc_res_local_error", rh->routeName());
}

TEST_F(ErrorRouteTest, createWithBusyReply) {
  auto rh = getErrorRoute(kErrorRouteConfigWithBusyReply);
  EXPECT_EQ("error|log|mc_res_busy", rh->routeName());
  auto reply = rh->route(GoodbyeRequest());
  EXPECT_EQ(*reply.result_ref(), carbon::Result::BUSY);
}

TEST_F(ErrorRouteTest, createWithUnknownReply) {
  EXPECT_THROW(
      getErrorRoute(kErrorRouteConfigWithUnknownReply), std::logic_error);
}

TEST_F(ErrorRouteTest, createWithAllParams) {
  auto rh = getErrorRoute(kErrorRouteConfigWithAllParams);
  EXPECT_EQ("error|log|mc_res_local_error|custom msg", rh->routeName());
}

TEST_F(ErrorRouteTest, createWithInvalidReply) {
  EXPECT_THROW(
      getErrorRoute(kErrorRouteConfigWithInvalidReply), std::logic_error);
}

TEST(ErrorRoute, create) {
  TestRouteHandle<ErrorRoute<TestRouterInfo>> rh;
  EXPECT_EQ("error|log|mc_res_local_error", rh.routeName());
}

TEST(ErrorRoute, createCustomMessage) {
  TestRouteHandle<ErrorRoute<TestRouterInfo>> rh("custom msg");
  EXPECT_EQ("error|log|mc_res_local_error|custom msg", rh.routeName());
}

TEST(ErrorRoute, route) {
  TestRouteHandle<ErrorRoute<TestRouterInfo>> rh;
  auto reply = rh.route(McGetRequest("key"));
  EXPECT_TRUE(isErrorResult(*reply.result_ref()));
}

TEST(ErrorRoute, routeCustomMessage) {
  TestRouteHandle<ErrorRoute<TestRouterInfo>> rh("custom msg");
  auto reply = rh.route(McGetRequest("key"));
  EXPECT_TRUE(isErrorResult(*reply.result_ref()));
  EXPECT_EQ("custom msg", *reply.message_ref());
}
