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

#include <thrift/conformance/data/PatchGenerator.h>

namespace apache::thrift::conformance::data::test {

TEST(PatchGeneratorTest, Basic) {
  auto suite = createPatchSuite();
  constexpr size_t kNumTypes = 9;
  EXPECT_EQ(*suite.name(), "PatchTest");
  EXPECT_EQ(*suite.tests()->at(0 * kNumTypes).name(), "Binary");

  const auto& test = suite.tests()->at(1);
  EXPECT_EQ(*test.name(), "Binary");
  ASSERT_GT(test.testCases()->size(), 0);

  {
    const auto& testCase = test.testCases()->at(0);
    EXPECT_EQ(*testCase.name(), "map<byte,bool>/assign.zero");
  }
}

} // namespace apache::thrift::conformance::data::test
