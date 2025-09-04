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

#include <gtest/gtest.h>

#include <folly/executors/CPUThreadPoolExecutor.h>
#include <thrift/lib/cpp2/server/ParallelConcurrencyController.h>
#include <thrift/lib/cpp2/server/ResourcePool.h>
#include <thrift/lib/cpp2/server/ResourcePoolHandle.h>
#include <thrift/lib/cpp2/server/RoundRobinRequestPile.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/test/util/TestHandler.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace ::testing;

namespace apache::thrift {
TEST(ResourcePoolSetTest, testDefaultPoolsOverride_overrideSync_expectCrash) {
  ResourcePoolSet set;

  set.setResourcePool(
      ResourcePoolHandle::defaultSync(), nullptr, nullptr, nullptr);
  EXPECT_THROW(
      set.setResourcePool(
          ResourcePoolHandle::defaultSync(), nullptr, nullptr, nullptr),
      std::invalid_argument);
}

TEST(ResourcePoolSetTest, testDefaultPoolsOverride_overrideAsync_expectCrash) {
  ResourcePoolSet set;

  set.setResourcePool(
      ResourcePoolHandle::defaultAsync(), nullptr, nullptr, nullptr);
  EXPECT_THROW(
      set.setResourcePool(
          ResourcePoolHandle::defaultAsync(), nullptr, nullptr, nullptr),
      std::invalid_argument);
}

TEST(
    ResourcePoolSetTest,
    testThriftServer_whenResourcePoolsSuppliedButNotAsyncPool_expectDefaultSyncAndAsyncPoolsCreated) {
  auto handler = std::make_shared<TestHandler>();
  auto server = std::make_shared<ScopedServerInterfaceThread>(
      handler, "::1", 0, [](ThriftServer& server) {
        // Set up thrift server with 2 RPs none of them is "default"
        {
          server.requireResourcePools();

          auto requestPile = std::make_unique<RoundRobinRequestPile>(
              RoundRobinRequestPile::Options());
          auto executor = std::make_shared<folly::CPUThreadPoolExecutor>(1);
          auto concurrencyController =
              std::make_unique<ParallelConcurrencyController>(
                  *requestPile, *executor);
          server.resourcePoolSet().addResourcePool(
              "first",
              std::move(requestPile),
              executor,
              std::move(concurrencyController),
              concurrency::PRIORITY::IMPORTANT);
        }
        {
          auto requestPile = std::make_unique<RoundRobinRequestPile>(
              RoundRobinRequestPile::Options());
          auto executor = std::make_shared<folly::CPUThreadPoolExecutor>(1);
          auto concurrencyController =
              std::make_unique<ParallelConcurrencyController>(
                  *requestPile, *executor);
          server.resourcePoolSet().addResourcePool(
              "second",
              std::move(requestPile),
              executor,
              std::move(concurrencyController),
              concurrency::PRIORITY::IMPORTANT);
        }
      });
  EXPECT_TRUE(server->getThriftServer().resourcePoolSet().hasResourcePool(
      ResourcePoolHandle::defaultAsync()));
  EXPECT_TRUE(server->getThriftServer().resourcePoolSet().hasResourcePool(
      ResourcePoolHandle::defaultSync()));
}
} // namespace apache::thrift
