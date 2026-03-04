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
