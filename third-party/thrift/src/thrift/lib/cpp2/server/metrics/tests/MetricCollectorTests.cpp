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

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <thrift/lib/cpp2/server/metrics/MetricCollector.h>
#include <thrift/lib/cpp2/server/metrics/Scope.h>
#include <thrift/lib/cpp2/server/metrics/tests/Utils.h>

using namespace apache::thrift;
using MockMetricCollectorBackend =
    apache::thrift::testing::MockMetricCollectorBackend;

TEST(MetricCollectorTest, testBackendDelegation) {
  MetricCollector metricCollector;
  auto metricCollectorBackend = std::make_shared<MockMetricCollectorBackend>();
  metricCollector.setBackend(metricCollectorBackend);

  EXPECT_CALL(*metricCollectorBackend, requestReceived()).Times(1);
  metricCollector.requestReceived();

  RequestRejectedScope scope;
  EXPECT_CALL(*metricCollectorBackend, requestRejected(::testing::Ref(scope)))
      .Times(1);
  metricCollector.requestRejected(scope);
}
