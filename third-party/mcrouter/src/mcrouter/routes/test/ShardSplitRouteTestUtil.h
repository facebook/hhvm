/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/routes/ShardSplitRoute.h"
#include "mcrouter/routes/test/RouteHandleTestUtil.h"

namespace facebook {
namespace memcache {
namespace mcrouter {
namespace test {

using FiberManagerContextTag =
    typename fiber_local<MemcacheRouterInfo>::ContextTypeTag;

constexpr size_t kNumSplits = 26 * 26 + 1;

template <class Request, class RouterInfo = MemcacheRouterInfo>
void testShardingForOp(ShardSplitter splitter, uint64_t requestFlags = 0) {
  using ShardSplitTestHandle =
      TestHandleImpl<typename RouterInfo::RouteHandleIf>;
  using ShardSplitRouteHandle =
      typename RouterInfo::template RouteHandle<ShardSplitRoute<RouterInfo>>;

  for (size_t i = 0; i < kNumSplits; ++i) {
    globals::HostidMock hostidMock(i);

    std::vector<std::shared_ptr<ShardSplitTestHandle>> handles{
        std::make_shared<ShardSplitTestHandle>(
            GetRouteTestData(carbon::Result::FOUND, "a"),
            UpdateRouteTestData(carbon::Result::FOUND),
            DeleteRouteTestData(carbon::Result::FOUND))};
    auto rh = get_route_handles(handles)[0];
    ShardSplitRouteHandle splitRoute(rh, splitter);

    TestFiberManager fm{FiberManagerContextTag()};
    fm.run([&] {
      mockFiberContext<RouterInfo>();
      Request req("test:123:");
      req.flags_ref() = requestFlags;
      auto reply = splitRoute.route(req);
      EXPECT_EQ(carbon::Result::FOUND, *reply.result_ref());
    });

    if (i == 0) {
      EXPECT_EQ(std::vector<std::string>{"test:123:"}, handles[0]->saw_keys);
    } else {
      EXPECT_EQ(
          std::vector<std::string>{folly::sformat(
              "test:123{}{}:",
              (char)('a' + (i - 1) % 26),
              (char)('a' + (i - 1) / 26))},
          handles[0]->saw_keys);
    }
  }
}

} // namespace test
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
