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

#include <random>

#include <gtest/gtest.h>
#include <thrift/lib/cpp2/protocol/Json5Protocol.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/test/testset/Populator.h>
#include <thrift/test/testset/gen-cpp2/testset_types_custom_protocol.h>

namespace apache::thrift {
namespace {

template <typename T>
class SerializationRoundTripTest : public testing::Test {
 protected:
  void SetUp() override {
    std::mt19937 rng;
    expected = test::populated<T>(rng);
  }

  void testSerializerRoundTrip(auto serializer) {
    auto serialized = serializer.template serialize<std::string>(expected);
    auto actual = serializer.template deserialize<T>(serialized);
    EXPECT_EQ(actual, expected);
  }

  T expected;
};

TYPED_TEST_CASE_P(SerializationRoundTripTest);

TYPED_TEST_P(SerializationRoundTripTest, CompactProtocol) {
  this->testSerializerRoundTrip(CompactSerializer{});
}

TYPED_TEST_P(SerializationRoundTripTest, BinaryProtocol) {
  this->testSerializerRoundTrip(BinarySerializer{});
}

TYPED_TEST_P(SerializationRoundTripTest, SimpleJSONProtocol) {
  this->testSerializerRoundTrip(SimpleJSONSerializer{});
}

TYPED_TEST_P(SerializationRoundTripTest, Json5Protocol) {
  auto json5 = Json5ProtocolUtils::toJson5(this->expected);
  auto actual = Json5ProtocolUtils::fromJson5<TypeParam>(json5);
  EXPECT_EQ(actual, this->expected);
}

REGISTER_TYPED_TEST_CASE_P(
    SerializationRoundTripTest,
    CompactProtocol,
    BinaryProtocol,
    SimpleJSONProtocol,
    Json5Protocol);

THRIFT_INST_TESTSET_ALL(SerializationRoundTripTest);

} // namespace
} // namespace apache::thrift
