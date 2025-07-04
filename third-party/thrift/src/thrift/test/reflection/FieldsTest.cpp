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

#include <thrift/test/reflection/gen-cpp2/class_name_types.h>
#include <thrift/test/reflection/gen-cpp2/reflection_types.h>

#include <type_traits>
#include <typeindex>

#include <gtest/gtest.h>
#include <folly/Utility.h>
#include <thrift/lib/cpp2/BadFieldAccess.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/cpp2/type/Testing.h>

namespace apache::thrift::type {
using apache::thrift::detail::st::private_access;

TEST(FieldsTest, Get) {
  test_cpp2::cpp_reflection::struct3 s;
  using Struct = test_cpp2::cpp_reflection::struct3;
  EXPECT_EQ(&(*op::get<field_id<2>, Struct>(s)), &*s.fieldA());

  s.fieldA() = 10;
  EXPECT_EQ((op::get<field_id<2>, Struct>(s)), 10);
  op::get<field_id<2>, Struct>(s) = 20;
  EXPECT_EQ(*s.fieldA(), 20);
  static_assert(std::is_same_v<
                decltype(s.fieldA()),
                decltype(op::get<field_id<2>, Struct>(s))>);

  s.fieldE()->ui() = 10;
  EXPECT_EQ((op::get<field_id<5>, Struct>(s)->ui()), 10);
  op::get<field_id<5>, Struct>(s)->us() = "20";
  EXPECT_EQ(s.fieldE()->us(), "20");
  static_assert(std::is_same_v<
                decltype(s.fieldE()),
                decltype(op::get<field_id<5>, Struct>(s))>);
}

TEST(FieldsTest, field_id_by_ordinal) {
  EXPECT_EQ(op::num_fields<test_cpp2::cpp_reflection::struct3>, 19);
}

TEST(UnionFieldsTest, Get) {
  test_cpp2::cpp_reflection::union1 u;
  using Union = test_cpp2::cpp_reflection::union1;

  EXPECT_THROW((*op::get<field_id<1>, Union>(u)), bad_field_access);

  u.ui() = 10;
  EXPECT_EQ((op::get<field_id<1>, Union>(u)), 10);
  EXPECT_THROW((*op::get<field_id<2>, Union>(u)), bad_field_access);
  static_assert(std::is_same_v<
                decltype(u.ui()),
                decltype(op::get<field_id<1>, Union>(u))>);
  EXPECT_EQ(&(*op::get<field_id<1>, Union>(u)), &*u.ui());

  op::get<field_id<1>, Union>(u) = 20;
  EXPECT_EQ(u.ui(), 20);
  EXPECT_EQ((op::get<field_id<1>, Union>(u)), 20);

  u.us() = "foo";
  EXPECT_EQ((*op::get<field_id<3>, Union>(u)), "foo");
  static_assert(std::is_same_v<
                decltype(u.us()),
                decltype(op::get<field_id<3>, Union>(u))>);
  EXPECT_THROW((*op::get<field_id<1>, Union>(u)), bad_field_access);
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
  static_assert(std::is_same_v<op::get_ordinal<Struct, Ordinal>, Ordinal>);
  static_assert(std::is_same_v<op::get_field_id<Struct, Ordinal>, Id>);
  static_assert(std::is_same_v<op::get_type_tag<Struct, Ordinal>, TypeTag>);
  static_assert(std::is_same_v<op::get_ident<Struct, Ordinal>, Ident>);
  static_assert(std::is_same_v<op::get_field_tag<Struct, Ordinal>, FieldTag>);
  if constexpr (Ordinal::value != FieldOrdinal{0}) {
    EXPECT_EQ((op::get_name_v<Struct, Ordinal>), identName);
  }

  static_assert(std::is_same_v<op::get_ordinal<Struct, Id>, Ordinal>);
  static_assert(std::is_same_v<op::get_field_id<Struct, Id>, Id>);
  static_assert(std::is_same_v<op::get_type_tag<Struct, Id>, TypeTag>);
  static_assert(std::is_same_v<op::get_ident<Struct, Id>, Ident>);
  static_assert(std::is_same_v<op::get_field_tag<Struct, Id>, FieldTag>);
  if constexpr (Ordinal::value != FieldOrdinal{0}) {
    EXPECT_EQ((op::get_name_v<Struct, Id>), identName);
  }

  if constexpr (is_type_tag_unique && !std::is_void_v<TypeTag>) {
    static_assert(std::is_same_v<op::get_ordinal<Struct, TypeTag>, Ordinal>);
    static_assert(std::is_same_v<op::get_field_id<Struct, TypeTag>, Id>);
    static_assert(std::is_same_v<op::get_type_tag<Struct, TypeTag>, TypeTag>);
    static_assert(std::is_same_v<op::get_ident<Struct, TypeTag>, Ident>);
    static_assert(std::is_same_v<op::get_field_tag<Struct, TypeTag>, FieldTag>);
    if constexpr (Ordinal::value != FieldOrdinal{0}) {
      EXPECT_EQ((op::get_name_v<Struct, TypeTag>), identName);
    }
  }

  if constexpr (!std::is_void_v<Ident>) {
    static_assert(std::is_same_v<op::get_ordinal<Struct, Ident>, Ordinal>);
    static_assert(std::is_same_v<op::get_field_id<Struct, Ident>, Id>);
    static_assert(std::is_same_v<op::get_type_tag<Struct, Ident>, TypeTag>);
    static_assert(std::is_same_v<op::get_ident<Struct, Ident>, Ident>);
    static_assert(std::is_same_v<op::get_field_tag<Struct, Ident>, FieldTag>);
    if constexpr (Ordinal::value != FieldOrdinal{0}) {
      EXPECT_EQ((op::get_name_v<Struct, Ident>), identName);
    }
  }

  if constexpr (!std::is_void_v<FieldTag>) {
    static_assert(std::is_same_v<op::get_ordinal<Struct, FieldTag>, Ordinal>);
    static_assert(std::is_same_v<op::get_field_id<Struct, FieldTag>, Id>);
    static_assert(std::is_same_v<op::get_type_tag<Struct, FieldTag>, TypeTag>);
    static_assert(std::is_same_v<op::get_ident<Struct, FieldTag>, Ident>);
    static_assert(
        std::is_same_v<op::get_field_tag<Struct, FieldTag>, FieldTag>);
    if constexpr (Ordinal::value != FieldOrdinal{0}) {
      EXPECT_EQ((op::get_name_v<Struct, FieldTag>), identName);
    }
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
  using TypeTag18 = cpp_type<
      std::unordered_map<std::string, ::test_cpp2::cpp_reflection::structB>,
      map<string_t, struct_t<::test_cpp2::cpp_reflection::structB>>>;
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
  checkField<struct3, field_ordinal<1>,  field_id<2>,  ident::fieldA, true,  TypeTag1,  FieldTag1>("fieldA");
  checkField<struct3, field_ordinal<2>,  field_id<1>,  ident::fieldB, true,  TypeTag2,  FieldTag2>("fieldB");
  checkField<struct3, field_ordinal<3>,  field_id<3>,  ident::fieldC, true,  TypeTag3,  FieldTag3>("fieldC");
  checkField<struct3, field_ordinal<4>,  field_id<4>,  ident::fieldD, true,  TypeTag4,  FieldTag4>("fieldD");
  checkField<struct3, field_ordinal<5>,  field_id<5>,  ident::fieldE, true,  TypeTag5,  FieldTag5>("fieldE");
  checkField<struct3, field_ordinal<6>,  field_id<6>,  ident::fieldF, false, TypeTag6,  FieldTag6>("fieldF");
  checkField<struct3, field_ordinal<7>,  field_id<7>,  ident::fieldG, true,  TypeTag7,  FieldTag7>("fieldG");
  checkField<struct3, field_ordinal<8>,  field_id<8>,  ident::fieldH, false, TypeTag8,  FieldTag8>("fieldH");
  checkField<struct3, field_ordinal<9>,  field_id<9>,  ident::fieldI, true,  TypeTag9,  FieldTag9>("fieldI");
  checkField<struct3, field_ordinal<10>, field_id<10>, ident::fieldJ, true,  TypeTag10, FieldTag10>("fieldJ");
  checkField<struct3, field_ordinal<11>, field_id<11>, ident::fieldK, true,  TypeTag11, FieldTag11>("fieldK");
  checkField<struct3, field_ordinal<12>, field_id<12>, ident::fieldL, true,  TypeTag12, FieldTag12>("fieldL");
  checkField<struct3, field_ordinal<13>, field_id<13>, ident::fieldM, true,  TypeTag13, FieldTag13>("fieldM");
  checkField<struct3, field_ordinal<14>, field_id<14>, ident::fieldN, false, TypeTag14, FieldTag14>("fieldN");
  checkField<struct3, field_ordinal<15>, field_id<15>, ident::fieldO, false, TypeTag15, FieldTag15>("fieldO");
  checkField<struct3, field_ordinal<16>, field_id<16>, ident::fieldP, true,  TypeTag16, FieldTag16>("fieldP");
  checkField<struct3, field_ordinal<17>, field_id<17>, ident::fieldQ, true,  TypeTag17, FieldTag17>("fieldQ");
  checkField<struct3, field_ordinal<18>, field_id<18>, ident::fieldR, true,  TypeTag18, FieldTag18>("fieldR");
  checkField<struct3, field_ordinal<19>, field_id<20>, ident::fieldS, true,  TypeTag19, FieldTag19>("fieldS");
  // clang-format on
}

struct IncompleteType;

TEST(FieldsTest, IsReflectionMetadata) {
  using namespace apache::thrift::type::detail;
  static_assert(is_type_tag_v<bool_t>);
  static_assert(is_type_tag_v<list<i32_t>>);
  static_assert(is_type_tag_v<field<
                    list<i32_t>,
                    FieldContext<test_cpp2::cpp_reflection::struct3, 1>>>);
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

  static_assert(is_ident_v<ident::fieldA>);
  static_assert(!is_ident_v<IncompleteType>);
  static_assert(!is_ident_v<int>);
  static_assert(!is_ident_v<void>);

  static_assert(is_id_v<bool_t>);
  static_assert(is_id_v<list<i32_t>>);
  static_assert(is_id_v<field<
                    list<i32_t>,
                    FieldContext<test_cpp2::cpp_reflection::struct3, 1>>>);
  static_assert(is_id_v<field_id<0>>);
  static_assert(is_id_v<field_id<1>>);
  static_assert(is_id_v<field_ordinal<0>>);
  static_assert(is_id_v<field_ordinal<1>>);
  static_assert(is_id_v<ident::fieldA>);
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

  static_assert(std::is_same_v<
                op::get_ordinal<Struct, field_ordinal<0>>,
                field_ordinal<0>>);
  static_assert(
      std::is_same_v<op::get_field_id<Struct, field_ordinal<0>>, field_id<0>>);
  static_assert(
      std::is_same_v<op::get_type_tag<Struct, field_ordinal<0>>, void>);
  static_assert(std::is_same_v<op::get_ident<Struct, field_ordinal<0>>, void>);
  static_assert(
      std::is_same_v<op::get_field_tag<Struct, field_ordinal<0>>, void>);

  static_assert(
      std::is_same_v<op::get_ordinal<Struct, field_id<200>>, field_ordinal<0>>);
  static_assert(
      std::is_same_v<op::get_field_id<Struct, field_id<200>>, field_id<0>>);
  static_assert(std::is_same_v<op::get_type_tag<Struct, field_id<200>>, void>);
  static_assert(std::is_same_v<op::get_ident<Struct, field_id<200>>, void>);
  static_assert(std::is_same_v<op::get_field_tag<Struct, field_id<200>>, void>);

  static_assert(
      std::is_same_v<op::get_ordinal<Struct, binary_t>, field_ordinal<0>>);
  static_assert(
      std::is_same_v<op::get_field_id<Struct, binary_t>, field_id<0>>);
  static_assert(std::is_same_v<op::get_type_tag<Struct, binary_t>, void>);
  static_assert(std::is_same_v<op::get_ident<Struct, binary_t>, void>);
  static_assert(std::is_same_v<op::get_field_tag<Struct, binary_t>, void>);

  static_assert(
      std::is_same_v<op::get_ordinal<Struct, ident::a>, field_ordinal<0>>);
  static_assert(
      std::is_same_v<op::get_field_id<Struct, ident::a>, field_id<0>>);
  static_assert(std::is_same_v<op::get_type_tag<Struct, ident::a>, void>);
  static_assert(std::is_same_v<op::get_ident<Struct, ident::a>, void>);
  static_assert(std::is_same_v<op::get_field_tag<Struct, ident::a>, void>);
}

template <typename Ident>
bool isIdentTag(folly::tag_t<Ident>) {
  return is_ident_v<Ident>;
}

TEST(FieldsTest, HelperAPIs) {
  using Struct = test_cpp2::cpp_reflection::struct3;

  static_assert(std::is_same_v<
                op::get_native_type<Struct, field_ordinal<1>>,
                std::int32_t>);
  static_assert(
      std::is_same_v<op::get_native_type<Struct, ident::fieldA>, std::int32_t>);
  static_assert(std::is_same_v<
                op::get_native_type<Struct, field_id<11>>,
                std::deque<std::string>>);
  EXPECT_EQ((op::get_field_id_v<Struct, field_ordinal<1>>), FieldId{2});
  EXPECT_EQ((op::get_ordinal_v<Struct, field_id<2>>), FieldOrdinal{1});

  int count = 0;
  op::for_each_field_id<Struct>([&](auto id) {
    EXPECT_TRUE(is_field_id_v<decltype(id)>);
    count++;
  });
  EXPECT_EQ(count, op::num_fields<Struct>);

  count = 0;
  op::for_each_ident<Struct>([&](auto id) {
    EXPECT_TRUE(isIdentTag(id));
    count++;
  });
  EXPECT_EQ(count, op::num_fields<Struct>);

  count = 0;
  op::for_each_ordinal<Struct>([&](auto id) {
    EXPECT_TRUE(is_ordinal_v<decltype(id)>);
    count++;
  });

  EXPECT_EQ(count, op::num_fields<Struct>);
}

TEST(FieldsTest, GetFieldNameCppName) {
  EXPECT_EQ(
      (op::get_name_v<
          test_cpp2::cpp_reflection::struct_with_renamed_field,
          field_ordinal<1>>),
      "fancy.idl.name");
}

TEST(FieldsTest, GetClassName) {
  EXPECT_EQ(op::get_class_name_v<test::MyStruct>, "MyStruct");
  EXPECT_EQ(op::get_class_name_v<test::MyUnion>, "MyUnion");
  EXPECT_EQ(op::get_class_name_v<test::MyException>, "MyException");
  EXPECT_EQ(op::get_class_name_v<test::RenamedMyStruct>, "MyStruct2");
  EXPECT_EQ(op::get_class_name_v<test::RenamedMyUnion>, "MyUnion2");
  EXPECT_EQ(op::get_class_name_v<test::RenamedMyException>, "MyException2");
}

TEST(FieldsTest, InvokeByFieldId) {
  using namespace test_cpp2::cpp_reflection;
  constexpr auto toValue = [](auto id) { return id.value; };
  constexpr auto notFound = [] { return FieldId{0}; };
  static_assert(
      op::invoke_by_field_id<struct3>(FieldId{18}, toValue, notFound) ==
      FieldId{18});
  static_assert(
      op::invoke_by_field_id<struct3>(FieldId{19}, toValue, notFound) ==
      FieldId{0});
  static_assert(
      op::invoke_by_field_id<struct3>(FieldId{20}, toValue, notFound) ==
      FieldId{20});
  static_assert(
      op::invoke_by_field_id<struct3>(FieldId{21}, toValue, notFound) ==
      FieldId{0});
  static_assert(
      op::invoke_by_field_id<struct3>(FieldId{0}, toValue, notFound) ==
      FieldId{0});

  for (std::int16_t id = 0; id <= 21; id++) {
    if (std::unordered_set{0, 19, 21}.contains(id)) {
      EXPECT_EQ(
          op::invoke_by_field_id<struct3>(FieldId{id}, toValue, notFound),
          FieldId{0});
    } else {
      EXPECT_EQ(
          op::invoke_by_field_id<struct3>(FieldId{id}, toValue, notFound),
          FieldId{id});
    }
  }

  for (std::int16_t id = 0; id <= 10; id++) {
    if (std::unordered_set{1, 2, 3, 6}.contains(id)) {
      EXPECT_EQ(
          op::invoke_by_field_id<struct4>(FieldId{id}, toValue, notFound),
          FieldId{id});
    } else {
      EXPECT_EQ(
          op::invoke_by_field_id<struct4>(FieldId{id}, toValue, notFound),
          FieldId{0});
    }
  }
}

TEST(FieldsTest, GetFieldRefType) {
  using test_cpp2::cpp_reflection::enum1;
  using test_cpp2::cpp_reflection::struct4;
  using test_cpp2::cpp_reflection::structA;
  static_assert(std::is_same_v<
                op::get_field_ref<struct4, ident::field0>,
                required_field_ref<std::int32_t&>>);
  static_assert(std::is_same_v<
                op::get_field_ref<struct4, ident::field1>,
                optional_field_ref<std::string&>>);
  static_assert(std::is_same_v<
                op::get_field_ref<struct4, ident::field2>,
                field_ref<enum1&>>);
  static_assert(std::is_same_v<
                op::get_field_ref<struct4, ident::field3>,
                std::unique_ptr<structA>>);
}
} // namespace apache::thrift::type
