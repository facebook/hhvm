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
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/server/RequestExpirationDelegate.h>
#include <thrift/lib/cpp2/server/RoundRobinRequestPile.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/server/test/RequestPileTestUtils.h>

using namespace apache::thrift;
using namespace apache::thrift::test;
using namespace apache::thrift::transport;
using namespace apache::thrift::concurrency;

namespace {

class MockResponseChannelRequest : public ResponseChannelRequest {
 public:
  explicit MockResponseChannelRequest(bool active) : active_(active) {}
  bool isActive() const override { return active_; }
  bool isOneway() const override { return false; }
  bool includeEnvelope() const override { return false; }
  void sendReply(
      ResponsePayload&&,
      MessageChannel::SendCallback*,
      folly::Optional<uint32_t>) override {}
  void sendErrorWrapped(folly::exception_wrapper, std::string) override {}
  bool tryStartProcessing() override { return true; }

 private:
  bool active_;
};

class MockRequestExpirationDelegate : public RequestExpirationDelegate {
 public:
  void processExpiredRequest(ServerRequest&&) override { ++expiredCount; }
  int expiredCount{0};
};

} // namespace

class RoundRobinRequestPileTest : public testing::Test,
                                  public RequestPileTestState,
                                  public RequestPileTestUtils {
 protected:
  // Create a ServerRequest with a MockResponseChannelRequest that reports
  // the given active state. For use in testing RequestExpirationDelegate.
  ServerRequest makeServerRequestWithActiveState(
      Priority priority, Bucket bucket, bool active) {
    auto* header = new THeader;
    extraHeaders_.emplace_back(header);

    auto* ctx = new Cpp2RequestContext(nullptr, header);
    extraContexts_.emplace_back(ctx);

    header->setReadHeaders(
        {{"PRIORITY", folly::to<std::string>(priority)},
         {"BUCKET", folly::to<std::string>(bucket)}});

    ResponseChannelRequest::UniquePtr reqPtr(
        new MockResponseChannelRequest(active));

    return ServerRequest(
        std::move(reqPtr),
        SerializedCompressedRequest(std::unique_ptr<folly::IOBuf>{}),
        ctx,
        static_cast<protocol::PROTOCOL_TYPES>(0),
        nullptr,
        nullptr,
        nullptr);
  }

  // Create a ServerRequest with a specific call priority set on THeader,
  // and optionally with a MethodMetadata pointer.
  ServerRequest makeServerRequestWithCallPriority(
      std::optional<concurrency::PRIORITY> callPriority = std::nullopt,
      const AsyncProcessor::MethodMetadata* metadata = nullptr) {
    auto* header = new THeader;
    extraHeaders_.emplace_back(header);

    if (callPriority.has_value()) {
      header->setCallPriority(*callPriority);
    }

    auto* ctx = new Cpp2RequestContext(nullptr, header);
    extraContexts_.emplace_back(ctx);

    return ServerRequest(
        nullptr,
        SerializedCompressedRequest(std::unique_ptr<folly::IOBuf>{}),
        ctx,
        static_cast<protocol::PROTOCOL_TYPES>(0),
        nullptr,
        nullptr,
        metadata);
  }

  std::vector<std::unique_ptr<THeader>> extraHeaders_;
  std::vector<std::unique_ptr<Cpp2RequestContext>> extraContexts_;
};

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
  EXPECT_EQ(counts, (std::vector<std::vector<uint64_t>>{{2}, {0}, {1}}));

  pile.dequeue();
  counts = pile.getRequestCounts();
  EXPECT_EQ(counts, (std::vector<std::vector<uint64_t>>{{1}, {0}, {1}}));

  pile.dequeue();
  pile.dequeue();
  counts = pile.getRequestCounts();
  EXPECT_EQ(counts, (std::vector<std::vector<uint64_t>>{{0}, {0}, {0}}));
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
  pile.setOnRequestAcceptedFunction([&](uint32_t pri, uint32_t bucket) {
    accepted.emplace_back(pri, bucket);
  });
  pile.setOnRequestRejectedFunction([&](uint32_t pri, uint32_t bucket) {
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
  pile.setOnRequestAcceptedFunction([&](uint32_t pri, uint32_t bucket) {
    accepted.emplace_back(pri, bucket);
  });
  pile.setOnRequestRejectedFunction([&](uint32_t pri, uint32_t bucket) {
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
  pile.setOnRequestAcceptedFunction([&](uint32_t pri, uint32_t bucket) {
    accepted.emplace_back(pri, bucket);
  });
  pile.setOnRequestRejectedFunction([&](uint32_t pri, uint32_t bucket) {
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
      {2, 3}, [](auto&) -> std::pair<uint32_t, uint32_t> { return {0, 0}; });
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

// --- Tests for RequestExpirationDelegate / Consumer paths ---

TEST_F(RoundRobinRequestPileTest, expiredRequestDiscardedWithDelegate) {
  // Multi-bucket pile with delegate set: expired requests should be discarded
  // and processExpiredRequest should be called on the delegate.
  RoundRobinRequestPile::Options opts({3}, makePileSelectionFunction());
  RoundRobinRequestPile pile(opts);

  MockRequestExpirationDelegate delegate;
  pile.setRequestExpirationDelegate(&delegate);

  // Enqueue an expired request (isActive=false)
  pile.enqueue(makeServerRequestWithActiveState(0, 0, /*active=*/false));
  EXPECT_EQ(pile.requestCount(), 1);

  // Dequeue should return nullopt because the request is expired
  auto result = pile.dequeue();
  EXPECT_EQ(result, std::nullopt);
  EXPECT_EQ(delegate.expiredCount, 1);
}

TEST_F(RoundRobinRequestPileTest, activeRequestReturnedWithDelegate) {
  // Multi-bucket pile with delegate set: active requests should be
  // returned normally.
  RoundRobinRequestPile::Options opts({3}, makePileSelectionFunction());
  RoundRobinRequestPile pile(opts);

  MockRequestExpirationDelegate delegate;
  pile.setRequestExpirationDelegate(&delegate);

  // Enqueue an active request (isActive=true)
  pile.enqueue(makeServerRequestWithActiveState(0, 0, /*active=*/true));

  auto result = pile.dequeue();
  ASSERT_TRUE(result.has_value());
  expectRequestToBelongToBucket(*result, 0, 0);
  EXPECT_EQ(delegate.expiredCount, 0);
}

TEST_F(RoundRobinRequestPileTest, dequeueObserverCalledOnExpiredRequest) {
  // When a request is expired and delegate is set, the dequeue observer
  // should be called once (inside Consumer, NOT dequeueImpl) for the
  // expired request.
  RoundRobinRequestPile::Options opts({3}, makePileSelectionFunction());
  RoundRobinRequestPile pile(opts);

  MockRequestExpirationDelegate delegate;
  pile.setRequestExpirationDelegate(&delegate);

  int observerCount = 0;
  pile.setDequeueObserver([&](const ServerRequest&) { ++observerCount; });

  pile.enqueue(makeServerRequestWithActiveState(0, 0, /*active=*/false));
  auto result = pile.dequeue();
  EXPECT_EQ(result, std::nullopt);
  // Observer is called inside Consumer for the expired request
  EXPECT_EQ(observerCount, 1);
  EXPECT_EQ(delegate.expiredCount, 1);
}

TEST_F(RoundRobinRequestPileTest, nullRequestWithDelegateConsumed) {
  // When request() is nullptr (no ResponseChannelRequest) but delegate is set,
  // the Consumer should still consume the request (the isActive check
  // short-circuits because request.request() == nullptr).
  RoundRobinRequestPile::Options opts({3}, makePileSelectionFunction());
  RoundRobinRequestPile pile(opts);

  MockRequestExpirationDelegate delegate;
  pile.setRequestExpirationDelegate(&delegate);

  // makeServerRequestForBucket creates requests with null
  // ResponseChannelRequest
  pile.enqueue(makeServerRequestForBucket(0, 0));

  auto result = pile.dequeue();
  ASSERT_TRUE(result.has_value());
  expectRequestToBelongToBucket(*result, 0, 0);
  // Delegate should NOT be called since request() is null
  EXPECT_EQ(delegate.expiredCount, 0);
}

// --- Tests for getDefaultPileSelectionFunc ---

TEST_F(RoundRobinRequestPileTest, getDefaultPileSelectionFunc) {
  // Shape: 5 priorities (matching HIGH_IMPORTANT..BEST_EFFORT)
  RoundRobinRequestPile::Options opts;
  opts.setShape({1, 1, 1, 1, 1});

  // Get the default selection function with NORMAL as default priority
  auto func = opts.getDefaultPileSelectionFunc(
      static_cast<unsigned>(concurrency::NORMAL));

  // Case 1: callPriority is in range (HIGH = 1 <= priorityLimit = 4)
  // Should use the callPriority directly.
  {
    auto req = makeServerRequestWithCallPriority(concurrency::HIGH);
    auto [pri, bucket] = func(req);
    EXPECT_EQ(pri, static_cast<unsigned>(concurrency::HIGH));
    EXPECT_EQ(bucket, 0u);
  }

  // Case 2: callPriority is N_PRIORITIES (not set / out of range)
  // and methodMetadata is null -> should use defaultPriority (NORMAL=3)
  {
    auto req = makeServerRequestWithCallPriority(std::nullopt, nullptr);
    auto [pri, bucket] = func(req);
    EXPECT_EQ(pri, static_cast<unsigned>(concurrency::NORMAL));
    EXPECT_EQ(bucket, 0u);
  }

  // Case 3: callPriority is N_PRIORITIES (out of range)
  // and methodMetadata has a priority set -> should use metadata's priority.
  {
    AsyncProcessor::MethodMetadata metadata(
        AsyncProcessor::MethodMetadata::ExecutorType::ANY,
        AsyncProcessor::MethodMetadata::InteractionType::NONE,
        RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
        concurrency::BEST_EFFORT,
        std::nullopt,
        false);
    auto req = makeServerRequestWithCallPriority(std::nullopt, &metadata);
    auto [pri, bucket] = func(req);
    EXPECT_EQ(pri, static_cast<unsigned>(concurrency::BEST_EFFORT));
    EXPECT_EQ(bucket, 0u);
  }

  // Case 4: callPriority is N_PRIORITIES (out of range)
  // and methodMetadata exists but has no priority set (nullopt)
  // -> should use defaultPriority.
  {
    AsyncProcessor::MethodMetadata metadata;
    auto req = makeServerRequestWithCallPriority(std::nullopt, &metadata);
    auto [pri, bucket] = func(req);
    EXPECT_EQ(pri, static_cast<unsigned>(concurrency::NORMAL));
    EXPECT_EQ(bucket, 0u);
  }

  // Case 5: Using a different defaultPriority
  {
    auto func2 = opts.getDefaultPileSelectionFunc(
        static_cast<unsigned>(concurrency::HIGH_IMPORTANT));
    auto req = makeServerRequestWithCallPriority(std::nullopt, nullptr);
    auto [pri, bucket] = func2(req);
    EXPECT_EQ(pri, static_cast<unsigned>(concurrency::HIGH_IMPORTANT));
    EXPECT_EQ(bucket, 0u);
  }
}

// --- Test for addInternalPriorities per-priority limits doubling ---

TEST(
    RoundRobinRequestPileMiscTest,
    addInternalPrioritiesDoublesPerPriorityLimits) {
  RoundRobinRequestPile::Options opts;
  opts.setShape({2, 3});
  opts.setNumMaxRequestsPerPriority({10, 20});

  auto newOpts = RoundRobinRequestPile::addInternalPriorities(opts);

  // Shape should be doubled: {2, 2, 3, 3}
  EXPECT_EQ(newOpts.numBucketsPerPriority, (std::vector<uint32_t>{2, 2, 3, 3}));

  // Per-priority limits should be doubled: {10, 10, 20, 20}
  ASSERT_EQ(newOpts.numMaxRequestsPerPriority.size(), 4);
  EXPECT_EQ(
      newOpts.numMaxRequestsPerPriority,
      (std::vector<uint32_t>{10, 10, 20, 20}));
}
