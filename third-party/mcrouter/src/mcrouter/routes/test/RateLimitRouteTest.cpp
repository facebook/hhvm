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

#include <folly/json/json.h>

#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/lib/test/RouteHandleTestUtil.h"
#include "mcrouter/routes/McrouterRouteHandle.h"
#include "mcrouter/routes/RateLimitRoute.h"

using namespace facebook::memcache;
using namespace facebook::memcache::mcrouter;

using std::make_shared;
using std::string;
using std::vector;

using TestHandle = TestHandleImpl<McrouterRouteHandleIf>;

/* These tests simply ensure that the various settings are read correctly.
   TokenBucket has its own tests */

namespace {

template <class Data, class Request>
void test(
    Data data,
    Data fallbackData,
    Request,
    carbon::Result ok,
    carbon::Result reject,
    carbon::Result fallbackRes,
    const std::string& type,
    bool burst = false,
    bool fallback = false) {
  vector<std::shared_ptr<TestHandle>> normalHandle{
      make_shared<TestHandle>(std::move(data)),
  };
  auto normalRh = get_route_handles(normalHandle)[0];

  vector<std::shared_ptr<TestHandle>> fallbackHandle{
      make_shared<TestHandle>(std::move(fallbackData)),
  };
  auto fallbackRh = get_route_handles(fallbackHandle)[0];

  auto json = parseJsonString(
      (burst
           ? string("{\"") + type + "_rate\": 4.0, \"" + type + "_burst\": 3.0}"
           : string("{\"") + type + "_rate\": 2.0}"));
  McrouterRouteHandle<RateLimitRoute<McrouterRouteHandleIf>> rh(
      normalRh, RateLimiter(json), fallback ? fallbackRh : nullptr);

  Request req("key");
  // McSetRequest requires value be set; this is a no-op for Get and Delete
  if (auto* value = const_cast<folly::IOBuf*>(carbon::valuePtrUnsafe(req))) {
    *value = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "value");
  }

  if (burst) {
    usleep(1001000);
    /* Rate is 4/sec, but we can only have 3 at a time */
    auto reply = rh.route(req);
    EXPECT_EQ(*reply.result_ref(), ok);
    reply = rh.route(req);
    EXPECT_EQ(*reply.result_ref(), ok);
    reply = rh.route(req);
    EXPECT_EQ(*reply.result_ref(), ok);
    reply = rh.route(req);
    EXPECT_EQ(*reply.result_ref(), fallback ? fallbackRes : reject);
  } else {
    usleep(501000);
    auto reply = rh.route(req);
    EXPECT_EQ(*reply.result_ref(), ok);
    reply = rh.route(req);
    EXPECT_EQ(*reply.result_ref(), fallback ? fallbackRes : reject);
  }
}

void testSets(bool burst = false, bool fallback = false) {
  test(
      UpdateRouteTestData(carbon::Result::STORED),
      UpdateRouteTestData(carbon::Result::LOCAL_ERROR),
      McSetRequest(),
      carbon::Result::STORED,
      carbon::Result::NOTSTORED,
      carbon::Result::LOCAL_ERROR,
      "sets",
      burst,
      fallback);
}

void testGets(bool burst = false, bool fallback = false) {
  test(
      GetRouteTestData(carbon::Result::FOUND, "a"),
      GetRouteTestData(carbon::Result::LOCAL_ERROR, "a"),
      McGetRequest(),
      carbon::Result::FOUND,
      carbon::Result::NOTFOUND,
      carbon::Result::LOCAL_ERROR,
      "gets",
      burst,
      fallback);
}

void testDeletes(bool burst = false, bool fallback = false) {
  test(
      DeleteRouteTestData(carbon::Result::DELETED),
      DeleteRouteTestData(carbon::Result::LOCAL_ERROR),
      McDeleteRequest(),
      carbon::Result::DELETED,
      carbon::Result::NOTFOUND,
      carbon::Result::LOCAL_ERROR,
      "deletes",
      burst,
      fallback);
}

} // namespace

TEST(rateLimitRouteTest, setsBasic) {
  testSets();
}
TEST(rateLimitRouteTest, setsBurst) {
  testSets(true);
}
TEST(rateLimitRouteTest, setsFallback) {
  testSets(false, true);
}
TEST(rateLimitRouteTest, getsBasic) {
  testGets();
}
TEST(rateLimitRouteTest, getsBurst) {
  testGets(true);
}
TEST(rateLimitRouteTest, getsFallback) {
  testGets(false, true);
}
TEST(rateLimitRouteTest, deletesBasic) {
  testDeletes();
}
TEST(rateLimitRouteTest, deletesBurst) {
  testDeletes(true);
}
TEST(rateLimitRouteTest, deletesFallback) {
  testDeletes(false, true);
}
