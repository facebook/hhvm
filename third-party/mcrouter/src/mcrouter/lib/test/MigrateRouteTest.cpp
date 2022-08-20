/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <ctime>
#include <functional>
#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/lib/routes/MigrateRoute.h"
#include "mcrouter/lib/test/RouteHandleTestUtil.h"
#include "mcrouter/lib/test/TestRouteHandle.h"

using namespace facebook::memcache;

using std::make_shared;
using std::string;
using std::vector;

using TestHandle = TestHandleImpl<TestRouteHandleIf>;

TEST(migrateRouteTest, migrate) {
  const auto now = time(nullptr);
  auto interval = 50;

  using TimeProviderFunc = std::function<time_t()>;

  vector<std::shared_ptr<TestHandle>> test_handles{
      make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "a"),
          UpdateRouteTestData(),
          DeleteRouteTestData(carbon::Result::DELETED)),
      make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "b"),
          UpdateRouteTestData(),
          DeleteRouteTestData(carbon::Result::NOTFOUND)),
  };
  auto route_handles = get_route_handles(test_handles);

  TestFiberManager fm;

  const string key_get = "key_get";
  const string key_del = "key_del";
  const auto hash =
      McGetRequest(key_get).key_ref()->routingKeyHash() % interval;
  const time_t start_time = now + 25;
  const time_t migration_time = start_time + interval + hash;
  const time_t before_migration = start_time + 1;
  const time_t during_migration = migration_time + 1;
  const time_t end_time = start_time + 2 * interval;
  // Sanity check the generated timestamps.
  EXPECT_GT(migration_time, start_time) << "hash(key_get): " << hash;
  EXPECT_GT(migration_time, before_migration) << "hash(key_get): " << hash;
  EXPECT_GT(end_time, during_migration) << "hash(key_get): " << hash;

  fm.runAll(
      {[&]() { // case 1: now < start_time
         TestRouteHandle<MigrateRoute<TestRouteHandleIf, TimeProviderFunc>> rh(
             route_handles[0], route_handles[1], start_time, interval, [=]() {
               return now;
             });

         McGetRequest req_get(key_get);
         int cnt = 0;
         RouteHandleTraverser<TestRouteHandleIf> t{
             [&cnt](const TestRouteHandleIf&) { ++cnt; }};
         rh.traverse(req_get, t);
         EXPECT_EQ(1, cnt);

         auto reply_get = rh.route(req_get);
         EXPECT_EQ("a", carbon::valueRangeSlow(reply_get).str());
         EXPECT_EQ(vector<string>{key_get}, test_handles[0]->saw_keys);
         EXPECT_NE(vector<string>{key_get}, test_handles[1]->saw_keys);
         (test_handles[0]->saw_keys).clear();
         (test_handles[1]->saw_keys).clear();

         McDeleteRequest req_del(key_del);
         cnt = 0;
         rh.traverse(req_del, t);
         EXPECT_EQ(1, cnt);

         auto reply_del = rh.route(req_del);
         EXPECT_EQ(carbon::Result::DELETED, *reply_del.result_ref());
         EXPECT_EQ(vector<string>{key_del}, test_handles[0]->saw_keys);
         EXPECT_NE(vector<string>{key_del}, test_handles[1]->saw_keys);
       },

       [&]() { // case 2: start_time < now < migration_time
         vector<std::shared_ptr<TestHandle>> test_handles_2{
             make_shared<TestHandle>(
                 GetRouteTestData(carbon::Result::FOUND, "a"),
                 UpdateRouteTestData(),
                 DeleteRouteTestData(carbon::Result::DELETED)),
             make_shared<TestHandle>(
                 GetRouteTestData(carbon::Result::NOTFOUND, "b"),
                 UpdateRouteTestData(),
                 DeleteRouteTestData(carbon::Result::NOTFOUND)),
         };
         auto route_handles_c2 = get_route_handles(test_handles_2);
         TestRouteHandle<MigrateRoute<TestRouteHandleIf, TimeProviderFunc>> rh(
             route_handles_c2[0],
             route_handles_c2[1],
             start_time,
             interval,
             [=]() { return before_migration; });

         McGetRequest req_get(key_get);
         int cnt = 0;
         RouteHandleTraverser<TestRouteHandleIf> t{
             [&cnt](const TestRouteHandleIf&) { ++cnt; }};
         rh.traverse(req_get, t);
         EXPECT_EQ(cnt, 1);

         auto reply_get = rh.route(req_get);
         EXPECT_EQ("a", carbon::valueRangeSlow(reply_get).str());
         EXPECT_EQ(vector<string>{key_get}, test_handles_2[0]->saw_keys);
         EXPECT_NE(vector<string>{key_get}, test_handles_2[1]->saw_keys);
         (test_handles_2[0]->saw_keys).clear();
         (test_handles_2[1]->saw_keys).clear();

         McDeleteRequest req_del(key_del);
         cnt = 0;
         rh.traverse(req_del, t);
         EXPECT_EQ(cnt, 2);

         auto reply_del = rh.route(req_del);
         EXPECT_EQ(carbon::Result::NOTFOUND, *reply_del.result_ref());
         EXPECT_EQ(vector<string>{key_del}, test_handles_2[0]->saw_keys);
         EXPECT_EQ(vector<string>{key_del}, test_handles_2[1]->saw_keys);
       },

       [&]() { // case 3: migration_time < curr_time < end_time
         vector<std::shared_ptr<TestHandle>> test_handles_3{
             make_shared<TestHandle>(
                 GetRouteTestData(carbon::Result::NOTFOUND, "a"),
                 UpdateRouteTestData(),
                 DeleteRouteTestData(carbon::Result::NOTFOUND)),
             make_shared<TestHandle>(
                 GetRouteTestData(carbon::Result::FOUND, "b"),
                 UpdateRouteTestData(),
                 DeleteRouteTestData(carbon::Result::DELETED)),
         };
         auto route_handles_c3 = get_route_handles(test_handles_3);
         TestRouteHandle<MigrateRoute<TestRouteHandleIf, TimeProviderFunc>> rh(
             route_handles_c3[0],
             route_handles_c3[1],
             start_time,
             interval,
             [=]() { return during_migration; });

         McGetRequest req_get(key_get);
         int cnt = 0;
         RouteHandleTraverser<TestRouteHandleIf> t{
             [&cnt](const TestRouteHandleIf&) { ++cnt; }};
         rh.traverse(req_get, t);
         EXPECT_EQ(1, cnt);

         auto reply_get = rh.route(req_get);
         EXPECT_EQ("b", carbon::valueRangeSlow(reply_get).str());
         EXPECT_NE(vector<string>{key_get}, test_handles_3[0]->saw_keys);
         EXPECT_EQ(vector<string>{key_get}, test_handles_3[1]->saw_keys);
         (test_handles_3[0]->saw_keys).clear();
         (test_handles_3[1]->saw_keys).clear();

         McDeleteRequest req_del(key_del);
         cnt = 0;
         rh.traverse(req_del, t);
         EXPECT_EQ(2, cnt);

         auto reply_del = rh.route(req_del);
         EXPECT_EQ(carbon::Result::NOTFOUND, *reply_del.result_ref());
         EXPECT_EQ(vector<string>{key_del}, test_handles_3[0]->saw_keys);
         EXPECT_EQ(vector<string>{key_del}, test_handles_3[1]->saw_keys);
       },

       [&]() { // case 4: now > end_time
         TestRouteHandle<MigrateRoute<TestRouteHandleIf, TimeProviderFunc>> rh(
             route_handles[0], route_handles[1], start_time, interval, [=]() {
               return end_time + 1;
             });

         McGetRequest req_get(key_get);
         int cnt = 0;
         RouteHandleTraverser<TestRouteHandleIf> t{
             [&cnt](const TestRouteHandleIf&) { ++cnt; }};
         rh.traverse(req_get, t);
         EXPECT_EQ(cnt, 1);

         auto reply_get = rh.route(req_get);
         EXPECT_EQ("b", carbon::valueRangeSlow(reply_get).str());
         EXPECT_NE(vector<string>{key_get}, test_handles[0]->saw_keys);
         EXPECT_EQ(vector<string>{key_get}, test_handles[1]->saw_keys);
         (test_handles[0]->saw_keys).clear();
         (test_handles[1]->saw_keys).clear();

         McDeleteRequest req_del(key_del);
         cnt = 0;
         rh.traverse(req_del, t);
         EXPECT_EQ(1, cnt);

         auto reply_del = rh.route(req_del);
         EXPECT_EQ(carbon::Result::NOTFOUND, *reply_del.result_ref());
         EXPECT_NE(vector<string>{key_del}, test_handles[0]->saw_keys);
         EXPECT_EQ(vector<string>{key_del}, test_handles[1]->saw_keys);
       }});
}

TEST(migrateRouteTest, leases) {
  vector<std::shared_ptr<TestHandle>> test_handles{
      make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "a"),
          UpdateRouteTestData(),
          DeleteRouteTestData(carbon::Result::DELETED)),
      make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "b"),
          UpdateRouteTestData(carbon::Result::BAD_KEY),
          DeleteRouteTestData(carbon::Result::NOTFOUND)),
  };
  auto route_handles = get_route_handles(test_handles);

  TestFiberManager fm;
  fm.run([&]() {
    const char* const key = "key";
    const time_t start_time = 100;
    auto interval = 10;
    time_t now = 101;
    auto tp_func = [&]() { return now; };
    TestRouteHandle<MigrateRoute<TestRouteHandleIf, decltype(tp_func)>> rh(
        route_handles[0], route_handles[1], start_time, interval, tp_func);

    // Lease-get request is routed to from_ at time start + 1.
    now = start_time + 1;
    McLeaseGetRequest lease_get(key);
    auto reply_get = rh.route(lease_get);
    EXPECT_EQ(0, *reply_get.leaseToken_ref());
    reply_get.leaseToken_ref() =
        0x1337; // Set non-zero lease token to check later.
    EXPECT_EQ("a", carbon::valueRangeSlow(reply_get).str());
    EXPECT_EQ(vector<string>{key}, test_handles[0]->saw_keys);
    EXPECT_EQ(vector<uint32_t>{0}, test_handles[0]->sawExptimes);
    EXPECT_NE(vector<string>{key}, test_handles[1]->saw_keys);
    test_handles[0]->saw_keys.clear();
    test_handles[1]->saw_keys.clear();
    test_handles[0]->sawExptimes.clear();

    // Lease-set is sent to from_ before migrating to to_.
    {
      now = start_time + interval - 1;
      McLeaseSetRequest lease_set(key);
      lease_set.value_ref() = *folly::IOBuf::copyBuffer("value");
      lease_set.exptime_ref() = start_time + 500;
      lease_set.leaseToken_ref() = *reply_get.leaseToken_ref();
      auto reply_set = rh.route(lease_set);
      EXPECT_EQ(vector<string>{key}, test_handles[0]->saw_keys);
      EXPECT_EQ(
          vector<uint32_t>{static_cast<uint32_t>(*lease_set.exptime_ref())},
          test_handles[0]->sawExptimes);
      EXPECT_EQ(vector<uint32_t>{}, test_handles[1]->sawExptimes);
      EXPECT_EQ(vector<string>{}, test_handles[1]->saw_keys);
      EXPECT_EQ(
          vector<int64_t>{*lease_set.leaseToken_ref()},
          test_handles[0]->sawLeaseTokensSet);
      EXPECT_EQ(vector<int64_t>{}, test_handles[1]->sawLeaseTokensSet);
      test_handles[0]->saw_keys.clear();
      test_handles[0]->sawExptimes.clear();
      test_handles[0]->sawLeaseTokensSet.clear();
    }

    // Lease-set is sent after migrating to to_, the lease on from_ should be
    // invalidated.
    {
      now = start_time + 2 * interval - 1;
      McLeaseSetRequest lease_set(key);
      lease_set.value_ref() = *folly::IOBuf::copyBuffer("value");
      lease_set.exptime_ref() = start_time + 1000;
      lease_set.leaseToken_ref() = *reply_get.leaseToken_ref();
      auto reply_set = rh.route(lease_set);
      EXPECT_TRUE(isErrorResult(*reply_set.result_ref()));
      EXPECT_EQ(vector<string>{}, test_handles[0]->saw_keys);
      EXPECT_EQ(vector<string>{key}, test_handles[1]->saw_keys);
      EXPECT_EQ(vector<uint32_t>{}, test_handles[0]->sawExptimes);
      EXPECT_EQ(
          vector<uint32_t>{static_cast<uint32_t>(*lease_set.exptime_ref())},
          test_handles[1]->sawExptimes);
      EXPECT_EQ(vector<int64_t>{}, test_handles[0]->sawLeaseTokensSet);
      EXPECT_EQ(
          vector<int64_t>{*lease_set.leaseToken_ref()},
          test_handles[1]->sawLeaseTokensSet);
      test_handles[1]->sawLeaseTokensSet.clear();

      // MigrateRoute sent an asynchronous lease-get invalidation to from_.
      fm.getFiberManager().runInMainContext(
          [&]() { fm.getFiberManager().loopUntilNoReady(); });
      EXPECT_EQ(vector<string>{key}, test_handles[0]->saw_keys);
      EXPECT_EQ(
          vector<uint32_t>{static_cast<uint32_t>(-1)},
          test_handles[0]->sawExptimes);
      EXPECT_EQ(
          vector<int64_t>{*lease_set.leaseToken_ref()},
          test_handles[0]->sawLeaseTokensSet);
      EXPECT_EQ(vector<int64_t>{}, test_handles[1]->sawLeaseTokensSet);
      test_handles[0]->sawExptimes.clear();
      test_handles[1]->sawExptimes.clear();
      test_handles[0]->sawLeaseTokensSet.clear();
    }
  });
}
