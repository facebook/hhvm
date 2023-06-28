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
#include <thrift/lib/cpp2/test/Structs.h>
#include <thrift/lib/cpp2/test/gen-cpp2/ProtocolBenchmark_types.h>

namespace apache::thrift::test {
struct ProtoBenchTest : ::testing::Test {};

// Checks that int generation is deterministic
TEST(ProtoBenchTest, MixedIntTest) {
  auto m1 = create<::thrift::benchmark::MixedInt>();
  auto m2 = create<::thrift::benchmark::MixedInt>();
  EXPECT_TRUE(m1 == m2);
}

TEST(ProtoBenchTest, BigListTest) {
  auto m1 = create<::thrift::benchmark::BigListInt>();
  auto m2 = create<::thrift::benchmark::BigListInt>();
  EXPECT_TRUE(m1 == m2);
}

TEST(ProtoBenchTest, LargeSetTest) {
  auto m1 = create<::thrift::benchmark::LargeSetInt>();
  auto m2 = create<::thrift::benchmark::LargeSetInt>();
  EXPECT_TRUE(m1 == m2);
}

TEST(ProtoBenchTest, LargeMapTest) {
  auto m1 = create<::thrift::benchmark::LargeMapInt>();
  auto m2 = create<::thrift::benchmark::LargeMapInt>();
  EXPECT_TRUE(m1 == m2);
}

} // namespace apache::thrift::test
