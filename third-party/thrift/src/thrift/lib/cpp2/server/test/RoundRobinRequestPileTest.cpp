/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <vector>

#include <gtest/gtest.h>
#include <folly/Benchmark.h>
#include <folly/executors/CPUThreadPoolExecutor.h>

#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/server/RoundRobinRequestPile.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/server/test/RequestPileTestUtils.h>

using namespace apache::thrift;
using namespace apache::thrift::test;
using namespace apache::thrift::transport;
using namespace apache::thrift::concurrency;

class RoundRobinRequestPileTest : public testing::Test,
                                  public RequestPileTestState,
                                  public RequestPileTestUtils {};

TEST_F(RoundRobinRequestPileTest, testRoundRobinDequeueForManyBuckets) {
  RoundRobinRequestPile::Options opts(
      {10, 10, 10, 10, 10}, makePileSelectionFunction());
  RoundRobinRequestPile pile(opts);

  pile.enqueue(makeServerRequestForBucket(0, 0));
  pile.enqueue(makeServerRequestForBucket(0, 0));
  pile.enqueue(makeServerRequestForBucket(0, 0));
  pile.enqueue(makeServerRequestForBucket(0, 1));
  pile.enqueue(makeServerRequestForBucket(0, 1));
  pile.enqueue(makeServerRequestForBucket(0, 2));
  pile.enqueue(makeServerRequestForBucket(0, 3));

  expectRequestToBelongToBucket(pile.dequeue().value(), 0, 0);
  expectRequestToBelongToBucket(pile.dequeue().value(), 0, 1);
  expectRequestToBelongToBucket(pile.dequeue().value(), 0, 2);
  expectRequestToBelongToBucket(pile.dequeue().value(), 0, 3);
  expectRequestToBelongToBucket(pile.dequeue().value(), 0, 0);
  expectRequestToBelongToBucket(pile.dequeue().value(), 0, 1);
  expectRequestToBelongToBucket(pile.dequeue().value(), 0, 0);
  EXPECT_EQ(pile.dequeue(), std::nullopt);

  pile.enqueue(makeServerRequestForBucket(0, 0));
  pile.enqueue(makeServerRequestForBucket(2, 0));
  pile.enqueue(makeServerRequestForBucket(2, 0));
  pile.enqueue(makeServerRequestForBucket(1, 0));
  pile.enqueue(makeServerRequestForBucket(2, 1));
  pile.enqueue(makeServerRequestForBucket(0, 0));
  pile.enqueue(makeServerRequestForBucket(0, 0));
  pile.enqueue(makeServerRequestForBucket(1, 1));

  expectRequestToBelongToBucket(pile.dequeue().value(), 0, 0);
  expectRequestToBelongToBucket(pile.dequeue().value(), 0, 0);
  expectRequestToBelongToBucket(pile.dequeue().value(), 0, 0);
  expectRequestToBelongToBucket(pile.dequeue().value(), 1, 0);
  expectRequestToBelongToBucket(pile.dequeue().value(), 1, 1);
  expectRequestToBelongToBucket(pile.dequeue().value(), 2, 0);
  expectRequestToBelongToBucket(pile.dequeue().value(), 2, 1);
  expectRequestToBelongToBucket(pile.dequeue().value(), 2, 0);
  EXPECT_EQ(pile.dequeue(), std::nullopt);
  EXPECT_EQ(pile.requestCount(), 0);
}

TEST_F(RoundRobinRequestPileTest, testDequeueForSingleBucket) {
  RoundRobinRequestPile::Options opts({1}, makePileSelectionFunction());
  RoundRobinRequestPile pile(opts);

  pile.enqueue(makeServerRequestForBucket(0, 0));
  pile.enqueue(makeServerRequestForBucket(0, 0));
  pile.enqueue(makeServerRequestForBucket(0, 0));

  expectRequestToBelongToBucket(pile.dequeue().value(), 0, 0);
  expectRequestToBelongToBucket(pile.dequeue().value(), 0, 0);
  expectRequestToBelongToBucket(pile.dequeue().value(), 0, 0);
  EXPECT_EQ(pile.dequeue(), std::nullopt);
  EXPECT_EQ(pile.requestCount(), 0);
}

TEST_F(RoundRobinRequestPileTest, testBucketSizeLimitForManyBuckets) {
  RoundRobinRequestPile::Options opts(
      {10, 10, 10, 10, 10}, makePileSelectionFunction());
  opts.numMaxRequests = 1;
  RoundRobinRequestPile pile(opts);

  {
    auto rejected = pile.enqueue(makeServerRequestForBucket(0, 0));
    ASSERT_FALSE(rejected) << "Should have enqueued successfully";
  }

  {
    auto rejected = pile.enqueue(makeServerRequestForBucket(0, 0));
    ASSERT_TRUE(rejected) << "Should have rejected request";
  }

  EXPECT_EQ(pile.requestCount(), 1);
  ASSERT_TRUE(pile.dequeue() != std::nullopt);
  ASSERT_TRUE(pile.dequeue() == std::nullopt);
  EXPECT_EQ(pile.requestCount(), 0);

  {
    auto rejected = pile.enqueue(makeServerRequestForBucket(0, 0));
    ASSERT_FALSE(rejected) << "Should have enqueued successfully";
  }
}

TEST_F(RoundRobinRequestPileTest, testBucketSizeLimitForSingleBucket) {
  RoundRobinRequestPile::Options opts({1}, makePileSelectionFunction());
  opts.numMaxRequests = 1;
  RoundRobinRequestPile pile(opts);

  {
    auto rejected = pile.enqueue(makeServerRequestForBucket(0, 0));
    ASSERT_FALSE(rejected) << "Should have enqueued successfully";
  }

  {
    auto rejected = pile.enqueue(makeServerRequestForBucket(0, 0));
    ASSERT_TRUE(rejected) << "Should have rejected request";
  }

  EXPECT_EQ(pile.requestCount(), 1);
  ASSERT_TRUE(pile.dequeue() != std::nullopt);
  ASSERT_TRUE(pile.dequeue() == std::nullopt);
  EXPECT_EQ(pile.requestCount(), 0);

  {
    auto rejected = pile.enqueue(makeServerRequestForBucket(0, 0));
    ASSERT_FALSE(rejected) << "Should have enqueued successfully";
  }
}

TEST_F(RoundRobinRequestPileTest, getRequestsCounts) {
  RoundRobinRequestPile::Options opts(
      {3, 4, 5, 6, 7}, makePileSelectionFunction());
  RoundRobinRequestPile pile(opts);

  // enqueue 7 requests
  pile.enqueue(makeServerRequestForBucket(2, 2));
  pile.enqueue(makeServerRequestForBucket(1, 0));
  pile.enqueue(makeServerRequestForBucket(2, 2));
  pile.enqueue(makeServerRequestForBucket(4, 0));
  pile.enqueue(makeServerRequestForBucket(4, 2));
  pile.enqueue(makeServerRequestForBucket(4, 2));
  pile.enqueue(makeServerRequestForBucket(4, 6));

  auto expectedRequestCounts = std::vector<std::vector<uint64_t>>{
      {0, 0, 0},
      {1, 0, 0, 0},
      {0, 0, 2, 0, 0},
      {0, 0, 0, 0, 0, 0},
      {1, 0, 2, 0, 0, 0, 1}};
  EXPECT_EQ(pile.getRequestCounts(), expectedRequestCounts);

  // dequeue 7 requests
  pile.dequeue();
  pile.dequeue();
  pile.dequeue();
  pile.dequeue();
  pile.dequeue();
  pile.dequeue();
  pile.dequeue();

  expectedRequestCounts = std::vector<std::vector<uint64_t>>{
      {0, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0}};
  EXPECT_EQ(pile.getRequestCounts(), expectedRequestCounts);
}

TEST_F(
    RoundRobinRequestPileTest, testPerPriorityBucketSizeLimitForManyBuckets) {
  RoundRobinRequestPile::Options opts(
      {10, 10, 10}, makePileSelectionFunction());
  // P0: no limit, P1: limit 2, P2: limit 1
  opts.setNumMaxRequestsPerPriority({0, 2, 1});
  RoundRobinRequestPile pile(opts);

  // P0 should accept unlimited requests (no limit)
  EXPECT_FALSE(pile.enqueue(makeServerRequestForBucket(0, 0)));
  EXPECT_FALSE(pile.enqueue(makeServerRequestForBucket(0, 0)));
  EXPECT_FALSE(pile.enqueue(makeServerRequestForBucket(0, 0)));

  // P1 should accept up to 2 per bucket
  EXPECT_FALSE(pile.enqueue(makeServerRequestForBucket(1, 0)));
  EXPECT_FALSE(pile.enqueue(makeServerRequestForBucket(1, 0)));
  EXPECT_TRUE(pile.enqueue(makeServerRequestForBucket(1, 0)))
      << "P1 bucket 0 should reject at limit 2";
  // Different bucket still has its own limit
  EXPECT_FALSE(pile.enqueue(makeServerRequestForBucket(1, 1)));
  EXPECT_FALSE(pile.enqueue(makeServerRequestForBucket(1, 1)));
  EXPECT_TRUE(pile.enqueue(makeServerRequestForBucket(1, 1)))
      << "P1 bucket 1 should reject at limit 2";

  // P2 should accept only 1 per bucket
  EXPECT_FALSE(pile.enqueue(makeServerRequestForBucket(2, 0)));
  EXPECT_TRUE(pile.enqueue(makeServerRequestForBucket(2, 0)))
      << "P2 bucket 0 should reject at limit 1";

  // Verify dequeue order: P0 first, then P1, then P2
  expectRequestToBelongToBucket(pile.dequeue().value(), 0, 0);
  expectRequestToBelongToBucket(pile.dequeue().value(), 0, 0);
  expectRequestToBelongToBucket(pile.dequeue().value(), 0, 0);
  expectRequestToBelongToBucket(pile.dequeue().value(), 1, 0);
  expectRequestToBelongToBucket(pile.dequeue().value(), 1, 1);
  expectRequestToBelongToBucket(pile.dequeue().value(), 1, 0);
  expectRequestToBelongToBucket(pile.dequeue().value(), 1, 1);
  expectRequestToBelongToBucket(pile.dequeue().value(), 2, 0);
  EXPECT_EQ(pile.dequeue(), std::nullopt);
}

TEST_F(
    RoundRobinRequestPileTest, testPerPriorityBucketSizeLimitForSingleBucket) {
  RoundRobinRequestPile::Options opts({1, 1, 1}, makePileSelectionFunction());
  // P0: no limit, P1: limit 2, P2: limit 1
  opts.setNumMaxRequestsPerPriority({0, 2, 1});
  RoundRobinRequestPile pile(opts);

  // P0 unlimited
  EXPECT_FALSE(pile.enqueue(makeServerRequestForBucket(0, 0)));
  EXPECT_FALSE(pile.enqueue(makeServerRequestForBucket(0, 0)));

  // P1 limit 2
  EXPECT_FALSE(pile.enqueue(makeServerRequestForBucket(1, 0)));
  EXPECT_FALSE(pile.enqueue(makeServerRequestForBucket(1, 0)));
  EXPECT_TRUE(pile.enqueue(makeServerRequestForBucket(1, 0)));

  // P2 limit 1
  EXPECT_FALSE(pile.enqueue(makeServerRequestForBucket(2, 0)));
  EXPECT_TRUE(pile.enqueue(makeServerRequestForBucket(2, 0)));

  EXPECT_EQ(pile.requestCount(), 5);
}

TEST_F(RoundRobinRequestPileTest, testPreEnqueueFilter) {
  RoundRobinRequestPile::Options opts({1, 1}, makePileSelectionFunction());
  // Reject all P1 requests
  opts.setPreEnqueueFilter(
      [](const ServerRequest& request)
          -> std::optional<ServerRequestRejection> {
        auto [priority, bucket] = *request.requestData().bucket;
        if (priority == 1) {
          return std::make_optional<ServerRequestRejection>(
              TApplicationException("P1 rejected"));
        }
        return std::nullopt;
      });
  RoundRobinRequestPile pile(opts);

  // P0 should be accepted
  EXPECT_FALSE(pile.enqueue(makeServerRequestForBucket(0, 0)));
  // P1 should be rejected
  EXPECT_TRUE(pile.enqueue(makeServerRequestForBucket(1, 0)));
  // P0 should still be accepted
  EXPECT_FALSE(pile.enqueue(makeServerRequestForBucket(0, 0)));

  EXPECT_EQ(pile.requestCount(), 2);
}

TEST_F(RoundRobinRequestPileTest, testPreEnqueueFilterWithRequestBytes) {
  RoundRobinRequestPile::Options opts({1}, makePileSelectionFunction());
  std::atomic<uint64_t> totalQueuedBytes{0};
  // Reject when totalQueuedBytes exceeds 200
  opts.setPreEnqueueFilter(
      [&totalQueuedBytes](const ServerRequest& request)
          -> std::optional<ServerRequestRejection> {
        auto bytes = request.requestContext()
            ? request.requestContext()->getWiredRequestBytes()
            : 0;
        auto current = totalQueuedBytes.load(std::memory_order_relaxed);
        if (current > 200) {
          return std::make_optional<ServerRequestRejection>(
              TApplicationException("too many bytes"));
        }
        totalQueuedBytes.fetch_add(bytes, std::memory_order_relaxed);
        return std::nullopt;
      });
  RoundRobinRequestPile pile(opts);
  pile.setDequeueObserver([&totalQueuedBytes](const ServerRequest& request) {
    auto bytes = request.requestContext()
        ? request.requestContext()->getWiredRequestBytes()
        : 0;
    totalQueuedBytes.fetch_sub(bytes, std::memory_order_relaxed);
  });

  // Enqueue requests with 100 bytes each
  auto req1 = makeServerRequestForBucket(0, 0);
  req1.requestContext()->setWiredRequestBytes(100);
  EXPECT_FALSE(pile.enqueue(std::move(req1))); // totalQueuedBytes=0, accepted
  EXPECT_EQ(totalQueuedBytes.load(), 100);

  auto req2 = makeServerRequestForBucket(0, 0);
  req2.requestContext()->setWiredRequestBytes(100);
  EXPECT_FALSE(pile.enqueue(std::move(req2))); // totalQueuedBytes=100, accepted
  EXPECT_EQ(totalQueuedBytes.load(), 200);

  auto req3 = makeServerRequestForBucket(0, 0);
  req3.requestContext()->setWiredRequestBytes(100);
  // totalQueuedBytes=200, not > 200, so accepted
  EXPECT_FALSE(pile.enqueue(std::move(req3)));
  EXPECT_EQ(totalQueuedBytes.load(), 300);

  auto req4 = makeServerRequestForBucket(0, 0);
  req4.requestContext()->setWiredRequestBytes(100);
  // totalQueuedBytes=300 > 200, so rejected
  EXPECT_TRUE(pile.enqueue(std::move(req4)));
  EXPECT_EQ(totalQueuedBytes.load(), 300);

  // Dequeue and verify bytes decrease
  pile.dequeue();
  EXPECT_EQ(totalQueuedBytes.load(), 200);

  pile.dequeue();
  EXPECT_EQ(totalQueuedBytes.load(), 100);

  pile.dequeue();
  EXPECT_EQ(totalQueuedBytes.load(), 0);
}

TEST_F(RoundRobinRequestPileTest, testDequeueObserver) {
  RoundRobinRequestPile::Options opts({1, 1}, makePileSelectionFunction());
  RoundRobinRequestPile pile(opts);

  std::vector<uint64_t> observedBytes;
  pile.setDequeueObserver([&](const ServerRequest& request) {
    auto bytes = request.requestContext()
        ? request.requestContext()->getWiredRequestBytes()
        : 0;
    observedBytes.push_back(bytes);
  });

  auto req1 = makeServerRequestForBucket(0, 0);
  req1.requestContext()->setWiredRequestBytes(100);
  pile.enqueue(std::move(req1));

  auto req2 = makeServerRequestForBucket(1, 0);
  req2.requestContext()->setWiredRequestBytes(200);
  pile.enqueue(std::move(req2));

  // Dequeue P0 first (higher priority)
  auto d1 = pile.dequeue();
  ASSERT_TRUE(d1.has_value());
  expectRequestToBelongToBucket(*d1, 0, 0);
  ASSERT_EQ(observedBytes.size(), 1);
  EXPECT_EQ(observedBytes[0], 100);

  // Dequeue P1
  auto d2 = pile.dequeue();
  ASSERT_TRUE(d2.has_value());
  expectRequestToBelongToBucket(*d2, 1, 0);
  ASSERT_EQ(observedBytes.size(), 2);
  EXPECT_EQ(observedBytes[1], 200);

  // No more requests
  EXPECT_FALSE(pile.dequeue().has_value());
  EXPECT_EQ(observedBytes.size(), 2); // observer not called for empty dequeue
}

TEST(RoundRobinRequestPileMiscTest, getDbgInfo) {
  RoundRobinRequestPile::Options opts(
      {11, 12, 13, 14, 15},
      [](auto&) -> std::pair<uint32_t, uint32_t> { return {0, 0}; });
  RoundRobinRequestPile pile(opts);

  auto result = pile.getDbgInfo();

  EXPECT_TRUE(
      (*result.name()).find("RoundRobinRequestPile") != std::string::npos);
  EXPECT_EQ(result.prioritiesCount().value(), 5);
  EXPECT_EQ((*result.bucketsPerPriority())[0], 11);
  EXPECT_EQ((*result.bucketsPerPriority())[1], 12);
  EXPECT_EQ((*result.bucketsPerPriority())[2], 13);
  EXPECT_EQ((*result.bucketsPerPriority())[3], 14);
  EXPECT_EQ((*result.bucketsPerPriority())[4], 15);
}

TEST_F(RoundRobinRequestPileTest, dequeueFromEmptyPile) {
  // Single-bucket
  {
    RoundRobinRequestPile::Options opts({1}, makePileSelectionFunction());
    RoundRobinRequestPile pile(opts);
    EXPECT_EQ(pile.dequeue(), std::nullopt);
    EXPECT_EQ(pile.requestCount(), 0);
  }
  // Multi-bucket
  {
    RoundRobinRequestPile::Options opts({5, 3}, makePileSelectionFunction());
    RoundRobinRequestPile pile(opts);
    EXPECT_EQ(pile.dequeue(), std::nullopt);
    EXPECT_EQ(pile.requestCount(), 0);
  }
}

TEST_F(RoundRobinRequestPileTest, defaultOptionsNoPileSelectionFunction) {
  RoundRobinRequestPile::Options opts;
  RoundRobinRequestPile pile(opts);

  // Without PileSelectionFunction, all requests route to (0, 0)
  pile.enqueue(makeServerRequestForBucket(0, 0));
  pile.enqueue(makeServerRequestForBucket(0, 0));
  pile.enqueue(makeServerRequestForBucket(0, 0));

  EXPECT_EQ(pile.requestCount(), 3);
  expectRequestToBelongToBucket(pile.dequeue().value(), 0, 0);
  expectRequestToBelongToBucket(pile.dequeue().value(), 0, 0);
  expectRequestToBelongToBucket(pile.dequeue().value(), 0, 0);
  EXPECT_EQ(pile.dequeue(), std::nullopt);
}

TEST_F(RoundRobinRequestPileTest, mixedShapeSingleAndMultiBucket) {
  // P0: 1 bucket (single-bucket path), P1: 3 buckets (multi-bucket path),
  // P2: 1 bucket (single-bucket path)
  RoundRobinRequestPile::Options opts({1, 3, 1}, makePileSelectionFunction());
  RoundRobinRequestPile pile(opts);

  // Enqueue across all priorities and buckets
  pile.enqueue(makeServerRequestForBucket(1, 0));
  pile.enqueue(makeServerRequestForBucket(1, 1));
  pile.enqueue(makeServerRequestForBucket(1, 2));
  pile.enqueue(makeServerRequestForBucket(1, 0));
  pile.enqueue(makeServerRequestForBucket(0, 0));
  pile.enqueue(makeServerRequestForBucket(2, 0));

  // P0 first (single-bucket)
  expectRequestToBelongToBucket(pile.dequeue().value(), 0, 0);
  // P1 round-robin across buckets
  expectRequestToBelongToBucket(pile.dequeue().value(), 1, 0);
  expectRequestToBelongToBucket(pile.dequeue().value(), 1, 1);
  expectRequestToBelongToBucket(pile.dequeue().value(), 1, 2);
  expectRequestToBelongToBucket(pile.dequeue().value(), 1, 0);
  // P2 last (single-bucket)
  expectRequestToBelongToBucket(pile.dequeue().value(), 2, 0);
  EXPECT_EQ(pile.dequeue(), std::nullopt);
}

TEST_F(RoundRobinRequestPileTest, strictPriorityOrderingSingleBucket) {
  RoundRobinRequestPile::Options opts({1, 1, 1}, makePileSelectionFunction());
  RoundRobinRequestPile pile(opts);

  // Enqueue in reverse priority order
  pile.enqueue(makeServerRequestForBucket(2, 0));
  pile.enqueue(makeServerRequestForBucket(1, 0));
  pile.enqueue(makeServerRequestForBucket(0, 0));
  pile.enqueue(makeServerRequestForBucket(2, 0));
  pile.enqueue(makeServerRequestForBucket(1, 0));

  // Dequeue respects strict priority ordering
  expectRequestToBelongToBucket(pile.dequeue().value(), 0, 0);
  expectRequestToBelongToBucket(pile.dequeue().value(), 1, 0);
  expectRequestToBelongToBucket(pile.dequeue().value(), 1, 0);
  expectRequestToBelongToBucket(pile.dequeue().value(), 2, 0);
  expectRequestToBelongToBucket(pile.dequeue().value(), 2, 0);
  EXPECT_EQ(pile.dequeue(), std::nullopt);
}

TEST_F(RoundRobinRequestPileTest, requestCountTracking) {
  // Mixed shape: P0 single-bucket, P1 multi-bucket
  RoundRobinRequestPile::Options opts({1, 3}, makePileSelectionFunction());
  RoundRobinRequestPile pile(opts);

  EXPECT_EQ(pile.requestCount(), 0);

  pile.enqueue(makeServerRequestForBucket(0, 0));
  EXPECT_EQ(pile.requestCount(), 1);

  pile.enqueue(makeServerRequestForBucket(1, 0));
  pile.enqueue(makeServerRequestForBucket(1, 1));
  EXPECT_EQ(pile.requestCount(), 3);

  pile.dequeue();
  EXPECT_EQ(pile.requestCount(), 2);

  pile.dequeue();
  pile.dequeue();
  EXPECT_EQ(pile.requestCount(), 0);

  // Dequeue from empty doesn't cause issues
  pile.dequeue();
  EXPECT_EQ(pile.requestCount(), 0);
}

TEST_F(RoundRobinRequestPileTest, getRequestCountsForSingleBucket) {
  RoundRobinRequestPile::Options opts({1, 1, 1}, makePileSelectionFunction());
  RoundRobinRequestPile pile(opts);

  pile.enqueue(makeServerRequestForBucket(0, 0));
  pile.enqueue(makeServerRequestForBucket(0, 0));
  pile.enqueue(makeServerRequestForBucket(2, 0));

  auto counts = pile.getRequestCounts();
  EXPECT_EQ(
      counts, (std::vector<std::vector<uint64_t>>{{2}, {0}, {1}}));

  pile.dequeue();
  counts = pile.getRequestCounts();
  EXPECT_EQ(
      counts, (std::vector<std::vector<uint64_t>>{{1}, {0}, {1}}));

  pile.dequeue();
  pile.dequeue();
  counts = pile.getRequestCounts();
  EXPECT_EQ(
      counts, (std::vector<std::vector<uint64_t>>{{0}, {0}, {0}}));
}

TEST_F(RoundRobinRequestPileTest, getDbgInfoWithLimitsAndQueuedRequests) {
  RoundRobinRequestPile::Options opts({2, 3}, makePileSelectionFunction());
  opts.numMaxRequests = 42;
  RoundRobinRequestPile pile(opts);

  pile.enqueue(makeServerRequestForBucket(0, 0));
  pile.enqueue(makeServerRequestForBucket(1, 1));
  pile.enqueue(makeServerRequestForBucket(1, 2));

  auto info = pile.getDbgInfo();
  EXPECT_EQ(info.perBucketRequestLimit().value(), 42);
  EXPECT_EQ(info.queuedRequestsCount().value(), 3);
  EXPECT_EQ(info.prioritiesCount().value(), 2);
}

TEST_F(RoundRobinRequestPileTest, acceptRejectCallbacksMultiBucket) {
  RoundRobinRequestPile::Options opts({3}, makePileSelectionFunction());
  opts.numMaxRequests = 1;
  RoundRobinRequestPile pile(opts);

  std::vector<std::pair<uint32_t, uint32_t>> accepted;
  std::vector<std::pair<uint32_t, uint32_t>> rejected;
  pile.setOnRequestAcceptedFunction(
      [&](uint32_t pri, uint32_t bucket) {
        accepted.emplace_back(pri, bucket);
      });
  pile.setOnRequestRejectedFunction(
      [&](uint32_t pri, uint32_t bucket) {
        rejected.emplace_back(pri, bucket);
      });

  // Accept to bucket 0
  pile.enqueue(makeServerRequestForBucket(0, 0));
  ASSERT_EQ(accepted.size(), 1);
  EXPECT_EQ(accepted[0].first, 0u);
  EXPECT_EQ(accepted[0].second, 0u);

  // Reject bucket 0 (limit reached)
  pile.enqueue(makeServerRequestForBucket(0, 0));
  ASSERT_EQ(rejected.size(), 1);
  EXPECT_EQ(rejected[0].first, 0u);
  EXPECT_EQ(rejected[0].second, 0u);
  EXPECT_EQ(accepted.size(), 1);

  // Accept to bucket 1 (different bucket has its own limit)
  pile.enqueue(makeServerRequestForBucket(0, 1));
  ASSERT_EQ(accepted.size(), 2);
  EXPECT_EQ(accepted[1].first, 0u);
  EXPECT_EQ(accepted[1].second, 1u);
}

TEST_F(RoundRobinRequestPileTest, acceptRejectCallbacksSingleBucket) {
  RoundRobinRequestPile::Options opts({1}, makePileSelectionFunction());
  opts.numMaxRequests = 1;
  RoundRobinRequestPile pile(opts);

  std::vector<std::pair<uint32_t, uint32_t>> accepted;
  std::vector<std::pair<uint32_t, uint32_t>> rejected;
  pile.setOnRequestAcceptedFunction(
      [&](uint32_t pri, uint32_t bucket) {
        accepted.emplace_back(pri, bucket);
      });
  pile.setOnRequestRejectedFunction(
      [&](uint32_t pri, uint32_t bucket) {
        rejected.emplace_back(pri, bucket);
      });

  pile.enqueue(makeServerRequestForBucket(0, 0));
  ASSERT_EQ(accepted.size(), 1);
  EXPECT_EQ(accepted[0].first, 0u);
  EXPECT_EQ(accepted[0].second, 0u);

  pile.enqueue(makeServerRequestForBucket(0, 0));
  ASSERT_EQ(rejected.size(), 1);
  EXPECT_EQ(rejected[0].first, 0u);
  EXPECT_EQ(rejected[0].second, 0u);
}

TEST_F(RoundRobinRequestPileTest, preEnqueueFilterRejectTriggersCallback) {
  RoundRobinRequestPile::Options opts({3}, makePileSelectionFunction());
  opts.setPreEnqueueFilter(
      [](const ServerRequest& request)
          -> std::optional<ServerRequestRejection> {
        auto [priority, bucket] = *request.requestData().bucket;
        if (bucket == 1) {
          return std::make_optional<ServerRequestRejection>(
              TApplicationException("rejected"));
        }
        return std::nullopt;
      });
  RoundRobinRequestPile pile(opts);

  std::vector<std::pair<uint32_t, uint32_t>> accepted;
  std::vector<std::pair<uint32_t, uint32_t>> rejected;
  pile.setOnRequestAcceptedFunction(
      [&](uint32_t pri, uint32_t bucket) {
        accepted.emplace_back(pri, bucket);
      });
  pile.setOnRequestRejectedFunction(
      [&](uint32_t pri, uint32_t bucket) {
        rejected.emplace_back(pri, bucket);
      });

  pile.enqueue(makeServerRequestForBucket(0, 0)); // accepted
  pile.enqueue(makeServerRequestForBucket(0, 1)); // rejected by filter

  ASSERT_EQ(accepted.size(), 1);
  EXPECT_EQ(accepted[0].first, 0u);
  EXPECT_EQ(accepted[0].second, 0u);

  ASSERT_EQ(rejected.size(), 1);
  EXPECT_EQ(rejected[0].first, 0u);
  EXPECT_EQ(rejected[0].second, 1u);
}

TEST_F(RoundRobinRequestPileTest, preEnqueueFilterMultiBucket) {
  RoundRobinRequestPile::Options opts({3, 3}, makePileSelectionFunction());
  // Reject requests to bucket 2 at any priority
  opts.setPreEnqueueFilter(
      [](const ServerRequest& request)
          -> std::optional<ServerRequestRejection> {
        auto [priority, bucket] = *request.requestData().bucket;
        if (bucket == 2) {
          return std::make_optional<ServerRequestRejection>(
              TApplicationException("bucket 2 rejected"));
        }
        return std::nullopt;
      });
  RoundRobinRequestPile pile(opts);

  EXPECT_FALSE(pile.enqueue(makeServerRequestForBucket(0, 0)));
  EXPECT_FALSE(pile.enqueue(makeServerRequestForBucket(0, 1)));
  EXPECT_TRUE(pile.enqueue(makeServerRequestForBucket(0, 2)));
  EXPECT_FALSE(pile.enqueue(makeServerRequestForBucket(1, 0)));
  EXPECT_TRUE(pile.enqueue(makeServerRequestForBucket(1, 2)));

  EXPECT_EQ(pile.requestCount(), 3);

  // Verify only non-rejected requests are dequeued
  expectRequestToBelongToBucket(pile.dequeue().value(), 0, 0);
  expectRequestToBelongToBucket(pile.dequeue().value(), 0, 1);
  expectRequestToBelongToBucket(pile.dequeue().value(), 1, 0);
  EXPECT_EQ(pile.dequeue(), std::nullopt);
}

TEST_F(RoundRobinRequestPileTest, dequeueObserverMultiBucket) {
  RoundRobinRequestPile::Options opts({3}, makePileSelectionFunction());
  RoundRobinRequestPile pile(opts);

  std::vector<std::pair<uint32_t, uint32_t>> observed;
  pile.setDequeueObserver([&](const ServerRequest& request) {
    observed.push_back(extractRequestBucketFromRequestData(request));
  });

  pile.enqueue(makeServerRequestForBucket(0, 0));
  pile.enqueue(makeServerRequestForBucket(0, 1));
  pile.enqueue(makeServerRequestForBucket(0, 2));

  pile.dequeue();
  pile.dequeue();
  pile.dequeue();

  ASSERT_EQ(observed.size(), 3);
  // Round-robin order
  EXPECT_EQ(observed[0].first, 0u);
  EXPECT_EQ(observed[0].second, 0u);
  EXPECT_EQ(observed[1].first, 0u);
  EXPECT_EQ(observed[1].second, 1u);
  EXPECT_EQ(observed[2].first, 0u);
  EXPECT_EQ(observed[2].second, 2u);

  // Observer not called for empty dequeue
  pile.dequeue();
  EXPECT_EQ(observed.size(), 3);
}

TEST_F(RoundRobinRequestPileTest, reEnqueueAfterDrainMultiBucket) {
  RoundRobinRequestPile::Options opts({3}, makePileSelectionFunction());
  RoundRobinRequestPile pile(opts);

  // First round
  pile.enqueue(makeServerRequestForBucket(0, 0));
  pile.enqueue(makeServerRequestForBucket(0, 1));
  pile.dequeue();
  pile.dequeue();
  EXPECT_EQ(pile.dequeue(), std::nullopt);
  EXPECT_EQ(pile.requestCount(), 0);

  // Re-enqueue after complete drain
  pile.enqueue(makeServerRequestForBucket(0, 2));
  pile.enqueue(makeServerRequestForBucket(0, 0));
  EXPECT_EQ(pile.requestCount(), 2);

  expectRequestToBelongToBucket(pile.dequeue().value(), 0, 2);
  expectRequestToBelongToBucket(pile.dequeue().value(), 0, 0);
  EXPECT_EQ(pile.dequeue(), std::nullopt);
}

TEST_F(RoundRobinRequestPileTest, reEnqueueAfterDrainSingleBucket) {
  RoundRobinRequestPile::Options opts({1, 1}, makePileSelectionFunction());
  RoundRobinRequestPile pile(opts);

  pile.enqueue(makeServerRequestForBucket(0, 0));
  pile.enqueue(makeServerRequestForBucket(1, 0));
  pile.dequeue();
  pile.dequeue();
  EXPECT_EQ(pile.dequeue(), std::nullopt);

  // Re-enqueue after complete drain
  pile.enqueue(makeServerRequestForBucket(1, 0));
  pile.enqueue(makeServerRequestForBucket(0, 0));

  // Priority ordering still holds
  expectRequestToBelongToBucket(pile.dequeue().value(), 0, 0);
  expectRequestToBelongToBucket(pile.dequeue().value(), 1, 0);
  EXPECT_EQ(pile.dequeue(), std::nullopt);
}

TEST(RoundRobinRequestPileMiscTest, describe) {
  RoundRobinRequestPile::Options opts(
      {2, 3},
      [](auto&) -> std::pair<uint32_t, uint32_t> { return {0, 0}; });
  RoundRobinRequestPile pile(opts);

  EXPECT_EQ(
      pile.describe(),
      "RoundRobinRequestPile priorities:2 Pri:0 Buckets:2 Pri:1 Buckets:3");
}

TEST(RoundRobinRequestPileMiscTest, optionsDescribe) {
  RoundRobinRequestPile::Options opts;
  opts.setName("test_pile");
  opts.setShape({2, 3});
  opts.numMaxRequests = 5;

  EXPECT_EQ(
      opts.describe(),
      "{Options name=test_pile numBucketsPerPriority={2,3} numMaxRequests=5 numMaxRequestsPerPriority={}}");

  opts.setNumMaxRequestsPerPriority({10, 20});
  EXPECT_EQ(
      opts.describe(),
      "{Options name=test_pile numBucketsPerPriority={2,3} numMaxRequests=5 numMaxRequestsPerPriority={10,20}}");
}

TEST(RoundRobinRequestPileMiscTest, optionsBuilderMethods) {
  RoundRobinRequestPile::Options opts;
  // Default: 1 priority with 1 bucket
  EXPECT_EQ(opts.numBucketsPerPriority, std::vector<uint32_t>{1});

  opts.setNumPriorities(3);
  EXPECT_EQ(opts.numBucketsPerPriority, (std::vector<uint32_t>{1, 1, 1}));

  opts.setNumBucketsPerPriority(1, 5);
  EXPECT_EQ(opts.numBucketsPerPriority, (std::vector<uint32_t>{1, 5, 1}));

  opts.setShape({2, 3, 4});
  EXPECT_EQ(opts.numBucketsPerPriority, (std::vector<uint32_t>{2, 3, 4}));

  // getNumMaxRequestsForPriority falls back to numMaxRequests
  opts.numMaxRequests = 10;
  EXPECT_EQ(opts.getNumMaxRequestsForPriority(0), 10);
  EXPECT_EQ(opts.getNumMaxRequestsForPriority(2), 10);

  // Per-priority limits override the global limit
  opts.setNumMaxRequestsPerPriority({0, 5, 20});
  EXPECT_EQ(opts.getNumMaxRequestsForPriority(0), 0);
  EXPECT_EQ(opts.getNumMaxRequestsForPriority(1), 5);
  EXPECT_EQ(opts.getNumMaxRequestsForPriority(2), 20);
}
