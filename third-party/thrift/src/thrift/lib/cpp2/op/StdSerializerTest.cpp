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

#include <thrift/lib/cpp2/op/StdSerializer.h>

#include <folly/portability/GTest.h>
#include <thrift/conformance/cpp2/Object.h>
#include <thrift/conformance/if/gen-cpp2/object_types_custom_protocol.h>
#include <thrift/lib/cpp2/op/Testing.h>

namespace apache::thrift::op {
namespace {

// Checks that the value roundtrips correctly for all standard protocols.
template <typename T>
void testStructRoundTrip(const T& value) {
  using type::StandardProtocol;
  using Tag = type::struct_t<T>;
  FBTHRIFT_SCOPED_CHECK(test::expectRoundTrip<Tag>(
      StdSerializer<Tag, StandardProtocol::Binary>(), value));
  FBTHRIFT_SCOPED_CHECK(test::expectRoundTrip<Tag>(
      StdSerializer<Tag, StandardProtocol::Compact>(), value));
  FBTHRIFT_SCOPED_CHECK(test::expectRoundTrip<Tag>(
      StdSerializer<Tag, StandardProtocol::Json>(), value));
  FBTHRIFT_SCOPED_CHECK(test::expectRoundTrip<Tag>(
      StdSerializer<Tag, StandardProtocol::SimpleJson>(), value));
}

TEST(StdSerializerTest, Struct) {
  using conformance::asValueStruct;
  FBTHRIFT_SCOPED_CHECK(testStructRoundTrip(asValueStruct<type::bool_t>(true)));
  FBTHRIFT_SCOPED_CHECK(testStructRoundTrip(asValueStruct<type::byte_t>(1)));
  FBTHRIFT_SCOPED_CHECK(testStructRoundTrip(asValueStruct<type::i16_t>(1)));
  FBTHRIFT_SCOPED_CHECK(testStructRoundTrip(asValueStruct<type::i32_t>(1)));
  FBTHRIFT_SCOPED_CHECK(testStructRoundTrip(asValueStruct<type::i64_t>(1)));
  FBTHRIFT_SCOPED_CHECK(testStructRoundTrip(asValueStruct<type::float_t>(1)));
  FBTHRIFT_SCOPED_CHECK(testStructRoundTrip(asValueStruct<type::double_t>(1)));
  FBTHRIFT_SCOPED_CHECK(
      testStructRoundTrip(asValueStruct<type::string_t>("hi")));
  FBTHRIFT_SCOPED_CHECK(
      testStructRoundTrip(asValueStruct<type::binary_t>("hi")));
}

} // namespace
} // namespace apache::thrift::op
