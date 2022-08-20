/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "mcrouter/lib/FailoverErrorsSettings.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/routes/FailoverRateLimiter.h"
#include "mcrouter/routes/FailoverWithExptimeRouteFactory.h"
#include "mcrouter/routes/McrouterRouteHandle.h"
#include "mcrouter/routes/test/RouteHandleTestUtil.h"

using namespace facebook::memcache;
using namespace facebook::memcache::mcrouter;

using std::make_shared;
using std::vector;

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace {
using FiberManagerContextTag =
    typename fiber_local<MemcacheRouterInfo>::ContextTypeTag;
} // namespace
} // namespace mcrouter
} // namespace memcache
} // namespace facebook

TEST(failoverWithExptimeRouteTest, success) {
  std::vector<std::shared_ptr<TestHandle>> normalHandle{
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "a")),
  };
  auto normalRh = get_route_handles(normalHandle);
  std::vector<std::shared_ptr<TestHandle>> failoverHandles{
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "b")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "c"))};

  auto rh = createFailoverWithExptimeRoute<McrouterRouterInfo>(
      normalRh[0],
      get_route_handles(failoverHandles),
      2,
      FailoverErrorsSettings(),
      nullptr);

  TestFiberManager fm{FiberManagerContextTag()};
  fm.run([&] {
    mockFiberContext();
    auto reply = rh->route(McGetRequest("0"));
    EXPECT_EQ("a", carbon::valueRangeSlow(reply).str());
    EXPECT_EQ(vector<uint32_t>{0}, normalHandle[0]->sawExptimes);
  });
}

TEST(failoverWithExptimeRouteTest, once) {
  std::vector<std::shared_ptr<TestHandle>> normalHandle{
      make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::TIMEOUT, "a"),
          UpdateRouteTestData(),
          DeleteRouteTestData(carbon::Result::TIMEOUT)),
  };
  auto normalRh = get_route_handles(normalHandle);
  std::vector<std::shared_ptr<TestHandle>> failoverHandles{
      make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "b"),
          UpdateRouteTestData(),
          DeleteRouteTestData(carbon::Result::NOTFOUND)),
      make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "c"),
          UpdateRouteTestData(),
          DeleteRouteTestData(carbon::Result::NOTFOUND))};

  auto rh = createFailoverWithExptimeRoute<McrouterRouterInfo>(
      normalRh[0],
      get_route_handles(failoverHandles),
      2,
      FailoverErrorsSettings(),
      nullptr);

  TestFiberManager fm{FiberManagerContextTag()};
  fm.run([&] {
    mockFiberContext();
    auto reply = rh->route(McGetRequest("0"));
    EXPECT_EQ("b", carbon::valueRangeSlow(reply).str());

    auto reply2 = rh->route(McDeleteRequest("1"));
    EXPECT_EQ(carbon::Result::NOTFOUND, *reply2.result_ref());
    EXPECT_EQ(vector<uint32_t>({0, 0}), normalHandle[0]->sawExptimes);
    EXPECT_EQ(vector<uint32_t>({0, 0}), failoverHandles[0]->sawExptimes);
    EXPECT_EQ(vector<uint32_t>{}, failoverHandles[1]->sawExptimes);
  });
}

TEST(failoverWithExptimeRouteTest, twice) {
  std::vector<std::shared_ptr<TestHandle>> normalHandle{
      make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::TIMEOUT, "a"),
          UpdateRouteTestData(),
          DeleteRouteTestData(carbon::Result::TIMEOUT)),
  };
  auto normalRh = get_route_handles(normalHandle);
  std::vector<std::shared_ptr<TestHandle>> failoverHandles{
      make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::TIMEOUT, "b"),
          UpdateRouteTestData(),
          DeleteRouteTestData(carbon::Result::TIMEOUT)),
      make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "c"),
          UpdateRouteTestData(),
          DeleteRouteTestData(carbon::Result::NOTFOUND))};

  auto rh = createFailoverWithExptimeRoute<McrouterRouterInfo>(
      normalRh[0],
      get_route_handles(failoverHandles),
      2,
      FailoverErrorsSettings(),
      nullptr);

  TestFiberManager fm{FiberManagerContextTag()};
  fm.run([&] {
    mockFiberContext();
    auto reply = rh->route(McGetRequest("0"));
    EXPECT_EQ("c", carbon::valueRangeSlow(reply).str());

    auto reply2 = rh->route(McDeleteRequest("1"));
    EXPECT_EQ(carbon::Result::NOTFOUND, *reply2.result_ref());
    EXPECT_EQ(vector<uint32_t>({0, 0}), normalHandle[0]->sawExptimes);
    EXPECT_EQ(vector<uint32_t>({0, 0}), failoverHandles[0]->sawExptimes);
    EXPECT_EQ(vector<uint32_t>({0, 0}), failoverHandles[1]->sawExptimes);
  });
}

TEST(failoverWithExptimeRouteTest, fail) {
  std::vector<std::shared_ptr<TestHandle>> normalHandle{
      make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::TIMEOUT, "a"),
          UpdateRouteTestData(),
          DeleteRouteTestData(carbon::Result::TIMEOUT)),
  };
  auto normalRh = get_route_handles(normalHandle);
  std::vector<std::shared_ptr<TestHandle>> failoverHandles{
      make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::TIMEOUT, "b"),
          UpdateRouteTestData(),
          DeleteRouteTestData(carbon::Result::TIMEOUT)),
      make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::TIMEOUT, "c"),
          UpdateRouteTestData(),
          DeleteRouteTestData(carbon::Result::TIMEOUT))};

  auto rh = createFailoverWithExptimeRoute<McrouterRouterInfo>(
      normalRh[0],
      get_route_handles(failoverHandles),
      2,
      FailoverErrorsSettings(),
      nullptr);

  TestFiberManager fm{FiberManagerContextTag()};
  fm.run([&] {
    mockFiberContext();
    auto reply = rh->route(McGetRequest("0"));

    /* Will return the last reply when ran out of targets */
    EXPECT_EQ("c", carbon::valueRangeSlow(reply).str());

    auto reply2 = rh->route(McDeleteRequest("1"));
    EXPECT_EQ(carbon::Result::TIMEOUT, *reply2.result_ref());
    EXPECT_EQ(vector<uint32_t>({0, 0}), normalHandle[0]->sawExptimes);
    EXPECT_EQ(vector<uint32_t>({0, 0}), failoverHandles[0]->sawExptimes);
    EXPECT_EQ(vector<uint32_t>({0, 0}), failoverHandles[1]->sawExptimes);
  });
}

void testFailoverGet(carbon::Result res) {
  std::vector<std::shared_ptr<TestHandle>> normalHandle{
      make_shared<TestHandle>(GetRouteTestData(res, "a")),
  };
  auto normalRh = get_route_handles(normalHandle);
  std::vector<std::shared_ptr<TestHandle>> failoverHandles{
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "b")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "c"))};

  auto rhNoFail = createFailoverWithExptimeRoute<McrouterRouterInfo>(
      normalRh[0],
      get_route_handles(failoverHandles),
      2,
      FailoverErrorsSettings(std::vector<std::string>{}),
      nullptr);

  TestFiberManager fm{FiberManagerContextTag()};
  fm.run([&] {
    mockFiberContext();
    auto reply = rhNoFail->route(McGetRequest("0"));
    EXPECT_EQ("a", carbon::valueRangeSlow(reply).str());
    EXPECT_EQ(vector<uint32_t>{0}, normalHandle[0]->sawExptimes);
  });

  auto rhFail = createFailoverWithExptimeRoute<McrouterRouterInfo>(
      normalRh[0],
      get_route_handles(failoverHandles),
      2,
      FailoverErrorsSettings(),
      nullptr);

  fm.run([&] {
    mockFiberContext();
    auto reply = rhFail->route(McGetRequest("0"));
    EXPECT_EQ("b", carbon::valueRangeSlow(reply).str());
  });
}

void testFailoverUpdate(carbon::Result res) {
  std::vector<std::shared_ptr<TestHandle>> normalHandle{
      make_shared<TestHandle>(UpdateRouteTestData(res)),
  };
  auto normalRh = get_route_handles(normalHandle);
  std::vector<std::shared_ptr<TestHandle>> failoverHandles{
      make_shared<TestHandle>(UpdateRouteTestData(carbon::Result::STORED)),
      make_shared<TestHandle>(UpdateRouteTestData(carbon::Result::STORED))};

  auto rhNoFail = createFailoverWithExptimeRoute<McrouterRouterInfo>(
      normalRh[0],
      get_route_handles(failoverHandles),
      2,
      FailoverErrorsSettings(std::vector<std::string>{}),
      nullptr);

  TestFiberManager fm{FiberManagerContextTag()};
  fm.run([&] {
    mockFiberContext();
    McSetRequest req("0");
    req.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "a");
    auto reply = rhNoFail->route(std::move(req));
    EXPECT_EQ(res, *reply.result_ref());
    EXPECT_EQ(vector<uint32_t>{0}, normalHandle[0]->sawExptimes);
    // only normal handle sees the key
    EXPECT_EQ(vector<std::string>{"0"}, normalHandle[0]->saw_keys);
    EXPECT_EQ(0, failoverHandles[0]->saw_keys.size());
    EXPECT_EQ(0, failoverHandles[1]->saw_keys.size());
  });

  auto rhFail = createFailoverWithExptimeRoute<McrouterRouterInfo>(
      normalRh[0],
      get_route_handles(failoverHandles),
      2,
      FailoverErrorsSettings(),
      nullptr);

  fm.run([&] {
    mockFiberContext();
    McSetRequest req("0");
    req.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "a");
    auto reply = rhFail->route(std::move(req));

    EXPECT_EQ(carbon::Result::STORED, *reply.result_ref());
    EXPECT_EQ(1, failoverHandles[0]->saw_keys.size());
    EXPECT_EQ(0, failoverHandles[1]->saw_keys.size());
  });
}

void testFailoverDelete(carbon::Result res) {
  std::vector<std::shared_ptr<TestHandle>> normalHandle{
      make_shared<TestHandle>(DeleteRouteTestData(res)),
  };
  auto normalRh = get_route_handles(normalHandle);
  std::vector<std::shared_ptr<TestHandle>> failoverHandles{
      make_shared<TestHandle>(DeleteRouteTestData(carbon::Result::DELETED)),
      make_shared<TestHandle>(DeleteRouteTestData(carbon::Result::DELETED))};

  auto rhNoFail = createFailoverWithExptimeRoute<McrouterRouterInfo>(
      normalRh[0],
      get_route_handles(failoverHandles),
      2,
      FailoverErrorsSettings(std::vector<std::string>{}),
      nullptr);

  TestFiberManager fm{FiberManagerContextTag()};
  fm.run([&] {
    mockFiberContext();
    McDeleteRequest req("0");
    auto reply = rhNoFail->route(McDeleteRequest("0"));
    EXPECT_EQ(vector<uint32_t>{0}, normalHandle[0]->sawExptimes);
    // only normal handle sees the key
    EXPECT_EQ(vector<std::string>{"0"}, normalHandle[0]->saw_keys);
    EXPECT_EQ(0, failoverHandles[0]->saw_keys.size());
    EXPECT_EQ(0, failoverHandles[1]->saw_keys.size());
  });

  auto rhFail = createFailoverWithExptimeRoute<McrouterRouterInfo>(
      normalRh[0],
      get_route_handles(failoverHandles),
      2,
      FailoverErrorsSettings(),
      nullptr);

  fm.run([&] {
    mockFiberContext();
    auto reply = rhFail->route(McDeleteRequest("0"));
    EXPECT_EQ(1, failoverHandles[0]->saw_keys.size());
    EXPECT_EQ(0, failoverHandles[1]->saw_keys.size());
  });
}

TEST(failoverWithExptimeRouteTest, noFailoverOnConnectTimeout) {
  testFailoverGet(carbon::Result::CONNECT_TIMEOUT);
  testFailoverUpdate(carbon::Result::CONNECT_TIMEOUT);
  testFailoverDelete(carbon::Result::CONNECT_TIMEOUT);
}

TEST(failoverWithExptimeRouteTest, noFailoverOnDataTimeout) {
  testFailoverGet(carbon::Result::TIMEOUT);
  testFailoverUpdate(carbon::Result::TIMEOUT);
  testFailoverDelete(carbon::Result::TIMEOUT);
}

TEST(failoverWithExptimeRouteTest, noFailoverOnTko) {
  testFailoverGet(carbon::Result::TKO);
  testFailoverUpdate(carbon::Result::TKO);
  testFailoverDelete(carbon::Result::TKO);
}

TEST(failoverWithExptimeRouteTest, noFailoverOnArithmetic) {
  std::vector<std::shared_ptr<TestHandle>> normalHandle{
      make_shared<TestHandle>(
          UpdateRouteTestData(carbon::Result::CONNECT_TIMEOUT)),
  };
  auto normalRh = get_route_handles(normalHandle);
  std::vector<std::shared_ptr<TestHandle>> failoverHandles{
      make_shared<TestHandle>(UpdateRouteTestData(carbon::Result::STORED)),
      make_shared<TestHandle>(UpdateRouteTestData(carbon::Result::STORED))};

  auto rh = createFailoverWithExptimeRoute<McrouterRouterInfo>(
      normalRh[0],
      get_route_handles(failoverHandles),
      2,
      FailoverErrorsSettings(),
      nullptr);

  TestFiberManager fm{FiberManagerContextTag()};
  fm.run([&] {
    mockFiberContext();
    McIncrRequest req("0");
    req.delta_ref() = 1;
    auto reply = rh->route(std::move(req));

    EXPECT_EQ(vector<uint32_t>{0}, normalHandle[0]->sawExptimes);
    // only normal handle sees the key
    EXPECT_EQ(vector<std::string>{"0"}, normalHandle[0]->saw_keys);
    EXPECT_EQ(0, failoverHandles[0]->saw_keys.size());
    EXPECT_EQ(0, failoverHandles[1]->saw_keys.size());
  });
}
