/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <algorithm>
#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "mcrouter/lib/carbon/example/gen/HelloGoodbyeRouterInfo.h"
#include "mcrouter/lib/test/RouteHandleTestUtil.h"
#include "mcrouter/lib/test/TestRouteHandle.h"

#include "mcrouter/routes/HashStopAllowListRoute.h"

#include "mcrouter/routes/test/RouteHandleTestBase.h"
#include "mcrouter/routes/test/RouteHandleTestUtil.h"

using namespace hellogoodbye;
using namespace facebook::memcache;
using namespace facebook::memcache::mcrouter;

using std::make_shared;
using std::vector;

namespace facebook::memcache::mcrouter {

using TestHandle = TestHandleImpl<MemcacheRouteHandleIf>;
using TestRouteHandle = std::shared_ptr<MemcacheRouteHandleIf>;
using HashStopAllowListRouteHandle =
    McrouterRouteHandle<HashStopAllowListRoute<MemcacheRouterInfo>>;

std::pair<std::vector<std::shared_ptr<TestHandle>>, TestRouteHandle>
getMockRouteHandle() {
  std::vector<std::shared_ptr<TestHandle>> handleVec{
      std::make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "a")),
  };
  return {std::move(handleVec), get_route_handles(handleVec)[0]};
}

TEST(HashStopAllowListRouteTest, NoMatch) {
  constexpr folly::StringPiece kRoutingConfig = R"(
  {
    "prefixes": [
        "foo",
        "baz"
    ]
  }
  )";

  auto [handleVec, mockRouteHandle] = getMockRouteHandle();
  auto rh = createHashStopAllowListRoute<MemcacheRouterInfo>(
      mockRouteHandle, folly::parseJson(kRoutingConfig));
  std::string key = "abc";

  McSetRequest reqSet(key);
  reqSet.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "value");

  auto reply = rh->route(reqSet);

  EXPECT_FALSE(handleVec[0]->saw_keys.empty());
  EXPECT_EQ(key, handleVec[0]->saw_keys[0]);
}

TEST(HashStopAllowListRouteTest, HashStopRejected) {
  constexpr folly::StringPiece kRoutingConfig = R"(
  {
    "prefixes": [
        "foo",
        "baz"
    ]
  }
  )";

  auto [handleVec, mockRouteHandle] = getMockRouteHandle();
  auto rh = createHashStopAllowListRoute<MemcacheRouterInfo>(
      mockRouteHandle, folly::parseJson(kRoutingConfig));
  std::string key = "abc|#|1";

  McSetRequest reqSet(key);
  reqSet.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "value");

  auto reply = rh->route(reqSet);

  EXPECT_EQ(carbon::Result::LOCAL_ERROR, *reply.result_ref());
  EXPECT_TRUE(handleVec[0]->saw_keys.empty());
}

TEST(HashStopAllowListRouteTest, HashStopAccepted) {
  constexpr folly::StringPiece kRoutingConfig = R"(
  {
    "prefixes": [
        "foo",
        "baz",
        "abc"
    ]
  }
  )";

  auto [handleVec, mockRouteHandle] = getMockRouteHandle();
  auto rh = createHashStopAllowListRoute<MemcacheRouterInfo>(
      mockRouteHandle, folly::parseJson(kRoutingConfig));
  std::string key = "abc|#|1";

  McSetRequest reqSet(key);
  reqSet.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "value");

  auto reply = rh->route(reqSet);

  EXPECT_FALSE(handleVec[0]->saw_keys.empty());
  EXPECT_EQ(key, handleVec[0]->saw_keys[0]);
}

TEST(HashStopAllowListRouteTest, HashStopAcceptAndReject) {
  constexpr folly::StringPiece kRoutingConfig = R"(
  {
    "prefixes": [
        "foo",
        "baz",
        "abc"
    ]
  }
  )";

  auto [handleVec, mockRouteHandle] = getMockRouteHandle();
  auto rh = createHashStopAllowListRoute<MemcacheRouterInfo>(
      mockRouteHandle, folly::parseJson(kRoutingConfig));
  std::string key = "zoo";

  McSetRequest reqSet(key);
  reqSet.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "ooz");
  auto reply = rh->route(reqSet);
  EXPECT_FALSE(handleVec[0]->saw_keys.empty());
  EXPECT_EQ(key, handleVec[0]->saw_keys[0]);

  key = "zoo|#|a";
  reqSet = McSetRequest(key);
  reqSet.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "b");
  reply = rh->route(reqSet);
  EXPECT_EQ(carbon::Result::LOCAL_ERROR, *reply.result_ref());

  key = "foo|#|a";
  reqSet = McSetRequest(key);
  reqSet.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "b");
  reply = rh->route(reqSet);
  EXPECT_FALSE(handleVec[0]->saw_keys.empty());
  EXPECT_EQ(key, handleVec[0]->saw_keys[1]);

  key = "baz|#|a";
  reqSet = McSetRequest(key);
  reqSet.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "b");
  reply = rh->route(reqSet);
  EXPECT_FALSE(handleVec[0]->saw_keys.empty());
  EXPECT_EQ(key, handleVec[0]->saw_keys[2]);

  key = "abc|#|a";
  reqSet = McSetRequest(key);
  reqSet.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "b");
  reply = rh->route(reqSet);
  EXPECT_FALSE(handleVec[0]->saw_keys.empty());
  EXPECT_EQ(key, handleVec[0]->saw_keys[3]);
}

} // namespace facebook::memcache::mcrouter
