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

TEST(ResourcePoolsTest, testConcurrencyLimit) {
  THRIFT_FLAG_SET_MOCK(experimental_use_resource_pools, true);
  if (!apache::thrift::useResourcePoolsFlagsSet()) {
    GTEST_SKIP() << "Invalid resource pools mode";
  }

  ScopedServerInterfaceThread runner(
      std::make_shared<BlockingCallTestService>());

  auto& thriftServer = runner.getThriftServer();

  // grab the resource pool
  // and set the number to 0
  auto& pools = thriftServer.resourcePoolSet();
  auto& asyncRP = pools.resourcePool(ResourcePoolHandle::defaultAsync());
  auto& cc = asyncRP.concurrencyController()->get();
  // block request
  cc.setExecutionLimitRequests(0);

  auto client = runner.newClient<Client<TestService>>();
  client->semifuture_echoInt(0);

  usleep(2000000);

  EXPECT_EQ(cc.getExecutionLimitRequests(), 0);
  EXPECT_EQ(pools.numQueued(), 1);

  // let request through
  cc.setExecutionLimitRequests(1);
}
