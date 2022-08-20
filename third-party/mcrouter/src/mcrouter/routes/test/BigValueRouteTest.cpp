/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include <folly/Conv.h>
#include <folly/Format.h>

#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/lib/test/RouteHandleTestUtil.h"
#include "mcrouter/routes/BigValueRoute.h"
#include "mcrouter/routes/McrouterRouteHandle.h"

using namespace facebook::memcache;
using namespace facebook::memcache::mcrouter;

using std::make_shared;
using std::vector;

using TestHandle = TestHandleImpl<McrouterRouteHandleIf>;

namespace {
constexpr int version = 1;
constexpr size_t threshold = 128;
constexpr BigValueRouteOptions opts(threshold, /* batchSize= */ 0);
} // namespace

TEST(BigValueRouteTest, smallvalue) {
  // for small values, this route handle simply passes it to child route handle
  vector<std::shared_ptr<TestHandle>> testHandles{make_shared<TestHandle>(
      GetRouteTestData(carbon::Result::FOUND, "a"),
      UpdateRouteTestData(carbon::Result::STORED),
      DeleteRouteTestData(carbon::Result::DELETED))};
  auto routeHandles = get_route_handles(testHandles);

  const std::string keyGet = "smallvalue_get";
  const std::string keySet = "smallvalue_set";

  TestFiberManager fm;
  fm.runAll({[&]() {
    McrouterRouteHandle<BigValueRoute> rh(routeHandles[0], opts);

    McGetRequest reqGet(keyGet);
    auto fGet = rh.route(reqGet);

    EXPECT_EQ("a", carbon::valueRangeSlow(fGet).str());
    EXPECT_EQ(testHandles[0]->saw_keys, vector<std::string>{keyGet});
    testHandles[0]->saw_keys.clear();

    McSetRequest reqSet(keySet);
    reqSet.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "value");

    auto fSet = rh.route(reqSet);
    EXPECT_EQ(carbon::Result::STORED, *fSet.result_ref());
    EXPECT_EQ(testHandles[0]->saw_keys, vector<std::string>{keySet});
  }});
}

TEST(BigValueRouteTest, bigvalue) {
  // for big values, used saw_keys of test route handle to verify that
  // get path and set path saw original key and chunk keys in correct sequesne.

  const std::string rand_suffix_get = "123456";
  const size_t num_chunks = 10;
  // initial reply of the form version-num_chunks-rand_suffix for get path
  const auto init_reply =
      folly::sformat("{}-{}-{}", version, num_chunks, rand_suffix_get);
  const auto init_reply_error = folly::sformat("{}-{}", version, num_chunks);
  vector<std::shared_ptr<TestHandle>> testHandles{
      make_shared<TestHandle>(GetRouteTestData(
          carbon::Result::FOUND, init_reply, MC_MSG_FLAG_BIG_VALUE)),
      make_shared<TestHandle>(GetRouteTestData(
          carbon::Result::FOUND, init_reply_error, MC_MSG_FLAG_BIG_VALUE)),
      make_shared<TestHandle>(UpdateRouteTestData(carbon::Result::STORED)),
      make_shared<TestHandle>(UpdateRouteTestData(carbon::Result::STORED)),
      make_shared<TestHandle>(UpdateRouteTestData(carbon::Result::STORED))};
  auto routeHandles = get_route_handles(testHandles);

  const std::string keyGet = "bigvalue_get";
  const std::string keySet = "bigvalue_set";

  TestFiberManager fm;
  fm.runAll({[&]() {
    { // Test Get Like path with init_reply in corect format
      McrouterRouteHandle<BigValueRoute> rh(routeHandles[0], opts);

      McGetRequest reqGet(keyGet);

      auto fGet = rh.route(reqGet);
      auto keys_get = testHandles[0]->saw_keys;
      EXPECT_EQ(num_chunks + 1, keys_get.size());
      // first get the result for original key
      EXPECT_EQ(keyGet, keys_get.front());

      std::string merged_str;
      // since reply for first key indicated that it is for a big get request,
      // perform get request on chunk keys
      for (size_t i = 1; i < num_chunks + 1; i++) {
        auto chunk_key =
            folly::sformat("{}:{}:{}", keyGet, i - 1, rand_suffix_get);
        EXPECT_EQ(keys_get[i], chunk_key);
        merged_str.append(init_reply);
      }

      // each chunk_key saw value as init_reply.
      // In GetLike path, it gets appended num_chunks time
      EXPECT_EQ(merged_str, carbon::valueRangeSlow(fGet).str());
    }

    { // Test Get Like path with init_reply_error
      McrouterRouteHandle<BigValueRoute> rh(routeHandles[1], opts);

      McGetRequest reqGet(keyGet);

      auto fGet = rh.route(reqGet);
      auto keys_get = testHandles[1]->saw_keys;
      EXPECT_EQ(1, keys_get.size());
      // first get the result for original key, then return
      // carbon::Result::NOTFOUND
      EXPECT_EQ(keyGet, keys_get.front());
      EXPECT_EQ(carbon::Result::NOTFOUND, *fGet.result_ref());
      EXPECT_EQ("", carbon::valueRangeSlow(fGet).str());
    }

    { // Test Update Like path with mc_op_set op
      McrouterRouteHandle<BigValueRoute> rh(routeHandles[2], opts);

      std::string bigValue = folly::to<std::string>(
          std::string(threshold * (num_chunks / 2), 't'),
          std::string(threshold * (num_chunks / 2), 's'));
      std::string chunk_type_1(threshold, 't');
      std::string chunk_type_2(threshold, 's');
      McSetRequest reqSet(keySet);
      reqSet.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, bigValue);

      auto fSet = rh.route(reqSet);
      auto keys_set = testHandles[2]->saw_keys;
      auto values_set = testHandles[2]->sawValues;
      EXPECT_EQ(num_chunks + 1, keys_set.size());
      std::string rand_suffix_set;
      // first set chunk values corresponding to chunk keys
      for (size_t i = 0; i < num_chunks; i++) {
        auto chunk_key_prefix = folly::sformat("{}:{}:", keySet, i);
        auto length = chunk_key_prefix.length();
        auto saw_prefix = keys_set[i].substr(0, length);
        EXPECT_EQ(chunk_key_prefix, saw_prefix);

        if (rand_suffix_set.empty()) { // rand_suffic same for all chunk_keys
          rand_suffix_set = keys_set[i].substr(length);
        } else {
          EXPECT_EQ(rand_suffix_set, keys_set[i].substr(length));
        }

        if (i < num_chunks / 2) {
          EXPECT_EQ(chunk_type_1, values_set[i]);
        } else {
          EXPECT_EQ(chunk_type_2, values_set[i]);
        }
      }

      // if set for chunk keys succeed,
      // set original key with chunks info as modified value
      EXPECT_EQ(keySet, keys_set[num_chunks]);
      auto chunks_info =
          folly::sformat("{}-{}-{}", version, num_chunks, rand_suffix_set);
      EXPECT_EQ(chunks_info, values_set[num_chunks]);
    }

    { // Test Update Like path with mc_op_lease_set op
      McrouterRouteHandle<BigValueRoute> rh(routeHandles[3], opts);

      std::string bigValue = folly::to<std::string>(
          std::string(threshold * (num_chunks / 2), 't'),
          std::string(threshold * (num_chunks / 2), 's'));

      McLeaseSetRequest reqSet(keySet);
      reqSet.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, bigValue);

      auto fSet = rh.route(reqSet);
      auto keys_set = testHandles[3]->saw_keys;
      auto operations_set = testHandles[3]->sawOperations;
      EXPECT_EQ(num_chunks + 1, keys_set.size());
      // first set chunk values corresponding to chunk keys
      for (size_t i = 0; i < num_chunks; i++) {
        auto chunk_key_prefix = folly::sformat("{}:{}:", keySet, i);
        auto length = chunk_key_prefix.length();
        auto saw_prefix = keys_set[i].substr(0, length);
        EXPECT_EQ(chunk_key_prefix, saw_prefix);

        EXPECT_EQ("set", operations_set[i]);
      }

      // if set for chunk keys succeed,
      // set original key with chunks info as modified value
      EXPECT_EQ(keySet, keys_set[num_chunks]);
      EXPECT_EQ("lease-set", operations_set[num_chunks]);
    }

    { // Test Update Like path with mc_op_add op
      McrouterRouteHandle<BigValueRoute> rh(routeHandles[4], opts);

      std::string bigValue = folly::to<std::string>(
          std::string(threshold * (num_chunks / 2), 't'),
          std::string(threshold * (num_chunks / 2), 's'));
      std::string chunk_type_1(threshold, 't');
      std::string chunk_type_2(threshold, 's');
      McAddRequest reqSet(keySet);
      reqSet.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, bigValue);

      auto fSet = rh.route(reqSet);
      auto keys_set = testHandles[4]->saw_keys;
      auto values_set = testHandles[4]->sawValues;
      auto operations_set = testHandles[4]->sawOperations;
      EXPECT_EQ(num_chunks + 1, keys_set.size());
      std::string rand_suffix_set;
      // first set chunk values corresponding to chunk keys
      for (size_t i = 0; i < num_chunks; i++) {
        auto chunk_key_prefix = folly::sformat("{}:{}:", keySet, i);
        auto length = chunk_key_prefix.length();
        auto saw_prefix = keys_set[i].substr(0, length);
        EXPECT_EQ(chunk_key_prefix, saw_prefix);
        EXPECT_EQ("set", operations_set[i]);

        if (rand_suffix_set.empty()) { // rand_suffic same for all chunk_keys
          rand_suffix_set = keys_set[i].substr(length);
        } else {
          EXPECT_EQ(rand_suffix_set, keys_set[i].substr(length));
        }

        if (i < num_chunks / 2) {
          EXPECT_EQ(chunk_type_1, values_set[i]);
        } else {
          EXPECT_EQ(chunk_type_2, values_set[i]);
        }
      }

      // if set for chunk keys succeed,
      // set original key with chunks info as modified value
      EXPECT_EQ(keySet, keys_set[num_chunks]);
      EXPECT_EQ("add", operations_set[num_chunks]);
      auto chunks_info =
          folly::sformat("{}-{}-{}", version, num_chunks, rand_suffix_set);
      EXPECT_EQ(chunks_info, values_set[num_chunks]);
    }
  }});
}
