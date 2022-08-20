/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "mcrouter/lib/HashFunctionType.h"
#include "mcrouter/lib/HashSelector.h"
#include "mcrouter/lib/mc/msg.h"
#include "mcrouter/lib/routes/AllAsyncRoute.h"
#include "mcrouter/lib/routes/AllFastestRoute.h"
#include "mcrouter/lib/routes/AllInitialRoute.h"
#include "mcrouter/lib/routes/AllMajorityRoute.h"
#include "mcrouter/lib/routes/AllSyncRoute.h"
#include "mcrouter/lib/routes/NullRoute.h"
#include "mcrouter/lib/routes/SelectionRoute.h"
#include "mcrouter/lib/test/AllSyncCollectionRoute.h"
#include "mcrouter/lib/test/RouteHandleTestUtil.h"
#include "mcrouter/lib/test/TestRouteHandle.h"

using namespace facebook::memcache;
using namespace facebook::memcache::mcrouter;

using std::make_shared;
using std::make_unique;
using std::string;
using std::vector;

using TestHandle = TestHandleImpl<TestRouteHandleIf>;

TEST(routeHandleTest, nullGet) {
  TestRouteHandle<NullRoute<TestRouteHandleIf>> rh;

  McGetRequest req("key");

  auto reply = rh.route(req);
  EXPECT_EQ(carbon::Result::NOTFOUND, *reply.result_ref());
}

TEST(routeHandleTest, nullSet) {
  TestRouteHandle<NullRoute<TestRouteHandleIf>> rh;
  McSetRequest req("key");
  req.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "value");
  auto reply = rh.route(std::move(req));
  EXPECT_EQ(carbon::Result::NOTSTORED, *reply.result_ref());
}

TEST(routeHandleTest, nullDelete) {
  TestRouteHandle<NullRoute<TestRouteHandleIf>> rh;
  auto reply = rh.route(McDeleteRequest("key"));
  EXPECT_EQ(carbon::Result::NOTFOUND, *reply.result_ref());
}

TEST(routeHandleTest, nullTouch) {
  TestRouteHandle<NullRoute<TestRouteHandleIf>> rh;
  auto reply = rh.route(McTouchRequest("key"));
  EXPECT_EQ(carbon::Result::NOTFOUND, *reply.result_ref());
}

TEST(routeHandleTest, nullIncr) {
  TestRouteHandle<NullRoute<TestRouteHandleIf>> rh;
  McIncrRequest req("key");
  req.delta_ref() = 1;
  auto reply = rh.route(std::move(req));
  EXPECT_EQ(carbon::Result::NOTFOUND, *reply.result_ref());
}

TEST(routeHandleTest, nullAppend) {
  TestRouteHandle<NullRoute<TestRouteHandleIf>> rh;
  McAppendRequest req("key");
  req.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "value");
  auto reply = rh.route(std::move(req));
  EXPECT_EQ(carbon::Result::NOTSTORED, *reply.result_ref());
}

TEST(routeHandleTest, nullPrepend) {
  TestRouteHandle<NullRoute<TestRouteHandleIf>> rh;
  McPrependRequest req("key");
  req.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "value");
  auto reply = rh.route(std::move(req));
  EXPECT_EQ(carbon::Result::NOTSTORED, *reply.result_ref());
}

TEST(routeHandleTest, allSync) {
  vector<std::shared_ptr<TestHandle>> test_handles{
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "a")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::NOTFOUND, "b")),
      make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::REMOTE_ERROR, "c"))};

  TestFiberManager fm;

  TestRouteHandle<AllSyncRoute<TestRouteHandleIf>> rh(
      get_route_handles(test_handles));

  fm.runAll({[&]() {
    auto reply = rh.route(McGetRequest("key"));

    /* Check that we got the worst result back */
    EXPECT_EQ(carbon::Result::REMOTE_ERROR, *reply.result_ref());
    EXPECT_EQ("c", carbon::valueRangeSlow(reply).str());

    for (auto& h : test_handles) {
      EXPECT_EQ(vector<string>{"key"}, h->saw_keys);
    }
  }});
}

TEST(routeHandleTest, allSyncTyped) {
  vector<std::shared_ptr<TestHandle>> test_handles{
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "a")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::NOTFOUND, "b")),
      make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::REMOTE_ERROR, "c"))};

  TestFiberManager fm;

  TestRouteHandle<AllSyncRoute<TestRouteHandleIf>> rh(
      get_route_handles(test_handles));

  fm.runAll({[&]() {
    McGetRequest req("key");

    auto reply = rh.route(req);

    /* Check that we got the worst result back */
    EXPECT_EQ(carbon::Result::REMOTE_ERROR, *reply.result_ref());
    EXPECT_EQ("c", coalesceAndGetRange(reply.value_ref()).str());

    for (auto& h : test_handles) {
      EXPECT_EQ(vector<string>{"key"}, h->saw_keys);
    }
  }});
}

TEST(routeHandleTest, allAsync) {
  vector<std::shared_ptr<TestHandle>> test_handles{
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "a")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::NOTFOUND, "b")),
      make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::REMOTE_ERROR, "c"))};

  TestFiberManager fm;

  TestRouteHandle<AllAsyncRoute<TestRouteHandleIf>> rh(
      get_route_handles(test_handles));

  fm.runAll({[&]() {
    auto reply = rh.route(McGetRequest("key"));

    /* Check that we got no result back */
    EXPECT_EQ(carbon::Result::NOTFOUND, *reply.result_ref());
  }});

  /* Check that everything is complete in the background */
  for (auto& h : test_handles) {
    EXPECT_EQ(vector<string>{"key"}, h->saw_keys);
  }
}

TEST(routeHandleTest, allInitial) {
  vector<std::shared_ptr<TestHandle>> test_handles{
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "a")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::NOTFOUND, "b")),
      make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::REMOTE_ERROR, "c")),
  };

  TestFiberManager fm;
  auto routeHandles = get_route_handles(test_handles);
  TestRouteHandle<AllInitialRoute<TestRouteHandleIf>> rh(routeHandles);

  fm.runAll({[&]() {
    auto reply = rh.route(McGetRequest("key"));

    /* Check that we got the initial result back */
    EXPECT_EQ(carbon::Result::FOUND, *reply.result_ref());
    EXPECT_EQ("a", carbon::valueRangeSlow(reply).str());
  }});

  /* Check that everything is complete in the background */
  for (auto& h : test_handles) {
    EXPECT_EQ(vector<string>{"key"}, h->saw_keys);
  }

  /* Check that traverse is correct */
  int cnt = 0;
  RouteHandleTraverser<TestRouteHandleIf> t{
      [&cnt](const TestRouteHandleIf&) { ++cnt; }};
  rh.traverse(McGetRequest("key"), t);
  EXPECT_EQ(cnt, routeHandles.size());
}

TEST(routeHandleTest, allMajority) {
  TestFiberManager fm;

  vector<std::shared_ptr<TestHandle>> test_handles{
      make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::REMOTE_ERROR, "a")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::NOTFOUND, "b")),
      make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::REMOTE_ERROR, "c"))};

  TestRouteHandle<AllMajorityRoute<TestRouteHandleIf>> rh(
      get_route_handles(test_handles));

  test_handles[1]->pause();

  fm.runAll({[&]() {
    auto reply = rh.route(McGetRequest("key"));

    /* Check that we got the majority reply
       without waiting for "b", which is paused */
    EXPECT_EQ(carbon::Result::REMOTE_ERROR, *reply.result_ref());

    EXPECT_EQ(vector<string>{"key"}, test_handles[0]->saw_keys);
    EXPECT_EQ(vector<string>{}, test_handles[1]->saw_keys);
    EXPECT_EQ(vector<string>{"key"}, test_handles[2]->saw_keys);

    test_handles[1]->unpause();
  }});

  /* Check that everything is complete in the background */
  for (auto& h : test_handles) {
    EXPECT_EQ(vector<string>{"key"}, h->saw_keys);
  }
}

TEST(routeHandleTest, allMajorityTie) {
  TestFiberManager fm;

  vector<std::shared_ptr<TestHandle>> test_handles{
      make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::REMOTE_ERROR, "a")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::NOTFOUND, "b")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::NOTFOUND, "c")),
      make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::REMOTE_ERROR, "d"))};

  TestRouteHandle<AllMajorityRoute<TestRouteHandleIf>> rh(
      get_route_handles(test_handles));

  fm.runAll({[&]() {
    auto reply = rh.route(McGetRequest("key"));

    /* Check that we got the _worst_ majority reply */
    EXPECT_EQ(carbon::Result::REMOTE_ERROR, *reply.result_ref());
  }});

  /* Check that everything is complete */
  for (auto& h : test_handles) {
    EXPECT_EQ(vector<string>{"key"}, h->saw_keys);
  }
}

TEST(routeHandleTest, allFastest) {
  TestFiberManager fm;

  vector<std::shared_ptr<TestHandle>> test_handles{
      make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::REMOTE_ERROR, "a")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::NOTFOUND, "b")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "c"))};

  TestRouteHandle<AllFastestRoute<TestRouteHandleIf>> rh(
      get_route_handles(test_handles));

  test_handles[1]->pause();

  fm.runAll({[&]() {
    auto reply = rh.route(McGetRequest("key"));

    /* Check that we got the fastest non-error result back
       ('b' is paused) */
    EXPECT_EQ(carbon::Result::FOUND, *reply.result_ref());
    EXPECT_EQ("c", carbon::valueRangeSlow(reply).str());

    EXPECT_EQ(vector<string>{"key"}, test_handles[0]->saw_keys);
    EXPECT_EQ(vector<string>{}, test_handles[1]->saw_keys);
    EXPECT_EQ(vector<string>{"key"}, test_handles[2]->saw_keys);

    test_handles[1]->unpause();
  }});

  /* Check that everything is complete in the background */
  for (auto& h : test_handles) {
    EXPECT_EQ(vector<string>{"key"}, h->saw_keys);
  }
}

class HashFunc {
 public:
  explicit HashFunc(size_t n) : n_(n) {}

  size_t operator()(folly::StringPiece key) const {
    return std::stoi(key.str()) % n_;
  }

  static std::string type() {
    return "HashFunc";
  }

  static HashFunctionType typeId() {
    return HashFunctionType::Unknown;
  }

 private:
  size_t n_;
};

TEST(routeHandleTest, hashNoSalt) {
  vector<std::shared_ptr<TestHandle>> test_handles{
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "a")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "b")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "c")),
  };
  auto outOfRangeRh = createNullRoute<typename TestRouterInfo::RouteHandleIf>();

  TestFiberManager fm;

  TestRouteHandle<SelectionRoute<TestRouterInfo, HashSelector<HashFunc>>> rh(
      get_route_handles(test_handles),
      HashSelector<HashFunc>(/* salt= */ "", HashFunc(test_handles.size())),
      std::move(outOfRangeRh));

  fm.run([&]() {
    auto reply = rh.route(McGetRequest("0"));
    EXPECT_EQ("a", carbon::valueRangeSlow(reply).str());
  });

  fm.run([&]() {
    auto reply = rh.route(McGetRequest("1"));
    EXPECT_EQ("b", carbon::valueRangeSlow(reply).str());
  });

  fm.run([&]() {
    auto reply = rh.route(McGetRequest("2"));
    EXPECT_EQ("c", carbon::valueRangeSlow(reply).str());
  });
}

TEST(routeHandleTest, hashSalt) {
  vector<std::shared_ptr<TestHandle>> test_handles{
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "a")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "b")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "c")),
  };
  auto outOfRangeRh = createNullRoute<typename TestRouterInfo::RouteHandleIf>();

  TestFiberManager fm;

  TestRouteHandle<SelectionRoute<TestRouterInfo, HashSelector<HashFunc>>> rh(
      get_route_handles(test_handles),
      HashSelector<HashFunc>(/* salt= */ "1", HashFunc(test_handles.size())),
      std::move(outOfRangeRh));

  fm.run([&]() {
    auto reply = rh.route(McGetRequest("0"));
    /* 01 % 3 == 1 */
    EXPECT_EQ("b", carbon::valueRangeSlow(reply).str());
  });

  fm.run([&]() {
    auto reply = rh.route(McGetRequest("1"));
    /* 11 % 3 == 2 */
    EXPECT_EQ("c", carbon::valueRangeSlow(reply).str());
  });

  fm.run([&]() {
    auto reply = rh.route(McGetRequest("2"));
    /* 21 % 3 == 0 */
    EXPECT_EQ("a", carbon::valueRangeSlow(reply).str());
  });
}

TEST(routeHandleTest, allSyncCollector) {
  vector<std::shared_ptr<TestHandle>> test_handles{
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::FOUND, "a")),
      make_shared<TestHandle>(GetRouteTestData(carbon::Result::NOTFOUND, "b")),
      make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::REMOTE_ERROR, "c"))};

  TestFiberManager fm;

  TestRouteHandle<AllSyncCollectionRoute<TestRouterInfo>> rh(
      get_route_handles(test_handles));

  fm.runAll({[&]() {
    auto reply = rh.route(McGetRequest("key"));

    /* Check that we got the worst result back */
    EXPECT_EQ(carbon::Result::REMOTE_ERROR, *reply.result_ref());
    EXPECT_EQ("c", carbon::valueRangeSlow(reply).str());

    for (auto& h : test_handles) {
      EXPECT_EQ(vector<string>{"key"}, h->saw_keys);
    }
  }});
}
