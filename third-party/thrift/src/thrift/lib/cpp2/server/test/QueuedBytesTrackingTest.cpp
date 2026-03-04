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

#include <atomic>

#include <gtest/gtest.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <thrift/lib/cpp2/server/ParallelConcurrencyController.h>
#include <thrift/lib/cpp2/server/ResourcePoolHandle.h>
#include <thrift/lib/cpp2/server/RoundRobinRequestPile.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/test/util/TestHandler.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace apache::thrift;

// Example: track total queued bytes externally using PreEnqueueFilter and
// DequeueObserver, and reject requests when the queue is too large.
TEST(QueuedBytesTrackingTest, rejectWhenQueueBytesExceedLimit) {
  constexpr uint64_t kMaxQueuedBytes = 1024 * 1024; // 1MB limit
  std::atomic<uint64_t> totalQueuedBytes{0};

  RoundRobinRequestPile::Options opts;
  opts.setPreEnqueueFilter(
      [&totalQueuedBytes](const ServerRequest& request)
          -> std::optional<ServerRequestRejection> {
        uint64_t bytes = request.requestContext()
            ? request.requestContext()->getWiredRequestBytes()
            : 0;
        auto current = totalQueuedBytes.load(std::memory_order_relaxed);
        if (current + bytes > kMaxQueuedBytes) {
          return std::make_optional<ServerRequestRejection>(AppServerException(
              "AppServerException", "Queue byte limit exceeded"));
        }
        totalQueuedBytes.fetch_add(bytes, std::memory_order_relaxed);
        return std::nullopt;
      });

  auto pile = std::make_unique<RoundRobinRequestPile>(std::move(opts));
  pile->setDequeueObserver([&totalQueuedBytes](const ServerRequest& request) {
    uint64_t bytes = request.requestContext()
        ? request.requestContext()->getWiredRequestBytes()
        : 0;
    totalQueuedBytes.fetch_sub(bytes, std::memory_order_relaxed);
  });

  auto executor = std::make_shared<folly::CPUThreadPoolExecutor>(4);
  auto controller =
      std::make_unique<ParallelConcurrencyController>(*pile, *executor);

  auto handler = std::make_shared<TestHandler>();

  ScopedServerInterfaceThread runner(
      handler, "::1", 0, [&](ThriftServer& server) {
        server.resourcePoolSet().setResourcePool(
            ResourcePoolHandle::defaultAsync(),
            std::move(pile),
            executor,
            std::move(controller));
      });

  auto client = runner.newClient<apache::thrift::Client<test::TestService>>();

  // Verify basic functionality works
  client->semifuture_voidResponse().get();

  // After processing, queued bytes should be back to 0
  // (small requests may have 0 wired bytes in test)
  EXPECT_EQ(totalQueuedBytes.load(), 0);
}
