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
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/config/RouteHandleBuilder.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/lib/network/gen/MemcacheRouterInfo.h"
#include "mcrouter/options.h"
#include "mcrouter/routes/McExtraRouteHandleProvider.h"
#include "mcrouter/routes/McRouteHandleProvider.h"
#include "mcrouter/routes/McrouterRouteHandle.h"
#include "mcrouter/routes/PrefixSelectorRoute.h"
#include "mcrouter/routes/ProxyRoute.h"
#include "mcrouter/routes/RouteSelectorMap.h"

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
      : router_(
            CarbonRouterInstance<McrouterRouterInfo>::init(
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

  RouteHandleFactory<McrouterRouteHandleIf>& factory() {
    return rhFactory_;
  }

  Proxy<MemcacheRouterInfo>& proxy() {
    return *router_->getProxy(0);
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

// Recognizable pass-through handle, so a traversal can tell whether the root
// of the tree is the one ProxyRoute built or one an extra provider put there.
template <class RouteHandleIf>
class MarkerRoute {
 public:
  static std::string routeName() {
    return "marker";
  }

  explicit MarkerRoute(std::shared_ptr<RouteHandleIf> child)
      : child_(std::move(child)) {}

  template <class Request>
  bool traverse(const Request& req, RouteHandleTraverser<RouteHandleIf>& t)
      const {
    return t(*child_, req);
  }

  template <class Request>
  ReplyT<Request> route(const Request& req) const {
    return child_->route(req);
  }

 private:
  std::shared_ptr<RouteHandleIf> child_;
};

class MarkerExtraProvider
    : public McExtraRouteHandleProvider<MemcacheRouterInfo> {
 public:
  McrouterRouteHandlePtr wrapRoot(McrouterRouteHandlePtr root) override {
    return makeRouteHandle<McrouterRouteHandleIf, MarkerRoute>(std::move(root));
  }
};

// Single-entry selector map, enough to build a ProxyRoute.
RouteSelectorMap<McrouterRouteHandleIf> nullRouteSelectors(TestSetup& setup) {
  RouteSelectorMap<McrouterRouteHandleIf> selectors;
  selectors[setup.proxy().getRouterOptions().default_route] =
      std::make_shared<PrefixSelectorRoute<McrouterRouteHandleIf>>(
          setup.factory(), parseJsonString(R"("NullRoute")"));
  return selectors;
}

// Name of the first route handle a traversal of `proxyRoute` reaches.
std::string rootRouteName(ProxyRoute<MemcacheRouterInfo>& proxyRoute) {
  std::string name;
  RouteHandleTraverser<McrouterRouteHandleIf> t{
      [&name](const McrouterRouteHandleIf& rh) {
        if (name.empty()) {
          name = rh.routeName();
        }
      }};
  proxyRoute.traverse(McGetRequest("key"), t);
  return name;
}

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
      "srroute|service=mcrouter.test.thrifttest.oregon.ucache_ab_conveyor_shadow_proxy|timeout=1000|connect_timeout=1000|keep_routing_prefix=false|salt=|asynclog_name=|set_routing_key=true|client_id=|write_checksum=false|all_checksum=false|jump_threads=false|return_error_on_mc_delete_failure=false|set_shard_id=false|sm_scope=|sm_domain=|skip_thread_affinity=false|set_request_timeout=true|num_affinity_threads=1|partition_start=0|partition_size=0",
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

// A provider that does not override wrapRoot leaves the tree alone, so
// existing routers keep the root that ProxyRoute built for them.
TEST(McRouteHandleProvider, wrap_root_defaults_to_identity) {
  TestSetup setup;
  auto rh = setup.getRoute(kConstShard);
  ASSERT_TRUE(setup.provider().extraProvider() != nullptr);
  EXPECT_EQ(rh, setup.provider().extraProvider()->wrapRoot(rh));
}

TEST(McRouteHandleProvider, proxy_route_without_provider_is_unwrapped) {
  TestSetup setup;
  auto selectors = nullRouteSelectors(setup);
  ProxyRoute<MemcacheRouterInfo> proxyRoute(
      setup.proxy(), selectors, RootRouteRolloutOpts{});
  EXPECT_EQ("root", rootRouteName(proxyRoute));
}

// The handle an extra provider returns from wrapRoot ends up above the root,
// so it sees every request no matter how the config selects routes.
TEST(McRouteHandleProvider, proxy_route_applies_wrap_root) {
  TestSetup setup;
  auto selectors = nullRouteSelectors(setup);
  MarkerExtraProvider extraProvider;
  ProxyRoute<MemcacheRouterInfo> proxyRoute(
      setup.proxy(), selectors, RootRouteRolloutOpts{}, &extraProvider);
  EXPECT_EQ("marker", rootRouteName(proxyRoute));
}
