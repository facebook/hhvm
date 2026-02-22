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
