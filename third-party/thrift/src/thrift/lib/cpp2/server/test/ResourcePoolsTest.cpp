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

#include <folly/futures/Future.h>

#include <thrift/lib/cpp2/server/ServerFlags.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestServiceAsyncClient.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace ::testing;

using namespace apache::thrift;
using namespace apache::thrift::test;

class BlockingCallTestService : public ServiceHandler<TestService> {
 public:
  folly::SemiFuture<int32_t> semifuture_echoInt(int32_t) override {
    return folly::makeSemiFuture(1);
  }
};

class ResourcePoolsTest : public Test {
 protected:
  void SetUp() override {
    THRIFT_FLAG_SET_MOCK(experimental_use_resource_pools, true);
    if (!apache::thrift::useResourcePoolsFlagsSet()) {
      GTEST_SKIP() << "Invalid resource pools mode";
    }

    runner_ = std::make_unique<ScopedServerInterfaceThread>(
        std::make_shared<BlockingCallTestService>());

    auto& thriftServer = runner_->getThriftServer();
    // Disable queue timeout so requests stay queued during the test instead of
    // being load-shed with TApplicationException::TIMEOUT.
    thriftServer.setQueueTimeout(std::chrono::milliseconds(0));
    pools_ = &thriftServer.resourcePoolSet();
    auto& asyncRP = pools_->resourcePool(ResourcePoolHandle::defaultAsync());
    cc_ = &asyncRP.concurrencyController()->get();
    cc_->setExecutionLimitRequests(0);

    client_ = runner_->newClient<Client<TestService>>();
  }

  std::vector<folly::SemiFuture<int32_t>> sendRequests(int count) {
    std::vector<folly::SemiFuture<int32_t>> futures;
    futures.reserve(count);
    for (int i = 0; i < count; ++i) {
      futures.push_back(client_->semifuture_echoInt(i));
    }
    return futures;
  }

  std::unique_ptr<ScopedServerInterfaceThread> runner_;
  ResourcePoolSet* pools_ = nullptr;
  ConcurrencyControllerInterface* cc_ = nullptr;
  std::unique_ptr<Client<TestService>> client_;
};

TEST_F(ResourcePoolsTest, testConcurrencyLimit) {
  client_->semifuture_echoInt(0);

  /* sleep override */ usleep(2000000);

  EXPECT_EQ(cc_->getExecutionLimitRequests(), 0);
  EXPECT_EQ(pools_->numQueued(), 1);

  cc_->setExecutionLimitRequests(1);
}

TEST_F(ResourcePoolsTest, MultipleRequestsQueued_whenBlocked_allQueued) {
  constexpr int kNumRequests = 5;
  auto futures = sendRequests(kNumRequests);

  /* sleep override */ usleep(2000000);

  EXPECT_EQ(pools_->numQueued(), kNumRequests);

  cc_->setExecutionLimitRequests(kNumRequests);
  for (auto& f : futures) {
    std::move(f).get();
  }
}

TEST_F(
    ResourcePoolsTest, ConcurrencyLimit_requestsProcessedAfterLimitIncrease) {
  constexpr int kNumRequests = 5;
  auto futures = sendRequests(kNumRequests);

  /* sleep override */ usleep(2000000);

  EXPECT_EQ(cc_->getExecutionLimitRequests(), 0);
  EXPECT_EQ(pools_->numQueued(), kNumRequests);

  cc_->setExecutionLimitRequests(kNumRequests);
  for (auto& f : futures) {
    EXPECT_EQ(std::move(f).get(), 1);
  }
}
