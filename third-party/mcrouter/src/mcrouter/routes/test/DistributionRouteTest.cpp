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

struct AxonMockResult {
  uint64_t bucketId;
  std::string region;
  std::string pool;
  std::string serialized;
  invalidation::DistributionOperation operation;
  invalidation::DistributionType type;
  std::string srcRegion;
};

namespace {
AxonMockResult setupAxonFn(std::shared_ptr<AxonContext>& ctx) {
  AxonMockResult res;
  ctx->writeProxyFn = [&](auto bucketId, auto&& payload, bool) {
    res.bucketId = bucketId;
    if (payload.find(invalidation::kRegion) != payload.end()) {
      res.region = payload.at(invalidation::kRegion);
    }
    if (payload.find(invalidation::kPool) != payload.end()) {
      res.pool = payload.at(invalidation::kPool);
    }
    if (payload.find(invalidation::kSerialized) != payload.end()) {
      res.serialized = payload.at(invalidation::kSerialized);
    }
    if (payload.find(invalidation::kOperation) != payload.end()) {
      res.operation = static_cast<invalidation::DistributionOperation>(
          std::stoi(payload.at(invalidation::kOperation)));
    }
    if (payload.find(invalidation::kType) != payload.end()) {
      res.type = static_cast<invalidation::DistributionType>(
          std::stoi(payload.at(invalidation::kType)));
    }
    if (payload.find(invalidation::kSourceRegion) != payload.end()) {
      res.srcRegion = payload.at(invalidation::kSourceRegion);
    }
    return true;
  };
  return res;
}
} // namespace

TEST(DistributionRouteTest, getSetAreForwardedToRpc) {
  std::vector<std::shared_ptr<TestHandle>> srHandleVec{
      std::make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "a")),
  };
  auto mockSrHandle = get_route_handles(srHandleVec)[0];

  constexpr folly::StringPiece kDistributionRouteConfig = R"(
  {
    "replay": false,
    "distribution_source_region": "oregon"
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
    "replay": false,
    "distribution_source_region": "oregon"
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
    "replay": false,
    "distribution_source_region": "oregon"
  }
  )";

  auto rh = makeDistributionRoute<MemcacheRouterInfo>(
      mockSrHandle, folly::parseJson(kDistributionRouteConfig));
  TestFiberManager<MemcacheRouterInfo> fm;
  fm.runAll({[&]() {
    mockFiberContext();
    fiber_local<MemcacheRouterInfo>::runWithLocals(
        [&]() { rh->route(McDeleteRequest("test1")); });
  }});

  EXPECT_FALSE(srHandleVec[0]->saw_keys.empty());
  EXPECT_EQ(srHandleVec[0]->saw_keys[0], "test1");
  EXPECT_EQ("delete", srHandleVec[0]->sawOperations[0]);
}

TEST(DistributionRouteTest, crossRegionDelete) {
  std::vector<std::shared_ptr<TestHandle>> srHandleVec{
      std::make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "a")),
  };
  auto mockSrHandle = get_route_handles(srHandleVec)[0];

  auto axonCtx = std::make_shared<AxonContext>();
  auto tmp = setupAxonFn(axonCtx);
  axonCtx->allDelete = false;
  axonCtx->fallbackAsynclog = false;
  axonCtx->poolFilter = "testPool";

  constexpr folly::StringPiece kDistributionRouteConfig = R"(
  {
    "replay": false,
    "distribution_source_region": "oregon"
  }
  )";

  auto rh = makeDistributionRoute<MemcacheRouterInfo>(
      mockSrHandle, folly::parseJson(kDistributionRouteConfig));
  TestFiberManager<MemcacheRouterInfo> fm;
  fm.runAll({[&]() {
    mockFiberContext();
    fiber_local<MemcacheRouterInfo>::runWithLocals([&]() {
      fiber_local<MemcacheRouterInfo>::setAxonCtx(axonCtx);
      fiber_local<MemcacheRouterInfo>::setBucketId(1234);
      fiber_local<MemcacheRouterInfo>::setDistributionTargetRegion("georgia");
      rh->route(McDeleteRequest("/georgia/default/test1"));
    });
  }});

  // the key is not routed to RPC:
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
  EXPECT_EQ(tmp.operation, invalidation::DistributionOperation::Delete);
  EXPECT_EQ(tmp.type, invalidation::DistributionType::Distribution);
  EXPECT_EQ(tmp.srcRegion, "oregon");
}

TEST(DistributionRouteTest, broadcastDeleteEnabledRpc) {
  std::vector<std::shared_ptr<TestHandle>> srHandleVec{
      std::make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "a")),
  };
  auto mockSrHandle = get_route_handles(srHandleVec)[0];

  auto axonCtx = std::make_shared<AxonContext>();
  auto tmp = setupAxonFn(axonCtx);
  axonCtx->allDelete = false;
  axonCtx->fallbackAsynclog = false;
  axonCtx->poolFilter = "testPool";

  constexpr folly::StringPiece kDistributionRouteConfig = R"(
  {
    "replay": false,
    "distribution_source_region": "oregon"
  }
  )";

  auto rh = makeDistributionRoute<MemcacheRouterInfo>(
      mockSrHandle, folly::parseJson(kDistributionRouteConfig));
  TestFiberManager<MemcacheRouterInfo> fm;
  fm.runAll({[&]() {
    mockFiberContext();
    fiber_local<MemcacheRouterInfo>::runWithLocals([&]() {
      fiber_local<MemcacheRouterInfo>::setAxonCtx(axonCtx);
      fiber_local<MemcacheRouterInfo>::setBucketId(1234);
      fiber_local<MemcacheRouterInfo>::setDistributionTargetRegion("");
      rh->route(McDeleteRequest("/*/*/test1"));
    });
  }});
  // the key is routed to RPC:
  EXPECT_FALSE(srHandleVec[0]->saw_keys.empty());
  EXPECT_EQ(srHandleVec[0]->saw_keys[0], "/*/*/test1");
  EXPECT_EQ("delete", srHandleVec[0]->sawOperations[0]);
  // spooled to axon:
  EXPECT_EQ(tmp.bucketId, 1234);
  EXPECT_EQ(tmp.region, "DistributionRoute");
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
  auto tmp = setupAxonFn(axonCtx);
  axonCtx->fallbackAsynclog = false;
  axonCtx->poolFilter = "testPool";

  constexpr folly::StringPiece kDistributionRouteConfig = R"(
  {
    "replay": true,
    "distribution_source_region": "oregon"
  }
  )";

  auto rh = makeDistributionRoute<MemcacheRouterInfo>(
      mockSrHandle, folly::parseJson(kDistributionRouteConfig));
  TestFiberManager<MemcacheRouterInfo> fm;
  fm.runAll({[&]() {
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
  }});

  // the key is not routed to RPC:
  EXPECT_TRUE(srHandleVec[0]->saw_keys.empty());
  // spooled to axon:
  EXPECT_EQ(tmp.bucketId, 1234);
  EXPECT_EQ(tmp.region, "DistributionRoute");
  EXPECT_EQ(tmp.pool, "testPool");
  auto req = apache::thrift::CompactSerializer::deserialize<McDeleteRequest>(
      tmp.serialized);
  EXPECT_EQ(
      static_cast<McDeleteRequestSource>(
          req.attributes_ref()->find(memcache::kMcDeleteReqAttrSource)->second),
      memcache::McDeleteRequestSource::CROSS_REGION_BROADCAST_INVALIDATION);
  EXPECT_EQ(tmp.operation, invalidation::DistributionOperation::Delete);
  EXPECT_EQ(tmp.type, invalidation::DistributionType::Async);
  EXPECT_EQ(tmp.srcRegion, "oregon");
}

TEST(DistributionRouteTest, crossRegionDirectedSpooledDelete) {
  std::vector<std::shared_ptr<TestHandle>> srHandleVec{
      std::make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "a")),
  };
  auto mockSrHandle = get_route_handles(srHandleVec)[0];

  auto axonCtx = std::make_shared<AxonContext>();
  auto tmp = setupAxonFn(axonCtx);
  axonCtx->fallbackAsynclog = false;
  axonCtx->poolFilter = "testPool";
  constexpr folly::StringPiece kDistributionRouteConfig = R"(
  {
    "replay": true,
    "distribution_source_region": "oregon"
  }
  )";

  auto rh = makeDistributionRoute<MemcacheRouterInfo>(
      mockSrHandle, folly::parseJson(kDistributionRouteConfig));
  TestFiberManager<MemcacheRouterInfo> fm;
  fm.runAll({[&]() {
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
  }});

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
  EXPECT_EQ(tmp.operation, invalidation::DistributionOperation::Delete);
  EXPECT_EQ(tmp.type, invalidation::DistributionType::Async);
  EXPECT_EQ(tmp.srcRegion, "oregon");
}

TEST(DistributionRouteTest, crossRegionSet) {
  std::vector<std::shared_ptr<TestHandle>> srHandleVec{
      std::make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "a")),
  };
  auto mockSrHandle = get_route_handles(srHandleVec)[0];

  auto axonCtx = std::make_shared<AxonContext>();
  auto tmp = setupAxonFn(axonCtx);
  axonCtx->fallbackAsynclog = false;
  axonCtx->poolFilter = "testPool";

  constexpr folly::StringPiece kDistributionRouteConfig = R"(
  {
    "replay": false,
    "distribution_source_region": "oregon"
  }
  )";

  auto rh = makeDistributionRoute<MemcacheRouterInfo>(
      mockSrHandle, folly::parseJson(kDistributionRouteConfig));

  mockFiberContext();
  fiber_local<MemcacheRouterInfo>::runWithLocals([&]() {
    fiber_local<MemcacheRouterInfo>::setAxonCtx(axonCtx);
    fiber_local<MemcacheRouterInfo>::setBucketId(1234);
    fiber_local<MemcacheRouterInfo>::setDistributionTargetRegion("georgia");
    auto req = McSetRequest("/georgia/default/test1");
    req.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "value");
    rh->route(req);
  });

  // the key is NOT routed to RPC:
  EXPECT_TRUE(srHandleVec[0]->saw_keys.empty());
  // spooled to axon:
  EXPECT_EQ(tmp.bucketId, 1234);
  EXPECT_EQ(tmp.region, "georgia");
  EXPECT_EQ(tmp.pool, "testPool");
  auto req2 = apache::thrift::CompactSerializer::deserialize<McSetRequest>(
      tmp.serialized);
  EXPECT_EQ(req2.key_ref()->fullKey().str(), "test1");
  EXPECT_EQ(tmp.operation, invalidation::DistributionOperation::Write);
  EXPECT_EQ(tmp.type, invalidation::DistributionType::Distribution);
  EXPECT_EQ(tmp.srcRegion, "oregon");
}

TEST(DistributionRouteTest, broadcastSet) {
  std::vector<std::shared_ptr<TestHandle>> srHandleVec{
      std::make_shared<TestHandle>(
          GetRouteTestData(carbon::Result::FOUND, "a")),
  };
  auto mockSrHandle = get_route_handles(srHandleVec)[0];

  auto axonCtx = std::make_shared<AxonContext>();
  auto tmp = setupAxonFn(axonCtx);
  axonCtx->fallbackAsynclog = false;
  axonCtx->poolFilter = "testPool";

  constexpr folly::StringPiece kDistributionRouteConfig = R"(
  {
    "replay": false,
    "distribution_source_region": "oregon"
  }
  )";

  auto rh = makeDistributionRoute<MemcacheRouterInfo>(
      mockSrHandle, folly::parseJson(kDistributionRouteConfig));
  TestFiberManager<MemcacheRouterInfo> fm;
  fm.runAll({[&]() {
    mockFiberContext();
    fiber_local<MemcacheRouterInfo>::runWithLocals([&]() {
      fiber_local<MemcacheRouterInfo>::setAxonCtx(axonCtx);
      fiber_local<MemcacheRouterInfo>::setBucketId(1234);
      fiber_local<MemcacheRouterInfo>::setDistributionTargetRegion("");
      auto req = McSetRequest("/*/*/test1");
      req.value_ref() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, "value");
      rh->route(req);
    });
  }});
  // the key is routed to RPC:
  EXPECT_FALSE(srHandleVec[0]->saw_keys.empty());
  EXPECT_EQ(srHandleVec[0]->saw_keys[0], "/*/*/test1");
  EXPECT_EQ("set", srHandleVec[0]->sawOperations[0]);
  // spooled to axon:
  EXPECT_EQ(tmp.bucketId, 1234);
  EXPECT_EQ(tmp.region, "DistributionRoute");
  EXPECT_EQ(tmp.pool, "testPool");
  auto req = apache::thrift::CompactSerializer::deserialize<McSetRequest>(
      tmp.serialized);
  EXPECT_EQ(req.key_ref()->fullKey(), "test1");
  EXPECT_EQ(tmp.operation, invalidation::DistributionOperation::Write);
  EXPECT_EQ(tmp.type, invalidation::DistributionType::Distribution);
  EXPECT_EQ(tmp.srcRegion, "oregon");
}

} // namespace facebook::memcache::mcrouter
