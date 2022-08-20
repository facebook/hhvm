/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <glog/logging.h>
#include <gtest/gtest.h>

#include "mcrouter/PoolFactory.h"
#include "mcrouter/Proxy.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/lib/network/gen/MemcacheRouterInfo.h"
#include "mcrouter/options.h"
#include "mcrouter/routes/McRouteHandleProvider.h"
#include "mcrouter/routes/test/RouteHandleTestUtil.h"

using namespace facebook::memcache;
using namespace facebook::memcache::mcrouter;

TEST(RouteHandleFactoryTest, sanity) {
  TestFiberManager fm;

  auto router = getTestRouter();
  auto proxy = router->getProxy(0);
  PoolFactory pf(
      folly::dynamic::object(),
      router->configApi(),
      folly::json::metadata_map{});
  McRouteHandleProvider<MemcacheRouterInfo> provider(*proxy, pf);
  RouteHandleFactory<MemcacheRouteHandleIf> factory(provider, proxy->getId());

  auto rh = factory.create("AllAsyncRoute|ErrorRoute");
  EXPECT_TRUE(rh != nullptr);
  fm.run([&rh]() {
    auto reply = rh->route(McGetRequest("a"));
    EXPECT_EQ(carbon::Result::NOTFOUND, *reply.result_ref());
  });

  rh = factory.create("AllFastestRoute|ErrorRoute");
  EXPECT_TRUE(rh != nullptr);
  fm.run([&rh]() {
    auto reply = rh->route(McGetRequest("a"));
    EXPECT_TRUE(isErrorResult(*reply.result_ref()));
  });

  rh = factory.create("AllInitialRoute|ErrorRoute");
  EXPECT_TRUE(rh != nullptr);
  fm.run([&rh]() {
    auto reply = rh->route(McGetRequest("a"));
    EXPECT_TRUE(isErrorResult(*reply.result_ref()));
  });

  rh = factory.create("AllMajorityRoute|ErrorRoute");
  EXPECT_TRUE(rh != nullptr);
  fm.run([&rh]() {
    auto reply = rh->route(McGetRequest("a"));
    EXPECT_TRUE(isErrorResult(*reply.result_ref()));
  });

  rh = factory.create("AllSyncRoute|ErrorRoute");
  EXPECT_TRUE(rh != nullptr);
  fm.run([&rh]() {
    auto reply = rh->route(McGetRequest("a"));
    EXPECT_TRUE(isErrorResult(*reply.result_ref()));
  });

  rh = factory.create("FailoverRoute|NullRoute");
  EXPECT_TRUE(rh != nullptr);
  fm.run([&rh]() {
    auto reply = rh->route(McGetRequest("a"));
    EXPECT_EQ(carbon::Result::NOTFOUND, *reply.result_ref());
  });

  rh = factory.create("HashRoute|ErrorRoute");
  EXPECT_TRUE(rh != nullptr);
  fm.run([&rh]() {
    auto reply = rh->route(McGetRequest("a"));
    EXPECT_TRUE(isErrorResult(*reply.result_ref()));
  });

  rh = factory.create("HostIdRoute|ErrorRoute");
  EXPECT_TRUE(rh != nullptr);
  fm.run([&rh]() {
    auto reply = rh->route(McGetRequest("a"));
    EXPECT_TRUE(isErrorResult(*reply.result_ref()));
  });

  rh = factory.create("LatestRoute|NullRoute");
  EXPECT_TRUE(rh != nullptr);
  fm.run([&rh]() {
    auto reply = rh->route(McGetRequest("a"));
    EXPECT_EQ(carbon::Result::NOTFOUND, *reply.result_ref());
  });

  rh = factory.create("LoggingRoute");
  EXPECT_TRUE(rh != nullptr);
  fm.run([&rh]() {
    mockFiberContext();
    auto reply = rh->route(McGetRequest("a"));
    EXPECT_EQ(carbon::Result::NOTFOUND, *reply.result_ref());
  });

  rh = factory.create("MissFailoverRoute|NullRoute");
  EXPECT_TRUE(rh != nullptr);
  fm.run([&rh]() {
    auto reply = rh->route(McGetRequest("a"));
    EXPECT_EQ(carbon::Result::NOTFOUND, *reply.result_ref());
  });

  rh = factory.create("RandomRoute|ErrorRoute");
  EXPECT_TRUE(rh != nullptr);
  fm.run([&rh]() {
    auto reply = rh->route(McGetRequest("a"));
    EXPECT_TRUE(isErrorResult(*reply.result_ref()));
  });
}
