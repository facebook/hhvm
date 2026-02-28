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
#include "mcrouter/PoolFactory.h"
#include "mcrouter/lib/carbon/example/gen/HelloGoodbyeRouterInfo.h"
#include "mcrouter/lib/config/RouteHandleFactory.h"
#include "mcrouter/options.h"
#include "mcrouter/routes/AllMajorityRouteFactory.h"
#include "mcrouter/routes/McRouteHandleProvider.h"
#include "mcrouter/routes/ShardSelectionRouteFactory.h"
#include "mcrouter/routes/test/RouteHandleTestBase.h"

using namespace facebook::memcache::mcrouter;
using namespace hellogoodbye;

namespace facebook {
namespace memcache {
namespace mcrouter {

class EagerShardSelector {
 public:
  explicit EagerShardSelector(std::unordered_map<uint32_t, uint32_t> shardsMap)
      : shardsMap_(std::move(shardsMap)) {}

  std::string type() const {
    return "basic-shard-selector";
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

class EagerShardSelectionRouteTest
    : public RouteHandleTestBase<HelloGoodbyeRouterInfo> {
 public:
  HelloGoodbyeRouterInfo::RouteHandlePtr getEagerShardSelectionRoute(
      folly::StringPiece jsonStr,
      const ChildrenFactoryMap<HelloGoodbyeRouterInfo>& childrenFactoryMap =
          {}) {
    return createEagerShardSelectionRoute<
        HelloGoodbyeRouterInfo,
        EagerShardSelector>(
        rhFactory_, folly::parseJson(jsonStr), childrenFactoryMap);
  }

  void testCreate(folly::StringPiece config) {
    auto rh = getEagerShardSelectionRoute(config);
    ASSERT_TRUE(rh);
    EXPECT_EQ("selection|basic-shard-selector", rh->routeName());
  }
};

TEST_F(EagerShardSelectionRouteTest, createPools) {
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
        ]
      },
      {
        "pool": {
          "type": "Pool",
          "name": "pool2",
          "servers": [ "localhost:12349", "localhost:12352" ],
          "protocol": "caret"
        },
        "shards": [
          [1, 2, 3],
          [4, 5, 6]
        ]
      }
    ],
    "children_settings" : {
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

TEST_F(EagerShardSelectionRouteTest, createMissingHost) {
  constexpr folly::StringPiece kSelectionRouteConfig = R"(
  {
    "children_type": "LoadBalancerRoute",
    "pools": [
      {
        "pool": {
          "type": "Pool",
          "name": "pool1",
          "servers": [ "localhost:12345" ],
          "protocol": "caret"
        },
        "shards": [
          [1, 2, 3],
          [3, 5, 6]
        ]
      }
    ],
    "children_settings" : {
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
        "EagerShardSelectionRoute: 'shards' must have the same number of "
        "entries as servers in 'pool'. Servers size: 1. Shards size: 2.",
        errorMsg);
  }
}

TEST_F(EagerShardSelectionRouteTest, createEmptyServersAndShards) {
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

TEST_F(EagerShardSelectionRouteTest, traverseAndCheckChildrenIsLoadBalancer) {
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
          "servers": [ "localhost:12302", "localhost:35602" ],
          "protocol": "caret"
        },
        "shards": [
          "1, 2, 3",
          "3, 5, 6"
        ]
      }
    ],
    "children_settings" : {
      "load_ttl_ms": 1000000,
      "default_server_load_percent": 99
    }
  }
  )";

  auto rh = getEagerShardSelectionRoute(kSelectionRouteConfig);
  ASSERT_TRUE(rh);
  EXPECT_EQ("selection|basic-shard-selector", rh->routeName());

  GoodbyeRequest req;

  req.shardId_ref() = 1;
  size_t iterations = 0;
  RouteHandleTraverser<HelloGoodbyeRouterInfo::RouteHandleIf> t{
      [&iterations](const HelloGoodbyeRouterInfo::RouteHandleIf& r) {
        ++iterations;
        if (iterations == 1) {
          EXPECT_EQ(
              folly::to<std::string>(
                  "loadbalancer|",
                  LoadBalancerRoute<HelloGoodbyeRouterInfo>::kWeightedHashing),
              r.routeName());
        }
      }};
  rh->traverse(req, t);
  EXPECT_GE(iterations, 1);
}

TEST_F(EagerShardSelectionRouteTest, traverseAndCheckChildrenIsFailover) {
  constexpr folly::StringPiece kSelectionRouteConfig = R"(
  {
    "children_type": "LatestRoute",
    "pools": [
      {
        "pool": {
          "type": "Pool",
          "name": "pool1",
          "servers": [ "localhost:12301", "localhost:35601" ],
          "protocol": "caret"
        },
        "shards": [
          "1, 2, 3",
          "3, 5"
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
        ]
      },
      {
        "pool": {
          "type": "Pool",
          "name": "pool3",
          "servers": [ "localhost:12303", "localhost:35603"],
          "protocol": "caret"
        },
        "shards": [
          "1, 2, 3",
          "3"
        ]
      }
    ],
    "children_settings" : {
      "failover_count": 2
    }
  }
  )";

  auto rh = getEagerShardSelectionRoute(kSelectionRouteConfig);
  ASSERT_TRUE(rh);
  EXPECT_EQ("selection|basic-shard-selector", rh->routeName());

  GoodbyeRequest req;

  // Shards 1 and 2 are served by 3 servers, with name starting
  // with "localhost:123"
  for (auto shardId : {1, 2}) {
    req.shardId_ref() = shardId;
    size_t iterations = 0;
    std::unordered_set<std::string> children;
    RouteHandleTraverser<HelloGoodbyeRouterInfo::RouteHandleIf> t{
        [&iterations,
         &children](const HelloGoodbyeRouterInfo::RouteHandleIf& r) {
          EXPECT_EQ(children.end(), children.find(r.routeName()));
          children.emplace(r.routeName());
          if (++iterations == 1) {
            EXPECT_EQ("failover", r.routeName());
          } else if (iterations > 1) {
            EXPECT_TRUE(r.routeName().find("host|") != std::string::npos);
            EXPECT_TRUE(
                r.routeName().find("localhost:123") != std::string::npos);
          }
        }};
    rh->traverse(req, t);
    // We should iterate 3 times, once for FailoveRoute, and 2 for hosts,
    // as failover_count is 2.
    EXPECT_EQ(iterations, 3);
  }

  // Shard 3 is served by all 6 servers.
  req.shardId_ref() = 5;
  size_t iterations = 0;
  std::unordered_set<std::string> children;
  RouteHandleTraverser<HelloGoodbyeRouterInfo::RouteHandleIf> t{
      [&iterations, &children](const HelloGoodbyeRouterInfo::RouteHandleIf& r) {
        EXPECT_EQ(children.end(), children.find(r.routeName()));
        children.emplace(r.routeName());
        if (++iterations == 1) {
          EXPECT_EQ("failover", r.routeName());
        } else if (iterations > 1) {
          EXPECT_TRUE(r.routeName().find("host|") != std::string::npos);
          EXPECT_TRUE(
              (r.routeName().find("localhost:123") != std::string::npos) ||
              (r.routeName().find("localhost:356") != std::string::npos));
        }
      }};
  rh->traverse(req, t);
  // We should iterate 3 times, once for FailoveRoute, and 2 for hosts,
  // as failover_count is 2.
  EXPECT_EQ(iterations, 3);

  // There is no shard 4.
  req.shardId_ref() = 4;
  iterations = 0;
  t = RouteHandleTraverser<HelloGoodbyeRouterInfo::RouteHandleIf>{
      [&iterations](const HelloGoodbyeRouterInfo::RouteHandleIf& r) {
        ++iterations;
        EXPECT_TRUE(r.routeName().find("error|") != std::string::npos);
      }};
  rh->traverse(req, t);
  // We should iterate just once, for ErrorRoute
  EXPECT_EQ(iterations, 1);

  // Shard 5 is served by 2 servers, with name starting with "localhost:356"
  req.shardId_ref() = 5;
  iterations = 0;
  children.clear();
  t = RouteHandleTraverser<HelloGoodbyeRouterInfo::RouteHandleIf>{
      [&iterations, &children](const HelloGoodbyeRouterInfo::RouteHandleIf& r) {
        EXPECT_EQ(children.end(), children.find(r.routeName()));
        children.emplace(r.routeName());
        if (++iterations == 1) {
          EXPECT_EQ("failover", r.routeName());
        } else if (iterations > 1) {
          EXPECT_TRUE(r.routeName().find("host|") != std::string::npos);
          EXPECT_TRUE(r.routeName().find("localhost:356") != std::string::npos);
        }
      }};
  rh->traverse(req, t);
  // We should iterate 3 times, once for FailoveRoute, and 2 for hosts,
  // as failover_count is 2.
  EXPECT_EQ(iterations, 3);

  // Shard 6 is served only by server "localhost:35602"
  req.shardId_ref() = 6;
  iterations = 0;
  t = RouteHandleTraverser<HelloGoodbyeRouterInfo::RouteHandleIf>{
      [&iterations](const HelloGoodbyeRouterInfo::RouteHandleIf& r) {
        ++iterations;
        EXPECT_TRUE(r.routeName().find("host|") != std::string::npos);
        EXPECT_TRUE(r.routeName().find("localhost:35602") != std::string::npos);
      }};
  rh->traverse(req, t);
  // We should iterate just once, for host "localhost:35602"
  // (FailoverRoute is optimized away).
  EXPECT_EQ(iterations, 1);
}

TEST_F(EagerShardSelectionRouteTest, customChildrenRoute) {
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
        ]
      }
    ]
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

  auto rh =
      getEagerShardSelectionRoute(kSelectionRouteConfig, childrenFactoryMap);
  ASSERT_TRUE(rh);
  EXPECT_EQ("selection|basic-shard-selector", rh->routeName());

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

TEST_F(EagerShardSelectionRouteTest, customJsonmRoute) {
  constexpr folly::StringPiece kSelectionRouteConfig = R"(
  {
    "children_type": "CustomJsonmRoute",
    "children_settings": {
      "type": "AllSyncRoute",
      "children": "$children_list$"
    },
    "pools": [
      {
        "pool": {
          "type": "Pool",
          "name": "pool1",
          "servers": [
            "localhost:12345",
            "localhost:12325"
          ],
          "protocol": "caret"
        },
        "shards": [
          "1, 2, 3",
          "2, 5, 6"
        ]
      },
      {
        "pool": {
          "type": "Pool",
          "name": "pool2",
          "servers": [
            "localhost:12302",
            "localhost:35602"
          ],
          "protocol": "caret"
        },
        "shards": [
          "1, 2, 3",
          "3, 5, 6"
        ]
      }
    ]
  }
  )";

  auto rh = getEagerShardSelectionRoute(kSelectionRouteConfig);
  ASSERT_TRUE(rh);
  EXPECT_EQ("selection|basic-shard-selector", rh->routeName());

  GoodbyeRequest req;

  req.shardId_ref() = 2;
  size_t iterations = 0;
  RouteHandleTraverser<HelloGoodbyeRouterInfo::RouteHandleIf> t{
      [&iterations](const HelloGoodbyeRouterInfo::RouteHandleIf& r) {
        ++iterations;
        if (iterations == 1) {
          EXPECT_EQ("all-sync", r.routeName());
        } else if (iterations == 2) {
          EXPECT_EQ(
              "host|pool=pool1|id=0|"
              "ap=localhost:12345:caret:plain:notcompressed|timeout=1000ms"
              "|keep_routing_prefix=false|msb=0",
              r.routeName());
        } else if (iterations == 3) {
          EXPECT_EQ(
              "host|pool=pool1|id=1|"
              "ap=localhost:12325:caret:plain:notcompressed|timeout=1000ms"
              "|keep_routing_prefix=false|msb=0",
              r.routeName());
        } else if (iterations == 4) {
          EXPECT_EQ(
              "host|pool=pool2|id=0|"
              "ap=localhost:12302:caret:plain:notcompressed|timeout=1000ms"
              "|keep_routing_prefix=false|msb=0",
              r.routeName());
        }
      }};
  rh->traverse(req, t);
  EXPECT_GE(iterations, 4);
}

TEST_F(EagerShardSelectionRouteTest, customJsonmRoute_twice) {
  constexpr folly::StringPiece kSelectionRouteConfig = R"(
  {
    "children_type": "CustomJsonmRoute",
    "children_settings": {
      "type": "AllSyncRoute",
      "children": [
        "$children_list$",
        "NullRoute",
        "$children_list$"
      ]
    },
    "pools": [
      {
        "pool": {
          "type": "Pool",
          "name": "pool1",
          "servers": [
            "localhost:12345",
            "localhost:12325"
          ],
          "protocol": "caret"
        },
        "shards": [
          "1, 2, 3",
          "2, 5, 6"
        ]
      },
      {
        "pool": {
          "type": "Pool",
          "name": "pool2",
          "servers": [
            "localhost:12302",
            "localhost:35602"
          ],
          "protocol": "caret"
        },
        "shards": [
          "1, 2, 3",
          "3, 5, 6"
        ]
      }
    ]
  }
  )";

  auto rh = getEagerShardSelectionRoute(kSelectionRouteConfig);
  ASSERT_TRUE(rh);
  EXPECT_EQ("selection|basic-shard-selector", rh->routeName());

  GoodbyeRequest req;

  req.shardId_ref() = 2;
  size_t iterations = 0;
  RouteHandleTraverser<HelloGoodbyeRouterInfo::RouteHandleIf> t{
      [&iterations](const HelloGoodbyeRouterInfo::RouteHandleIf& r) {
        ++iterations;
        if (iterations == 1) {
          EXPECT_EQ("all-sync", r.routeName());
        } else if (iterations == 2) {
          EXPECT_EQ(
              "host|pool=pool1|id=0|"
              "ap=localhost:12345:caret:plain:notcompressed|timeout=1000ms"
              "|keep_routing_prefix=false|msb=0",
              r.routeName());
        } else if (iterations == 3) {
          EXPECT_EQ(
              "host|pool=pool1|id=1|"
              "ap=localhost:12325:caret:plain:notcompressed|timeout=1000ms"
              "|keep_routing_prefix=false|msb=0",
              r.routeName());
        } else if (iterations == 4) {
          EXPECT_EQ(
              "host|pool=pool2|id=0|"
              "ap=localhost:12302:caret:plain:notcompressed|timeout=1000ms"
              "|keep_routing_prefix=false|msb=0",
              r.routeName());
        } else if (iterations == 5) {
          EXPECT_EQ("null", r.routeName());
        } else if (iterations == 6) {
          EXPECT_EQ(
              "host|pool=pool1|id=0|"
              "ap=localhost:12345:caret:plain:notcompressed|timeout=1000ms"
              "|keep_routing_prefix=false|msb=0",
              r.routeName());
        } else if (iterations == 7) {
          EXPECT_EQ(
              "host|pool=pool1|id=1|"
              "ap=localhost:12325:caret:plain:notcompressed|timeout=1000ms"
              "|keep_routing_prefix=false|msb=0",
              r.routeName());
        } else if (iterations == 8) {
          EXPECT_EQ(
              "host|pool=pool2|id=0|"
              "ap=localhost:12302:caret:plain:notcompressed|timeout=1000ms"
              "|keep_routing_prefix=false|msb=0",
              r.routeName());
        }
      }};
  rh->traverse(req, t);
  EXPECT_GE(iterations, 8);
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
