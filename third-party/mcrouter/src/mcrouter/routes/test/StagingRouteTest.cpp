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

#include "mcrouter/routes/McrouterRouteHandle.h"
#include "mcrouter/routes/StagingRoute.h"
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
using RouteHandle = MemcacheRouteHandle<StagingRoute>;

class StagingRouteTest : public RouteHandleTestBase<HelloGoodbyeRouterInfo> {
 public:
  template <class Request, class TestData>
  void createAndRun(
      const Request& req,
      TestData warmResult,
      TestData stagingResult,
      carbon::Result expectedReply) {
    warm_ = std::make_shared<TestHandle>(warmResult);
    staging_ = std::make_shared<TestHandle>(stagingResult);

    rh_ = std::make_shared<RouteHandle>(RouteHandle(warm_->rh, staging_->rh));

    TestFiberManager fm;
    fm.runAll({[&]() {
      auto reply = rh_->route(req);

      // we should always expect the warm reply
      EXPECT_EQ(*reply.result_ref(), expectedReply);
    }});
  }

  void verifyOperations(
      std::vector<std::string> expectedWarmKeys,
      std::vector<std::string> expectedWarmOps,
      std::vector<std::string> expectedWarmValues,
      std::vector<std::string> expectedStagingKeys,
      std::vector<std::string> expectedStagingOps,
      std::vector<std::string> expectedStagingValues) {
    // verify warm side
    EXPECT_EQ(warm_->saw_keys.empty(), expectedWarmKeys.empty());
    EXPECT_EQ(warm_->saw_keys, expectedWarmKeys);

    EXPECT_EQ(warm_->sawOperations.empty(), expectedWarmOps.empty());
    EXPECT_EQ(warm_->sawOperations, expectedWarmOps);

    EXPECT_EQ(warm_->sawValues, expectedWarmValues);

    // verify staging side: should have seen 2 operation: metaget and then add
    // respectively
    EXPECT_EQ(staging_->saw_keys.empty(), expectedStagingKeys.empty());
    EXPECT_EQ(staging_->saw_keys, expectedStagingKeys);

    EXPECT_EQ(staging_->sawOperations.empty(), expectedStagingOps.empty());
    EXPECT_EQ(staging_->sawOperations, expectedStagingOps);

    EXPECT_EQ(staging_->sawValues, expectedStagingValues);
  }

  std::shared_ptr<RouteHandle> rh_;
  std::shared_ptr<TestHandle> warm_;
  std::shared_ptr<TestHandle> staging_;
};

TEST_F(StagingRouteTest, GetTestWarmHitStagingAdd) {
  // test get: hit on warm, miss on stage. --> add to staging
  McGetRequest req("abc");
  createAndRun(
      req,
      GetRouteTestData(carbon::Result::FOUND, "a_value"),
      GetRouteTestData(carbon::Result::NOTFOUND, "b_value"),
      carbon::Result::FOUND);

  verifyOperations(
      /* warm */
      std::vector<std::string>{"abc", "abc"},
      std::vector<std::string>{"get", "metaget"},
      std::vector<std::string>{},
      /* staging */
      std::vector<std::string>{"abc", "abc"},
      std::vector<std::string>{"metaget", "add"},
      std::vector<std::string>{"a_value"});
}

TEST_F(StagingRouteTest, LeaseGetTestWarmHitStagingAdd) {
  McLeaseGetRequest req("abc");
  createAndRun(
      req,
      GetRouteTestData(carbon::Result::FOUND, "a_value"),
      GetRouteTestData(carbon::Result::NOTFOUND, "b_value"),
      carbon::Result::FOUND);

  verifyOperations(
      /* warm */
      std::vector<std::string>{"abc", "abc"},
      std::vector<std::string>{"lease-get", "metaget"},
      std::vector<std::string>{},
      /* staging */
      std::vector<std::string>{"abc", "abc"},
      std::vector<std::string>{"metaget", "add"},
      std::vector<std::string>{"a_value"});
}

TEST_F(StagingRouteTest, GetWarmMiss) {
  // test get: miss on warm, hit on stage. --> only see warm miss
  McGetRequest req("abc");
  createAndRun(
      req,
      GetRouteTestData(carbon::Result::NOTFOUND, "a"),
      GetRouteTestData(carbon::Result::FOUND, "a"),
      carbon::Result::NOTFOUND);

  verifyOperations(
      /* warm */
      std::vector<std::string>{"abc"},
      std::vector<std::string>{"get"},
      std::vector<std::string>{},
      /* staging */
      std::vector<std::string>{},
      std::vector<std::string>{},
      std::vector<std::string>{});
}

TEST_F(StagingRouteTest, LeaseGetWarmMiss) {
  // test get: miss on warm, hit on stage. --> only see warm miss
  McLeaseGetRequest req("abc");
  createAndRun(
      req,
      GetRouteTestData(carbon::Result::NOTFOUND, "a"),
      GetRouteTestData(carbon::Result::FOUND, "a"),
      carbon::Result::NOTFOUND);

  verifyOperations(
      /* warm */
      std::vector<std::string>{"abc"},
      std::vector<std::string>{"lease-get"},
      std::vector<std::string>{},
      /* staging */
      std::vector<std::string>{},
      std::vector<std::string>{},
      std::vector<std::string>{});
}

TEST_F(StagingRouteTest, TestSet) {
  // test set: send to both warm/staging
  McSetRequest req("abc");
  req.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "set_value");

  createAndRun(
      req,
      UpdateRouteTestData(carbon::Result::STORED, 0),
      UpdateRouteTestData(carbon::Result::NOTSTORED, 0),
      carbon::Result::STORED);

  verifyOperations(
      /* warm */
      std::vector<std::string>{"abc"},
      std::vector<std::string>{"set"},
      std::vector<std::string>{"set_value"},
      /* staging */
      std::vector<std::string>{"abc"},
      std::vector<std::string>{"set"},
      std::vector<std::string>{"set_value"});
}

TEST_F(StagingRouteTest, TestSetFail) {
  // test set fail, staging should not be hit
  McSetRequest req("abc");
  req.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "set_value");

  createAndRun(
      req,
      UpdateRouteTestData(carbon::Result::UNKNOWN, 0),
      UpdateRouteTestData(carbon::Result::STORED, 0),
      carbon::Result::UNKNOWN);

  verifyOperations(
      /* warm */
      std::vector<std::string>{"abc"},
      std::vector<std::string>{"set"},
      std::vector<std::string>{"set_value"},
      /* staging */
      std::vector<std::string>{},
      std::vector<std::string>{},
      std::vector<std::string>{});
}

TEST_F(StagingRouteTest, TestLeaseSet) {
  // test lease-set: lease-set on warm, translate to set on staging
  McLeaseSetRequest req("abc");
  req.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "set_value");

  createAndRun(
      req,
      UpdateRouteTestData(carbon::Result::STORED, 0),
      UpdateRouteTestData(carbon::Result::NOTSTORED, 0),
      carbon::Result::STORED);

  verifyOperations(
      /* warm */
      std::vector<std::string>{"abc"},
      std::vector<std::string>{"lease-set"},
      std::vector<std::string>{"set_value"},

      /* staging */
      std::vector<std::string>{"abc"},
      std::vector<std::string>{"set"},
      std::vector<std::string>{"set_value"});
}

TEST_F(StagingRouteTest, TestDelete) {
  // test delete: send to both.
  McDeleteRequest req("abc");
  createAndRun(
      req,
      DeleteRouteTestData(carbon::Result::DELETED),
      DeleteRouteTestData(carbon::Result::UNKNOWN),
      carbon::Result::UNKNOWN); // worse reply

  verifyOperations(
      /* warm */
      std::vector<std::string>{"abc"},
      std::vector<std::string>{"delete"},
      std::vector<std::string>{},
      /* staging */
      std::vector<std::string>{"abc"},
      std::vector<std::string>{"delete"},
      std::vector<std::string>{});
}

TEST_F(StagingRouteTest, TestCas) {
  McCasRequest req("abc");
  req.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "set_value");
  createAndRun(
      req,
      UpdateRouteTestData(carbon::Result::STORED, 0),
      UpdateRouteTestData(carbon::Result::NOTSTORED, 0),
      carbon::Result::STORED);

  verifyOperations(
      /* warm */
      std::vector<std::string>{"abc"},
      std::vector<std::string>{"cas"},
      std::vector<std::string>{"set_value"},
      /* staging */
      std::vector<std::string>{"abc"},
      std::vector<std::string>{"set"},
      std::vector<std::string>{"set_value"});
}

TEST_F(StagingRouteTest, TestGets) {
  McGetsRequest req("abc");
  createAndRun(
      req,
      GetRouteTestData(carbon::Result::FOUND, "a_value"),
      GetRouteTestData(carbon::Result::NOTFOUND, "b_value"),
      carbon::Result::FOUND);

  verifyOperations(
      /* warm */
      std::vector<std::string>{"abc"},
      std::vector<std::string>{"gets"},
      std::vector<std::string>{},
      /* staging */
      std::vector<std::string>{"abc"},
      std::vector<std::string>{"metaget"},
      std::vector<std::string>{});
}

TEST_F(StagingRouteTest, TestGetsNotFound) {
  McGetsRequest req("abc");
  createAndRun(
      req,
      GetRouteTestData(carbon::Result::NOTFOUND, "a_value"),
      GetRouteTestData(carbon::Result::UNKNOWN, "b_value"),
      carbon::Result::NOTFOUND);

  verifyOperations(
      /* warm */
      std::vector<std::string>{"abc"},
      std::vector<std::string>{"gets"},
      std::vector<std::string>{},
      /* staging */
      std::vector<std::string>{},
      std::vector<std::string>{},
      std::vector<std::string>{});
}

TEST_F(StagingRouteTest, TestMetaget) {
  McMetagetRequest req("abc");
  createAndRun(
      req,
      GetRouteTestData(carbon::Result::FOUND, "a"),
      GetRouteTestData(carbon::Result::NOTFOUND, "a"),
      carbon::Result::FOUND);

  verifyOperations(
      /* warm */
      std::vector<std::string>{"abc"},
      std::vector<std::string>{"metaget"},
      std::vector<std::string>{},
      /* staging */
      std::vector<std::string>{"abc"},
      std::vector<std::string>{"metaget"},
      std::vector<std::string>{});
}

TEST_F(StagingRouteTest, TestAddSuccess) {
  // test general operations for non-special cases.
  // route to warm, if successful, async route to staging.
  McAddRequest req("abc");
  req.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "set_value");
  createAndRun(
      req,
      UpdateRouteTestData(carbon::Result::STORED, 0),
      UpdateRouteTestData(carbon::Result::NOTSTORED, 0),
      carbon::Result::STORED);

  verifyOperations(
      /* warm */
      std::vector<std::string>{"abc"},
      std::vector<std::string>{"add"},
      std::vector<std::string>{"set_value"},
      /* staging */
      std::vector<std::string>{"abc"},
      std::vector<std::string>{"add"},
      std::vector<std::string>{"set_value"});
}

TEST_F(StagingRouteTest, TestAddFailure) {
  // test general operations for non-special cases.
  // route to warm, if successful, async route to staging.
  McAddRequest req("abc");
  req.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "set_value");
  createAndRun(
      req,
      UpdateRouteTestData(carbon::Result::TIMEOUT, 0),
      UpdateRouteTestData(carbon::Result::STORED, 0),
      carbon::Result::TIMEOUT);

  verifyOperations(
      /* warm */
      std::vector<std::string>{"abc"},
      std::vector<std::string>{"add"},
      std::vector<std::string>{"set_value"},
      /* staging */
      std::vector<std::string>{},
      std::vector<std::string>{},
      std::vector<std::string>{});
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
