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

#include <thrift/conformance/data/CompatibilityGenerator.h>

#include <gtest/gtest.h>
#include <thrift/conformance/cpp2/AnyRegistry.h>

namespace apache::thrift::conformance::data {

TEST(TestGeneratorTest, RoundTripSuite) {
  auto suite = createCompatibilitySuite();

  constexpr size_t kNumProtocols = 2;
  constexpr size_t kNumTypes = 9;
  EXPECT_EQ(suite.name(), "CompatibilityTest");

  // type tag dependent tests (kNumProtocols * kNumTypes) + type tag independent
  // tests (kNumProtocols)
  constexpr size_t kTestCount = kNumProtocols * kNumTypes + kNumProtocols;
  ASSERT_EQ(suite.tests()->size(), kTestCount);

  EXPECT_EQ(suite.tests()->at(0).name(), "Binary");
  EXPECT_EQ(suite.tests()->at(1 * kNumTypes + 1).name(), "Compact");

  const auto& test = suite.tests()->at(2);
  EXPECT_EQ(test.name(), "Binary");

  {
    const auto& testCase = test.testCases()->at(0);
    EXPECT_EQ(testCase.name(), "testset.byte/AddField");
    EXPECT_TRUE(testCase.test()->roundTrip_ref());
  }
  {
    const auto& testCase = test.testCases()->at(1);
    EXPECT_EQ(testCase.name(), "testset.byte/RemoveField/zero");
    EXPECT_TRUE(testCase.test()->roundTrip_ref());
  }
}

} // namespace apache::thrift::conformance::data
