/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <unordered_set>
#include <vector>

#include <gtest/gtest.h>

#include <folly/Range.h>
#include <folly/json/json.h>

#include "mcrouter/CarbonRouterInstance.h"
#include "mcrouter/McrouterInstance.h"
#include "mcrouter/PoolFactory.h"
#include "mcrouter/lib/carbon/example/gen/HelloGoodbyeRouterInfo.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/lib/test/RouteHandleTestUtil.h"
#include "mcrouter/lib/test/TestRouteHandle.h"
#include "mcrouter/options.h"
#include "mcrouter/routes/AllMajorityRouteFactory.h"
#include "mcrouter/routes/McRouteHandleProvider.h"
#include "mcrouter/routes/ShardSelectionRouteFactory.h"
#include "mcrouter/routes/test/RouteHandleTestBase.h"
#include "mcrouter/routes/test/RouteHandleTestUtil.h"

using namespace facebook::memcache;
using namespace facebook::memcache::mcrouter;
using namespace hellogoodbye;

namespace {

using FiberManagerContextTag =
    typename fiber_local<MemcacheRouterInfo>::ContextTypeTag;

McrouterInstance* getRouter() {
  McrouterOptions opts = defaultTestOptions();
  opts.config = "{ \"route\": \"NullRoute\" }";
  return McrouterInstance::init("test_shadow", opts);
}

std::shared_ptr<ProxyRequestContextWithInfo<MemcacheRouterInfo>> getContext() {
  return ProxyRequestContextWithInfo<MemcacheRouterInfo>::createRecording(
      *getRouter()->getProxy(0), nullptr);
}

} // anonymous namespace

namespace facebook {
namespace memcache {
namespace mcrouter {

class EagerShardShadowSelectorPolicy {
 public:
  explicit EagerShardShadowSelectorPolicy(
      std::unordered_map<std::string, size_t>& shadowResults)
      : shadowResults_(shadowResults) {}

  template <class Reply>
  folly::Function<void(const Reply&)> makePostShadowReplyFn() const {
    folly::Function<void(const Reply&)> fn = [&](const Reply& reply) {
      std::string v = *reply.message_ref();
      auto it = shadowResults_.find(v);
      if (it != shadowResults_.end()) {
        shadowResults_[v]++;
      } else {
        shadowResults_.emplace(v, 1);
      }
    };
    return fn;
  }

 private:
  std::unordered_map<std::string, size_t>& shadowResults_;
};

class EagerShardShadowSelector {
 public:
  explicit EagerShardShadowSelector(
      std::unordered_map<uint32_t, uint32_t> shardsMap)
      : shardsMap_(std::move(shardsMap)) {}

  std::string type() const {
    return "basic-shadow-shard-selector";
  }

  template <class Request>
  size_t select(const Request& req, size_t /* size */) const {
    auto dest = shardsMap_.find(*req.shardId_ref());
    if (dest == shardsMap_.end()) {
      // if the shard is not found in the map, return a value outside of range
      // of valid destinations (i.e. >= size), so that we error the request.
      return std::numeric_limits<size_t>::max();
    }
    return dest->second;
  }

 private:
  const std::unordered_map<uint32_t, uint32_t> shardsMap_;
};

class EagerShardSelectionShadowRouteTest
    : public RouteHandleTestBase<HelloGoodbyeRouterInfo> {
 public:
  HelloGoodbyeRouterInfo::RouteHandlePtr getEagerShardSelectionShadowRoute(
      folly::StringPiece jsonStr,
      const ChildrenFactoryMap<HelloGoodbyeRouterInfo>& childrenFactoryMap = {},
      const ChildrenFactoryMap<HelloGoodbyeRouterInfo>&
          shadowChildrenFactoryMap = {},
      const folly::Optional<EagerShardShadowSelectorPolicy>&
          shadowSelectorPolicy = folly::none) {
    return createEagerShardSelectionShadowRoute<
        HelloGoodbyeRouterInfo,
        EagerShardShadowSelector,
        std::unordered_map<uint32_t, uint32_t>,
        EagerShardShadowSelectorPolicy>(
        rhFactory_,
        folly::parseJson(jsonStr),
        childrenFactoryMap,
        shadowChildrenFactoryMap,
        shadowSelectorPolicy,
        /* seed */ 0);
  }

  void testCreate(
      folly::StringPiece config,
      bool shards = true,
      bool shadowShards = true) {
    auto rh = getEagerShardSelectionShadowRoute(config);
    ASSERT_TRUE(rh);
    if (!shards && !shadowShards) {
      EXPECT_EQ(
          "error|log|mc_res_local_error|EagerShardSelectionShadowRoute has an empty list of destinations",
          rh->routeName());
    } else if (!shards && shadowShards) {
      EXPECT_EQ(
          "selection|basic-shadow-shard-selector|shadow_enabled",
          rh->routeName());
    } else if (shards && !shadowShards) {
      EXPECT_EQ("selection|basic-shadow-shard-selector", rh->routeName());
    } else {
      EXPECT_EQ(
          "selection|basic-shadow-shard-selector|shadow_enabled",
          rh->routeName());
    }
  }
};

TEST_F(EagerShardSelectionShadowRouteTest, latestRouteTest) {
  constexpr folly::StringPiece kSelectionRouteConfig = R"(
  {
    "children_type": "LatestRoute",
    "pools": [
      {
        "pool": {
          "type": "Pool",
          "name": "pool1",
          "servers": [ "localhost:12345", "localhost:12312" ],
          "protocol": "caret"
        },
        "shards": [
          "1, 2, 3",
          "4, 5, 6"
        ],
        "shadow_shards": [
          "7",
          "8"
        ]
      }
    ],
    "children_settings" : {
      "failover_count": 2
    },
    "shadow_children_type": "LatestRoute",
    "shadow_children_settings" : {
      "failover_count": 2
    }
  }
  )";

  try {
    testCreate(kSelectionRouteConfig);
  } catch (const std::exception& e) {
    FAIL() << "Configuration failed, but should have succeeded. Exception: "
           << e.what();
  }
}

TEST_F(EagerShardSelectionShadowRouteTest, customChildrenRoute) {
  constexpr folly::StringPiece kSelectionRouteConfig = R"(
  {
    "children_type": "AllMajorityRoute",
    "children_settings": {},
    "pools": [
      {
        "pool": {
          "type": "Pool",
          "name": "pool1",
          "servers": [ "localhost:12345", "localhost:12325" ],
          "protocol": "caret"
        },
        "shards": [
          "1, 2, 3",
          "3, 5, 6"
        ],
        "shadow_shards": [
          "7",
          "8"
        ]
      },
      {
        "pool": {
          "type": "Pool",
          "name": "pool2",
          "servers": [ "localhost:12302", "localhost:35602" ],
          "protocol": "caret"
        },
        "shards": [
          "1, 2, 3",
          "3, 5, 6"
        ],
        "shadow_shards": [
          "7",
          "8"
        ]
      }
    ],
    "shadow_children_settings": {},
    "shadow_children_type": "AllMajorityRoute"
  }
  )";

  ChildrenFactoryMap<HelloGoodbyeRouterInfo> childrenFactoryMap{
      {"AllMajorityRoute",
       [](RouteHandleFactory<HelloGoodbyeRouterInfo::RouteHandleIf>&,
          const folly::dynamic&,
          std::vector<HelloGoodbyeRouterInfo::RouteHandlePtr> children) {
         return createAllMajorityRoute<HelloGoodbyeRouterInfo>(
             std::move(children));
       }}};

  ChildrenFactoryMap<HelloGoodbyeRouterInfo> shadowChildrenFactoryMap{
      {"AllMajorityRoute",
       [](RouteHandleFactory<HelloGoodbyeRouterInfo::RouteHandleIf>&,
          const folly::dynamic&,
          std::vector<HelloGoodbyeRouterInfo::RouteHandlePtr> children) {
         return createAllMajorityRoute<HelloGoodbyeRouterInfo>(
             std::move(children));
       }}};

  auto rh = getEagerShardSelectionShadowRoute(
      kSelectionRouteConfig, childrenFactoryMap, shadowChildrenFactoryMap);
  ASSERT_TRUE(rh);
  EXPECT_EQ(
      "selection|basic-shadow-shard-selector|shadow_enabled", rh->routeName());

  GoodbyeRequest req;

  req.shardId_ref() = 1;
  size_t iterations = 0;
  RouteHandleTraverser<HelloGoodbyeRouterInfo::RouteHandleIf> t{
      [&iterations](const HelloGoodbyeRouterInfo::RouteHandleIf& r) {
        ++iterations;
        if (iterations == 1) {
          EXPECT_EQ("all-majority", r.routeName());
        }
      }};
  rh->traverse(req, t);
  EXPECT_GE(iterations, 1);
}

TEST_F(EagerShardSelectionShadowRouteTest, createPool) {
  constexpr folly::StringPiece kSelectionRouteConfig = R"(
  {
    "children_type": "LoadBalancerRoute",
    "pools": [
      {
        "pool": {
          "type": "Pool",
          "name": "pool1",
          "servers": [ "localhost:12345", "localhost:12312" ],
          "protocol": "caret"
        },
        "shards": [
          "1, 2, 3",
          "4, 5, 6"
        ],
        "shadow_shards": [
          "7",
          "8"
        ]
      }
    ],
    "children_settings" : {
      "load_ttl_ms": 100
    },
    "shadow_children_type": "LoadBalancerRoute",
    "shadow_children_settings" : {
      "load_ttl_ms": 100
    }
  }
  )";

  try {
    testCreate(kSelectionRouteConfig);
  } catch (const std::exception& e) {
    FAIL() << "Configuration failed, but should have succeeded. Exception: "
           << e.what();
  }
}

TEST_F(EagerShardSelectionShadowRouteTest, createPoolNoShards) {
  constexpr folly::StringPiece kSelectionRouteConfig = R"(
  {
    "children_type": "LoadBalancerRoute",
    "pools": [
      {
        "pool": {
          "type": "Pool",
          "name": "pool1",
          "servers": [ "localhost:12345", "localhost:12312" ],
          "protocol": "caret"
        },
        "shards": [
          "",
          ""
        ],
        "shadow_shards": [
          "7",
          "8"
        ]
      }
    ],
    "children_settings" : {
      "load_ttl_ms": 100
    },
    "shadow_children_settings" : {
      "load_ttl_ms": 100
    },
    "shadow_children_type": "LoadBalancerRoute"
  }
  )";

  try {
    testCreate(kSelectionRouteConfig, false, true);
  } catch (const std::exception& e) {
    FAIL() << "Configuration failed, but should have succeeded. Exception: "
           << e.what();
  }
}

TEST_F(EagerShardSelectionShadowRouteTest, createPoolNoShadowShards) {
  constexpr folly::StringPiece kSelectionRouteConfig = R"(
  {
    "children_type": "LoadBalancerRoute",
    "pools": [
      {
        "pool": {
          "type": "Pool",
          "name": "pool1",
          "servers": [ "localhost:12345", "localhost:12312" ],
          "protocol": "caret"
        },
        "shards": [
          "1,2,3",
          "4,5,6"
        ],
        "shadow_shards": [
          "",
          ""
        ]
      }
    ],
    "children_settings" : {
      "load_ttl_ms": 100
    },
    "shadow_children_type": "LoadBalancerRoute",
    "shadow_children_settings" : {
      "load_ttl_ms": 100
    }
  }
  )";

  try {
    testCreate(kSelectionRouteConfig, true, false);
  } catch (const std::exception& e) {
    FAIL() << "Configuration failed, but should have succeeded. Exception: "
           << e.what();
  }
}

TEST_F(EagerShardSelectionShadowRouteTest, createPoolDisjoint) {
  constexpr folly::StringPiece kSelectionRouteConfig = R"(
  {
    "children_type": "LoadBalancerRoute",
    "pools": [
      {
        "pool": {
          "type": "Pool",
          "name": "pool1",
          "servers": [ "localhost:12345", "localhost:12312" ],
          "protocol": "caret"
        },
        "shards": [
          "1,2,3",
          "4,5,6"
        ],
        "shadow_shards": [
          "7,8,9",
          "10,11,12"
        ]
      }
    ],
    "children_settings" : {
      "load_ttl_ms": 100
    },
    "shadow_children_type": "LoadBalancerRoute",
    "shadow_children_settings" : {
      "load_ttl_ms": 100
    }
  }
  )";

  try {
    testCreate(kSelectionRouteConfig);
  } catch (const std::exception& e) {
    FAIL() << "Configuration failed, but should have succeeded. Exception: "
           << e.what();
  }
}

TEST_F(EagerShardSelectionShadowRouteTest, createPoolNoShardsNoShadowShards) {
  constexpr folly::StringPiece kSelectionRouteConfig = R"(
  {
    "children_type": "LoadBalancerRoute",
    "pools": [
      {
        "pool": {
          "type": "Pool",
          "name": "pool1",
          "servers": [ "localhost:12345", "localhost:12312" ],
          "protocol": "caret"
        },
        "shards": [
          "",
          ""
        ],
        "shadow_shards": [
          "",
          ""
        ]
      }
    ],
    "children_settings" : {
      "load_ttl_ms": 100
    },
    "shadow_children_type": "LoadBalancerRoute",
    "shadow_children_settings" : {
      "load_ttl_ms": 100
    }
  }
  )";

  try {
    testCreate(kSelectionRouteConfig, false, false);
  } catch (const std::exception& e) {
    FAIL() << "Configuration failed, but should have succeeded. Exception: "
           << e.what();
  }
}

TEST_F(EagerShardSelectionShadowRouteTest, createMissingShadowHost) {
  constexpr folly::StringPiece kSelectionRouteConfig = R"(
  {
    "children_type": "LoadBalancerRoute",
    "pools": [
      {
        "pool": {
          "type": "Pool",
          "name": "pool1",
          "servers": [ "localhost:12345", "localhost:12312" ],
          "protocol": "caret"
        },
        "shards": [
          "1,2,3",
          "4,5,6"
        ],
        "shadow_shards": [
          "1",
          "2",
          "3"
        ]
      }
    ],
    "children_settings" : {
      "load_ttl_ms": 100
    },
    "shadow_children_type": "LoadBalancerRoute",
    "shadow_children_settings" : {
      "load_ttl_ms": 100
    }
  }
  )";

  // should throw, because we have one extra entry in "shards" array
  // when compared to "servers" array.
  try {
    testCreate(kSelectionRouteConfig);
    FAIL() << "Config is invalid (there's one missing host)."
           << " Should have thrown.";
  } catch (const std::exception& e) {
    std::string errorMsg = e.what();
    EXPECT_EQ(
        "EagerShardSelectionRoute: 'shadow_shards' must have the same number of "
        "entries as servers in 'pool'. Servers size: 2. Shards size: 3.",
        errorMsg);
  }
}

TEST_F(EagerShardSelectionShadowRouteTest, createEmptyServersAndShards) {
  constexpr folly::StringPiece kSelectionRouteConfig = R"(
  {
    "children_type": "LoadBalancerRoute",
    "pools": [
      {
        "pool": {
          "type": "Pool",
          "name": "pool1",
          "servers": [ "localhost:12345", "localhost:12325" ],
          "protocol": "caret"
        },
        "shards": [
          "1, 2, 3",
          "3, 5, 6"
        ],
        "shadow_shards": [
          "4, 5, 6",
          "1, 2, 3"
        ]
      },
      {
        "pool": {
          "type": "Pool",
          "name": "pool2",
          "servers": [ ],
          "protocol": "caret"
        },
        "shards": [ ],
        "shadow_shards": [ ]
      }
    ],
    "children_settings" : {
      "load_ttl_ms": 100
    },
    "shadow_children_type": "LoadBalancerRoute",
    "shadow_children_settings" : {
      "load_ttl_ms": 100
    }
  }
  )";

  // should configure fine because number of servers and number of shards
  // matches in both cases.
  try {
    testCreate(kSelectionRouteConfig);
  } catch (const std::exception& e) {
    FAIL() << "Configuration failed, but should have succeeded. Exception: "
           << e.what();
  }
}

TEST_F(EagerShardSelectionShadowRouteTest, createMissingShadowShards) {
  constexpr folly::StringPiece kSelectionRouteConfig = R"(
  {
    "children_type": "LoadBalancerRoute",
    "pools": [
      {
        "pool": {
          "type": "Pool",
          "name": "pool1",
          "servers": [ "localhost:12345", "localhost:12325" ],
          "protocol": "caret"
        },
        "shards": [
          "1, 2, 3",
          "3, 5, 6"
        ]
      },
      {
        "pool": {
          "type": "Pool",
          "name": "pool2",
          "servers": [ ],
          "protocol": "caret"
        },
        "shards": [ ]
      }
    ],
    "children_settings" : {
      "load_ttl_ms": 100
    },
    "shadow_children_type": "LoadBalancerRoute",
    "shadow_children_settings" : {
      "load_ttl_ms": 100
    }
  }
  )";

  // should configure fine because number of servers and number of shards
  // matches in both cases.
  try {
    testCreate(kSelectionRouteConfig, true, false);
  } catch (const std::exception& e) {
    FAIL() << "Configuration failed, but should have succeeded. Exception: "
           << e.what();
  }
}

TEST_F(EagerShardSelectionShadowRouteTest, basicRequestTest) {
  constexpr folly::StringPiece kSelectionRouteConfig = R"(
  {
    "children_type": "LoadBalancerRoute",
    "pools": [
      {
        "pool": {
          "type": "Pool",
          "name": "pool1",
          "servers": [
            {
              "type": "ErrorRoute",
              "response": "h1",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h2",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h3",
              "enable_logging": false
            }
          ],
          "protocol": "caret"
        },
        "shards": [
          "1,2",
          "2,3",
          ""
        ],
        "shadow_shards": [
          "",
          "",
          "1"
        ]
      }
    ],
    "children_settings" : {
      "load_ttl_ms": 100
    },
    "shadow_children_type": "LoadBalancerRoute",
    "shadow_children_settings" : {
      "load_ttl_ms": 100
    }
  }
  )";

  TestFiberManager<MemcacheRouterInfo> fm;
  std::unordered_map<std::string, size_t> normalResults;
  std::unordered_map<std::string, size_t> shadowResults;
  EagerShardShadowSelectorPolicy policy(shadowResults);
  auto rh =
      getEagerShardSelectionShadowRoute(kSelectionRouteConfig, {}, {}, policy);

  auto ctx = getContext();
  fm.run([&]() {
    for (int i = 0; i < 100; i++) {
      GoodbyeRequest req;
      req.shardId_ref() = 1;
      auto reply = rh->route(req);
      std::string v = *reply.message_ref();
      auto it = normalResults.find(v);
      if (it != normalResults.end()) {
        normalResults[v]++;
      } else {
        normalResults.emplace(v, 1);
      }
    }
  });
  // 100 requests should hit h1 through normal path
  EXPECT_TRUE(normalResults["h1"] == 100);
  // 50 requests should hit h3 through shadow path
  EXPECT_TRUE(shadowResults["h3"] >= 40 && shadowResults["h3"] <= 60);
}

TEST_F(EagerShardSelectionShadowRouteTest, basicShardTest) {
  constexpr folly::StringPiece kSelectionRouteConfig = R"(
  {
    "children_type": "LoadBalancerRoute",
    "pools": [
      {
        "pool": {
          "type": "Pool",
          "name": "pool1",
          "servers": [
            {
              "type": "ErrorRoute",
              "response": "h1",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h2",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h3",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h4",
              "enable_logging": false
            }
          ],
          "protocol": "caret"
        },
        "shards": [
          "1,2",
          "1,3",
          "1",
          "3"
        ],
        "shadow_shards": [
          "",
          "",
          "",
          "1"
        ]
      }
    ],
    "children_settings" : {
      "load_ttl_ms": 100,
      "algorithm": "two-random-choices"
    },
    "shadow_children_type": "LoadBalancerRoute",
    "shadow_children_settings" : {
      "load_ttl_ms": 100,
      "algorithm": "two-random-choices"
    }
  }
  )";

  TestFiberManager<MemcacheRouterInfo> fm;
  std::unordered_map<std::string, size_t> normalResults;
  std::unordered_map<std::string, size_t> shadowResults;
  EagerShardShadowSelectorPolicy policy(shadowResults);
  auto rh =
      getEagerShardSelectionShadowRoute(kSelectionRouteConfig, {}, {}, policy);

  auto ctx = getContext();
  fm.run([&]() {
    for (int i = 0; i < 100; i++) {
      GoodbyeRequest req;
      req.shardId_ref() = 1;
      auto reply = rh->route(req);
      std::string v = *reply.message_ref();
      auto it = normalResults.find(v);
      if (it != normalResults.end()) {
        normalResults[v]++;
      } else {
        normalResults.emplace(v, 1);
      }
    }
  });
  size_t normalTotal = 0;
  for (const auto& kv : normalResults) {
    normalTotal += kv.second;
  }
  // 100 requests distributed between h1, h2 and h3 evenly
  EXPECT_TRUE(normalTotal == 100);
  // 25 requests should hit h4 through shadow path
  EXPECT_TRUE(shadowResults["h4"] >= 15 && shadowResults["h4"] <= 35);
}

TEST_F(EagerShardSelectionShadowRouteTest, basicShardWithWeightsTest) {
  constexpr folly::StringPiece kSelectionRouteConfig = R"(
  {
    "children_type": "LoadBalancerRoute",
    "pools": [
      {
        "pool": {
          "type": "Pool",
          "name": "pool1",
          "servers": [
            {
              "type": "ErrorRoute",
              "response": "h1",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h2",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h3",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h4",
              "enable_logging": false
            }
          ],
          "protocol": "caret"
        },
        "shards": [
          "1,2",
          "1,3",
          "1",
          "3"
        ],
        "shadow_shards": [
          "",
          "",
          "",
          "1"
        ]
      }
    ],
    "children_settings" : {
      "load_ttl_ms": 100,
      "algorithm": "two-random-choices"
    },
    "shadow_children_type": "LoadBalancerRoute",
    "shadow_weights": 2.0,
    "shadow_children_settings" : {
      "load_ttl_ms": 100,
      "algorithm": "two-random-choices"
    }
  }
  )";

  TestFiberManager<MemcacheRouterInfo> fm;
  std::unordered_map<std::string, size_t> normalResults;
  std::unordered_map<std::string, size_t> shadowResults;
  EagerShardShadowSelectorPolicy policy(shadowResults);
  auto rh =
      getEagerShardSelectionShadowRoute(kSelectionRouteConfig, {}, {}, policy);

  auto ctx = getContext();
  fm.run([&]() {
    for (int i = 0; i < 100; i++) {
      GoodbyeRequest req;
      req.shardId_ref() = 1;
      auto reply = rh->route(req);
      std::string v = *reply.message_ref();
      auto it = normalResults.find(v);
      if (it != normalResults.end()) {
        normalResults[v]++;
      } else {
        normalResults.emplace(v, 1);
      }
    }
  });
  size_t normalTotal = 0;
  for (const auto& kv : normalResults) {
    normalTotal += kv.second;
  }
  // 100 requests distributed between h1, h2 and h3 evenly
  EXPECT_TRUE(normalTotal == 100);
  // 40 requests should hit h4 through shadow path
  EXPECT_TRUE(shadowResults["h4"] >= 30 && shadowResults["h4"] <= 50);
}

TEST_F(EagerShardSelectionShadowRouteTest, basicShardWithLowWeightsTest) {
  constexpr folly::StringPiece kSelectionRouteConfig = R"(
  {
    "children_type": "LoadBalancerRoute",
    "pools": [
      {
        "pool": {
          "type": "Pool",
          "name": "pool1",
          "servers": [
            {
              "type": "ErrorRoute",
              "response": "h1",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h2",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h3",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h4",
              "enable_logging": false
            }
          ],
          "protocol": "caret"
        },
        "shards": [
          "1,2",
          "3",
          "3",
          "3"
        ],
        "shadow_shards": [
          "",
          "1",
          "1",
          "1"
        ]
      }
    ],
    "children_settings" : {
      "load_ttl_ms": 100,
      "algorithm": "two-random-choices"
    },
    "shadow_children_type": "LoadBalancerRoute",
    "shadow_weights": 0.5,
    "shadow_children_settings" : {
      "load_ttl_ms": 100,
      "algorithm": "two-random-choices"
    }
  }
  )";

  TestFiberManager<MemcacheRouterInfo> fm;
  std::unordered_map<std::string, size_t> normalResults;
  std::unordered_map<std::string, size_t> shadowResults;
  EagerShardShadowSelectorPolicy policy(shadowResults);
  auto rh =
      getEagerShardSelectionShadowRoute(kSelectionRouteConfig, {}, {}, policy);

  auto ctx = getContext();
  fm.run([&]() {
    for (int i = 0; i < 100; i++) {
      GoodbyeRequest req;
      req.shardId_ref() = 1;
      auto reply = rh->route(req);
      std::string v = *reply.message_ref();
      auto it = normalResults.find(v);
      if (it != normalResults.end()) {
        normalResults[v]++;
      } else {
        normalResults.emplace(v, 1);
      }
    }
  });
  size_t normalTotal = 0;
  for (const auto& kv : normalResults) {
    normalTotal += kv.second;
  }
  size_t shadowTotal = 0;
  for (const auto& kv : shadowResults) {
    shadowTotal += kv.second;
  }
  // 100 requests hit h1
  EXPECT_TRUE(normalTotal == 100);
  // ~37 requests should hit distributed between h2, h3, h4
  EXPECT_TRUE(shadowTotal >= 20 && shadowTotal <= 45);
}

TEST_F(EagerShardSelectionShadowRouteTest, basicShadowShardTest) {
  constexpr folly::StringPiece kSelectionRouteConfig = R"(
  {
    "children_type": "LoadBalancerRoute",
    "pools": [
      {
        "pool": {
          "type": "Pool",
          "name": "pool1",
          "servers": [
            {
              "type": "ErrorRoute",
              "response": "h1",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h2",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h3",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h4",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h5",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h6",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h7",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h8",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h9",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h10",
              "enable_logging": false
            }
          ],
          "protocol": "caret"
        },
        "shards": [
          "1",
          "3",
          "2",
          "",
          "",
          "",
          "",
          "",
          "",
          ""
        ],
        "shadow_shards": [
          "",
          "1",
          "1",
          "1",
          "1",
          "1",
          "1",
          "1",
          "1",
          "1"
        ]
      }
    ],
    "children_settings" : {
      "load_ttl_ms": 100,
      "algorithm": "two-random-choices"
    },
    "shadow_children_type": "LoadBalancerRoute",
    "shadow_children_settings" : {
      "load_ttl_ms": 10,
      "algorithm": "two-random-choices"
    }
  }
  )";

  TestFiberManager<MemcacheRouterInfo> fm;
  std::unordered_map<std::string, size_t> normalResults;
  std::unordered_map<std::string, size_t> shadowResults;
  EagerShardShadowSelectorPolicy policy(shadowResults);
  auto rh =
      getEagerShardSelectionShadowRoute(kSelectionRouteConfig, {}, {}, policy);

  auto ctx = getContext();
  fm.run([&]() {
    for (int i = 0; i < 100; i++) {
      GoodbyeRequest req;
      req.shardId_ref() = 1;
      auto reply = rh->route(req);
      std::string v = *reply.message_ref();
      auto it = normalResults.find(v);
      if (it != normalResults.end()) {
        normalResults[v]++;
      } else {
        normalResults.emplace(v, 1);
      }
    }
  });
  // 100 requests hit h1
  EXPECT_TRUE(normalResults["h1"] = 100);
  // 90 requests evenly distributed on shadow path
  int32_t shadowTotal = 0;
  for (const auto& kv : shadowResults) {
    shadowTotal += kv.second;
  }
  EXPECT_TRUE(shadowTotal >= 80 && shadowTotal <= 95);
}

TEST_F(EagerShardSelectionShadowRouteTest, basicShadowShardWithShardTest) {
  constexpr folly::StringPiece kSelectionRouteConfig = R"(
  {
    "children_type": "LoadBalancerRoute",
    "pools": [
      {
        "pool": {
          "type": "Pool",
          "name": "pool1",
          "servers": [
            {
              "type": "ErrorRoute",
              "response": "h1",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h2",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h3",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h4",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h5",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h6",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h7",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h8",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h9",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h10",
              "enable_logging": false
            }
          ],
          "protocol": "caret"
        },
        "shards": [
          "2",
          "",
          "",
          "",
          "",
          "",
          "",
          "",
          "",
          ""
        ],
        "shadow_shards": [
          "",
          "1",
          "1",
          "1",
          "1",
          "1",
          "1",
          "1",
          "1",
          "1"
        ]
      }
    ],
    "out_of_range": "ErrorRoute|Out of range!",
    "children_settings" : {
      "load_ttl_ms": 100,
      "algorithm": "two-random-choices"
    },
    "shadow_children_type": "LoadBalancerRoute",
    "shadow_children_settings" : {
      "load_ttl_ms": 10,
      "algorithm": "two-random-choices"
    }
  }
  )";

  TestFiberManager<MemcacheRouterInfo> fm;
  std::unordered_map<std::string, size_t> shadowResults;
  EagerShardShadowSelectorPolicy policy(shadowResults);
  auto rh =
      getEagerShardSelectionShadowRoute(kSelectionRouteConfig, {}, {}, policy);

  auto ctx = getContext();
  fm.run([&]() {
    for (int i = 0; i < 100; i++) {
      GoodbyeRequest req;
      req.shardId_ref() = 1;
      auto reply = rh->route(req);
      EXPECT_EQ("Out of range!", *reply.message_ref());
    }
  });
  // 100 requests on shadow path
  int32_t shadowTotal = 0;
  for (const auto& kv : shadowResults) {
    shadowTotal += kv.second;
  }
  EXPECT_TRUE(shadowTotal == 100);
}

TEST_F(EagerShardSelectionShadowRouteTest, basicShadowShardWithNoShardTest) {
  constexpr folly::StringPiece kSelectionRouteConfig = R"(
  {
    "children_type": "LoadBalancerRoute",
    "pools": [
      {
        "pool": {
          "type": "Pool",
          "name": "pool1",
          "servers": [
            {
              "type": "ErrorRoute",
              "response": "h1",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h2",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h3",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h4",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h5",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h6",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h7",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h8",
              "enable_logging": false
            },
            {
              "type": "ErrorRoute",
              "response": "h9",
              "enable_logging": false
            }
          ],
          "protocol": "caret"
        },
        "shards": [
          "",
          "",
          "",
          "",
          "",
          "",
          "",
          "",
          ""
        ],
        "shadow_shards": [
          "1",
          "1",
          "",
          "",
          "1",
          "",
          "1",
          "",
          ""
        ]
      }
    ],
    "out_of_range": "ErrorRoute|Out of range!",
    "children_settings" : {
      "load_ttl_ms": 100,
      "algorithm": "two-random-choices"
    },
    "shadow_children_type": "LoadBalancerRoute",
    "shadow_children_settings" : {
      "load_ttl_ms": 10,
      "algorithm": "two-random-choices"
    }
  }
  )";

  TestFiberManager<MemcacheRouterInfo> fm;
  std::unordered_map<std::string, size_t> shadowResults;
  EagerShardShadowSelectorPolicy policy(shadowResults);
  auto rh =
      getEagerShardSelectionShadowRoute(kSelectionRouteConfig, {}, {}, policy);

  auto ctx = getContext();
  fm.run([&]() {
    for (int i = 0; i < 100; i++) {
      GoodbyeRequest req;
      req.shardId_ref() = 1;
      auto reply = rh->route(req);
      EXPECT_EQ("Out of range!", *reply.message_ref());
    }
  });
  // 100 requests on shadow path
  int32_t shadowTotal = 0;
  for (const auto& kv : shadowResults) {
    shadowTotal += kv.second;
  }
  EXPECT_TRUE(shadowTotal == 100);
  // 25 requests should hit h1 through shadow path
  EXPECT_TRUE(shadowResults["h1"] >= 15 && shadowResults["h1"] <= 35);
  // 25 requests should hit h2 through shadow path
  EXPECT_TRUE(shadowResults["h2"] >= 15 && shadowResults["h2"] <= 35);
  // 25 requests should hit h3 through shadow path
  EXPECT_TRUE(shadowResults["h5"] >= 15 && shadowResults["h3"] <= 35);
  // 25 requests should hit h4 through shadow path
  EXPECT_TRUE(shadowResults["h7"] >= 15 && shadowResults["h4"] <= 35);

  // All others should receive no traffic
  EXPECT_EQ(shadowResults["h3"], 0);
  EXPECT_EQ(shadowResults["h4"], 0);
  EXPECT_EQ(shadowResults["h6"], 0);
  EXPECT_EQ(shadowResults["h8"], 0);
  EXPECT_EQ(shadowResults["h9"], 0);
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
