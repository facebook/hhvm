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

#include <thrift/test/reflection/gen-cpp2/reflection_types.h>

#include <type_traits>
#include <typeindex>

#include <folly/Utility.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/BadFieldAccess.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/cpp2/type/Testing.h>

namespace apache::thrift::type {
using apache::thrift::detail::st::struct_private_access;

TEST(FieldsTest, Get) {
  test_cpp2::cpp_reflection::struct3 s;
  using Struct = test_cpp2::cpp_reflection::struct3;
  EXPECT_EQ(&(*op::get<Struct, field_id<2>>(s)), &*s.fieldA());

  s.fieldA() = 10;
  EXPECT_EQ((op::get<Struct, field_id<2>>(s)), 10);
  op::get<Struct, field_id<2>>(s) = 20;
  EXPECT_EQ(*s.fieldA(), 20);
  test::
      same_tag<decltype(s.fieldA()), decltype(op::get<Struct, field_id<2>>(s))>;

  s.fieldE()->ui_ref() = 10;
  EXPECT_EQ((op::get<Struct, field_id<5>>(s)->ui_ref()), 10);
  op::get<Struct, field_id<5>>(s)->us_ref() = "20";
  EXPECT_EQ(s.fieldE()->us_ref(), "20");
  test::
      same_tag<decltype(s.fieldE()), decltype(op::get<Struct, field_id<5>>(s))>;
}

TEST(FieldsTest, field_id_by_ordinal) {
  EXPECT_EQ(op::size_v<test_cpp2::cpp_reflection::struct3>, 19);
}

TEST(UnionFieldsTest, Get) {
  test_cpp2::cpp_reflection::union1 u;
  using Union = test_cpp2::cpp_reflection::union1;

  EXPECT_THROW((*op::get<Union, field_id<1>>(u)), bad_field_access);

  u.ui_ref() = 10;
  EXPECT_EQ((op::get<Union, field_id<1>>(u)), 10);
  EXPECT_THROW((*op::get<Union, field_id<2>>(u)), bad_field_access);
  test::
      same_tag<decltype(u.ui_ref()), decltype(op::get<Union, field_id<1>>(u))>;
  EXPECT_EQ(&(*op::get<Union, field_id<1>>(u)), &*u.ui_ref());

  op::get<Union, field_id<1>>(u) = 20;
  EXPECT_EQ(u.ui_ref(), 20);
  EXPECT_EQ((op::get<Union, field_id<1>>(u)), 20);

  u.us_ref() = "foo";
  EXPECT_EQ((*op::get<Union, field_id<3>>(u)), "foo");
  test::
      same_tag<decltype(u.us_ref()), decltype(op::get<Union, field_id<3>>(u))>;
  EXPECT_THROW((*op::get<Union, field_id<1>>(u)), bad_field_access);
}

template <
    class Struct,
    class Ordinal,
    class Id,
    class Ident,
    bool is_type_tag_unique,
    class TypeTag,
    class FieldTag>
void checkField(const char* identName) {
  if constexpr (Ordinal::value != Ordinal{}) {
    test::same_tag<Id, struct_private_access::field_id<Struct, Ordinal>>;
  }
  test::same_tag<TypeTag, struct_private_access::type_tag<Struct, Ordinal>>;
  test::same_tag<Ident, struct_private_access::ident<Struct, Ordinal>>;
  test::same_tag<Ordinal, struct_private_access::ordinal<Struct, Id>>;
  test::same_tag<Ordinal, struct_private_access::ordinal<Struct, Ident>>;

  if constexpr (is_type_tag_unique) {
    test::same_tag<Ordinal, struct_private_access::ordinal<Struct, TypeTag>>;
  }

  test::same_tag<op::get_ordinal<Struct, Ordinal>, Ordinal>;
  test::same_tag<op::get_field_id<Struct, Ordinal>, Id>;
  test::same_tag<op::get_type_tag<Struct, Ordinal>, TypeTag>;
  test::same_tag<op::get_ident<Struct, Ordinal>, Ident>;
  test::same_tag<op::get_field_tag<Struct, Ordinal>, FieldTag>;
  EXPECT_EQ((op::get_name<Struct, Ordinal>), identName);

  test::same_tag<op::get_ordinal<Struct, Id>, Ordinal>;
  test::same_tag<op::get_field_id<Struct, Id>, Id>;
  test::same_tag<op::get_type_tag<Struct, Id>, TypeTag>;
  test::same_tag<op::get_ident<Struct, Id>, Ident>;
  test::same_tag<op::get_field_tag<Struct, Id>, FieldTag>;
  EXPECT_EQ((op::get_name<Struct, Id>), identName);

  if constexpr (is_type_tag_unique && !std::is_void_v<TypeTag>) {
    test::same_tag<op::get_ordinal<Struct, TypeTag>, Ordinal>;
    test::same_tag<op::get_field_id<Struct, TypeTag>, Id>;
    test::same_tag<op::get_type_tag<Struct, TypeTag>, TypeTag>;
    test::same_tag<op::get_ident<Struct, TypeTag>, Ident>;
    test::same_tag<op::get_field_tag<Struct, TypeTag>, FieldTag>;
    EXPECT_EQ((op::get_name<Struct, TypeTag>), identName);
  }

  if constexpr (!std::is_void_v<Ident>) {
    test::same_tag<op::get_ordinal<Struct, Ident>, Ordinal>;
    test::same_tag<op::get_field_id<Struct, Ident>, Id>;
    test::same_tag<op::get_type_tag<Struct, Ident>, TypeTag>;
    test::same_tag<op::get_ident<Struct, Ident>, Ident>;
    test::same_tag<op::get_field_tag<Struct, Ident>, FieldTag>;
    EXPECT_EQ((op::get_name<Struct, Ident>), identName);
  }

  if constexpr (!std::is_void_v<FieldTag>) {
    test::same_tag<op::get_ordinal<Struct, FieldTag>, Ordinal>;
    test::same_tag<op::get_field_id<Struct, FieldTag>, Id>;
    test::same_tag<op::get_type_tag<Struct, FieldTag>, TypeTag>;
    test::same_tag<op::get_ident<Struct, FieldTag>, Ident>;
    test::same_tag<op::get_field_tag<Struct, FieldTag>, FieldTag>;
    EXPECT_EQ((op::get_name<Struct, FieldTag>), identName);
  }
}

TEST(FieldsTest, UnifiedAPIs) {
  using test_cpp2::cpp_reflection::struct3;

  using TypeTag1 = i32_t;
  using TypeTag2 = string_t;
  using TypeTag3 = enum_t<::test_cpp2::cpp_reflection::enum1>;
  using TypeTag4 = enum_t<::test_cpp2::cpp_reflection::enum2>;
  using TypeTag5 = union_t<::test_cpp2::cpp_reflection::union1>;
  using TypeTag6 = union_t<::test_cpp2::cpp_reflection::union2>;
  using TypeTag7 = struct_t<::test_cpp2::cpp_reflection::struct1>;
  using TypeTag8 = union_t<::test_cpp2::cpp_reflection::union2>;
  using TypeTag9 = list<i32_t>;
  using TypeTag10 = list<string_t>;
  using TypeTag11 = cpp_type<std::deque<std::string>, list<string_t>>;
  using TypeTag12 = list<struct_t<::test_cpp2::cpp_reflection::structA>>;
  using TypeTag13 = set<i32_t>;
  using TypeTag14 = set<string_t>;
  using TypeTag15 = set<string_t>;
  using TypeTag16 = set<struct_t<::test_cpp2::cpp_reflection::structB>>;
  using TypeTag17 =
      map<string_t, struct_t<::test_cpp2::cpp_reflection::structA>>;
  using TypeTag18 =
      map<string_t, struct_t<::test_cpp2::cpp_reflection::structB>>;
  using TypeTag19 = map<binary_t, binary_t>;

  using FieldTag1 = type::field<TypeTag1, FieldContext<struct3, 2>>;
  using FieldTag2 = type::field<TypeTag2, FieldContext<struct3, 1>>;
  using FieldTag3 = type::field<TypeTag3, FieldContext<struct3, 3>>;
  using FieldTag4 = type::field<TypeTag4, FieldContext<struct3, 4>>;
  using FieldTag5 = type::field<TypeTag5, FieldContext<struct3, 5>>;
  using FieldTag6 = type::field<TypeTag6, FieldContext<struct3, 6>>;
  using FieldTag7 = type::field<TypeTag7, FieldContext<struct3, 7>>;
  using FieldTag8 = type::field<TypeTag8, FieldContext<struct3, 8>>;
  using FieldTag9 = type::field<TypeTag9, FieldContext<struct3, 9>>;
  using FieldTag10 = type::field<TypeTag10, FieldContext<struct3, 10>>;
  using FieldTag11 = type::field<TypeTag11, FieldContext<struct3, 11>>;
  using FieldTag12 = type::field<TypeTag12, FieldContext<struct3, 12>>;
  using FieldTag13 = type::field<TypeTag13, FieldContext<struct3, 13>>;
  using FieldTag14 = type::field<TypeTag14, FieldContext<struct3, 14>>;
  using FieldTag15 = type::field<TypeTag15, FieldContext<struct3, 15>>;
  using FieldTag16 = type::field<TypeTag16, FieldContext<struct3, 16>>;
  using FieldTag17 = type::field<TypeTag17, FieldContext<struct3, 17>>;
  using FieldTag18 = type::field<TypeTag18, FieldContext<struct3, 18>>;
  using FieldTag19 = type::field<TypeTag19, FieldContext<struct3, 20>>;

  // clang-format off
  checkField<struct3, field_ordinal<0>,  field_id<0>,  void,        true,  void,      void>("");
  checkField<struct3, field_ordinal<1>,  field_id<2>,  tag::fieldA, true,  TypeTag1,  FieldTag1>("fieldA");
  checkField<struct3, field_ordinal<2>,  field_id<1>,  tag::fieldB, true,  TypeTag2,  FieldTag2>("fieldB");
  checkField<struct3, field_ordinal<3>,  field_id<3>,  tag::fieldC, true,  TypeTag3,  FieldTag3>("fieldC");
  checkField<struct3, field_ordinal<4>,  field_id<4>,  tag::fieldD, true,  TypeTag4,  FieldTag4>("fieldD");
  checkField<struct3, field_ordinal<5>,  field_id<5>,  tag::fieldE, true,  TypeTag5,  FieldTag5>("fieldE");
  checkField<struct3, field_ordinal<6>,  field_id<6>,  tag::fieldF, false, TypeTag6,  FieldTag6>("fieldF");
  checkField<struct3, field_ordinal<7>,  field_id<7>,  tag::fieldG, true,  TypeTag7,  FieldTag7>("fieldG");
  checkField<struct3, field_ordinal<8>,  field_id<8>,  tag::fieldH, false, TypeTag8,  FieldTag8>("fieldH");
  checkField<struct3, field_ordinal<9>,  field_id<9>,  tag::fieldI, true,  TypeTag9,  FieldTag9>("fieldI");
  checkField<struct3, field_ordinal<10>, field_id<10>, tag::fieldJ, true,  TypeTag10, FieldTag10>("fieldJ");
  checkField<struct3, field_ordinal<11>, field_id<11>, tag::fieldK, true,  TypeTag11, FieldTag11>("fieldK");
  checkField<struct3, field_ordinal<12>, field_id<12>, tag::fieldL, true,  TypeTag12, FieldTag12>("fieldL");
  checkField<struct3, field_ordinal<13>, field_id<13>, tag::fieldM, true,  TypeTag13, FieldTag13>("fieldM");
  checkField<struct3, field_ordinal<14>, field_id<14>, tag::fieldN, false, TypeTag14, FieldTag14>("fieldN");
  checkField<struct3, field_ordinal<15>, field_id<15>, tag::fieldO, false, TypeTag15, FieldTag15>("fieldO");
  checkField<struct3, field_ordinal<16>, field_id<16>, tag::fieldP, true,  TypeTag16, FieldTag16>("fieldP");
  checkField<struct3, field_ordinal<17>, field_id<17>, tag::fieldQ, true,  TypeTag17, FieldTag17>("fieldQ");
  checkField<struct3, field_ordinal<18>, field_id<18>, tag::fieldR, true,  TypeTag18, FieldTag18>("fieldR");
  checkField<struct3, field_ordinal<19>, field_id<20>, tag::fieldS, true,  TypeTag19, FieldTag19>("fieldS");
  // clang-format off
}

struct IncompleteType;

TEST(FieldsTest, IsReflectionMetadata) {
  using namespace apache::thrift::type::detail;
  static_assert(is_type_tag_v<bool_t>);
  static_assert(is_type_tag_v<list<i32_t>>);
  static_assert(is_type_tag_v<field<list<i32_t>, FieldContext<test_cpp2::cpp_reflection::struct3, 1>>>);
  static_assert(!is_type_tag_v<bool>);
  static_assert(!is_type_tag_v<std::int32_t>);
  static_assert(!is_type_tag_v<std::vector<int32_t>>);
  static_assert(!is_type_tag_v<void>);
  static_assert(!is_type_tag_v<IncompleteType>);

  static_assert(is_field_id_v<field_id<0>>);
  static_assert(is_field_id_v<field_id<1>>);
  static_assert(!is_field_id_v<field_ordinal<1>>);
  static_assert(!is_field_id_v<void>);
  static_assert(!is_field_id_v<IncompleteType>);

  static_assert(is_ordinal_v<field_ordinal<0>>);
  static_assert(is_ordinal_v<field_ordinal<1>>);
  static_assert(!is_ordinal_v<field_id<1>>);
  static_assert(!is_ordinal_v<void>);
  static_assert(!is_ordinal_v<IncompleteType>);

  static_assert(is_ident_v<tag::fieldA>);
  static_assert(!is_ident_v<IncompleteType>);
  static_assert(!is_ident_v<int>);
  static_assert(!is_ident_v<void>);

  static_assert(is_id_v<bool_t>);
  static_assert(is_id_v<list<i32_t>>);
  static_assert(is_id_v<field<list<i32_t>, FieldContext<test_cpp2::cpp_reflection::struct3, 1>>>);
  static_assert(is_id_v<field_id<0>>);
  static_assert(is_id_v<field_id<1>>);
  static_assert(is_id_v<field_ordinal<0>>);
  static_assert(is_id_v<field_ordinal<1>>);
  static_assert(is_id_v<tag::fieldA>);
  static_assert(!is_id_v<bool>);
  static_assert(!is_id_v<std::int32_t>);
  static_assert(!is_id_v<std::vector<int32_t>>);
  static_assert(!is_id_v<IncompleteType>);
  static_assert(!is_id_v<IncompleteType>);
  static_assert(!is_id_v<int>);

  using Struct = test_cpp2::cpp_reflection::struct3;
  // TODO(ytj): We need to figure out a way to test compile error
  // op::get_field_id<Struct, int>{}; // compile error
  // op::get_ordinal<Struct, int>{}; // compile error
  // op::get_type_tag<Struct, int>{}; // compile error
  // op::get_ident<Struct, int>{}; // compile error
  // op::get_native_type<Struct, int>{}; // compile error
}

TEST(FieldsTest, NotFoundFieldInfo) {
  using Struct = test_cpp2::cpp_reflection::struct3;

  test::same_tag<op::get_ordinal<Struct, field_ordinal<0>>, field_ordinal<0>>;
  test::same_tag<op::get_field_id<Struct, field_ordinal<0>>, field_id<0>>;
  test::same_tag<op::get_type_tag<Struct, field_ordinal<0>>, void>;
  test::same_tag<op::get_ident<Struct, field_ordinal<0>>, void>;
  test::same_tag<op::get_field_tag<Struct, field_ordinal<0>>, void>;

  test::same_tag<op::get_ordinal<Struct, field_id<200>>, field_ordinal<0>>;
  test::same_tag<op::get_field_id<Struct, field_id<200>>, field_id<0>>;
  test::same_tag<op::get_type_tag<Struct, field_id<200>>, void>;
  test::same_tag<op::get_ident<Struct, field_id<200>>, void>;
  test::same_tag<op::get_field_tag<Struct, field_id<200>>, void>;

  test::same_tag<op::get_ordinal<Struct, binary_t>, field_ordinal<0>>;
  test::same_tag<op::get_field_id<Struct, binary_t>, field_id<0>>;
  test::same_tag<op::get_type_tag<Struct, binary_t>, void>;
  test::same_tag<op::get_ident<Struct, binary_t>, void>;
  test::same_tag<op::get_field_tag<Struct, binary_t>, void>;

  test::same_tag<op::get_ordinal<Struct, tag::a>, field_ordinal<0>>;
  test::same_tag<op::get_field_id<Struct, tag::a>, field_id<0>>;
  test::same_tag<op::get_type_tag<Struct, tag::a>, void>;
  test::same_tag<op::get_ident<Struct, tag::a>, void>;
  test::same_tag<op::get_field_tag<Struct, tag::a>, void>;
}


template <typename Ident>
bool isIdentTag(folly::tag_t<Ident>) {
  return is_ident_v<Ident>;
}

TEST(FieldsTest, HelperAPIs) {
  using Struct = test_cpp2::cpp_reflection::struct3;

  test::same_tag<op::get_native_type<Struct, field_ordinal<1>>, std::int32_t>;
  test::same_tag<op::get_native_type<Struct, tag::fieldA>, std::int32_t>;
  test::same_tag<op::get_native_type<Struct, field_id<11>>, std::deque<std::string>>;
  EXPECT_EQ((op::get_field_id_v<Struct, field_ordinal<1>>), FieldId{2});
  EXPECT_EQ((op::get_ordinal_v<Struct, field_id<2>>), FieldOrdinal{1});

  int count = 0;
  op::for_each_field_id<Struct>([&](auto id) {
    EXPECT_TRUE(is_field_id_v<decltype(id)>);
    count++;
  });
  EXPECT_EQ(count, op::size_v<Struct>);

  count = 0;
  op::for_each_ident<Struct>([&](auto id) {
    EXPECT_TRUE(isIdentTag(id));
    count++;
  });
  EXPECT_EQ(count, op::size_v<Struct>);

  count = 0;
  op::for_each_ordinal<Struct>([&](auto id) {
    EXPECT_TRUE(is_ordinal_v<decltype(id)>);
    count++;
  });

  EXPECT_EQ(count, op::size_v<Struct>);
}

TEST(FieldsTest, GetFieldNameCppName) {
  EXPECT_EQ((op::get_name<test_cpp2::cpp_reflection::struct_with_renamed_field, field_ordinal<1>>), "fancy.idl.name");
}
} // namespace apache::thrift::type
