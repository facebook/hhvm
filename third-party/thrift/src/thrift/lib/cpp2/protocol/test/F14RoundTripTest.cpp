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

#include <type_traits>
#include <vector>

#include <gtest/gtest.h>

#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/Module_types.h>

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::test;

namespace {

class F14RoundTripTest : public testing::Test {};

} // namespace

// F14Vector* containers have the additional guarantee (over unordered
// containers) that iteration order roundtrips.
TEST_F(F14RoundTripTest, RoundTrip) {
  StructWithF14VectorContainers s0;
  for (size_t i = 0; i < 5; ++i) {
    s0.m()->emplace(i, i);
    s0.s()->emplace(i);
  }

  const auto serialized = CompactSerializer::serialize<string>(s0);
  StructWithF14VectorContainers s1;
  CompactSerializer::deserialize(serialized, s1);

  auto as_vector = [](const auto& c) {
    return vector<typename std::decay_t<decltype(c)>::value_type>(
        c.begin(), c.end());
  };

  EXPECT_EQ(as_vector(*s0.m()), as_vector(*s1.m()));
  EXPECT_EQ(as_vector(*s0.s()), as_vector(*s1.s()));
}
