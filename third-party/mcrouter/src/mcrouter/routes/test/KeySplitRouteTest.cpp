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

#include "mcrouter/routes/KeySplitRoute.h"
#include "mcrouter/routes/McrouterRouteHandle.h"
#include "mcrouter/routes/test/RouteHandleTestBase.h"

using namespace hellogoodbye;
using namespace facebook::memcache;
using namespace facebook::memcache::mcrouter;

using std::make_shared;
using std::vector;

namespace facebook {
namespace memcache {
namespace mcrouter {

using TestHandle = TestHandleImpl<MemcacheRouteHandleIf>;
using RouteHandle = McrouterRouteHandle<KeySplitRoute<MemcacheRouterInfo>>;

class KeySplitRouteTest : public RouteHandleTestBase<MemcacheRouterInfo> {
 public:
  std::shared_ptr<MemcacheRouteHandleIf>
  getKeySplitRoute(size_t numReplicas, bool allSync, bool firstHit = false) {
    const auto jsonStr = fmt::format(
        R"TMPL(
              {{
                "replicas": {numReplicas},
                "all_sync": {allSync},
                "first_hit": {firstHit},
                "destination": "NullRoute"
              }}
              )TMPL",
        fmt::arg(
            "numReplicas",
            folly::to<std::string>(static_cast<int>(numReplicas))),
        fmt::arg("allSync", allSync),
        fmt::arg("firstHit", firstHit));
    return makeKeySplitRoute<MemcacheRouterInfo>(
        rhFactory_, folly::parseJson(jsonStr));
  }

  // This helper function is used to test `makeKeySplitRoute` and that the route
  // can be initialized
  bool testCreateForNumReplicas(
      size_t numReplicas,
      bool allSync,
      bool firstHit = false) {
    if (testRh_)
      testRh_.reset();
    try {
      testRh_ = getKeySplitRoute(numReplicas, allSync, firstHit);
    } catch (const std::exception&) {
      return false;
    }
    return true;
  }

  void testCreate(size_t numReplicas, bool allSync, bool firstHit = false) {
    // set up the route handle under test
    if (th_)
      th_.reset();
    if (rh_)
      rh_.reset();
    th_ = std::make_shared<TestHandle>(
        GetRouteTestData(carbon::Result::FOUND, "a"));
    rh_ = std::make_shared<RouteHandle>(
        RouteHandle(th_->rh, numReplicas, allSync, firstHit));
    replicas_ = numReplicas;
    allSync_ = allSync;
    firstHit_ = firstHit;
  }

  template <class Request>
  void testLeases(const Request& req, size_t numReplicas = 10) {
    // lease operations with allsync enabled should never broadcast.
    // it should behave the same as if allsync was not set.
    for (size_t j = 0; j < 2; ++j) {
      // test both allsync and non-allsync
      testCreate(numReplicas, j == 0);

      for (size_t i = 0; i < numReplicas; ++i) {
        globals::HostidMock hostidMock(i);

        // submit get based on our replica
        auto reply = rh_->route(req);

        // verify
        std::string expectedKeyReplica;
        if (req.key_ref()->hasHashStop()) {
          expectedKeyReplica = genSplitKeyWitHashStop(req, i);
        } else {
          expectedKeyReplica = folly::to<std::string>(
              req.key_ref()->fullKey(),
              kMemcacheReplicaSeparator,
              i % numReplicas);
        }
        EXPECT_FALSE(th_->saw_keys.empty());

        // first replica is the original key
        if (i == 0) {
          EXPECT_EQ(req.key_ref()->fullKey(), th_->saw_keys[0]);
        } else {
          EXPECT_EQ(expectedKeyReplica, th_->saw_keys[0]);
        }

        // reset
        th_->saw_keys.clear();
      }
    }
  }

  template <class Request>
  void testAllSync(const Request& req, size_t hostid, size_t numReplicas = 10) {
    globals::HostidMock hostidMock(hostid);
    std::string key = folly::to<std::string>(req.key_ref()->fullKey());

    // create expected keys, the first key is not modified.
    std::vector<std::string> expectedKeys;
    if (req.key_ref()->hasHashStop()) {
      for (size_t i = 1; i < numReplicas; ++i) {
        expectedKeys.push_back(genSplitKeyWitHashStop(req, i));
      }
      // Changing the order for hashstops to enable the seen keys to be sorted
      expectedKeys.push_back(key);
    } else {
      expectedKeys.push_back(key);
      for (size_t i = 1; i < numReplicas; ++i) {
        expectedKeys.push_back(
            folly::to<std::string>(key, kMemcacheReplicaSeparator, i));
      }
    }

    // all sync req will be done through fibers
    TestFiberManager<MemcacheRouterInfo> fm;
    fm.runAll({[&]() { auto reply = rh_->route(req); }});

    // verify
    EXPECT_FALSE(th_->saw_keys.empty());
    std::sort(th_->saw_keys.begin(), th_->saw_keys.end());
    EXPECT_EQ(vector<std::string>{expectedKeys}, th_->saw_keys);

    // reset
    th_->saw_keys.clear();
  }

  template <class Request>
  std::string genSplitKeyWitHashStop(const Request& req, size_t replica) {
    return folly::to<std::string>(
        req.key_ref()->routingKey(),
        kMemcacheReplicaSeparator,
        replica,
        req.key_ref()->afterRoutingKey());
  }

  static constexpr folly::StringPiece kMemcacheReplicaSeparator = "::";
  std::shared_ptr<TestHandle> th_;
  std::shared_ptr<RouteHandle> rh_;
  std::shared_ptr<MemcacheRouteHandleIf> testRh_;
  folly::StringPiece key_;
  size_t replicas_;
  bool allSync_;
  bool firstHit_;
};

constexpr folly::StringPiece KeySplitRouteTest::kMemcacheReplicaSeparator;

TEST_F(KeySplitRouteTest, NoAllSyncSet) {
  size_t numReplicas = 10;
  testCreate(numReplicas, false);
  std::string key = "abc";

  for (size_t i = 0; i < numReplicas; ++i) {
    globals::HostidMock hostidMock(i);

    // single set should not use the fiber manager
    McSetRequest reqSet(key);
    reqSet.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "value");

    auto reply = rh_->route(reqSet);

    // verify
    auto expectedKeyReplica =
        folly::to<std::string>(key, kMemcacheReplicaSeparator, i % numReplicas);
    EXPECT_FALSE(th_->saw_keys.empty());
    // first replica is the original key
    if (i == 0) {
      EXPECT_EQ(key, th_->saw_keys[0]);
    } else {
      EXPECT_EQ(expectedKeyReplica, th_->saw_keys[0]);
    }

    th_->saw_keys.clear();
  }
}

TEST_F(KeySplitRouteTest, NoAllSyncSetHashStop) {
  size_t numReplicas = 10;
  testCreate(numReplicas, false);
  std::string routingKey = "abc";
  std::string keySuffix = "|#|ignore_me";
  std::string key = routingKey + keySuffix;

  for (size_t i = 0; i < numReplicas; ++i) {
    globals::HostidMock hostidMock(i);

    // single set should not use the fiber manager
    McSetRequest reqSet(key);
    reqSet.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "value");

    auto reply = rh_->route(reqSet);

    // verify
    auto expectedKeyReplica = folly::to<std::string>(
        routingKey, kMemcacheReplicaSeparator, i % numReplicas, keySuffix);
    EXPECT_FALSE(th_->saw_keys.empty());
    // first replica is the original key
    if (i == 0) {
      EXPECT_EQ(key, th_->saw_keys[0]);
    } else {
      EXPECT_EQ(expectedKeyReplica, th_->saw_keys[0]);
    }

    th_->saw_keys.clear();
  }
}

TEST_F(KeySplitRouteTest, AllSyncSet) {
  testCreate(10, true);

  std::string key = "abc";
  McSetRequest reqSet(key);
  reqSet.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "value");

  for (size_t i = 0; i < 10; ++i) {
    testAllSync(reqSet, i);
  }
}

TEST_F(KeySplitRouteTest, AllSyncSetHashStop) {
  testCreate(10, true);

  std::string routingKey = "abc";
  std::string keySuffix = "|#|ignore_me";
  std::string key = routingKey + keySuffix;
  McSetRequest reqSet(key);
  reqSet.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "value");

  for (size_t i = 0; i < 10; ++i) {
    testAllSync(reqSet, i);
  }
}

TEST_F(KeySplitRouteTest, Get) {
  for (size_t j = 0; j < 2; ++j) {
    size_t numReplicas = 10;
    constexpr folly::StringPiece key = "abc";
    testCreate(numReplicas, j == 0);

    for (size_t i = 0; i < numReplicas; ++i) {
      globals::HostidMock hostidMock(i);

      // submit get based on our replica
      auto reply = rh_->route(McGetRequest(key));

      // verify
      auto expectedKeyReplica = folly::to<std::string>(
          key, kMemcacheReplicaSeparator, i % numReplicas);
      EXPECT_FALSE(th_->saw_keys.empty());

      // first replica is the original key
      if (i == 0) {
        EXPECT_EQ(key, th_->saw_keys[0]);
      } else {
        EXPECT_EQ(expectedKeyReplica, th_->saw_keys[0]);
      }

      th_->saw_keys.clear();
    }
  }
}

TEST_F(KeySplitRouteTest, GetWithHashStop) {
  for (size_t j = 0; j < 2; ++j) {
    size_t numReplicas = 10;
    std::string routingKey = "abc";
    std::string keySuffix = "|#|ignore_me";
    std::string key = routingKey + keySuffix;
    testCreate(numReplicas, j == 0);

    for (size_t i = 0; i < numReplicas; ++i) {
      globals::HostidMock hostidMock(i);

      // submit get based on our replica
      auto reply = rh_->route(McGetRequest(key));

      // verify
      auto expectedKeyReplica = folly::to<std::string>(
          routingKey, kMemcacheReplicaSeparator, i % numReplicas, keySuffix);
      EXPECT_FALSE(th_->saw_keys.empty());

      // first replica is the original key
      if (i == 0) {
        EXPECT_EQ(key, th_->saw_keys[0]);
      } else {
        EXPECT_EQ(expectedKeyReplica, th_->saw_keys[0]);
      }

      th_->saw_keys.clear();
    }
  }
}

TEST_F(KeySplitRouteTest, DeleteAllSync) {
  // test both allsync and non-allsync
  for (size_t mode = 0; mode < 2; ++mode) {
    testCreate(10, mode == 0);

    std::string key = "abc";
    McDeleteRequest req(key);

    for (size_t i = 0; i < 10; ++i) {
      testAllSync(req, i);
    }
  }
}

TEST_F(KeySplitRouteTest, DeleteAllSyncHashStop) {
  // test both allsync and non-allsync
  for (size_t mode = 0; mode < 2; ++mode) {
    testCreate(10, mode == 0);

    std::string routingKey = "abc";
    std::string keySuffix = "|#|ignore_me";
    std::string key = routingKey + keySuffix;
    McDeleteRequest req(key);

    for (size_t i = 0; i < 10; ++i) {
      testAllSync(req, i);
    }
  }
}

TEST_F(KeySplitRouteTest, LeasesSetTest) {
  constexpr folly::StringPiece key = "abc";
  McLeaseSetRequest reqSet(key);
  reqSet.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "value");

  testLeases(reqSet);
}

TEST_F(KeySplitRouteTest, LeasesSetTestHashStop) {
  std::string routingKey = "abc";
  std::string keySuffix = "|#|ignore_me";
  std::string key = routingKey + keySuffix;
  McLeaseSetRequest reqSet(key);
  reqSet.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "value");

  testLeases(reqSet);
}

TEST_F(KeySplitRouteTest, LeasesGetTest) {
  constexpr folly::StringPiece key = "abc";
  McLeaseGetRequest req(key);

  testLeases(req);
}

TEST_F(KeySplitRouteTest, LeasesGetTestHashStop) {
  std::string routingKey = "abc";
  std::string keySuffix = "|#|ignore_me";
  std::string key = routingKey + keySuffix;
  McLeaseGetRequest req(key);

  testLeases(req);
}

TEST_F(KeySplitRouteTest, FirstHitTest) {
  constexpr folly::StringPiece key = "abc";
  McLeaseGetRequest req(key);

  auto th = std::make_shared<TestHandle>(GetRouteTestData());
  th->setResultGenerator([key](std::string reqKey) {
    return reqKey == key ? carbon::Result::FOUND : carbon::Result::NOTFOUND;
  });
  auto rh = std::make_shared<RouteHandle>(RouteHandle(th->rh, 3, true, true));

  // create expected keys, the first key is not modified.
  std::vector<std::string> expectedKeys;
  expectedKeys.push_back(key.toString());
  for (size_t i = 1; i < 3; ++i) {
    expectedKeys.push_back(
        folly::to<std::string>(key, kMemcacheReplicaSeparator, i));
  }

  TestFiberManager<MemcacheRouterInfo> fm;
  fm.runAll({[&]() {
    auto reply = rh->route(req);
    EXPECT_EQ(*reply.result_ref(), carbon::Result::FOUND);
  }});
  EXPECT_FALSE(th->saw_keys.empty());

  EXPECT_EQ(vector<std::string>{expectedKeys}, th->saw_keys);

  th->setResultGenerator([key](std::string reqKey) {
    return reqKey != key ? carbon::Result::FOUND : carbon::Result::NOTFOUND;
  });
  th->saw_keys = {};

  fm.runAll({[&]() {
    auto reply = rh->route(req);
    EXPECT_EQ(*reply.result_ref(), carbon::Result::FOUND);
  }});

  EXPECT_EQ(vector<std::string>{expectedKeys}, th->saw_keys);
}

TEST_F(KeySplitRouteTest, FirstHitTestHashStop) {
  std::string routingKey = "abc";
  std::string keySuffix = "|#|ignore_me";
  std::string key = routingKey + keySuffix;
  McLeaseGetRequest req(key);

  auto th = std::make_shared<TestHandle>(GetRouteTestData());
  th->setResultGenerator([key](std::string reqKey) {
    return reqKey == key ? carbon::Result::FOUND : carbon::Result::NOTFOUND;
  });
  auto rh = std::make_shared<RouteHandle>(RouteHandle(th->rh, 3, true, true));

  // create expected keys, the first key is not modified.
  std::vector<std::string> expectedKeys;
  expectedKeys.push_back(key);
  for (size_t i = 1; i < 3; ++i) {
    expectedKeys.push_back(folly::to<std::string>(
        routingKey, kMemcacheReplicaSeparator, i, keySuffix));
  }

  TestFiberManager<MemcacheRouterInfo> fm;
  fm.runAll({[&]() {
    auto reply = rh->route(req);
    EXPECT_EQ(*reply.result_ref(), carbon::Result::FOUND);
  }});
  EXPECT_FALSE(th->saw_keys.empty());

  EXPECT_EQ(vector<std::string>{expectedKeys}, th->saw_keys);

  th->setResultGenerator([key](std::string reqKey) {
    return reqKey != key ? carbon::Result::FOUND : carbon::Result::NOTFOUND;
  });
  th->saw_keys = {};

  fm.runAll({[&]() {
    auto reply = rh->route(req);
    EXPECT_EQ(*reply.result_ref(), carbon::Result::FOUND);
  }});

  EXPECT_EQ(vector<std::string>{expectedKeys}, th->saw_keys);
}

TEST_F(KeySplitRouteTest, FirstHitWorstCaseTest) {
  constexpr folly::StringPiece key = "abc";
  McLeaseGetRequest req(key);

  auto th = std::make_shared<TestHandle>(
      GetRouteTestData(carbon::Result::NOTFOUND, "a"));
  auto rh = std::make_shared<RouteHandle>(RouteHandle(th->rh, 3, true, true));

  // create expected keys, the first key is not modified.
  std::vector<std::string> expectedKeys;
  expectedKeys.push_back(key.toString());
  for (size_t i = 1; i < 3; ++i) {
    expectedKeys.push_back(
        folly::to<std::string>(key, kMemcacheReplicaSeparator, i));
  }

  TestFiberManager<MemcacheRouterInfo> fm;
  fm.runAll({[&]() {
    auto reply = rh->route(req);
    EXPECT_EQ(*reply.result_ref(), carbon::Result::NOTFOUND);
  }});

  EXPECT_FALSE(th->saw_keys.empty());
  std::sort(th->saw_keys.begin(), th->saw_keys.end());
  EXPECT_EQ(vector<std::string>{expectedKeys}, th->saw_keys);
}

TEST_F(KeySplitRouteTest, FirstHitWorstCaseTestHashStop) {
  std::string routingKey = "abc";
  std::string keySuffix = "|#|ignore_me";
  std::string key = routingKey + keySuffix;
  McLeaseGetRequest req(key);

  auto th = std::make_shared<TestHandle>(
      GetRouteTestData(carbon::Result::NOTFOUND, "a"));
  auto rh = std::make_shared<RouteHandle>(RouteHandle(th->rh, 3, true, true));

  // create expected keys, the first key is not modified.
  std::vector<std::string> expectedKeys;
  for (size_t i = 1; i < 3; ++i) {
    expectedKeys.push_back(folly::to<std::string>(
        routingKey, kMemcacheReplicaSeparator, i, keySuffix));
  }
  expectedKeys.push_back(key);

  TestFiberManager<MemcacheRouterInfo> fm;
  fm.runAll({[&]() {
    auto reply = rh->route(req);
    EXPECT_EQ(*reply.result_ref(), carbon::Result::NOTFOUND);
  }});

  EXPECT_FALSE(th->saw_keys.empty());
  std::sort(th->saw_keys.begin(), th->saw_keys.end());
  EXPECT_EQ(vector<std::string>{expectedKeys}, th->saw_keys);
}

TEST_F(KeySplitRouteTest, TestReplicaCounts) {
  EXPECT_TRUE(testCreateForNumReplicas(2, false));
  EXPECT_TRUE(testCreateForNumReplicas(999, false));
  EXPECT_TRUE(testCreateForNumReplicas(1000, false));

  EXPECT_FALSE(testCreateForNumReplicas(0, false));
  EXPECT_FALSE(testCreateForNumReplicas(1, false));
  EXPECT_FALSE(testCreateForNumReplicas(1001, false));
}

TEST_F(KeySplitRouteTest, TestLongKey) {
  globals::HostidMock hostidMock(1);
  testCreate(5, true);

  {
    // key is too long --> route normally, no replication
    std::string key = std::string(255, 'a');
    McGetRequest req(key);
    auto reply = rh_->route(req);
    EXPECT_FALSE(th_->saw_keys.empty());
    EXPECT_EQ(key, th_->saw_keys[0]);
    th_->saw_keys.clear();
  }

  {
    // key is exactly the max allowable length --> replicate
    std::string key = std::string(250, 'a');
    McGetRequest req(key);
    auto reply = rh_->route(req);
    auto expectedKeyReplica =
        folly::to<std::string>(key, kMemcacheReplicaSeparator, 1 % 5);
    EXPECT_FALSE(th_->saw_keys.empty());
    EXPECT_EQ(expectedKeyReplica, th_->saw_keys[0]);
    th_->saw_keys.clear();
  }
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
