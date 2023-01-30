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
template <typename T, typename Tag = type::infer_tag<T>>
void testRoundTrip(const T& value) {
  using type::StandardProtocol;
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
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(asValueStruct<type::bool_t>(true)));
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(asValueStruct<type::byte_t>(1)));
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(asValueStruct<type::i16_t>(1)));
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(asValueStruct<type::i32_t>(1)));
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(asValueStruct<type::i64_t>(1)));
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(asValueStruct<type::float_t>(1)));
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(asValueStruct<type::double_t>(1)));
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(asValueStruct<type::string_t>("hi")));
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(asValueStruct<type::binary_t>("hi")));
}

TEST(StdSerializerTest, PrimaryTypes) {
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(true));
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(std::int8_t(16)));
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(std::int16_t(16)));
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(std::int32_t(32)));
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(std::int64_t(64)));
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(1.1f));
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(2.2));
  FBTHRIFT_SCOPED_CHECK((testRoundTrip<std::string, type::string_t>("hi")));
  FBTHRIFT_SCOPED_CHECK((testRoundTrip<std::string, type::binary_t>("hi")));
}

} // namespace
} // namespace apache::thrift::op
