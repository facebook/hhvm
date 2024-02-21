/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include <folly/json/dynamic.h>

#include "mcrouter/lib/FailoverErrorsSettings.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/routes/LatestRoute.h"
#include "mcrouter/routes/McrouterRouteHandle.h"
#include "mcrouter/routes/test/RouteHandleTestUtil.h"

using namespace facebook::memcache;
using namespace facebook::memcache::mcrouter;

using facebook::memcache::globals::HostidMock;
using std::make_shared;

TEST(latestRouteTest, one) {
  std::vector<std::shared_ptr<TestHandle>> test_handles{
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "a")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "b")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "c")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "d")),
  };

  mockFiberContext();
  folly::dynamic settings = folly::dynamic::object("failover_count", 3);
  auto rh = createLatestRoute<McrouterRouterInfo>(
      settings, get_route_handles(test_handles), 0);

  auto first = replyFor(*rh, "key")[0] - 'a';

  /* While first is good, will keep sending to it */
  EXPECT_EQ(std::string(1, 'a' + first), replyFor(*rh, "key"));
  EXPECT_EQ(std::string(1, 'a' + first), replyFor(*rh, "key"));
  test_handles[first]->setTko();
  /* first is TKO, send to other one */
  auto second = replyFor(*rh, "key")[0] - 'a';
  EXPECT_NE(first, second);
  test_handles[first]->unsetTko();
  test_handles[second]->setTko();
  /* first is not TKO */
  EXPECT_EQ(std::string(1, 'a' + first), replyFor(*rh, "key"));
  test_handles[first]->setTko();
  /* first and second are now TKO */
  auto third = replyFor(*rh, "key")[0] - 'a';
  EXPECT_NE(first, third);
  EXPECT_NE(second, third);
  test_handles[third]->setTko();
  /* three boxes are now TKO, we hit the failover limit */
  auto reply = rh->route(McGetRequest("key"));
  EXPECT_EQ(carbon::Result::TKO, *reply.result_ref());
}

TEST(latestRouteTest, weights) {
  HostidMock hostidMock(123);
  std::vector<std::shared_ptr<TestHandle>> test_handles{
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "a")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "b")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "c")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "d")),
  };

  mockFiberContext();
  folly::dynamic settings = folly::dynamic::object;
  settings["failover_count"] = 3;
  settings["thread_local_failover"] = true;
  settings["weights"] = folly::dynamic::array(.25, .5, .75, 1);
  std::vector<size_t> hits_per_index;
  hits_per_index.resize(4);
  for (int i = 0; i < 10000; i++) {
    auto rh = createLatestRoute<McrouterRouterInfo>(
        settings,
        get_route_handles(test_handles),
        /* threadId */ i);
    auto index = replyFor(*rh, "key")[0] - 'a';
    hits_per_index[index]++;
  }
  // The expected values below depend on the mocked host ID (123). The numbers
  // roughly conform to the weights - 25% ~1000, 50% ~2000, 75% ~3000,
  // and 100% ~4000
  EXPECT_EQ(959, hits_per_index[0]);
  EXPECT_EQ(1888, hits_per_index[1]);
  EXPECT_EQ(3073, hits_per_index[2]);
  EXPECT_EQ(4080, hits_per_index[3]);
}

TEST(latestRouteTest, thread_local_failover) {
  std::vector<std::shared_ptr<TestHandle>> test_handles{
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "a")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "b")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "c")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "d")),
  };

  mockFiberContext();
  folly::dynamic settings = folly::dynamic::object;
  settings["failover_count"] = 3;
  settings["thread_local_failover"] = true;
  // verify we don't always get the same index

  auto rh = createLatestRoute<McrouterRouterInfo>(
      settings,
      get_route_handles(test_handles),
      /* threadId */ 0);
  auto last_thread_reply = replyFor(*rh, "key");
  auto replies_differ = false;
  for (int i = 1; i < 10; i++) {
    rh = createLatestRoute<McrouterRouterInfo>(
        settings,
        get_route_handles(test_handles),
        /* threadId */ i);
    auto thread_reply = replyFor(*rh, "key");
    if (thread_reply != last_thread_reply) {
      replies_differ = true;
      break;
    }
    last_thread_reply = thread_reply;
  }
  EXPECT_TRUE(replies_differ);

  // Disable thread_local_failover
  settings["thread_local_failover"] = false;
  rh = createLatestRoute<McrouterRouterInfo>(
      settings,
      get_route_handles(test_handles),
      /* threadId */ 0);
  last_thread_reply = replyFor(*rh, "key");
  for (int i = 1; i < 10; i++) {
    rh = createLatestRoute<McrouterRouterInfo>(
        settings,
        get_route_handles(test_handles),
        /* threadId */ i);
    auto thread_reply = replyFor(*rh, "key");
    EXPECT_EQ(thread_reply, last_thread_reply);
    last_thread_reply = thread_reply;
  }
}

TEST(latestRouteTest, leasePairingNoName) {
  std::vector<std::shared_ptr<TestHandle>> test_handles{
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "a")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "b")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "c")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "d")),
  };

  mockFiberContext();
  folly::dynamic settings =
      folly::dynamic::object("enable_lease_pairing", true)("failover_count", 3);

  EXPECT_ANY_THROW({
    auto rh = createLatestRoute<McrouterRouterInfo>(
        settings, get_route_handles(test_handles), 0);
  });
}

TEST(latestRouteTest, leasePairingWithName) {
  std::vector<std::shared_ptr<TestHandle>> test_handles{
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "a")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "b")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "c")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "d")),
  };

  mockFiberContext();
  folly::dynamic settings = folly::dynamic::object(
      "enable_lease_pairing", true)("name", "01")("failover_count", 3);

  // Should not throw, as the name was provided
  auto rh = createLatestRoute<McrouterRouterInfo>(
      settings, get_route_handles(test_handles), 0);
}
