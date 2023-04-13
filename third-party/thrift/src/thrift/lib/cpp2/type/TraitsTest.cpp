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

#include <list>
#include <unordered_map>
#include <unordered_set>

#include <thrift/lib/cpp2/type/Traits.h>

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/type/BaseType.h>
#include <thrift/lib/cpp2/type/Name.h>
#include <thrift/lib/cpp2/type/NativeType.h>
#include <thrift/lib/cpp2/type/Testing.h>
#include <thrift/lib/cpp2/type/ThriftType.h>
#include <thrift/lib/cpp2/type/Type.h>
#include <thrift/lib/thrift/gen-cpp2/protocol_types_custom_protocol.h>
#include <thrift/lib/thrift/gen-cpp2/type_types.h>

namespace apache::thrift::type {
namespace {
using test::FieldAdapter;
using test::FieldValue;
using test::TestAdapter;
using test::TestStruct;
using test::TestValue;

template <typename Types, typename Tag, bool Expected>
void testContains() {
  static_assert(Types::template contains<Tag>() == Expected);
  if constexpr (
      is_concrete_v<Tag> &&
      // TODO(afuller): Support concrete named types.
      !named_types::contains<Tag>()) {
    EXPECT_EQ(Types::contains(base_type_v<Tag>), Expected);
  }
}

template <
    typename Tag,
    typename StandardType,
    typename NativeType = StandardType>
void test_concrete_type() {
  test::same_type<standard_type<Tag>, StandardType>;
  test::same_type<native_type<Tag>, NativeType>;

  using field_tag = field<Tag, FieldContext<TestStruct, 0>>;
  test::same_type<standard_type<field_tag>, StandardType>;
  test::same_type<native_type<field_tag>, NativeType>;
}

TEST(TraitsTest, Bool) {
  using tag = bool_t;
  testContains<primitive_types, tag, true>();
  testContains<structured_types, tag, false>();
  testContains<singular_types, tag, true>();
  testContains<container_types, tag, false>();
  testContains<composite_types, tag, false>();

  EXPECT_EQ(base_type_v<tag>, BaseType::Bool);
  EXPECT_EQ(getName<tag>(), "bool");
  test_concrete_type<tag, bool>();
}

TEST(TraitsTest, Byte) {
  using tag = byte_t;
  testContains<primitive_types, tag, true>();
  testContains<structured_types, tag, false>();
  testContains<singular_types, tag, true>();
  testContains<container_types, tag, false>();
  testContains<composite_types, tag, false>();

  EXPECT_EQ(base_type_v<tag>, BaseType::Byte);
  EXPECT_EQ(getName<tag>(), "byte");
  test_concrete_type<tag, int8_t>();
}

TEST(TraitsTest, I16) {
  using tag = i16_t;
  testContains<primitive_types, tag, true>();
  testContains<structured_types, tag, false>();
  testContains<singular_types, tag, true>();
  testContains<container_types, tag, false>();
  testContains<composite_types, tag, false>();

  EXPECT_EQ(base_type_v<tag>, BaseType::I16);
  EXPECT_EQ(getName<tag>(), "i16");
  test_concrete_type<tag, int16_t>();
}

TEST(TraitsTest, I32) {
  using tag = i32_t;
  testContains<primitive_types, tag, true>();
  testContains<structured_types, tag, false>();
  testContains<singular_types, tag, true>();
  testContains<container_types, tag, false>();
  testContains<composite_types, tag, false>();

  EXPECT_EQ(base_type_v<tag>, BaseType::I32);
  EXPECT_EQ(getName<tag>(), "i32");
  test_concrete_type<tag, int32_t>();
}

TEST(TraitsTest, I64) {
  using tag = i64_t;
  testContains<primitive_types, tag, true>();
  testContains<structured_types, tag, false>();
  testContains<singular_types, tag, true>();
  testContains<container_types, tag, false>();
  testContains<composite_types, tag, false>();

  EXPECT_EQ(base_type_v<tag>, BaseType::I64);
  EXPECT_EQ(getName<tag>(), "i64");
  test_concrete_type<tag, int64_t>();
}

TEST(TraitsTest, Enum) {
  using tag = enum_c;
  testContains<primitive_types, tag, true>();
  testContains<structured_types, tag, false>();
  testContains<singular_types, tag, true>();
  testContains<container_types, tag, false>();
  testContains<composite_types, tag, false>();

  EXPECT_EQ(base_type_v<tag>, BaseType::Enum);
  EXPECT_EQ(getName<tag>(), "enum");

  using tag_t = enum_t<BaseTypeEnum>;
  EXPECT_EQ(base_type_v<tag_t>, BaseType::Enum);
  // TODO(afuller): Remove dep of fatal.
  // EXPECT_EQ(getName<tag_t>(), "type.BaseType");
  test_concrete_type<tag_t, BaseTypeEnum>();

  using tag_at = adapted<StaticCastAdapter<BaseType, BaseTypeEnum>, tag_t>;
  EXPECT_EQ(base_type_v<tag_at>, BaseType::Enum);
  EXPECT_EQ(getName<tag_at>(), "apache::thrift::type::BaseType");
  test_concrete_type<tag_at, BaseTypeEnum, BaseType>();
}

TEST(TraitsTest, Float) {
  using tag = float_t;
  testContains<primitive_types, tag, true>();
  testContains<structured_types, tag, false>();
  testContains<singular_types, tag, true>();
  testContains<container_types, tag, false>();
  testContains<composite_types, tag, false>();

  EXPECT_EQ(base_type_v<tag>, BaseType::Float);
  EXPECT_EQ(getName<tag>(), "float");
  test_concrete_type<tag, float>();
}

TEST(TraitsTest, Double) {
  using tag = double_t;
  testContains<primitive_types, tag, true>();
  testContains<structured_types, tag, false>();
  testContains<singular_types, tag, true>();
  testContains<container_types, tag, false>();
  testContains<composite_types, tag, false>();

  EXPECT_EQ(base_type_v<tag>, BaseType::Double);
  EXPECT_EQ(getName<tag>(), "double");
  test_concrete_type<tag, double>();
}

TEST(TraitsTest, String) {
  using tag = string_t;
  testContains<primitive_types, tag, true>();
  testContains<structured_types, tag, false>();
  testContains<singular_types, tag, true>();
  testContains<container_types, tag, false>();
  testContains<composite_types, tag, false>();

  EXPECT_EQ(base_type_v<tag>, BaseType::String);
  EXPECT_EQ(getName<tag>(), "string");
  test_concrete_type<tag, std::string>();
}

TEST(TraitsTest, Binary) {
  using tag = binary_t;
  testContains<primitive_types, tag, true>();
  testContains<structured_types, tag, false>();
  testContains<singular_types, tag, true>();
  testContains<container_types, tag, false>();
  testContains<composite_types, tag, false>();

  EXPECT_EQ(base_type_v<tag>, BaseType::Binary);
  EXPECT_EQ(getName<tag>(), "binary");
  test_concrete_type<tag, std::string>();
}

TEST(TraitsTest, Struct) {
  using tag = struct_c;
  testContains<primitive_types, tag, false>();
  testContains<structured_types, tag, true>();
  testContains<singular_types, tag, true>();
  testContains<container_types, tag, false>();
  testContains<composite_types, tag, true>();

  EXPECT_EQ(base_type_v<tag>, BaseType::Struct);
  EXPECT_EQ(getName<tag>(), "struct");

  using tag_t = struct_t<protocol::Object>;
  EXPECT_EQ(base_type_v<tag_t>, BaseType::Struct);
  // TODO(afuller): Remove dep of fatal.
  // EXPECT_EQ(getName<tag_t>(), "object.Object");
  test_concrete_type<tag_t, protocol::Object>();
}

TEST(TraitsTest, Union) {
  using tag = union_c;
  testContains<primitive_types, tag, false>();
  testContains<structured_types, tag, true>();
  testContains<singular_types, tag, true>();
  testContains<container_types, tag, false>();
  testContains<composite_types, tag, true>();

  EXPECT_EQ(base_type_v<tag>, BaseType::Union);
  EXPECT_EQ(getName<tag>(), "union");

  using tag_t = union_t<protocol::Value>;
  EXPECT_EQ(base_type_v<tag_t>, BaseType::Union);
  // TODO(afuller): Remove dep of fatal.
  // EXPECT_EQ(getName<tag_t>(), "object.Value");
  test_concrete_type<tag_t, protocol::Value>();
}

TEST(TraitsTest, Exception) {
  using tag = exception_c;
  testContains<primitive_types, tag, false>();
  testContains<structured_types, tag, true>();
  testContains<singular_types, tag, true>();
  testContains<container_types, tag, false>();
  testContains<composite_types, tag, true>();

  EXPECT_EQ(base_type_v<tag>, BaseType::Exception);
  EXPECT_EQ(getName<tag>(), "exception");

  // TODO(afuller): Add a test exception and test the concrete form.
}

TEST(TraitsTest, List) {
  using tag = list_c;
  testContains<primitive_types, tag, false>();
  testContains<structured_types, tag, false>();
  testContains<singular_types, tag, false>();
  testContains<container_types, tag, true>();
  testContains<composite_types, tag, true>();

  EXPECT_EQ(base_type_v<tag>, BaseType::List);
  EXPECT_EQ(getName<tag>(), "list");

  using tag_c = list<struct_c>;
  EXPECT_EQ(base_type_v<tag_c>, BaseType::List);
  EXPECT_EQ(getName<tag_c>(), "list<struct>");

  using tag_t = list<string_t>;
  EXPECT_EQ(base_type_v<tag_t>, BaseType::List);
  EXPECT_EQ(getName<tag_t>(), "list<string>");
  test_concrete_type<tag_t, std::vector<std::string>>();
}

TEST(TraitsTest, Set) {
  using tag = set_c;
  testContains<primitive_types, tag, false>();
  testContains<structured_types, tag, false>();
  testContains<singular_types, tag, false>();
  testContains<container_types, tag, true>();
  testContains<composite_types, tag, true>();

  EXPECT_EQ(base_type_v<tag>, BaseType::Set);
  EXPECT_EQ(getName<tag>(), "set");

  using tag_c = set<struct_c>;
  EXPECT_EQ(base_type_v<tag_c>, BaseType::Set);
  EXPECT_EQ(getName<tag_c>(), "set<struct>");

  using tag_t = set<string_t>;
  EXPECT_EQ(base_type_v<tag_t>, BaseType::Set);
  EXPECT_EQ(getName<tag_t>(), "set<string>");
  test_concrete_type<tag_t, std::set<std::string>>();
}

TEST(TraitsTest, Map) {
  using tag = map_c;
  testContains<primitive_types, tag, false>();
  testContains<structured_types, tag, false>();
  testContains<singular_types, tag, false>();
  testContains<container_types, tag, true>();
  testContains<composite_types, tag, true>();

  EXPECT_EQ(base_type_v<tag>, BaseType::Map);
  EXPECT_EQ(getName<tag>(), "map");

  using tag_c = map<struct_c, struct_c>;
  EXPECT_EQ(base_type_v<tag_c>, BaseType::Map);
  EXPECT_EQ(getName<tag_c>(), "map<struct, struct>");

  using tag_kc = map<struct_c, byte_t>;
  EXPECT_EQ(base_type_v<tag_kc>, BaseType::Map);
  EXPECT_EQ(getName<tag_kc>(), "map<struct, byte>");

  using tag_vc = map<byte_t, struct_c>;
  EXPECT_EQ(base_type_v<tag_vc>, BaseType::Map);
  EXPECT_EQ(getName<tag_vc>(), "map<byte, struct>");

  using tag_t = map<i16_t, i32_t>;
  EXPECT_EQ(base_type_v<tag_t>, BaseType::Map);
  EXPECT_EQ(getName<tag_t>(), "map<i16, i32>");
  test_concrete_type<tag_t, std::map<int16_t, int32_t>>();
}

TEST(TraitsTest, Adapted) {
  using tag = adapted<TestAdapter, i64_t>;
  // All traits that operate on the standard type, match the given tag.
  testContains<primitive_types, tag, true>();
  testContains<structured_types, tag, false>();
  testContains<singular_types, tag, true>();
  testContains<container_types, tag, false>();
  testContains<composite_types, tag, false>();
  EXPECT_EQ(base_type_v<tag>, BaseType::I64);

  // The name and native_type have changed.
  EXPECT_EQ(getName<tag>(), folly::pretty_name<TestValue<long>>());
  test_concrete_type<tag, int64_t, TestValue<int64_t>>();
}

TEST(TraitsTest, FieldAdapted) {
  using tag = field<adapted<FieldAdapter, i64_t>, FieldContext<TestStruct, 1>>;
  // All traits that operate on the standard type, match the given tag.
  testContains<primitive_types, tag, true>();
  testContains<structured_types, tag, false>();
  testContains<singular_types, tag, true>();
  testContains<container_types, tag, false>();
  testContains<composite_types, tag, false>();
  EXPECT_EQ(base_type_v<tag>, BaseType::I64);

  // TODO(dokwon): Enable getName for field adapted tag.
  // The name and native_type have changed.
  // EXPECT_EQ(
  //     getName<tag>(), folly::pretty_name<FieldValue<int64_t, TestStruct,
  //     1>>());
  test::same_type<standard_type<tag>, int64_t>;
  test::same_type<native_type<tag>, FieldValue<int64_t, TestStruct, 1>>;
}

TEST(TraitsTest, CppType) {
  using tag = cpp_type<uint64_t, i64_t>;
  // All traits that operate on the standard type, match the given tag.
  testContains<primitive_types, tag, true>();
  testContains<structured_types, tag, false>();
  testContains<singular_types, tag, true>();
  testContains<container_types, tag, false>();
  testContains<composite_types, tag, false>();
  EXPECT_EQ(base_type_v<tag>, BaseType::I64);

  // The name and native_type have changed.
  EXPECT_EQ(getName<tag>(), folly::pretty_name<uint64_t>());
  test_concrete_type<tag, int64_t, uint64_t>();
}

TEST(TraitsTest, AdaptedListElems) {
  using tag_t = list<adapted<TestAdapter, i64_t>>;
  EXPECT_EQ(base_type_v<tag_t>, BaseType::List);

  EXPECT_EQ(
      getName<tag_t>(),
      fmt::format("list<{}>", folly::pretty_name<TestValue<long>>()));
  test_concrete_type<
      tag_t,
      std::vector<int64_t>,
      std::vector<TestValue<int64_t>>>();
}

TEST(TraitsTest, Filter) {
  test::same_type<
      primitive_types::filter<bound::is_concrete>,
      detail::types<
          bool_t,
          byte_t,
          i16_t,
          i32_t,
          i64_t,
          float_t,
          double_t,
          string_t,
          binary_t>>;
}

} // namespace
} // namespace apache::thrift::type
