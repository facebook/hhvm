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
  using namespace std::chrono_literals;
  constexpr auto initialConfig =
      ThriftServerInitialConfig()
          .maxRequests(1)
          .maxConnections(2)
          .maxResponseSize(3)
          .useClientTimeout(false)
          .taskExpireTimeout(4ms)
          .streamExpireTimeout(5ms)
          .queueTimeout(6ms)
          .socketQueueTimeout(7ns)
          .egressMemoryLimit(8)
          .egressBufferBackpressureThreshold(9)
          .ingressMemoryLimit(10)
          .minPayloadSizeToEnforceIngressMemoryLimit(11);
  ThriftServer server(initialConfig);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  EXPECT_EQ(server.getMaxRequests(), 1);
  EXPECT_EQ(server.getMaxConnections(), 2);
  EXPECT_EQ(server.getMaxResponseSize(), 3);
  EXPECT_EQ(server.getUseClientTimeout(), false);
  EXPECT_EQ(server.getTaskExpireTime().count(), 4);
  EXPECT_EQ(server.getStreamExpireTime().count(), 5);
  EXPECT_EQ(server.getQueueTimeout().count(), 6);
  EXPECT_EQ((**server.getSocketQueueTimeout()).count(), 7);
  EXPECT_EQ(server.getEgressMemoryLimit(), 8);
  EXPECT_EQ(server.getEgressBufferBackpressureThreshold(), 9);
  EXPECT_EQ(server.getIngressMemoryLimit(), 10);
  EXPECT_EQ(server.getMinPayloadSizeToEnforceIngressMemoryLimit(), 11);
}
