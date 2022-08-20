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

#include <thrift/conformance/data/TestGenerator.h>

#include <folly/portability/GTest.h>
#include <thrift/conformance/cpp2/AnyRegistry.h>
#include <thrift/conformance/cpp2/AnyStructSerializer.h>
#include <thrift/conformance/cpp2/Object.h>
#include <thrift/conformance/cpp2/ThriftTypeInfo.h>
#include <thrift/conformance/if/gen-cpp2/object_types_custom_protocol.h>

namespace apache::thrift::conformance::data {

TEST(TestGeneratorTest, RoundTripSuite) {
  auto suite = createRoundTripSuite();
  constexpr size_t kNumProtocols = 2;
  constexpr size_t kNumTypes = 8;
  EXPECT_EQ(*suite.name(), "RoundTripTest");
  ASSERT_EQ(suite.tests()->size(), kNumProtocols * kNumTypes);
  EXPECT_EQ(*suite.tests()->at(0 * kNumTypes).name(), "Binary");
  EXPECT_EQ(*suite.tests()->at(1 * kNumTypes).name(), "Compact");

  const auto& test = suite.tests()->at(1);
  EXPECT_EQ(*test.name(), "Binary");
  ASSERT_GT(test.testCases()->size(), 0);

  {
    const auto& testCase = test.testCases()->at(0);
    EXPECT_EQ(*testCase.name(), "byte/zero");
    EXPECT_TRUE(testCase.test()->roundTrip_ref());
  }
  {
    const auto& testCase = test.testCases()->at(1);
    EXPECT_EQ(*testCase.name(), "testset.byte/zero");
    EXPECT_TRUE(testCase.test()->roundTrip_ref());
  }
}

} // namespace apache::thrift::conformance::data
