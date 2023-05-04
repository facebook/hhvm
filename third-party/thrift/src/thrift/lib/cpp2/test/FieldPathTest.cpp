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
#include <thrift/conformance/cpp2/Protocol.h>
#include <thrift/conformance/data/ValueGenerator.h>
#include <thrift/conformance/if/gen-cpp2/conformance_types_custom_protocol.h>
#include <thrift/conformance/if/gen-cpp2/protocol_types_custom_protocol.h>
#include <thrift/lib/cpp2/protocol/FieldPath.h>
#include <thrift/lib/cpp2/protocol/Object.h>

namespace apache::thrift::protocol {
namespace {

Path makePath(const std::vector<std::int64_t>& path) {
  Path ret;
  for (auto segId : path) {
    ret.path()->push_back(static_cast<PathSegmentId>(segId));
  }
  return ret;
}

TEST(FieldPathTest, Byte) {
  Object object;
  object[FieldId{10}] = asValueStruct<type::byte_t>(100);
  object[FieldId{20}] = asValueStruct<type::i16_t>(200);
  object[FieldId{30}] = asValueStruct<type::i32_t>(300);
  object[FieldId{40}] = asValueStruct<type::i64_t>(400);
  object[FieldId{50}] = asValueStruct<type::string_t>("500");

  auto protocol = ::apache::thrift::conformance::Protocol("hi").asStruct();
  object[FieldId{60}] = asValueStruct<type::union_c>(protocol);

  auto test_object = [&protocol](auto&& obj) {
    EXPECT_EQ(*get(obj, makePath({10})), asValueStruct<type::byte_t>(100));
    EXPECT_EQ(*get(obj, makePath({20})), asValueStruct<type::i16_t>(200));
    EXPECT_EQ(*get(obj, makePath({30})), asValueStruct<type::i32_t>(300));
    EXPECT_EQ(*get(obj, makePath({40})), asValueStruct<type::i64_t>(400));
    EXPECT_EQ(*get(obj, makePath({50})), asValueStruct<type::string_t>("500"));
    EXPECT_EQ(
        *get(obj, makePath({60})), asValueStruct<type::union_c>(protocol));
    EXPECT_EQ(
        *get(obj, makePath({60, 1})),
        asValueStruct<type::enum_c>(
            ::apache::thrift::conformance::StandardProtocol::Custom));
    EXPECT_EQ(
        *get(obj, makePath({60, 2})), asValueStruct<type::binary_t>("hi"));

    EXPECT_EQ(get(obj, makePath({})), nullptr);
    EXPECT_EQ(get(obj, makePath({11})), nullptr);
    EXPECT_EQ(get(obj, makePath({60, 3})), nullptr);
    EXPECT_EQ(get(obj, makePath({60, 2, 1})), nullptr);
  };

  test_object(object);
  test_object(std::as_const(object));

  *get(object, makePath({10})) = asValueStruct<type::string_t>("42");
  EXPECT_EQ(object[FieldId{10}].stringValue_ref(), "42");
}

} // namespace
} // namespace apache::thrift::protocol
