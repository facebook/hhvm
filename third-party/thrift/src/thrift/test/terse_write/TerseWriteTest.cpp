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

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>
#include <folly/Traits.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/JSONProtocol.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/protocol/SimpleJSONProtocol.h>
#include <thrift/lib/cpp2/protocol/TableBasedSerializerImpl.h>
#include <thrift/test/terse_write/gen-cpp2/deprecated_terse_write_types.h>
#include <thrift/test/terse_write/gen-cpp2/deprecated_terse_write_types_custom_protocol.h>
#include <thrift/test/terse_write/gen-cpp2/tablebased_terse_write_types.h>
#include <thrift/test/terse_write/gen-cpp2/tablebased_terse_write_types_custom_protocol.h>
#include <thrift/test/terse_write/gen-cpp2/terse_write_types.h>

namespace apache::thrift::test {

template <class T>
struct TerseWriteTests : ::testing::Test {};

template <typename T>
void assign_fields(T& obj) {
  using enum_type = folly::remove_cvref_t<decltype(*obj.enum_field())>;

  obj.bool_field() = true;
  obj.byte_field() = 1;
  obj.short_field() = 2;
  obj.int_field() = 3;
  obj.long_field() = 4;
  obj.float_field() = 5;
  obj.double_field() = 6;
  obj.string_field() = "7";
  obj.binary_field() = "8";
  obj.enum_field() = enum_type::ME1;
  obj.list_field() = {1};
  obj.set_field() = {1};
  obj.map_field() = std::map<int32_t, int32_t>{{1, 1}};
  obj.struct_field()->field1() = 1;
  obj.union_field()->struct_field_ref().emplace().field1() = 1;
}

template <typename T>
void assign_fields_with_intrinsic_default(T& obj) {
  using enum_type = folly::remove_cvref_t<decltype(*obj.enum_field())>;

  obj.bool_field() = false;
  obj.byte_field() = 0;
  obj.short_field() = 0;
  obj.int_field() = 0;
  obj.long_field() = 0;
  obj.float_field() = 0.0;
  obj.double_field() = 0.0;
  obj.string_field() = "";
  obj.binary_field() = "";
  obj.enum_field() = enum_type::ME0;
  obj.list_field()->clear();
  obj.set_field()->clear();
  obj.map_field()->clear();
  obj.struct_field()->field1() = 0;
  apache::thrift::clear(*obj.union_field());
}

template <typename T>
void expect_assigned(const T& obj) {
  using enum_type = folly::remove_cvref_t<decltype(*obj.enum_field())>;

  EXPECT_EQ(obj.bool_field(), true);
  EXPECT_EQ(obj.byte_field(), 1);
  EXPECT_EQ(obj.short_field(), 2);
  EXPECT_EQ(obj.int_field(), 3);
  EXPECT_EQ(obj.long_field(), 4);
  EXPECT_EQ(obj.float_field(), 5);
  EXPECT_EQ(obj.double_field(), 6);
  EXPECT_EQ(obj.string_field(), "7");
  EXPECT_EQ(obj.binary_field(), "8");
  EXPECT_EQ(obj.enum_field(), enum_type::ME1);
  EXPECT_FALSE(obj.list_field()->empty());
  EXPECT_FALSE(obj.set_field()->empty());
  EXPECT_FALSE(obj.map_field()->empty());
  EXPECT_EQ(obj.struct_field()->field1(), 1);
  EXPECT_EQ(obj.union_field()->struct_field_ref()->field1(), 1);
}

template <typename T>
void expect_intrinsic_default(const T& obj) {
  using enum_type = folly::remove_cvref_t<decltype(*obj.enum_field())>;

  EXPECT_EQ(obj.bool_field(), false);
  EXPECT_EQ(obj.byte_field(), 0);
  EXPECT_EQ(obj.short_field(), 0);
  EXPECT_EQ(obj.int_field(), 0);
  EXPECT_EQ(obj.long_field(), 0);
  EXPECT_EQ(obj.float_field(), 0);
  EXPECT_EQ(obj.double_field(), 0);
  EXPECT_EQ(obj.string_field(), "");
  EXPECT_EQ(obj.binary_field(), "");
  EXPECT_EQ(obj.enum_field(), enum_type::ME0);
  EXPECT_TRUE(obj.list_field()->empty());
  EXPECT_TRUE(obj.set_field()->empty());
  EXPECT_TRUE(obj.map_field()->empty());
  EXPECT_TRUE(apache::thrift::empty(*obj.struct_field()));
  EXPECT_TRUE(apache::thrift::empty(*obj.union_field()));
}

TYPED_TEST_CASE_P(TerseWriteTests);
TYPED_TEST_P(TerseWriteTests, assign) {
  TypeParam obj;
  assign_fields(obj);
  expect_assigned(obj);
  apache::thrift::clear(obj);
  expect_intrinsic_default(obj);
}

template <typename ThriftStruct, typename Serializer>
void create_serialize_and_deserialize_test() {
  ThriftStruct obj;
  assign_fields(obj);

  auto objs = Serializer::template serialize<std::string>(obj);
  ThriftStruct objd;
  Serializer::deserialize(objs, objd);

  expect_assigned(objd);
}

TYPED_TEST_P(TerseWriteTests, serialize_and_deserialize) {
  create_serialize_and_deserialize_test<TypeParam, BinarySerializer>();
  create_serialize_and_deserialize_test<TypeParam, CompactSerializer>();
  create_serialize_and_deserialize_test<TypeParam, JSONSerializer>();
  create_serialize_and_deserialize_test<TypeParam, SimpleJSONSerializer>();
}

template <typename ThriftStruct, typename Serializer>
void create_serialize_empty_test() {
  ThriftStruct obj;
  terse_write::EmptyStruct empty;
  assign_fields(obj);

  auto emptys = Serializer::template serialize<std::string>(empty);
  auto objs = Serializer::template serialize<std::string>(obj);

  EXPECT_NE(emptys, objs);

  assign_fields_with_intrinsic_default(obj);

  objs = Serializer::template serialize<std::string>(obj);

  // A terse-write field will skip serialization that if it is equal to the
  // intrinsic default, so since all fields in obj are set to the intrinsic
  // default the serialization should equal to the empty.
  EXPECT_EQ(emptys, objs);
}

TYPED_TEST_P(TerseWriteTests, serialize_empty) {
  create_serialize_empty_test<TypeParam, BinarySerializer>();
  create_serialize_empty_test<TypeParam, CompactSerializer>();
  create_serialize_empty_test<TypeParam, JSONSerializer>();
  create_serialize_empty_test<TypeParam, SimpleJSONSerializer>();
}

TYPED_TEST_P(TerseWriteTests, empty) {
  TypeParam obj;
  EXPECT_TRUE(apache::thrift::empty(obj));
  assign_fields(obj);
  EXPECT_FALSE(apache::thrift::empty(obj));
  assign_fields_with_intrinsic_default(obj);
  EXPECT_TRUE(apache::thrift::empty(obj));
}

REGISTER_TYPED_TEST_CASE_P(
    TerseWriteTests, assign, serialize_and_deserialize, serialize_empty, empty);

using TerseWriteStructs = ::testing::Types<
    terse_write::FieldLevelTerseStruct,
    terse_write::StructLevelTerseStruct,
    deprecated_terse_write::FieldLevelTerseStruct,
    deprecated_terse_write::StructLevelTerseStruct,
    tablebased_terse_write::FieldLevelTerseStruct,
    tablebased_terse_write::StructLevelTerseStruct>;
INSTANTIATE_TYPED_TEST_CASE_P(
    TerseWriteTest, TerseWriteTests, TerseWriteStructs);

template <class T>
struct TerseWriteSerializerTests : ::testing::Test {};
TYPED_TEST_CASE_P(TerseWriteSerializerTests);

template <typename T, typename Serializer>
void test_mixed_fields_struct() {
  T obj;

  obj.terse_int_field() = 1;
  obj.def_int_field() = 2;
  obj.opt_int_field() = 3;

  auto objs = Serializer::template serialize<std::string>(obj);
  T objd;
  Serializer::template deserialize(objs, objd);

  EXPECT_EQ(obj, objd);
}

template <typename T, typename Serializer>
void test_nested_mixed_struct() {
  T obj;

  apache::thrift::clear(obj);

  auto objs = Serializer::template serialize<std::string>(obj);
  T objd;

  EXPECT_EQ(objd.mixed_field()->terse_int_field(), 0);
  EXPECT_EQ(objd.mixed_field()->def_int_field(), 0);
  EXPECT_EQ(objd.mixed_field()->opt_int_field().value_unchecked(), 0);

  Serializer::template deserialize(objs, objd);

  EXPECT_EQ(objd.mixed_field()->terse_int_field(), 0);
  EXPECT_EQ(objd.mixed_field()->def_int_field(), 0);
  EXPECT_EQ(objd.mixed_field()->opt_int_field().value_unchecked(), 0);
}

TYPED_TEST_P(TerseWriteSerializerTests, MixedFieldsStruct) {
  test_mixed_fields_struct<terse_write::MixedFieldsStruct, TypeParam>();
  test_mixed_fields_struct<
      tablebased_terse_write::MixedFieldsStruct,
      TypeParam>();
}

TYPED_TEST_P(TerseWriteSerializerTests, NestedMixedStruct) {
  test_nested_mixed_struct<terse_write::NestedMixedStruct, TypeParam>();
  test_nested_mixed_struct<
      tablebased_terse_write::NestedMixedStruct,
      TypeParam>();
}

TYPED_TEST_P(TerseWriteSerializerTests, CppRefTerseStruct) {
  terse_write::CppRefTerseStruct obj;

  EXPECT_TRUE(apache::thrift::empty(obj));

  obj.unique_int_field() = std::make_unique<int32_t>(1);
  obj.shared_int_field() = std::make_shared<int32_t>(2);
  obj.shared_const_int_field() = std::make_shared<int32_t>(3);
  obj.intern_boxed_field()->field1() = 4;

  auto objs = TypeParam::template serialize<std::string>(obj);
  terse_write::CppRefTerseStruct objd;
  TypeParam::template deserialize(objs, objd);

  EXPECT_EQ(obj, objd);
}

TYPED_TEST_P(TerseWriteSerializerTests, CppRefTerseStruct_Empty) {
  terse_write::CppRefTerseStruct obj;
  terse_write::EmptyStruct empty;

  EXPECT_TRUE(apache::thrift::empty(obj));

  obj.unique_int_field() = std::make_unique<int32_t>(1);
  obj.shared_int_field() = std::make_unique<int32_t>(2);
  obj.shared_const_int_field() = std::make_unique<int32_t>(3);
  obj.intern_boxed_field()->field1() = 4;

  EXPECT_FALSE(apache::thrift::empty(obj));

  obj.unique_int_field() = std::make_unique<int32_t>(0);
  obj.shared_int_field() = std::make_unique<int32_t>(0);
  obj.shared_const_int_field() = std::make_unique<int32_t>(0);
  obj.intern_boxed_field()->field1() = 0;

  EXPECT_TRUE(apache::thrift::empty(obj));

  obj.intern_boxed_field().reset();

  EXPECT_TRUE(apache::thrift::empty(obj));

  auto objs = TypeParam::template serialize<std::string>(obj);
  auto emptys = TypeParam::template serialize<std::string>(empty);

  EXPECT_EQ(objs, emptys);
}

TYPED_TEST_P(TerseWriteSerializerTests, CustomStringFields) {
  tablebased_terse_write::CustomStringFields obj;
  terse_write::EmptyStruct empty;

  obj.iobuf_field() = folly::IOBuf::wrapBufferAsValue("3", 1);
  obj.iobuf_ptr_field() = folly::IOBuf::copyBuffer("4");

  auto emptys = TypeParam::template serialize<std::string>(empty);
  auto objs = TypeParam::template serialize<std::string>(obj);

  EXPECT_NE(emptys, objs);

  obj.iobuf_field()->clear();
  (*obj.iobuf_ptr_field())->clear();

  objs = TypeParam::template serialize<std::string>(obj);

  EXPECT_EQ(emptys, objs);
}

TYPED_TEST_P(TerseWriteSerializerTests, CustomStringFieldsDeserialization) {
  terse_write::EmptyStruct empty;

  auto emptys = TypeParam::template serialize<std::string>(empty);

  tablebased_terse_write::CustomStringFields objd;
  TypeParam::template deserialize(emptys, objd);

  EXPECT_TRUE(objd.iobuf_field()->empty());
  EXPECT_FALSE((*objd.iobuf_ptr_field()));
}

TYPED_TEST_P(TerseWriteSerializerTests, EmptiableStructField) {
  tablebased_terse_write::EmptiableStructField obj;
  terse_write::EmptyStruct empty;

  obj.emptiable_struct()->opt_int_field() = 1;
  obj.emptiable_struct()->terse_int_field() = 2;

  auto emptys = TypeParam::template serialize<std::string>(empty);
  auto objs = TypeParam::template serialize<std::string>(obj);

  EXPECT_NE(emptys, objs);

  obj.emptiable_struct()->opt_int_field().reset();
  obj.emptiable_struct()->terse_int_field() = 0;

  objs = TypeParam::template serialize<std::string>(obj);

  // The terse `emptiable_struct` field does not get serialized, since the
  // optional `opt_int_field` field is not explicitly set, and the terse
  // `terse_int_field` field is equal to the intrinsic default.
  EXPECT_EQ(emptys, objs);
}

template <typename Prot, typename T, typename T1, typename T2, typename T3>
void terse_struct_optimization_test() {
  SCOPED_TRACE(folly::pretty_name<T>());
  terse_write::EmptyStruct empty;
  T obj;
  // 'T1' has 'field1'.
  T1 obj1;
  // 'T2' has 'field2'.
  T2 obj2;
  // 'T3' has 'field3'.
  T3 obj3;

  EXPECT_EQ(
      Prot::template serialize<std::string>(obj),
      Prot::template serialize<std::string>(empty));

  obj.field1()->field1() = 1;
  obj1.field1()->field1() = 1;

  EXPECT_EQ(
      Prot::template serialize<std::string>(obj),
      Prot::template serialize<std::string>(obj1));

  apache::thrift::clear(obj);

  obj.field2()->field1() = 1;
  obj2.field2()->field1() = 1;

  EXPECT_EQ(
      Prot::template serialize<std::string>(obj),
      Prot::template serialize<std::string>(obj2));

  apache::thrift::clear(obj);

  obj.field3()->field1() = 1;
  obj3.field3()->field1() = 1;

  EXPECT_EQ(
      Prot::template serialize<std::string>(obj),
      Prot::template serialize<std::string>(obj3));
}

TYPED_TEST_P(TerseWriteSerializerTests, TerseStructs) {
  terse_struct_optimization_test<
      TypeParam,
      terse_write::TerseStructs,
      terse_write::TerseStructs1,
      terse_write::TerseStructs2,
      terse_write::TerseStructs3>();
  terse_struct_optimization_test<
      TypeParam,
      tablebased_terse_write::TerseStructs,
      tablebased_terse_write::TerseStructs1,
      tablebased_terse_write::TerseStructs2,
      tablebased_terse_write::TerseStructs3>();
}

TYPED_TEST_P(TerseWriteSerializerTests, AdaptedFields) {
  terse_write::AdaptedFields obj;
  terse_write::EmptyStruct empty;

  obj.field1() = Wrapper<int32_t>{1};
  obj.field2() = Wrapper<int32_t>{2};
  obj.field3() = AdaptedWithContext<int32_t, terse_write::AdaptedFields, 3>{3};
  obj.field4() = Wrapper<Wrapper<int32_t>>{{4}};
  obj.field5() =
      AdaptedWithContext<Wrapper<int32_t>, terse_write::AdaptedFields, 5>{
          Wrapper<int32_t>{5}};

  auto emptys = TypeParam::template serialize<std::string>(empty);
  auto objs = TypeParam::template serialize<std::string>(obj);
  EXPECT_NE(emptys, objs);

  obj.field1() = Wrapper<int32_t>{0};
  obj.field2() = Wrapper<int32_t>{0};
  obj.field3() = AdaptedWithContext<int32_t, terse_write::AdaptedFields, 3>{0};
  obj.field4() = Wrapper<Wrapper<int32_t>>{{0}};
  obj.field5() =
      AdaptedWithContext<Wrapper<int32_t>, terse_write::AdaptedFields, 5>{
          Wrapper<int32_t>{0}};

  objs = TypeParam::template serialize<std::string>(obj);
  terse_write::AdaptedFields objd;
  TypeParam::template deserialize(objs, objd);

  EXPECT_EQ(obj, objd);
  EXPECT_EQ(emptys, objs);
}

TYPED_TEST_P(TerseWriteSerializerTests, AdaptedStringFields) {
  terse_write::AdaptedStringFields obj;
  terse_write::EmptyStruct empty;

  obj.field1() = Wrapper<std::string>{"1"};
  obj.field2() = Wrapper<std::string>{"2"};
  obj.field3() =
      AdaptedWithContext<std::string, terse_write::AdaptedStringFields, 3>{"3"};
  obj.field4() = Wrapper<Wrapper<std::string>>{{"4"}};
  obj.field5() = AdaptedWithContext<
      Wrapper<std::string>,
      terse_write::AdaptedStringFields,
      5>{Wrapper<std::string>{{"5"}}};

  auto emptys = TypeParam::template serialize<std::string>(empty);
  auto objs = TypeParam::template serialize<std::string>(obj);
  EXPECT_NE(emptys, objs);

  obj.field1() = Wrapper<std::string>{""};
  obj.field2() = Wrapper<std::string>{""};
  obj.field3() =
      AdaptedWithContext<std::string, terse_write::AdaptedStringFields, 3>{""};
  obj.field4() = Wrapper<Wrapper<std::string>>{{""}};
  obj.field5() = AdaptedWithContext<
      Wrapper<std::string>,
      terse_write::AdaptedStringFields,
      5>{Wrapper<std::string>{{""}}};

  objs = TypeParam::template serialize<std::string>(obj);
  terse_write::AdaptedStringFields objd;
  TypeParam::template deserialize(objs, objd);

  EXPECT_EQ(obj, objd);
  EXPECT_EQ(emptys, objs);
}

TYPED_TEST_P(TerseWriteSerializerTests, AdaptedListFields) {
  terse_write::AdaptedListFields obj;
  terse_write::EmptyStruct empty;

  obj.field1() = Wrapper<std::vector<int32_t>>{{1}};
  obj.field2() = Wrapper<std::vector<int32_t>>{{2}};
  obj.field3() = AdaptedWithContext<
      std::vector<int32_t>,
      terse_write::AdaptedListFields,
      3>{{3}};
  obj.field4() = Wrapper<Wrapper<std::vector<int32_t>>>{{{4}}};
  obj.field5() = AdaptedWithContext<
      Wrapper<std::vector<int32_t>>,
      terse_write::AdaptedListFields,
      5>{Wrapper<std::vector<int32_t>>{{std::vector<int32_t>{5}}}};

  auto emptys = TypeParam::template serialize<std::string>(empty);
  auto objs = TypeParam::template serialize<std::string>(obj);
  EXPECT_NE(emptys, objs);

  obj.field1() = Wrapper<std::vector<int32_t>>{{}};
  obj.field2() = Wrapper<std::vector<int32_t>>{{}};
  obj.field3() = AdaptedWithContext<
      std::vector<int32_t>,
      terse_write::AdaptedListFields,
      3>{std::vector<int32_t>{}};
  obj.field4() = Wrapper<Wrapper<std::vector<int32_t>>>{{{}}};
  obj.field5() = AdaptedWithContext<
      Wrapper<std::vector<int32_t>>,
      terse_write::AdaptedListFields,
      5>{Wrapper<std::vector<int32_t>>{{std::vector<int32_t>{}}}};

  objs = TypeParam::template serialize<std::string>(obj);
  terse_write::AdaptedListFields objd;
  TypeParam::template deserialize(objs, objd);

  EXPECT_EQ(obj, objd);
  EXPECT_EQ(emptys, objs);
}

TYPED_TEST_P(TerseWriteSerializerTests, TerseException) {
  terse_write::TerseException obj;
  terse_write::EmptyStruct empty;

  obj.msg() = "message";

  auto emptys = TypeParam::template serialize<std::string>(empty);
  auto objs = TypeParam::template serialize<std::string>(obj);
  EXPECT_NE(emptys, objs);

  obj.msg() = "";

  objs = TypeParam::template serialize<std::string>(obj);
  terse_write::TerseException objd;
  TypeParam::template deserialize(objs, objd);

  EXPECT_EQ(obj, objd);
  EXPECT_EQ(emptys, objs);
}

REGISTER_TYPED_TEST_CASE_P(
    TerseWriteSerializerTests,
    AdaptedFields,
    AdaptedStringFields,
    AdaptedListFields,
    MixedFieldsStruct,
    NestedMixedStruct,
    CppRefTerseStruct,
    CppRefTerseStruct_Empty,
    CustomStringFields,
    CustomStringFieldsDeserialization,
    EmptiableStructField,
    TerseException,
    TerseStructs);

using Serializers = ::testing::Types<
    BinarySerializer,
    CompactSerializer,
    JSONSerializer,
    SimpleJSONSerializer>;
INSTANTIATE_TYPED_TEST_CASE_P(
    TerseWriteTest, TerseWriteSerializerTests, Serializers);

// Test CompactProtocol's bookkeeping of previously serialized field id for
// multiple-level nested fields.
TEST(TerseWriteTest, CompactProtocolFieldIds) {
  terse_write::ThreeLevelTerseStructs obj;

  obj.field1()->field1()->field1() = 1;
  obj.field3()->field2()->field1() = 2;
  obj.field5()->field3()->field1() = 3;
  obj.field6()->field1()->field1() = 4;
  obj.field8()->field2()->field1() = 5;
  obj.field10()->field3()->field1() = 6;

  std::string objs = CompactSerializer::serialize<std::string>(obj);
  terse_write::ThreeLevelTerseStructs objd;
  CompactSerializer::deserialize(objs, objd);

  EXPECT_EQ(*objd.field1()->field1()->field1(), 1);
  EXPECT_TRUE(apache::thrift::empty(*objd.field1()->field2()));
  EXPECT_TRUE(apache::thrift::empty(*objd.field1()->field3()));

  EXPECT_TRUE(apache::thrift::empty(*objd.field2()));

  EXPECT_TRUE(apache::thrift::empty(*objd.field3()->field1()));
  EXPECT_EQ(*objd.field3()->field2()->field1(), 2);
  EXPECT_TRUE(apache::thrift::empty(*objd.field3()->field3()));

  EXPECT_TRUE(apache::thrift::empty(*objd.field4()));

  EXPECT_TRUE(apache::thrift::empty(*objd.field5()->field1()));
  EXPECT_TRUE(apache::thrift::empty(*objd.field5()->field2()));
  EXPECT_EQ(*objd.field5()->field3()->field1(), 3);

  EXPECT_EQ(*objd.field6()->field1()->field1(), 4);
  EXPECT_TRUE(apache::thrift::empty(*objd.field6()->field2()));
  EXPECT_TRUE(apache::thrift::empty(*objd.field6()->field3()));

  EXPECT_TRUE(apache::thrift::empty(*objd.field7()));

  EXPECT_TRUE(apache::thrift::empty(*objd.field8()->field1()));
  EXPECT_EQ(*objd.field8()->field2()->field1(), 5);
  EXPECT_TRUE(apache::thrift::empty(*objd.field8()->field3()));

  EXPECT_TRUE(apache::thrift::empty(*objd.field9()));

  EXPECT_TRUE(apache::thrift::empty(*objd.field10()->field1()));
  EXPECT_TRUE(apache::thrift::empty(*objd.field10()->field2()));
  EXPECT_EQ(*objd.field10()->field3()->field1(), 6);
}
} // namespace apache::thrift::test
