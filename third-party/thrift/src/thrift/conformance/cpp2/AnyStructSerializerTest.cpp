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

#include <thrift/conformance/cpp2/AnyStructSerializer.h>

#include <folly/portability/GTest.h>
#include <thrift/conformance/cpp2/Testing.h>
#include <thrift/lib/cpp2/protocol/Object.h>

namespace apache::thrift::conformance {
namespace {
using ::apache::thrift::protocol::asValueStruct;

TEST(SerializerTest, BaseTypes) {
  THRIFT_SCOPED_CHECK(checkRoundTrip(asValueStruct<type::bool_t>(true)));
  THRIFT_SCOPED_CHECK(checkRoundTrip(asValueStruct<type::byte_t>(1)));
  THRIFT_SCOPED_CHECK(checkRoundTrip(asValueStruct<type::i16_t>(1)));
  THRIFT_SCOPED_CHECK(checkRoundTrip(asValueStruct<type::i32_t>(1)));
  THRIFT_SCOPED_CHECK(checkRoundTrip(asValueStruct<type::i64_t>(1)));
  THRIFT_SCOPED_CHECK(checkRoundTrip(asValueStruct<type::float_t>(1)));
  THRIFT_SCOPED_CHECK(checkRoundTrip(asValueStruct<type::double_t>(1)));
  THRIFT_SCOPED_CHECK(checkRoundTrip(asValueStruct<type::string_t>("hi")));
  THRIFT_SCOPED_CHECK(checkRoundTrip(asValueStruct<type::binary_t>("hi")));
}

} // namespace
} // namespace apache::thrift::conformance
