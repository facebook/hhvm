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

#include <folly/portability/GTest.h>

#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/server/metrics/tests/Utils.h>
#include <thrift/lib/cpp2/test/util/TestInterface.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace apache::thrift;
using MockMetricCollector = apache::thrift::testing::MockMetricCollector;

TEST(MetricCollectorTest, testServerSetMetricCollector) {
  ThriftServer server1;

  EXPECT_NO_THROW(
      server1.setMetricCollector(std::make_unique<MockMetricCollector>()));

  // Test calling set more than once fails
  EXPECT_THROW(
      server1.setMetricCollector(std::make_unique<MockMetricCollector>()),
      std::logic_error);

  // Test calling set after the server starts fails
  ScopedServerInterfaceThread ssit{std::make_shared<TestInterface>()};
  EXPECT_DEATH(
      ssit.getThriftServer().setMetricCollector(
          std::make_unique<MockMetricCollector>()),
      "Check failed: configMutable()");
}

TEST(MetricCollectorTest, testServerGetMetricCollector) {
  auto collector = std::make_unique<MockMetricCollector>();
  auto* collectorPtr = collector.get();

  ThriftServer server;

  // No metric collector set yet
  EXPECT_EQ(server.getMetricCollector(), nullptr);

  server.setMetricCollector(std::move(collector));

  // Return previously set metric collector
  EXPECT_EQ(server.getMetricCollector(), collectorPtr);
}
