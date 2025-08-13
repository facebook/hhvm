/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <string>

#include <gtest/gtest.h>

#include <folly/io/async/EventBase.h>
#include <folly/json/json.h>

#include "mcrouter/CarbonRouterInstance.h"
#include "mcrouter/PoolFactory.h"
#include "mcrouter/Proxy.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/lib/network/gen/MemcacheRouterInfo.h"
#include "mcrouter/options.h"
#include "mcrouter/routes/McRouteHandleProvider.h"
#include "mcrouter/routes/McrouterRouteHandle.h"

using namespace facebook::memcache;
using namespace facebook::memcache::mcrouter;

namespace {

const char* const kMemcacheConfig = "mcrouter/test/test_ascii.json";

const char* const kConstShard =
    R"({
  "type": "HashRoute",
  "children": "ErrorRoute",
  "hash_func": "ConstShard"
 })";

const char* const kInvalidHashFunc =
    R"({
  "type": "HashRoute",
  "children": ["ErrorRoute", "ErrorRoute"],
  "hash_func": "InvalidHashFunc"
 })";

const char* const kWarmUp =
    R"({
   "type": "WarmUpRoute",
   "cold": "ErrorRoute",
   "warm": "NullRoute"
 })";

const char* const kPoolRoute =
    R"({
   "type": "PoolRoute",
   "pool": { "name": "mock", "servers": [ ] },
   "hash": { "hash_func": "Crc32" }
 })";

const char* const kPoolRouteInvalidFanout =
    R"({
   "type": "PoolRoute",
   "pool": { "name": "mock", "servers": [ ], "additional_fanout": 40000},
   "hash": { "hash_func": "Crc32" },
 })";

const char* const kBucketizedSRRoute =
    R"({
   "type": "SRRoute",
   "service_name": "mcrouter.test.thrifttest.oregon.ucache_ab_conveyor_shadow_proxy",
   "server_timeout": 200,
   "asynclog_name": "test.asynclog",
   "axonlog": false,
   "bucketize": true,
   "total_buckets": 1000,
   "bucketization_keyspace": "tst"
})";

const char* const kBucketizedPoolRoute =
    R"({
   "type": "PoolRoute",
   "pool": { "name": "mock", "servers": [ ] },
   "pool_id": "mcrouter.test.thrifttest.oregon.ucache_ab_conveyor_shadow_proxy",
   "hash": "WeightedCh3",
   "asynclog_name": "test.asynclog",
   "axonlog": false,
   "bucketize": true,
   "total_buckets": 1000,
   "bucketization_keyspace": "tst"
})";

const char* const kSimpleSRRoute =
    R"({
   "type": "SRRoute",
   "service_name": "mcrouter.test.thrifttest.oregon.ucache_ab_conveyor_shadow_proxy",
   "asynclog": false,
})";

struct TestSetup {
 public:
  TestSetup()
      : router_(CarbonRouterInstance<McrouterRouterInfo>::init(
            "test_get_route",
            getOpts())),
        poolFactory_(
            folly::dynamic::object(),
            router_->configApi(),
            folly::json::metadata_map{}),
        rhProvider_(*router_->getProxy(0), poolFactory_),
        rhFactory_(rhProvider_, 0) {}

  McRouteHandleProvider<MemcacheRouterInfo>& provider() {
    return rhProvider_;
  }

  McrouterRouteHandlePtr getRoute(const char* jsonStr) {
    return rhFactory_.create(parseJsonString(jsonStr));
  }

 private:
  CarbonRouterInstance<McrouterRouterInfo>* router_;
  PoolFactory poolFactory_;
  McRouteHandleProvider<MemcacheRouterInfo> rhProvider_;
  RouteHandleFactory<McrouterRouteHandleIf> rhFactory_;

  static McrouterOptions getOpts() {
    auto opts = defaultTestOptions();
    opts.enable_service_router = true;
    opts.config = std::string("file:") + kMemcacheConfig;
    return opts;
  }
};

} // namespace

TEST(McRouteHandleProviderTest, sanity) {
  auto rh = TestSetup().getRoute(kConstShard);
  EXPECT_TRUE(rh != nullptr);
  EXPECT_EQ("error|log|mc_res_local_error", rh->routeName());
}

TEST(McRouteHandleProviderTest, invalid_func) {
  try {
    auto rh = TestSetup().getRoute(kInvalidHashFunc);
  } catch (const std::logic_error&) {
    return;
  }
  FAIL() << "No exception thrown";
}

TEST(McRouteHandleProvider, warmup) {
  auto rh = TestSetup().getRoute(kWarmUp);
  EXPECT_TRUE(rh != nullptr);
  EXPECT_EQ("warm-up", rh->routeName());
}

TEST(McRouteHandleProvider, pool_route) {
  TestSetup setup;
  auto rh = setup.getRoute(kPoolRoute);
  EXPECT_TRUE(rh != nullptr);
  EXPECT_EQ("asynclog:mock", rh->routeName());
  auto asynclogRoutes = setup.provider().releaseAsyncLogRoutes();
  EXPECT_EQ(1, asynclogRoutes.size());
  EXPECT_EQ("asynclog:mock", asynclogRoutes["mock"]->routeName());
}

TEST(McRouteHandleProvider, sr_route) {
  TestSetup setup;
  auto rh = setup.getRoute(kSimpleSRRoute);
  EXPECT_TRUE(rh != nullptr);
  EXPECT_EQ(
      "srroute|service=mcrouter.test.thrifttest.oregon.ucache_ab_conveyor_shadow_proxy|timeout=1000|connect_timeout=1000|keep_routing_prefix=false|salt=|asynclog_name=|set_routing_key=true|client_id=|write_checksum=false|all_checksum=false|jump_threads=false|return_error_on_mc_delete_failure=false|set_shard_id=false|sm_scope=|sm_domain=|skip_thread_affinity=false|set_request_timeout=true",
      rh->routeName());
}

TEST(McRouteHandleProvider, pool_route_with_invalid_fanout) {
  try {
    TestSetup setup;
    auto rh = setup.getRoute(kPoolRouteInvalidFanout);
  } catch (const std::logic_error&) {
    return;
  }
  FAIL() << "No exception thrown";
}

TEST(McRouteHandleProvider, bucketized_sr_route_and_mcreplay_asynclogRoutes) {
  TestSetup setup;
  auto rh = setup.getRoute(kBucketizedSRRoute);
  EXPECT_TRUE(rh != nullptr);
  EXPECT_EQ(
      "bucketize|total_buckets=1000|bucketization_keyspace=tst|prefix_map_enabled=false",
      rh->routeName());
  auto asynclogRoutes = setup.provider().releaseAsyncLogRoutes();
  EXPECT_EQ(1, asynclogRoutes.size());
  EXPECT_EQ(
      "bucketize|total_buckets=1000|bucketization_keyspace=tst|prefix_map_enabled=false",
      asynclogRoutes["test.asynclog"]->routeName());
}

TEST(McRouteHandleProvider, bucketized_pool_route_and_mcreplay_asynclogRoutes) {
  TestSetup setup;
  auto rh = setup.getRoute(kBucketizedPoolRoute);
  EXPECT_TRUE(rh != nullptr);
  EXPECT_EQ(
      "bucketize|total_buckets=1000|bucketization_keyspace=tst|prefix_map_enabled=false",
      rh->routeName());
  auto asynclogRoutes = setup.provider().releaseAsyncLogRoutes();
  EXPECT_EQ(1, asynclogRoutes.size());
  EXPECT_EQ(
      "bucketize|total_buckets=1000|bucketization_keyspace=tst|prefix_map_enabled=false",
      asynclogRoutes["test.asynclog"]->routeName());
}
