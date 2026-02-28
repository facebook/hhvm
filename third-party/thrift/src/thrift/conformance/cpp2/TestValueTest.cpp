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

#include <thrift/conformance/cpp2/TestValue.h>

#include <random>

#include <gtest/gtest.h>
#include <thrift/conformance/cpp2/AnyRegistry.h>
#include <thrift/conformance/cpp2/Protocol.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/test/testset/Populator.h>
#include <thrift/test/testset/gen-cpp2/testset_types_custom_protocol.h>

namespace apache::thrift::conformance {
namespace {

template <typename T>
class TestValueTest : public testing::Test {};

TYPED_TEST_CASE_P(TestValueTest);

TYPED_TEST_P(TestValueTest, RoundTrip) {
  std::mt19937 rng;
  auto expected = test::populated_if_not_adapted<TypeParam>(rng);

  // Convert to an EncodeValue.
  EncodeValue value = asEncodeValue(expected);
  // Encode to an Any.
  Any any = encodeValue<BinaryProtocolWriter>(
      getStandardProtocol<StandardProtocol::Binary>(), value);
  // Load any back to a struct_all.
  auto actual = AnyRegistry::generated().load<TypeParam>(any);

  EXPECT_EQ(actual, expected);
}

REGISTER_TYPED_TEST_CASE_P(TestValueTest, RoundTrip);
THRIFT_INST_TESTSET_ALL(TestValueTest);

} // namespace
} // namespace apache::thrift::conformance
