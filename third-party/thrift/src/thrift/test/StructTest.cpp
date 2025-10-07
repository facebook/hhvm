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

#include <memory>
#include <thrift/test/gen-cpp2/structs_terse_types.h>
#include <thrift/test/gen-cpp2/structs_types.h>

#include <gtest/gtest.h>
#include <folly/Traits.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>

namespace apache::thrift::test {
namespace {

using apache::thrift::detail::st::struct_private_access;

TEST(StructTest, compilation_terse_writes_refs_shared) {
  BasicRefsSharedTerseWrites a;
  (void)a;
}

TEST(StructTest, serialization_terse_writes_refs_shared) {
  using apache::thrift::CompactSerializer;

  BasicRefsSharedTerseWrites a;
  EXPECT_FALSE(apache::thrift::empty(a));

  a.shared_field() = std::make_shared<HasInt>();
  a.shared_field()->field() = 3;

  a.shared_fields() = std::make_shared<std::vector<HasInt>>();
  a.shared_fields()->emplace_back();
  a.shared_fields()->back().field() = 4;
  a.shared_fields()->emplace_back();
  a.shared_fields()->back().field() = 5;
  a.shared_fields()->emplace_back();
  a.shared_fields()->back().field() = 6;

  a.shared_fields_const() = std::make_shared<const std::vector<HasInt>>();

  const std::string serialized = CompactSerializer::serialize<std::string>(a);

  BasicRefsSharedTerseWrites b;
  CompactSerializer::deserialize(serialized, b);

  EXPECT_EQ(a, b);
}

TEST(StructTest, serialization_terse_writes_default_values) {
  using apache::thrift::CompactSerializer;

  BasicRefsSharedTerseWrites empty;

  BasicRefsSharedTerseWrites defaults;
  defaults.shared_field_req() = std::make_shared<HasInt>();
  defaults.shared_fields_req() = std::make_shared<std::vector<HasInt>>();

  // This struct has terse writes enabled, so the default values set above
  // should not be part of the serialization.
  EXPECT_EQ(
      CompactSerializer::serialize<std::string>(empty),
      CompactSerializer::serialize<std::string>(defaults));
}

TEST(StructTest, equal_to) {
  std::equal_to<Basic> op;

  {
    Basic a;
    Basic b;

    *a.def_field() = 3;
    *b.def_field() = 3;
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    a.def_field().ensure();
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    b.def_field().ensure();
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    *a.def_field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    *b.def_field() = 4;
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));
  }
  {
    Basic a;
    Basic b;

    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    a.opt_field() = 3;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.opt_field() = 3;
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    a.opt_field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.opt_field() = 4;
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));
  }
  {
    Basic a;
    Basic b;

    *a.req_field() = 3;
    *b.req_field() = 3;
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    *a.req_field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    *b.req_field() = 4;
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));
  }
  {
    Basic a;
    Basic b;

    a.req_field() = 3;
    b.req_field() = 3;
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    a.req_field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.req_field() = 4;
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));
  }
}

// currently, binary fields are handled specially, so they get their own tests
TEST(StructTest, equal_to_binary) {
  std::equal_to<BasicBinaries> op;

  {
    BasicBinaries a;
    BasicBinaries b;

    *a.def_field() = "hello";
    *b.def_field() = "hello";
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    a.def_field().ensure();
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    b.def_field().ensure();
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    *a.def_field() = "world";
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    *b.def_field() = "world";
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));
  }
  {
    BasicBinaries a;
    BasicBinaries b;

    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    a.opt_field() = "hello";
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.opt_field() = "hello";
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    a.opt_field() = "world";
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.opt_field() = "world";
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));
  }
  {
    BasicBinaries a;
    BasicBinaries b;

    *a.req_field() = "hello";
    *b.req_field() = "hello";
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    *a.req_field() = "world";
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    *b.req_field() = "world";
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));
  }
  {
    BasicBinaries a;
    BasicBinaries b;

    a.req_field() = "hello";
    b.req_field() = "hello";
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    a.req_field() = "world";
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.req_field() = "world";
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));
  }
}

// current equal-to codegen implements def/opt/req ref fields with a single
// code path, so no need to test the different cases separately
TEST(StructTest, equal_to_refs) {
  std::equal_to<BasicRefs> op;

  {
    BasicRefs a;
    BasicRefs b;

    a.def_field() = nullptr;
    b.def_field() = nullptr;
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    a.def_field() = std::make_unique<HasInt>();
    *a.def_field()->field() = 3;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.def_field() = std::make_unique<HasInt>();
    *b.def_field()->field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    *a.def_field()->field() = 4;
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));
  }
  {
    BasicRefs a;
    BasicRefs b;

    a.def_field() = nullptr;
    b.def_field() = nullptr;
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    a.def_field() = std::make_unique<HasInt>();
    *a.def_field()->field() = 3;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.def_field() = std::make_unique<HasInt>();
    *b.def_field()->field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    *a.def_field()->field() = 4;
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));
  }
}

TEST(StructTest, equal_to_refs_shared) {
  std::equal_to<BasicRefsShared> op;

  {
    BasicRefsShared a;
    BasicRefsShared b;

    a.def_field() = nullptr;
    b.def_field() = nullptr;
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    a.def_field() = std::make_shared<HasInt>();
    *a.def_field()->field() = 3;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.def_field() = std::make_shared<HasInt>();
    *b.def_field()->field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    *a.def_field()->field() = 4;
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));

    b.def_field() = a.def_field();
    EXPECT_TRUE(op(a, b));
    EXPECT_TRUE(op(b, a));
  }
}

TEST(StructTest, less) {
  std::less<Basic> op;

  {
    Basic a;
    Basic b;

    *b.def_field() = 3;
    *a.def_field() = 3;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.def_field().ensure();
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    a.def_field().ensure();
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    *b.def_field() = 4;
    EXPECT_TRUE(op(a, b));
    EXPECT_FALSE(op(b, a));

    *a.def_field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));
  }
  {
    Basic a;
    Basic b;

    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.opt_field() = 3;
    EXPECT_TRUE(op(a, b));
    EXPECT_FALSE(op(b, a));

    a.opt_field() = 3;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.opt_field() = 4;
    EXPECT_TRUE(op(a, b));
    EXPECT_FALSE(op(b, a));

    a.opt_field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));
  }
  {
    Basic a;
    Basic b;

    *b.req_field() = 3;
    *a.req_field() = 3;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    *b.req_field() = 4;
    EXPECT_TRUE(op(a, b));
    EXPECT_FALSE(op(b, a));

    *a.req_field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));
  }
  {
    Basic a;
    Basic b;

    b.req_field() = 3;
    a.req_field() = 3;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.req_field() = 4;
    EXPECT_TRUE(op(a, b));
    EXPECT_FALSE(op(b, a));

    a.req_field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));
  }
}

// currently, binary fields are handled specially, so they get their own tests
TEST(StructTest, less_binary) {
  std::less<BasicBinaries> op;

  {
    BasicBinaries a;
    BasicBinaries b;

    *b.def_field() = "hello";
    *a.def_field() = "hello";
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.def_field().ensure();
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    a.def_field().ensure();
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    *b.def_field() = "world";
    EXPECT_TRUE(op(a, b));
    EXPECT_FALSE(op(b, a));

    *a.def_field() = "world";
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));
  }
  {
    BasicBinaries a;
    BasicBinaries b;

    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.opt_field() = "hello";
    EXPECT_TRUE(op(a, b));
    EXPECT_FALSE(op(b, a));

    a.opt_field() = "hello";
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.opt_field() = "world";
    EXPECT_TRUE(op(a, b));
    EXPECT_FALSE(op(b, a));

    a.opt_field() = "world";
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));
  }
  {
    BasicBinaries a;
    BasicBinaries b;

    *b.req_field() = "hello";
    *a.req_field() = "hello";
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    *b.req_field() = "world";
    EXPECT_TRUE(op(a, b));
    EXPECT_FALSE(op(b, a));

    *a.req_field() = "world";
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));
  }
  {
    BasicBinaries a;
    BasicBinaries b;

    b.req_field() = "hello";
    a.req_field() = "hello";
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.req_field() = "world";
    EXPECT_TRUE(op(a, b));
    EXPECT_FALSE(op(b, a));

    a.req_field() = "world";
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));
  }
}

// current less codegen implements def/opt/req ref fields with a single
// code path, so no need to test the different cases separately
TEST(StructTest, less_refs) {
  std::less<BasicRefs> op;

  {
    BasicRefs a;
    BasicRefs b;

    b.def_field() = nullptr;
    a.def_field() = nullptr;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.def_field() = std::make_unique<HasInt>();
    *b.def_field()->field() = 3;
    EXPECT_TRUE(op(a, b));
    EXPECT_FALSE(op(b, a));

    a.def_field() = std::make_unique<HasInt>();
    *a.def_field()->field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_TRUE(op(b, a));

    *b.def_field()->field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));
  }
}

TEST(StructTest, less_refs_shared) {
  std::less<BasicRefsShared> op;

  {
    BasicRefsShared a;
    BasicRefsShared b;

    b.def_field() = nullptr;
    a.def_field() = nullptr;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.def_field() = std::make_unique<HasInt>();
    *b.def_field()->field() = 3;
    EXPECT_TRUE(op(a, b));
    EXPECT_FALSE(op(b, a));

    a.def_field() = std::make_unique<HasInt>();
    *a.def_field()->field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_TRUE(op(b, a));

    *b.def_field()->field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.def_field() = a.def_field();
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));
  }
  {
    BasicRefsShared a;
    BasicRefsShared b;

    b.def_field() = nullptr;
    a.def_field() = nullptr;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.def_field() = std::make_unique<HasInt>();
    *b.def_field()->field() = 3;
    EXPECT_TRUE(op(a, b));
    EXPECT_FALSE(op(b, a));

    a.def_field() = std::make_unique<HasInt>();
    *a.def_field()->field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_TRUE(op(b, a));

    *b.def_field()->field() = 4;
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));

    b.def_field() = a.def_field();
    EXPECT_FALSE(op(a, b));
    EXPECT_FALSE(op(b, a));
  }
}

TEST(StructTest, small_sorted_vector) {
  using Set = folly::small_sorted_vector_set<int32_t>;
  using Map = folly::small_sorted_vector_map<int32_t, int32_t>;
  using serializer = apache::thrift::BinarySerializer;
  using Type = HasSmallSortedVector;
  EXPECT_TRUE(
      (std::is_same<folly::remove_cvref_t<decltype(*Type().set_field())>, Set>::
           value));
  EXPECT_TRUE(
      (std::is_same<folly::remove_cvref_t<decltype(*Type().map_field())>, Map>::
           value));

  Type o;
  o.set_field()->insert({1, 3, 5});
  o.map_field()->insert({{1, 4}, {3, 12}, {5, 20}});
  auto a = serializer::deserialize<HasSmallSortedVector>(
      serializer::serialize<std::string>(o));
  EXPECT_EQ(o, a);
  EXPECT_EQ(*o.set_field(), *a.set_field());
  EXPECT_EQ(*o.map_field(), *a.map_field());
}

TEST(StructTest, noexcept_move_annotation) {
  EXPECT_TRUE(std::is_nothrow_move_constructible<NoexceptMoveStruct>::value);
  EXPECT_TRUE(std::is_nothrow_move_assignable<NoexceptMoveStruct>::value);
  NoexceptMoveStruct a;
  a.string_field() = "hello world";
  NoexceptMoveStruct b(std::move(a));
  EXPECT_EQ(b.string_field().value(), "hello world");
  NoexceptMoveStruct c;
  c = std::move(b);
  EXPECT_EQ(c.string_field().value(), "hello world");
}

TEST(StructTest, clear) {
  Basic obj;
  obj.def_field() = 7;
  obj.req_field() = 8;
  obj.opt_field() = 9;
  apache::thrift::clear(obj);
  EXPECT_FALSE(obj.def_field().is_set());
  EXPECT_EQ(0, *obj.def_field());
  EXPECT_EQ(0, *obj.req_field());
  EXPECT_FALSE(obj.opt_field());
}

TEST(StructTest, EmptiableOptionalFieldsStruct) {
  EmptiableOptionalFieldsStruct obj;
  EXPECT_TRUE(apache::thrift::empty(obj));

  obj.int_field() = 1;
  EXPECT_FALSE(apache::thrift::empty(obj));

  apache::thrift::clear(obj);
  EXPECT_TRUE(apache::thrift::empty(obj));
}

TEST(StructTest, NotEmptiableStruct) {
  NotEmptiableStruct obj;
  EXPECT_FALSE(apache::thrift::empty(obj));

  NotEmptiableTerseFieldsStruct obj2;
  EXPECT_FALSE(apache::thrift::empty(obj2));
}

TEST(StructTest, EmptyTerseStruct) {
  EmptyTerseStruct obj;
  EXPECT_TRUE(apache::thrift::empty(obj));
}

TEST(StructTest, EmptiableTerseFieldsStruct) {
  EmptiableTerseFieldsStruct obj;
  EXPECT_TRUE(apache::thrift::empty(obj));

  obj.bool_field() = true;
  EXPECT_FALSE(apache::thrift::empty(obj));

  apache::thrift::clear(obj);
  EXPECT_TRUE(apache::thrift::empty(obj));
}

TEST(StructTest, OptionalFieldsStruct) {
  OptionalFieldsStruct obj;
  EXPECT_TRUE(apache::thrift::empty(obj));

  obj.boxed_field() = HasInt();
  EXPECT_FALSE(apache::thrift::empty(obj));

  apache::thrift::clear(obj);
  EXPECT_TRUE(apache::thrift::empty(obj));
}

TEST(StructTest, OptionalFieldsTerseStruct) {
  OptionalFieldsTerseStruct obj;
  EXPECT_TRUE(apache::thrift::empty(obj));

  obj.boxed_field() = HasInt();
  EXPECT_FALSE(apache::thrift::empty(obj));

  apache::thrift::clear(obj);
  EXPECT_TRUE(apache::thrift::empty(obj));
}

TEST(StructTest, RefsWithStringAndContainerTerseWrites) {
  {
    RefsWithStringAndContainerTerseWrites obj;
    EXPECT_TRUE(apache::thrift::empty(obj));
  }
  {
    RefsWithStringAndContainerTerseWrites obj;
    obj.string_list_field() = std::make_shared<std::vector<std::string>>(
        std::vector<std::string>{"1", "2", "3"});
    EXPECT_FALSE(apache::thrift::empty(obj));
    apache::thrift::clear(obj);
    EXPECT_TRUE(apache::thrift::empty(obj));
  }
  {
    RefsWithStringAndContainerTerseWrites obj;
    obj.string_field() = std::make_shared<std::string>("123");
    EXPECT_FALSE(apache::thrift::empty(obj));
    apache::thrift::clear(obj);
    EXPECT_TRUE(apache::thrift::empty(obj));
  }
}

TEST(StructTest, NotEligibleForConstexpr) {
  [[maybe_unused]] NotEligibleForConstexpr foo;
}

template <class T>
protocol::Object serializeToObject(const T& t) {
  auto buf = CompactSerializer::serialize<folly::IOBufQueue>(t).move();
  return protocol::parseObject<CompactProtocolReader>(*buf);
}

template <class Id, class T>
bool serializedField(const T& t) {
  return serializeToObject(t).contains(op::get_field_id_v<T, Id>);
}

TEST(StructTest, TerseFields) {
  TerseFieldsWithCustomDefault terse;
  // Other non-optional cpp.ref fields are initialized but exceptions aren't
  terse.cpp_shared_ref_exception_field() = std::make_shared<NestedException>();

  static_assert(apache::thrift::detail::qualifier::
                    is_deprecated_terse_writes_with_custom_default_field<
                        TerseFieldsWithCustomDefault,
                        type::field_id<4>>::value);
  // redundant custom default
  static_assert(!apache::thrift::detail::qualifier::
                    is_deprecated_terse_writes_with_custom_default_field<
                        TerseFieldsWithCustomDefault,
                        type::field_id<19>>::value);

  // Primitive types
  EXPECT_EQ(terse.bool_field(), true);
  EXPECT_EQ(terse.byte_field(), 10);
  EXPECT_EQ(terse.short_field(), 20);
  EXPECT_EQ(terse.int_field(), 30);
  EXPECT_EQ(terse.long_field(), 40);
  EXPECT_EQ(terse.float_field(), 50);
  EXPECT_EQ(terse.double_field(), 60);
  EXPECT_EQ(terse.binary_field(), "70");
  EXPECT_EQ(terse.string_field(), "80");

  // Containers
  EXPECT_EQ(terse.list_field()->size(), 1);
  EXPECT_EQ(*terse.list_field()->begin(), 90);
  EXPECT_EQ(terse.set_field()->size(), 1);
  EXPECT_EQ(*terse.set_field()->begin(), 100);
  EXPECT_EQ(terse.map_field()->size(), 1);
  EXPECT_EQ(terse.map_field()->begin()->first, 110);
  EXPECT_EQ(terse.map_field()->begin()->second, 10);

  // Structures
  EXPECT_EQ(terse.struct_field()->int_field(), 42);
  EXPECT_EQ(terse.exception_field()->int_field(), 42);
  // Custom default on union is not honored.
  EXPECT_EQ(
      terse.union_field()->getType(),
      apache::thrift::test::NestedUnion::Type::__EMPTY__);

  // cpp.ref Structures
  EXPECT_NE(terse.cpp_ref_struct_field(), nullptr);
  EXPECT_NE(terse.cpp_ref_union_field(), nullptr);
  // cpp.ref exceptions are nullptr by default
  EXPECT_EQ(terse.cpp_ref_exception_field(), nullptr);

  // Numeric fields are not serialized if they equal custom default
  EXPECT_FALSE(serializedField<ident::bool_field>(terse));
  EXPECT_FALSE(serializedField<ident::byte_field>(terse));
  EXPECT_FALSE(serializedField<ident::short_field>(terse));
  EXPECT_FALSE(serializedField<ident::int_field>(terse));
  EXPECT_FALSE(serializedField<ident::long_field>(terse));
  EXPECT_FALSE(serializedField<ident::float_field>(terse));
  EXPECT_FALSE(serializedField<ident::double_field>(terse));

  // string/binary and container fields are serialized if they are not intrinsic
  // default
  EXPECT_TRUE(serializedField<ident::string_field>(terse));
  EXPECT_TRUE(serializedField<ident::binary_field>(terse));
  EXPECT_TRUE(serializedField<ident::list_field>(terse));
  EXPECT_TRUE(serializedField<ident::set_field>(terse));
  EXPECT_TRUE(serializedField<ident::map_field>(terse));

  // Structure fields are always serialized
  EXPECT_TRUE(serializedField<ident::struct_field>(terse));
  EXPECT_TRUE(serializedField<ident::union_field>(terse));
  EXPECT_TRUE(serializedField<ident::exception_field>(terse));
  EXPECT_TRUE(serializedField<ident::cpp_ref_struct_field>(terse));
  EXPECT_TRUE(serializedField<ident::cpp_ref_union_field>(terse));
  EXPECT_TRUE(serializedField<ident::cpp_shared_ref_struct_field>(terse));
  EXPECT_TRUE(serializedField<ident::cpp_shared_ref_union_field>(terse));
  EXPECT_TRUE(serializedField<ident::cpp_shared_ref_exception_field>(terse));

  // cpp.ref exceptions are nullptr by default
  EXPECT_FALSE(serializedField<ident::cpp_ref_exception_field>(terse));
  terse.cpp_ref_exception_field() = std::make_unique<NestedException>();
  EXPECT_TRUE(serializedField<ident::cpp_ref_exception_field>(terse));

  // Change `terse` to intrinsic default
  apache::thrift::clear(terse);
  terse.cpp_ref_struct_field() = nullptr;
  terse.cpp_ref_union_field() = nullptr;
  terse.cpp_ref_exception_field() = nullptr;
  // Null non-optional cpp.ref is a bug so we have chosen not to preserve the
  // behavior. terse.cpp_shared_ref_struct_field() = nullptr;
  // terse.cpp_shared_ref_union_field() = nullptr;
  // terse.cpp_shared_ref_exception_field() = nullptr;

  // Numeric fields are serialized if they don't equal custom default
  EXPECT_TRUE(serializedField<ident::bool_field>(terse));
  EXPECT_TRUE(serializedField<ident::byte_field>(terse));
  EXPECT_TRUE(serializedField<ident::short_field>(terse));
  EXPECT_TRUE(serializedField<ident::int_field>(terse));
  EXPECT_TRUE(serializedField<ident::long_field>(terse));
  EXPECT_TRUE(serializedField<ident::float_field>(terse));
  EXPECT_TRUE(serializedField<ident::double_field>(terse));

  // string/binary and container fields are not serialized if they are intrinsic
  // default
  EXPECT_FALSE(serializedField<ident::string_field>(terse));
  EXPECT_FALSE(serializedField<ident::binary_field>(terse));
  EXPECT_FALSE(serializedField<ident::list_field>(terse));
  EXPECT_FALSE(serializedField<ident::set_field>(terse));
  EXPECT_FALSE(serializedField<ident::map_field>(terse));

  // Structure fields are always serialized
  EXPECT_TRUE(serializedField<ident::struct_field>(terse));
  EXPECT_TRUE(serializedField<ident::union_field>(terse));
  EXPECT_TRUE(serializedField<ident::exception_field>(terse));
  EXPECT_TRUE(serializedField<ident::cpp_shared_ref_struct_field>(terse));
  EXPECT_TRUE(serializedField<ident::cpp_shared_ref_union_field>(terse));
  EXPECT_TRUE(serializedField<ident::cpp_shared_ref_exception_field>(terse));

  // @cpp.Ref has special logic that it skips nullptr fields
  EXPECT_FALSE(serializedField<ident::cpp_ref_struct_field>(terse));
  EXPECT_FALSE(serializedField<ident::cpp_ref_union_field>(terse));
  EXPECT_FALSE(serializedField<ident::cpp_ref_exception_field>(terse));
}

TEST(StructTest, DataCorruptionWhenContainerFieldsHaveDefaults) {
  // Round-tripping an empty container with custom default does not preserve the
  // value!
  TerseFieldsWithCustomDefault terse;
  apache::thrift::clear(terse);
  auto serialized = CompactSerializer::serialize<std::string>(terse);
  auto read =
      CompactSerializer::deserialize<TerseFieldsWithCustomDefault>(serialized);
  EXPECT_NE(read.list_field(), terse.list_field());
  EXPECT_NE(read.set_field(), terse.set_field());
  EXPECT_NE(read.map_field(), terse.map_field());
  EXPECT_NE(read.string_field(), terse.string_field());
  EXPECT_NE(read.binary_field(), terse.binary_field());
}

TEST(StructTest, TestInitListEmplaceContainers) {
  AllContainersStruct obj;
  obj.list_field().emplace({1, 2});
  obj.set_field().emplace({1, 2});
  obj.map_field().emplace({{1, 1}, {2, 2}});
  obj.list_field_opt().emplace({1, 2});
  obj.set_field_opt().emplace({1, 2});
  obj.map_field_opt().emplace({{1, 1}, {2, 2}});
}

TEST(StructTest, has_value) {
  {
    Basic b;

    // Unqualified (i.e., "always-present") field
    // NOTE: The test below captures the current behavior, which is NOT exactly
    // desirable: we believe that `has_value()` should ALWAYS return true for
    // always-present fields (as their name indicates). but that has
    // historically not been the behavior of this API. This is why the method is
    // marked as deprecated.
    //
    // Ideally, the next line should be flipped, i.e.:
    // EXPECT_TRUE(b.def_field().has_value());
    EXPECT_FALSE(b.def_field().has_value());

    // Optional field
    EXPECT_FALSE(b.opt_field().has_value());
    b.opt_field() = 42;
    EXPECT_TRUE(b.opt_field().has_value());
    b.opt_field().reset();
    EXPECT_FALSE(b.opt_field().has_value());

    // [Deprecated] "required" field
    EXPECT_TRUE(b.req_field().has_value());
  }

  {
    // Boxed optional (primitive) field

    OptionalFieldsStruct s;
    EXPECT_FALSE(s.boxed_field().has_value());
    s.boxed_field().ensure();
    EXPECT_TRUE(s.boxed_field().has_value());
    s.boxed_field().reset();
    EXPECT_FALSE(s.boxed_field().has_value());
  }

  {
    // Boxed optional (container) field

    EmptiableOptionalFieldsStruct s;
    EXPECT_FALSE(s.int_list_field_ref().has_value());
    s.int_list_field_ref().ensure();
    EXPECT_TRUE(s.int_list_field_ref().has_value());
    s.int_list_field_ref().reset();
    EXPECT_FALSE(s.int_list_field_ref().has_value());
  }
}

} // namespace
} // namespace apache::thrift::test
