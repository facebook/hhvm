/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>
#include <random>
#include <vector>

#include <gtest/gtest.h>

#include <folly/json/dynamic.h>

#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/routes/DefaultShadowPolicy.h"
#include "mcrouter/routes/ShadowRoute.h"
#include "mcrouter/routes/ShadowRouteIf.h"
#include "mcrouter/routes/test/RouteHandleTestUtil.h"

using namespace facebook::memcache;
using namespace facebook::memcache::mcrouter;

using std::make_shared;
using std::string;
using std::vector;

TEST(shadowRouteTest, defaultPolicy) {
  vector<std::shared_ptr<TestHandle>> normalHandle{
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "a")),
  };
  auto normalRh = get_route_handles(normalHandle)[0];

  vector<std::shared_ptr<TestHandle>> shadowHandles{
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "b")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "c")),
  };

  TestFiberManager<McrouterRouterInfo> fm;

  auto settings = ShadowSettings::create(
      folly::dynamic::object("index_range", folly::dynamic::array(0, 1)),
      *getTestRouter());

  auto shadowRhs = get_route_handles(shadowHandles);
  McrouterShadowData shadowData{
      {std::move(shadowRhs[0]), settings},
      {std::move(shadowRhs[1]), settings},
  };

  McrouterRouteHandle<ShadowRoute<McrouterRouterInfo, DefaultShadowPolicy>> rh(
      normalRh, std::move(shadowData), DefaultShadowPolicy());

  fm.run([&]() {
    mockFiberContext();
    auto reply = rh.route(McGetRequest("key"));

    EXPECT_EQ(carbon::Result::FOUND, *reply.result_ref());
    EXPECT_EQ("a", carbon::valueRangeSlow(reply).str());
  });

  EXPECT_TRUE(shadowHandles[0]->saw_keys.empty());
  EXPECT_TRUE(shadowHandles[1]->saw_keys.empty());
  settings->setKeyRange(0, 1);

  fm.run([&]() {
    mockFiberContext();
    auto reply = rh.route(McGetRequest("key"));

    EXPECT_EQ(carbon::Result::FOUND, *reply.result_ref());
    EXPECT_EQ("a", carbon::valueRangeSlow(reply).str());
  });

  EXPECT_EQ(shadowHandles[0]->saw_keys, vector<string>{"key"});
  EXPECT_EQ(shadowHandles[1]->saw_keys, vector<string>{"key"});
}
