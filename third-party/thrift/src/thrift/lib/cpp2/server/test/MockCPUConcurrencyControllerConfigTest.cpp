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

#include <thrift/lib/cpp2/server/CPUConcurrencyController.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>

using namespace ::testing;

using namespace apache::thrift;

bool kMakeCPUConcurrencyControllerConfigCalled{false};

namespace apache::thrift::detail {
THRIFT_PLUGGABLE_FUNC_SET(
    folly::observer::Observer<apache::thrift::CPUConcurrencyController::Config>,
    makeCPUConcurrencyControllerConfig,
    BaseThriftServer*) {
  kMakeCPUConcurrencyControllerConfigCalled = true;

  return folly::observer::makeObserver([]() {
    CPUConcurrencyController::Config config;
    config.concurrencyLowerBound = 2222;
    return config;
  });
}
} // namespace apache::thrift::detail

TEST(MockCpuConcurrencyControllerConfigTest, testOverride) {
  kMakeCPUConcurrencyControllerConfigCalled = false;

  ThriftServer server;
  server.setMockCPUConcurrencyControllerConfig(
      CPUConcurrencyController::Config{.concurrencyLowerBound = 1111});
  folly::observer_detail::ObserverManager::waitForAllUpdates();

  auto config = server.getCPUConcurrencyController().config();
  ASSERT_EQ(config->concurrencyLowerBound, 1111);
  ASSERT_TRUE(kMakeCPUConcurrencyControllerConfigCalled);
}

TEST(MockCpuConcurrencyControllerConfigTest, testBase) {
  kMakeCPUConcurrencyControllerConfigCalled = false;

  ThriftServer server;
  auto config = server.getCPUConcurrencyController().config();
  ASSERT_EQ(config->concurrencyLowerBound, 2222);
  ASSERT_TRUE(kMakeCPUConcurrencyControllerConfigCalled);
}
