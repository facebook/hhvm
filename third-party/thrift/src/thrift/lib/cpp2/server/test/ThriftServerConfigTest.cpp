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
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/server/ThriftServerConfig.h>

using namespace ::testing;
using namespace apache::thrift;

TEST(ThriftServerInitialConfigTest, testSetOneConfigKnob) {
  constexpr auto initialConfig = ThriftServerInitialConfig().maxRequests(1024);
  ThriftServer server(initialConfig);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  EXPECT_EQ(detail::getThriftServerConfig(server).getMaxRequests().get(), 1024);
}

TEST(ThriftServerInitialConfigTest, testSetAllConfigKnobs) {
  constexpr std::chrono::milliseconds queueTimeout{5};
  constexpr auto initialConfig =
      ThriftServerInitialConfig().maxRequests(2048).queueTimeout(queueTimeout);
  ThriftServer server(initialConfig);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  EXPECT_EQ(detail::getThriftServerConfig(server).getMaxRequests().get(), 2048);
  EXPECT_EQ(
      detail::getThriftServerConfig(server).getQueueTimeout().get(),
      queueTimeout);
}
