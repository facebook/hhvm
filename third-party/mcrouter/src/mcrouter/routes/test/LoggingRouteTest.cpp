/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "mcrouter/lib/test/TestRouteHandle.h"
#include "mcrouter/routes/LoggingRoute.h"
#include "mcrouter/routes/test/RouteHandleTestUtil.h"

using TestRouterInfo = facebook::memcache::TestRouterInfo;
using TestHandle =
    facebook::memcache::TestHandleImpl<facebook::memcache::TestRouteHandleIf>;
using LoggingRoute = facebook::memcache::mcrouter::LoggingRoute<TestRouterInfo>;

TEST(LoggingRouteTest, basic) {
  std::shared_ptr<TestHandle> test_handle = std::make_shared<TestHandle>(
      facebook::memcache::GetRouteTestData(carbon::Result::FOUND, "a"));

  auto rh = facebook::memcache::mcrouter::createLoggingRoute<TestRouterInfo>(
      test_handle->rh);
  facebook::memcache::McGetRequest req("key");
  req.setSourceIpAddr(folly::IPAddress("1.2.3.4"));

  facebook::memcache::TestFiberManager<facebook::memcache::TestRouterInfo> fm;
  fm.run([&]() {
    facebook::memcache::mcrouter::mockFiberContext();
    facebook::memcache::mcrouter::getTestRouter()->setPostprocessCallback(
        [](const folly::dynamic&,
           const folly::dynamic&,
           const char* const,
           const folly::StringPiece ip) { EXPECT_EQ("1.2.3.4", ip); });
    auto reply = rh->route(req);
    EXPECT_EQ("a", carbon::valueRangeSlow(reply).str());
  });
}
