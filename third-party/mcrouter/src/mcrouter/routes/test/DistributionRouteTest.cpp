/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <vector>

#include <folly/Range.h>
#include <gtest/gtest.h>

#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/lib/invalidation/McInvalidationDefs.h"
#include "mcrouter/lib/invalidation/McInvalidationKvPairs.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/lib/network/gen/MemcacheRouterInfo.h"
#include "mcrouter/routes/DistributionRoute.h"
#include "mcrouter/routes/test/RouteHandleTestUtil.h"
#include "thrift/lib/cpp2/protocol/Serializer.h"

namespace facebook::memcache::mcrouter {

TEST(DistributionRouteTest, getSetAreForwardedToRpc) {
  std::vector<std::shared_ptr<TestHandle>> srHandleVec{
      std::make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "a")),
  };
  auto mockSrHandle = get_route_handles(srHandleVec)[0];

  constexpr folly::StringPiece kDistributionRouteConfig = R"(
  {
    "distributed_delete_rpc_enabled": true,
    "replay": false
  }
  )";

  auto rh = makeDistributionRoute<MemcacheRouterInfo>(
      mockSrHandle, folly::parseJson(kDistributionRouteConfig));
  mockFiberContext();
  TestFiberManager<MemcacheRouterInfo> fm;
  fm.runAll({[&]() { rh->route(McGetRequest("getReq")); }});
  EXPECT_FALSE(srHandleVec[0]->saw_keys.empty());
  EXPECT_EQ(srHandleVec[0]->saw_keys[0], "getReq");
  EXPECT_EQ("get", srHandleVec[0]->sawOperations[0]);
  fm.runAll({[&]() { rh->route(McSetRequest("setReq")); }});
  EXPECT_EQ(srHandleVec[0]->saw_keys[1], "setReq");
  EXPECT_EQ("set", srHandleVec[0]->sawOperations[1]);
}

TEST(DistributionRouteTest, getSetAreForwardedToRpc2) {
  std::vector<std::shared_ptr<TestHandle>> srHandleVec{
      std::make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "a")),
  };
  auto mockSrHandle = get_route_handles(srHandleVec)[0];

  constexpr folly::StringPiece kDistributionRouteConfig = R"(
  {
    "distributed_delete_rpc_enabled": true,
    "replay": false
  }
  )";

  auto rh = makeDistributionRoute<MemcacheRouterInfo>(
      mockSrHandle, folly::parseJson(kDistributionRouteConfig));
  mockFiberContext();
  TestFiberManager<MemcacheRouterInfo> fm;
  fm.runAll({[&]() { rh->route(McGetRequest("getReq")); }});
  EXPECT_FALSE(srHandleVec[0]->saw_keys.empty());
  EXPECT_EQ(srHandleVec[0]->saw_keys[0], "getReq");
  EXPECT_EQ("get", srHandleVec[0]->sawOperations[0]);
  fm.runAll({[&]() { rh->route(McSetRequest("setReq")); }});
  EXPECT_EQ(srHandleVec[0]->saw_keys[1], "setReq");
  EXPECT_EQ("set", srHandleVec[0]->sawOperations[1]);
}

TEST(DistributionRouteTest, deleteForwardedToRpcIfDisabled) {
  std::vector<std::shared_ptr<TestHandle>> srHandleVec{
      std::make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "a")),
  };
  auto mockSrHandle = get_route_handles(srHandleVec)[0];

  constexpr folly::StringPiece kDistributionRouteConfig = R"(
  {
    "distributed_delete_rpc_enabled": false,
    "replay": false
  }
  )";

  auto rh = makeDistributionRoute<MemcacheRouterInfo>(
      mockSrHandle, folly::parseJson(kDistributionRouteConfig));
  mockFiberContext();
  fiber_local<MemcacheRouterInfo>::runWithLocals(
      [&]() { rh->route(McDeleteRequest("test1")); });
  EXPECT_FALSE(srHandleVec[0]->saw_keys.empty());
  EXPECT_EQ(srHandleVec[0]->saw_keys[0], "test1");
  EXPECT_EQ("delete", srHandleVec[0]->sawOperations[0]);
}

TEST(DistributionRouteTest, crossRegionDeleteDisabledRpc) {
  std::vector<std::shared_ptr<TestHandle>> srHandleVec{
      std::make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "a")),
  };
  auto mockSrHandle = get_route_handles(srHandleVec)[0];

  auto axonCtx = std::make_shared<AxonContext>();
  struct Tmp {
    uint64_t bucketId;
    std::string region;
    std::string pool;
    std::string serialized;
  };
  auto tmp = Tmp{};
  axonCtx->writeProxyFn = [&](auto bucketId, auto&& payload) {
    tmp.bucketId = bucketId;
    if (payload.find(invalidation::kRegion) != payload.end()) {
      tmp.region = payload.at(invalidation::kRegion);
    }
    if (payload.find(invalidation::kPool) != payload.end()) {
      tmp.pool = payload.at(invalidation::kPool);
    }
    if (payload.find(invalidation::kSerialized) != payload.end()) {
      tmp.serialized = payload.at(invalidation::kSerialized);
    }
    return true;
  };
  axonCtx->allDelete = false;
  axonCtx->fallbackAsynclog = false;
  axonCtx->poolFilter = "testPool";

  constexpr folly::StringPiece kDistributionRouteConfig = R"(
  {
    "distributed_delete_rpc_enabled": false,
    "replay": false
  }
  )";

  auto rh = makeDistributionRoute<MemcacheRouterInfo>(
      mockSrHandle, folly::parseJson(kDistributionRouteConfig));
  mockFiberContext();
  fiber_local<MemcacheRouterInfo>::runWithLocals([&]() {
    fiber_local<MemcacheRouterInfo>::setAxonCtx(axonCtx);
    fiber_local<MemcacheRouterInfo>::setBucketId(1234);
    fiber_local<MemcacheRouterInfo>::setDistributionTargetRegion("georgia");
    rh->route(McDeleteRequest("/georgia/default/test1"));
  });

  // no keys routed to RPC:
  EXPECT_TRUE(srHandleVec[0]->saw_keys.empty());
  // spooled to axon:
  EXPECT_EQ(tmp.bucketId, 1234);
  EXPECT_EQ(tmp.region, "georgia");
  EXPECT_EQ(tmp.pool, "testPool");
  auto req = apache::thrift::CompactSerializer::deserialize<McDeleteRequest>(
      tmp.serialized);
  EXPECT_EQ(
      static_cast<McDeleteRequestSource>(
          req.attributes_ref()->find(memcache::kMcDeleteReqAttrSource)->second),
      memcache::McDeleteRequestSource::CROSS_REGION_DIRECTED_INVALIDATION);
}

TEST(DistributionRouteTest, crossRegionDeleteEnabledRpc) {
  std::vector<std::shared_ptr<TestHandle>> srHandleVec{
      std::make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "a")),
  };
  auto mockSrHandle = get_route_handles(srHandleVec)[0];

  auto axonCtx = std::make_shared<AxonContext>();
  struct Tmp {
    uint64_t bucketId;
    std::string region;
    std::string pool;
    std::string serialized;
  };
  auto tmp = Tmp{};
  axonCtx->writeProxyFn = [&](auto bucketId, auto&& payload) {
    tmp.bucketId = bucketId;
    if (payload.find(invalidation::kRegion) != payload.end()) {
      tmp.region = payload.at(invalidation::kRegion);
    }
    if (payload.find(invalidation::kPool) != payload.end()) {
      tmp.pool = payload.at(invalidation::kPool);
    }
    if (payload.find(invalidation::kSerialized) != payload.end()) {
      tmp.serialized = payload.at(invalidation::kSerialized);
    }
    return true;
  };
  axonCtx->allDelete = false;
  axonCtx->fallbackAsynclog = false;
  axonCtx->poolFilter = "testPool";

  constexpr folly::StringPiece kDistributionRouteConfig = R"(
  {
    "distributed_delete_rpc_enabled": true,
    "replay": false
  }
  )";

  auto rh = makeDistributionRoute<MemcacheRouterInfo>(
      mockSrHandle, folly::parseJson(kDistributionRouteConfig));
  mockFiberContext();
  fiber_local<MemcacheRouterInfo>::runWithLocals([&]() {
    fiber_local<MemcacheRouterInfo>::setAxonCtx(axonCtx);
    fiber_local<MemcacheRouterInfo>::setBucketId(1234);
    fiber_local<MemcacheRouterInfo>::setDistributionTargetRegion("georgia");
    rh->route(McDeleteRequest("/georgia/default/test1"));
  });

  // the key is routed to RPC:
  EXPECT_FALSE(srHandleVec[0]->saw_keys.empty());
  EXPECT_EQ(srHandleVec[0]->saw_keys[0], "/georgia/default/test1");
  EXPECT_EQ("delete", srHandleVec[0]->sawOperations[0]);
  // spooled to axon:
  EXPECT_EQ(tmp.bucketId, 1234);
  EXPECT_EQ(tmp.region, "georgia");
  EXPECT_EQ(tmp.pool, "testPool");
  auto req = apache::thrift::CompactSerializer::deserialize<McDeleteRequest>(
      tmp.serialized);
  EXPECT_EQ(
      static_cast<McDeleteRequestSource>(
          req.attributes_ref()->find(memcache::kMcDeleteReqAttrSource)->second),
      memcache::McDeleteRequestSource::CROSS_REGION_DIRECTED_INVALIDATION);
}

TEST(DistributionRouteTest, broadcastDeleteDisabledRpc) {
  std::vector<std::shared_ptr<TestHandle>> srHandleVec{
      std::make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "a")),
  };
  auto mockSrHandle = get_route_handles(srHandleVec)[0];

  auto axonCtx = std::make_shared<AxonContext>();
  struct Tmp {
    uint64_t bucketId;
    std::string region;
    std::string pool;
    std::string serialized;
  };
  auto tmp = Tmp{};
  axonCtx->writeProxyFn = [&](auto bucketId, auto&& payload) {
    tmp.bucketId = bucketId;
    if (payload.find(invalidation::kRegion) != payload.end()) {
      tmp.region = payload.at(invalidation::kRegion);
    }
    if (payload.find(invalidation::kPool) != payload.end()) {
      tmp.pool = payload.at(invalidation::kPool);
    }
    if (payload.find(invalidation::kSerialized) != payload.end()) {
      tmp.serialized = payload.at(invalidation::kSerialized);
    }
    return true;
  };
  axonCtx->allDelete = false;
  axonCtx->fallbackAsynclog = false;
  axonCtx->poolFilter = "testPool";

  constexpr folly::StringPiece kDistributionRouteConfig = R"(
  {
    "distributed_delete_rpc_enabled": false,
    "replay": false
  }
  )";

  auto rh = makeDistributionRoute<MemcacheRouterInfo>(
      mockSrHandle, folly::parseJson(kDistributionRouteConfig));
  mockFiberContext();
  fiber_local<MemcacheRouterInfo>::runWithLocals([&]() {
    fiber_local<MemcacheRouterInfo>::setAxonCtx(axonCtx);
    fiber_local<MemcacheRouterInfo>::setBucketId(1234);
    fiber_local<MemcacheRouterInfo>::setDistributionTargetRegion("");
    rh->route(McDeleteRequest("/*/*/test1"));
  });

  // the key is not routed to RPC:
  EXPECT_TRUE(srHandleVec[0]->saw_keys.empty());
  // spooled to axon:
  EXPECT_EQ(tmp.bucketId, 1234);
  EXPECT_TRUE(tmp.region.empty());
  EXPECT_EQ(tmp.pool, "testPool");
  auto req = apache::thrift::CompactSerializer::deserialize<McDeleteRequest>(
      tmp.serialized);
  EXPECT_EQ(
      static_cast<McDeleteRequestSource>(
          req.attributes_ref()->find(memcache::kMcDeleteReqAttrSource)->second),
      memcache::McDeleteRequestSource::CROSS_REGION_BROADCAST_INVALIDATION);
}

TEST(DistributionRouteTest, broadcastDeleteEnabledRpc) {
  std::vector<std::shared_ptr<TestHandle>> srHandleVec{
      std::make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "a")),
  };
  auto mockSrHandle = get_route_handles(srHandleVec)[0];

  auto axonCtx = std::make_shared<AxonContext>();
  struct Tmp {
    uint64_t bucketId;
    std::string region;
    std::string pool;
    std::string serialized;
  };
  auto tmp = Tmp{};
  axonCtx->writeProxyFn = [&](auto bucketId, auto&& payload) {
    tmp.bucketId = bucketId;
    if (payload.find(invalidation::kRegion) != payload.end()) {
      tmp.region = payload.at(invalidation::kRegion);
    }
    if (payload.find(invalidation::kPool) != payload.end()) {
      tmp.pool = payload.at(invalidation::kPool);
    }
    if (payload.find(invalidation::kSerialized) != payload.end()) {
      tmp.serialized = payload.at(invalidation::kSerialized);
    }
    return true;
  };
  axonCtx->allDelete = false;
  axonCtx->fallbackAsynclog = false;
  axonCtx->poolFilter = "testPool";

  constexpr folly::StringPiece kDistributionRouteConfig = R"(
  {
    "distributed_delete_rpc_enabled": true,
    "replay": false
  }
  )";

  auto rh = makeDistributionRoute<MemcacheRouterInfo>(
      mockSrHandle, folly::parseJson(kDistributionRouteConfig));
  mockFiberContext();
  fiber_local<MemcacheRouterInfo>::runWithLocals([&]() {
    fiber_local<MemcacheRouterInfo>::setAxonCtx(axonCtx);
    fiber_local<MemcacheRouterInfo>::setBucketId(1234);
    fiber_local<MemcacheRouterInfo>::setDistributionTargetRegion("");
    rh->route(McDeleteRequest("/*/*/test1"));
  });
  // the key is routed to RPC:
  EXPECT_FALSE(srHandleVec[0]->saw_keys.empty());
  EXPECT_EQ(srHandleVec[0]->saw_keys[0], "/*/*/test1");
  EXPECT_EQ("delete", srHandleVec[0]->sawOperations[0]);
  // spooled to axon:
  EXPECT_EQ(tmp.bucketId, 1234);
  EXPECT_TRUE(tmp.region.empty());
  EXPECT_EQ(tmp.pool, "testPool");
  auto req = apache::thrift::CompactSerializer::deserialize<McDeleteRequest>(
      tmp.serialized);
  EXPECT_EQ(
      static_cast<McDeleteRequestSource>(
          req.attributes_ref()->find(memcache::kMcDeleteReqAttrSource)->second),
      memcache::McDeleteRequestSource::CROSS_REGION_BROADCAST_INVALIDATION);
}

TEST(DistributionRouteTest, broadcastSpooledDelete) {
  std::vector<std::shared_ptr<TestHandle>> srHandleVec{
      std::make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "a")),
  };
  auto mockSrHandle = get_route_handles(srHandleVec)[0];

  auto axonCtx = std::make_shared<AxonContext>();
  struct Tmp {
    uint64_t bucketId;
    std::string region;
    std::string pool;
    std::string serialized;
  };
  auto tmp = Tmp{};
  axonCtx->writeProxyFn = [&](auto bucketId, auto&& payload) {
    tmp.bucketId = bucketId;
    if (payload.find(invalidation::kRegion) != payload.end()) {
      tmp.region = payload.at(invalidation::kRegion);
    }
    if (payload.find(invalidation::kPool) != payload.end()) {
      tmp.pool = payload.at(invalidation::kPool);
    }
    if (payload.find(invalidation::kSerialized) != payload.end()) {
      tmp.serialized = payload.at(invalidation::kSerialized);
    }
    return true;
  };
  axonCtx->fallbackAsynclog = false;
  axonCtx->poolFilter = "testPool";

  constexpr folly::StringPiece kDistributionRouteConfig = R"(
  {
    "distributed_delete_rpc_enabled": false,
    "replay": true
  }
  )";

  auto rh = makeDistributionRoute<MemcacheRouterInfo>(
      mockSrHandle, folly::parseJson(kDistributionRouteConfig));
  mockFiberContext();
  fiber_local<MemcacheRouterInfo>::runWithLocals([&]() {
    fiber_local<MemcacheRouterInfo>::setAxonCtx(axonCtx);
    fiber_local<MemcacheRouterInfo>::setBucketId(1234);
    auto req = McDeleteRequest("/*/*/test1");
    req = addDeleteRequestSource(
        req,
        memcache::McDeleteRequestSource::CROSS_REGION_BROADCAST_INVALIDATION);
    rh->route(req);
  });

  // the key is not routed to RPC:
  EXPECT_TRUE(srHandleVec[0]->saw_keys.empty());
  // spooled to axon:
  EXPECT_EQ(tmp.bucketId, 1234);
  EXPECT_TRUE(tmp.region.empty());
  EXPECT_EQ(tmp.pool, "testPool");
  auto req = apache::thrift::CompactSerializer::deserialize<McDeleteRequest>(
      tmp.serialized);
  EXPECT_EQ(
      static_cast<McDeleteRequestSource>(
          req.attributes_ref()->find(memcache::kMcDeleteReqAttrSource)->second),
      memcache::McDeleteRequestSource::CROSS_REGION_BROADCAST_INVALIDATION);
}

TEST(DistributionRouteTest, crossRegionDirectedSpooledDelete) {
  std::vector<std::shared_ptr<TestHandle>> srHandleVec{
      std::make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "a")),
  };
  auto mockSrHandle = get_route_handles(srHandleVec)[0];

  auto axonCtx = std::make_shared<AxonContext>();
  struct Tmp {
    uint64_t bucketId;
    std::string region;
    std::string pool;
    std::string serialized;
  };
  auto tmp = Tmp{};
  axonCtx->writeProxyFn = [&](auto bucketId, auto&& payload) {
    tmp.bucketId = bucketId;
    if (payload.find(invalidation::kRegion) != payload.end()) {
      tmp.region = payload.at(invalidation::kRegion);
    }
    if (payload.find(invalidation::kPool) != payload.end()) {
      tmp.pool = payload.at(invalidation::kPool);
    }
    if (payload.find(invalidation::kSerialized) != payload.end()) {
      tmp.serialized = payload.at(invalidation::kSerialized);
    }
    return true;
  };
  axonCtx->fallbackAsynclog = false;
  axonCtx->poolFilter = "testPool";
  constexpr folly::StringPiece kDistributionRouteConfig = R"(
  {
    "distributed_delete_rpc_enabled": false,
    "replay": true
  }
  )";

  auto rh = makeDistributionRoute<MemcacheRouterInfo>(
      mockSrHandle, folly::parseJson(kDistributionRouteConfig));
  mockFiberContext();
  fiber_local<MemcacheRouterInfo>::runWithLocals([&]() {
    fiber_local<MemcacheRouterInfo>::setAxonCtx(axonCtx);
    fiber_local<MemcacheRouterInfo>::setBucketId(1234);
    auto req = McDeleteRequest("/altoona/default/test1");
    req = addDeleteRequestSource(
        req,
        memcache::McDeleteRequestSource::CROSS_REGION_DIRECTED_INVALIDATION);
    rh->route(req);
  });

  // the key is not routed to RPC:
  EXPECT_TRUE(srHandleVec[0]->saw_keys.empty());
  // spooled to axon:
  EXPECT_EQ(tmp.bucketId, 1234);
  EXPECT_EQ(tmp.region, "altoona");
  EXPECT_EQ(tmp.pool, "testPool");
  auto req = apache::thrift::CompactSerializer::deserialize<McDeleteRequest>(
      tmp.serialized);
  EXPECT_EQ(
      static_cast<McDeleteRequestSource>(
          req.attributes_ref()->find(memcache::kMcDeleteReqAttrSource)->second),
      memcache::McDeleteRequestSource::CROSS_REGION_DIRECTED_INVALIDATION);
}

} // namespace facebook::memcache::mcrouter
