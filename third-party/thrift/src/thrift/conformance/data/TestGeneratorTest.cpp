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

#include <gtest/gtest.h>

namespace apache::thrift::conformance::data {

TEST(TestGeneratorTest, RoundTripSuite) {
  auto suite = createRoundTripSuite();
  constexpr size_t kNumTests = 148;
  EXPECT_EQ(*suite.name(), "RoundTripTest");
  EXPECT_EQ(*suite.tests()->at(1).name(), "Binary");
  EXPECT_EQ(*suite.tests()->at(kNumTests / 2 + 1).name(), "Compact");

  const auto& test = suite.tests()->at(0);
  EXPECT_EQ(*test.name(), "Binary");
  ASSERT_GT(test.testCases()->size(), 0);

  {
    const auto& testCase = test.testCases()->at(0);
    EXPECT_EQ(*testCase.name(), "bool/true");
    EXPECT_TRUE(testCase.test()->roundTrip_ref());
  }
  {
    const auto& testCase = test.testCases()->at(1);
    EXPECT_EQ(*testCase.name(), "testset.bool/true");
    EXPECT_TRUE(testCase.test()->roundTrip_ref());
  }
}

} // namespace apache::thrift::conformance::data
