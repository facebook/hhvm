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

#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/Module_types_custom_protocol.h>

using namespace std;
using namespace folly;
using namespace apache::thrift;
using namespace apache::thrift::test;

namespace {

class SimpleJSONProtocolTest : public testing::Test {};

using S = SimpleJSONSerializer;

template <typename T>
struct action_traits_impl;
template <typename C, typename A>
struct action_traits_impl<void (C::*)(A&) const> {
  using arg_type = A;
};
template <typename C, typename A>
struct action_traits_impl<void (C::*)(A&)> {
  using arg_type = A;
};
template <typename F>
using action_traits = action_traits_impl<decltype(&F::operator())>;
template <typename F>
using arg = typename action_traits<F>::arg_type;

template <typename F>
arg<F> returning(F&& f) {
  arg<F> ret;
  f(ret);
  return ret;
}

} // namespace

TEST_F(SimpleJSONProtocolTest, roundtrip_struct_with_empty_map_string_i64) {
  //  cpp2 -> str -> cpp2
  using type = StructWithEmptyMap;
  const auto orig = type{};
  const auto serialized = S::serialize<string>(orig);
  LOG(INFO) << serialized;
  type deserialized;
  const auto size = S::deserialize(serialized, deserialized);
  EXPECT_EQ(serialized.size(), size);
  EXPECT_EQ(orig, deserialized);
}

TEST_F(SimpleJSONProtocolTest, roundtrip_one_of_each_float_range) {
  // Test values for OneOfEach.{myDouble, myFloat} respectively
  const vector<tuple<double, float>> kTestCases = {
      {numeric_limits<double>::lowest(), numeric_limits<float>::lowest()},
      {nextafter(numeric_limits<double>::lowest(), 0.0),
       nextafter(numeric_limits<float>::lowest(), 0.f)},
      {numeric_limits<double>::max(), numeric_limits<float>::max()},
      {nextafter(numeric_limits<double>::max(), 0.0),
       nextafter(numeric_limits<float>::max(), 0.f)},
      {-0.0, -0.f},
      {0.0, 0.f},
      {123456.0, 123456.f},
  };

  for (const auto& testCase : kTestCases) {
    auto orig = OneOfEach{};
    tie(*orig.myDouble_ref(), *orig.myFloat_ref()) = testCase;
    const auto serialized = S::serialize<string>(orig);
    OneOfEach deserialized;
    const auto size = S::deserialize(serialized, deserialized);
    EXPECT_EQ(serialized.size(), size) << serialized;
    EXPECT_EQ(orig, deserialized) << serialized;
  }
}

TEST_F(SimpleJSONProtocolTest, roundtrip_non_string_map_keys) {
  auto orig = NonStringMapKeyFields{};
  orig.f1() = {{false, 1}, {true, 2}};
  orig.f2()->emplace(24, 3);
  orig.f3()->emplace(42, 4);
  orig.f4()->emplace(100, 5);
  orig.f5()->emplace(200, 6);
  orig.f6()->emplace(12345.f, 7);
  orig.f7()->emplace(12345.0, 8);
  const auto serialized = S::serialize<string>(orig);
  NonStringMapKeyFields deserialized;
  const auto size = S::deserialize(serialized, deserialized);
  EXPECT_EQ(serialized.size(), size);
  EXPECT_EQ(orig, deserialized);
}

TEST_F(SimpleJSONProtocolTest, roundtrip_binary) {
  StructWithBinaryField orig;
  orig.field() = {0x00, 0x01, 0x02};
  const auto serialized = S::serialize<string>(orig);
  StructWithBinaryField deserialized;
  const auto size = S::deserialize(serialized, deserialized);
  EXPECT_EQ(serialized.size(), size);
  EXPECT_EQ(orig, deserialized);
}

TEST_F(SimpleJSONProtocolTest, roundtrip_binary_without_padding) {
  StructWithBinaryField orig;
  orig.field() = "0123";
  const std::string serialized = "{\"field\":\"MDEyMw\"}";
  StructWithBinaryField deserialized;
  const auto size = S::deserialize(serialized, deserialized);
  EXPECT_EQ(serialized.size(), size);
  EXPECT_EQ(orig, deserialized);
}

TEST_F(SimpleJSONProtocolTest, roundtrip_binary_with_padding) {
  StructWithBinaryField orig;
  orig.field() = "0123";
  const std::string serialized = "{\"field\":\"MDEyMw==\"}";
  StructWithBinaryField deserialized;
  const auto size = S::deserialize(serialized, deserialized);
  EXPECT_EQ(serialized.size(), size);
  EXPECT_EQ(orig, deserialized);
}

TEST_F(SimpleJSONProtocolTest, roundtrip_binary_disallow_padding) {
  StructWithBinaryField orig;
  orig.field() = "0123";
  const std::string serialized = "{\"field\":\"MDEyMw==\"}";

  // Deserialize with allowBase64Padding disabled
  StructWithBinaryField deserialized;
  SimpleJSONProtocolReader reader;
  folly::IOBuf buf{folly::IOBuf::WRAP_BUFFER, folly::StringPiece(serialized)};
  reader.setInput(&buf);
  reader.setAllowBase64Padding(false);
  deserialized.read(&reader);
  auto const size = reader.getCursor().getCurrentPosition();

  EXPECT_EQ(serialized.size(), size);
  EXPECT_NE(orig, deserialized);
}
