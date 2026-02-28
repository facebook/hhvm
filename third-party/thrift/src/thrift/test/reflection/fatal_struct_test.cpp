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

#include <thrift/lib/cpp2/TypeClass.h>
#include <thrift/test/AdapterTest.h>
#include <thrift/test/reflection/gen-cpp2/reflection_fatal_types.h>

#include <thrift/lib/cpp2/reflection/internal/test_helpers.h>

#include <gtest/gtest.h>

namespace ident = apache::thrift::ident;

FATAL_S(struct1s, "struct1");
FATAL_S(field0s, "field0");
FATAL_S(field1s, "field1");
FATAL_S(field2s, "field2");
FATAL_S(field3s, "field3");
FATAL_S(field4s, "field4");
FATAL_S(field5s, "field5");
FATAL_S(fieldAs, "fieldA");
FATAL_S(fieldBs, "fieldB");
FATAL_S(fieldCs, "fieldC");
FATAL_S(fieldDs, "fieldD");
FATAL_S(fieldEs, "fieldE");
FATAL_S(fieldFs, "fieldF");
FATAL_S(fieldGs, "fieldG");

template <apache::thrift::field_id_t Id>
using field_id = std::integral_constant<apache::thrift::field_id_t, Id>;

template <apache::thrift::optionality Optionality>
using required =
    std::integral_constant<apache::thrift::optionality, Optionality>;

namespace test_cpp2::cpp_reflection {

using apache::thrift::detail::st::IsThriftClass;
using apache::thrift::detail::st::IsThriftUnion;

static_assert(IsThriftClass<union1>::value);
static_assert(IsThriftClass<union2>::value);
static_assert(IsThriftClass<union3>::value);
static_assert(IsThriftClass<struct1>::value);
static_assert(IsThriftClass<struct2>::value);
static_assert(IsThriftClass<struct3>::value);
static_assert(!IsThriftClass<enum1>::value);
static_assert(!IsThriftClass<enum2>::value);
static_assert(!IsThriftClass<enum3>::value);
static_assert(IsThriftUnion<union1>::value);
static_assert(IsThriftUnion<union2>::value);
static_assert(IsThriftUnion<union3>::value);
static_assert(!IsThriftUnion<struct1>::value);
static_assert(!IsThriftUnion<struct2>::value);
static_assert(!IsThriftUnion<struct3>::value);
static_assert(!IsThriftUnion<enum1>::value);
static_assert(!IsThriftUnion<enum2>::value);
static_assert(!IsThriftUnion<enum3>::value);

TEST(FatalStruct, Struct1SanityCheck) {
  using traits = apache::thrift::reflect_struct<struct1>;

  EXPECT_SAME<struct1, traits::type>();
  EXPECT_EQ("struct1", traits::getName());

  EXPECT_SAME<traits, apache::thrift::try_reflect_struct<struct1, void>>();
  EXPECT_SAME<void, apache::thrift::try_reflect_struct<int, void>>();

  EXPECT_EQ("field0", traits::member::field0::getName());
  EXPECT_EQ("field1", traits::member::field1::getName());
  EXPECT_EQ("field2", traits::member::field2::getName());
  EXPECT_EQ("field3", traits::member::field3::getName());
  EXPECT_EQ("field4", traits::member::field4::getName());
  EXPECT_EQ("field5", traits::member::field5::getName());

  struct1 pod;
  *pod.field0() = 19;
  pod.field1() = "hello";
  *pod.field2() = enum1::field2;
  *pod.field3() = enum2::field1_2;
  pod.field4() = {};
  pod.field4()->set_ud(5.6);
  pod.field5()->set_us_2("world");

  EXPECT_EQ(*pod.field0(), traits::member::field0::getter{}(pod));
  EXPECT_EQ(*pod.field1(), traits::member::field1::getter{}(pod));
  EXPECT_EQ(*pod.field2(), traits::member::field2::getter{}(pod));
  EXPECT_EQ(*pod.field3(), traits::member::field3::getter{}(pod));
  EXPECT_EQ(*pod.field4(), traits::member::field4::getter{}(pod));
  EXPECT_EQ(*pod.field5(), traits::member::field5::getter{}(pod));

  traits::member::field0::getter{}(pod) = 98;
  EXPECT_EQ(98, *pod.field0());

  traits::member::field1::getter{}(pod) = "test";
  EXPECT_EQ("test", *pod.field1());

  traits::member::field2::getter{}(pod) = enum1::field0;
  EXPECT_EQ(enum1::field0, *pod.field2());

  traits::member::field3::getter{}(pod) = enum2::field2_2;
  EXPECT_EQ(enum2::field2_2, *pod.field3());

  traits::member::field4::getter{}(pod).set_ui(56);
  EXPECT_EQ(union1::Type::ui, pod.field4()->getType());
  EXPECT_EQ(56, pod.field4()->get_ui());

  traits::member::field5::getter{}(pod).set_ue_2(enum1::field1);
  EXPECT_EQ(union2::Type::ue_2, pod.field5()->getType());
  EXPECT_EQ(enum1::field1, pod.field5()->get_ue_2());

  EXPECT_SAME<
      std::int32_t,
      fatal::get<traits::members, ident::field0, fatal::get_type::tag>::type>();
  EXPECT_SAME<
      std::string,
      fatal::get<traits::members, ident::field1, fatal::get_type::tag>::type>();
  EXPECT_SAME<
      enum1,
      fatal::get<traits::members, ident::field2, fatal::get_type::tag>::type>();
  EXPECT_SAME<
      enum2,
      fatal::get<traits::members, ident::field3, fatal::get_type::tag>::type>();
  EXPECT_SAME<
      union1,
      fatal::get<traits::members, ident::field4, fatal::get_type::tag>::type>();
  EXPECT_SAME<
      union2,
      fatal::get<traits::members, ident::field5, fatal::get_type::tag>::type>();

  EXPECT_SAME<
      field_id<1>,
      fatal::get<traits::members, ident::field0, fatal::get_type::tag>::id>();
  EXPECT_SAME<
      field_id<2>,
      fatal::get<traits::members, ident::field1, fatal::get_type::tag>::id>();
  EXPECT_SAME<
      field_id<4>,
      fatal::get<traits::members, ident::field2, fatal::get_type::tag>::id>();
  EXPECT_SAME<
      field_id<8>,
      fatal::get<traits::members, ident::field3, fatal::get_type::tag>::id>();
  EXPECT_SAME<
      field_id<16>,
      fatal::get<traits::members, ident::field4, fatal::get_type::tag>::id>();
  EXPECT_SAME<
      field_id<32>,
      fatal::get<traits::members, ident::field5, fatal::get_type::tag>::id>();

  EXPECT_SAME<
      required<apache::thrift::optionality::required>,
      fatal::get<traits::members, ident::field0, fatal::get_type::tag>::
          optional>();
  EXPECT_SAME<
      required<apache::thrift::optionality::optional>,
      fatal::get<traits::members, ident::field1, fatal::get_type::tag>::
          optional>();
  EXPECT_SAME<
      required<apache::thrift::optionality::required_of_writer>,
      fatal::get<traits::members, ident::field2, fatal::get_type::tag>::
          optional>();
  EXPECT_SAME<
      required<apache::thrift::optionality::required>,
      fatal::get<traits::members, ident::field3, fatal::get_type::tag>::
          optional>();
  EXPECT_SAME<
      required<apache::thrift::optionality::optional>,
      fatal::get<traits::members, ident::field4, fatal::get_type::tag>::
          optional>();
  EXPECT_SAME<
      required<apache::thrift::optionality::required_of_writer>,
      fatal::get<traits::members, ident::field5, fatal::get_type::tag>::
          optional>();

  EXPECT_EQ(
      98,
      (fatal::get<traits::members, ident::field0, fatal::get_type::tag>::
           getter{}(pod)));
  EXPECT_EQ(
      "test",
      (fatal::get<traits::members, ident::field1, fatal::get_type::tag>::
           getter{}(pod)));
  EXPECT_EQ(
      enum1::field0,
      (fatal::get<traits::members, ident::field2, fatal::get_type::tag>::
           getter{}(pod)));
  EXPECT_EQ(
      enum2::field2_2,
      (fatal::get<traits::members, ident::field3, fatal::get_type::tag>::
           getter{}(pod)));
  EXPECT_EQ(
      union1::Type::ui,
      (fatal::get<traits::members, ident::field4, fatal::get_type::tag>::
           getter{}(pod)
               .getType()));
  EXPECT_EQ(
      56,
      (fatal::get<traits::members, ident::field4, fatal::get_type::tag>::
           getter{}(pod)
               .get_ui()));
  EXPECT_EQ(
      union2::Type::ue_2,
      (fatal::get<traits::members, ident::field5, fatal::get_type::tag>::
           getter{}(pod)
               .getType()));
  EXPECT_EQ(
      enum1::field1,
      (fatal::get<traits::members, ident::field5, fatal::get_type::tag>::
           getter{}(pod)
               .get_ue_2()));

  EXPECT_SAME<
      apache::thrift::type_class::integral,
      fatal::get<traits::members, ident::field0, fatal::get_type::tag>::
          type_class>();
  EXPECT_SAME<
      apache::thrift::type_class::string,
      fatal::get<traits::members, ident::field1, fatal::get_type::tag>::
          type_class>();
  EXPECT_SAME<
      apache::thrift::type_class::enumeration,
      fatal::get<traits::members, ident::field2, fatal::get_type::tag>::
          type_class>();
  EXPECT_SAME<
      apache::thrift::type_class::enumeration,
      fatal::get<traits::members, ident::field3, fatal::get_type::tag>::
          type_class>();
  EXPECT_SAME<
      apache::thrift::type_class::variant,
      fatal::get<traits::members, ident::field4, fatal::get_type::tag>::
          type_class>();
  EXPECT_SAME<
      apache::thrift::type_class::variant,
      fatal::get<traits::members, ident::field5, fatal::get_type::tag>::
          type_class>();

  EXPECT_SAME<
      traits::member::field0,
      fatal::get<traits::members, ident::field0, fatal::get_type::tag>>();
  EXPECT_SAME<
      traits::member::field1,
      fatal::get<traits::members, ident::field1, fatal::get_type::tag>>();
  EXPECT_SAME<
      traits::member::field2,
      fatal::get<traits::members, ident::field2, fatal::get_type::tag>>();
  EXPECT_SAME<
      traits::member::field3,
      fatal::get<traits::members, ident::field3, fatal::get_type::tag>>();
  EXPECT_SAME<
      traits::member::field4,
      fatal::get<traits::members, ident::field4, fatal::get_type::tag>>();
  EXPECT_SAME<
      traits::member::field5,
      fatal::get<traits::members, ident::field5, fatal::get_type::tag>>();
}

TEST(FatalStruct, SetMethods) {
  using info = apache::thrift::reflect_struct<struct4>;
  using req_field =
      fatal::get<info::members, ident::field0, fatal::get_type::tag>;
  using opt_field =
      fatal::get<info::members, ident::field1, fatal::get_type::tag>;
  using def_field =
      fatal::get<info::members, ident::field2, fatal::get_type::tag>;

  using ref_field =
      fatal::get<info::members, ident::field3, fatal::get_type::tag>;

  struct4 a;
  EXPECT_TRUE(req_field::is_set(a));
  EXPECT_TRUE(req_field::mark_set(a, true));
  EXPECT_TRUE(req_field::is_set(a));
  EXPECT_FALSE(req_field::mark_set(a, false));
  EXPECT_TRUE(req_field::is_set(a));

  EXPECT_FALSE(opt_field::is_set(a));
  EXPECT_TRUE(opt_field::mark_set(a, true));
  EXPECT_TRUE(opt_field::is_set(a));
  EXPECT_FALSE(opt_field::mark_set(a, false));
  EXPECT_FALSE(opt_field::is_set(a));

  EXPECT_FALSE(def_field::is_set(a));
  EXPECT_TRUE(def_field::mark_set(a, true));
  EXPECT_TRUE(def_field::is_set(a));
  EXPECT_FALSE(def_field::mark_set(a, false));
  EXPECT_FALSE(def_field::is_set(a));

  EXPECT_TRUE(ref_field::is_set(a));
  EXPECT_TRUE(ref_field::mark_set(a, true));
}

TEST(FatalStruct, FieldRefGetter) {
  using info = apache::thrift::reflect_struct<struct4>;
  using req_field =
      fatal::get<info::members, ident::field0, fatal::get_type::tag>;
  using opt_field =
      fatal::get<info::members, ident::field1, fatal::get_type::tag>;
  using def_field =
      fatal::get<info::members, ident::field2, fatal::get_type::tag>;

  using ref_field =
      fatal::get<info::members, ident::field3, fatal::get_type::tag>;

  struct4 a;

  req_field::field_ref_getter req;
  EXPECT_TRUE(req(a).has_value());
  req(a) = 1;
  EXPECT_EQ(a.field0(), 1);
  a.field0() = 2;
  EXPECT_EQ(req(a), 2);
  static_assert( //
      std::is_same<
          apache::thrift::required_field_ref<std::int32_t&>,
          decltype(req(a))>::value);

  opt_field::field_ref_getter opt;
  EXPECT_FALSE(opt(a));
  opt(a) = "1";
  EXPECT_EQ(a.field1(), "1");
  a.field1() = "2";
  EXPECT_EQ(opt(a), "2");
  static_assert( //
      std::is_same<
          apache::thrift::optional_field_ref<std::string&>,
          decltype(opt(a))>::value);

  def_field::field_ref_getter def;
  EXPECT_FALSE(def(a).is_set());
  def(a) = enum1::field1;
  EXPECT_EQ(a.field2(), enum1::field1);
  a.field2() = enum1::field2;
  EXPECT_EQ(def(a), enum1::field2);
  static_assert(
      std::is_same<apache::thrift::field_ref<enum1&>, decltype(def(a))>::value);

  ref_field::field_ref_getter ref;
  EXPECT_TRUE(ref(a));
  ref(a)->a() = 1;
  EXPECT_EQ(a.field3()->a(), 1);
  a.field3()->a() = 2;
  EXPECT_EQ(ref(a)->a(), 2);
  static_assert(
      std::is_same<std::unique_ptr<structA>&, decltype(ref(a))>::value);
}

TEST(FatalStruct, RenamedField) {
  using meta = apache::thrift::reflect_struct<struct_with_renamed_field>;
  using fmeta = meta::member::boring_cxx_name;
  auto fname = fmeta::getName().str();
  EXPECT_EQ("fancy.idl.name", fname);
}

TEST(FatalStruct, StructWithAdaptedField) {
  using info = apache::thrift::reflect_struct<StructWithAdaptedField>;
  using type_adapted_field =
      fatal::get<info::members, ident::typeAdapted, fatal::get_type::tag>;
  using field_adapted_field =
      fatal::get<info::members, ident::fieldAdapted, fatal::get_type::tag>;
  StructWithAdaptedField a;

  using type_adapted_type = apache::thrift::test::Wrapper<IntStruct>;
  using field_adapted_type = apache::thrift::test::
      AdaptedWithContext<IntStruct, StructWithAdaptedField, 3>;

  static_assert( //
      std::is_same_v<
          info::member::typeAdapted::type_class,
          apache::thrift::type_class::structure>);
  static_assert( //
      std::is_same_v<
          info::member::typeAdapted::type_class,
          apache::thrift::type_class::structure>);
  static_assert( //
      std::is_same_v< //
          info::member::fieldAdapted::type,
          field_adapted_type>);
  static_assert( //
      std::is_same_v<
          info::member::fieldAdapted::type_class,
          apache::thrift::type_class::structure>);

  type_adapted_field::field_ref_getter get_type_adapted;
  static_assert( //
      std::is_same_v<
          apache::thrift::field_ref<type_adapted_type&>,
          decltype(get_type_adapted(a))>);

  field_adapted_field::field_ref_getter get_field_adapted;
  static_assert( //
      std::is_same_v<
          apache::thrift::field_ref<field_adapted_type&>,
          decltype(get_field_adapted(a))>);
}

TEST(FatalStruct, Terse) {
  using traits = apache::thrift::reflect_struct<struct2>;
  EXPECT_SAME<
      required<apache::thrift::optionality::terse>,
      fatal::get<traits::members, ident::fieldA, fatal::get_type::tag>::
          optional>();
}

} // namespace test_cpp2::cpp_reflection
