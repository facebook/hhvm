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

namespace test_cpp2 {
namespace cpp_reflection {

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

TEST(fatal_struct, struct1_sanity_check) {
  using traits = apache::thrift::reflect_struct<struct1>;

  EXPECT_SAME<struct1, traits::type>();
  EXPECT_SAME<struct1s, traits::name>();

  EXPECT_SAME<traits, apache::thrift::try_reflect_struct<struct1, void>>();
  EXPECT_SAME<void, apache::thrift::try_reflect_struct<int, void>>();

  EXPECT_SAME<field0s, traits::member::field0::name>();
  EXPECT_SAME<field1s, traits::member::field1::name>();
  EXPECT_SAME<field2s, traits::member::field2::name>();
  EXPECT_SAME<field3s, traits::member::field3::name>();
  EXPECT_SAME<field4s, traits::member::field4::name>();
  EXPECT_SAME<field5s, traits::member::field5::name>();

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
      field0s,
      fatal::get<traits::members, field0s, fatal::get_type::name>::name>();
  EXPECT_SAME<
      field1s,
      fatal::get<traits::members, field1s, fatal::get_type::name>::name>();
  EXPECT_SAME<
      field2s,
      fatal::get<traits::members, field2s, fatal::get_type::name>::name>();
  EXPECT_SAME<
      field3s,
      fatal::get<traits::members, field3s, fatal::get_type::name>::name>();
  EXPECT_SAME<
      field4s,
      fatal::get<traits::members, field4s, fatal::get_type::name>::name>();
  EXPECT_SAME<
      field5s,
      fatal::get<traits::members, field5s, fatal::get_type::name>::name>();

  EXPECT_SAME<
      std::int32_t,
      fatal::get<traits::members, field0s, fatal::get_type::name>::type>();
  EXPECT_SAME<
      std::string,
      fatal::get<traits::members, field1s, fatal::get_type::name>::type>();
  EXPECT_SAME<
      enum1,
      fatal::get<traits::members, field2s, fatal::get_type::name>::type>();
  EXPECT_SAME<
      enum2,
      fatal::get<traits::members, field3s, fatal::get_type::name>::type>();
  EXPECT_SAME<
      union1,
      fatal::get<traits::members, field4s, fatal::get_type::name>::type>();
  EXPECT_SAME<
      union2,
      fatal::get<traits::members, field5s, fatal::get_type::name>::type>();

  EXPECT_SAME<
      field_id<1>,
      fatal::get<traits::members, field0s, fatal::get_type::name>::id>();
  EXPECT_SAME<
      field_id<2>,
      fatal::get<traits::members, field1s, fatal::get_type::name>::id>();
  EXPECT_SAME<
      field_id<4>,
      fatal::get<traits::members, field2s, fatal::get_type::name>::id>();
  EXPECT_SAME<
      field_id<8>,
      fatal::get<traits::members, field3s, fatal::get_type::name>::id>();
  EXPECT_SAME<
      field_id<16>,
      fatal::get<traits::members, field4s, fatal::get_type::name>::id>();
  EXPECT_SAME<
      field_id<32>,
      fatal::get<traits::members, field5s, fatal::get_type::name>::id>();

  EXPECT_SAME<
      required<apache::thrift::optionality::required>,
      fatal::get<traits::members, field0s, fatal::get_type::name>::optional>();
  EXPECT_SAME<
      required<apache::thrift::optionality::optional>,
      fatal::get<traits::members, field1s, fatal::get_type::name>::optional>();
  EXPECT_SAME<
      required<apache::thrift::optionality::required_of_writer>,
      fatal::get<traits::members, field2s, fatal::get_type::name>::optional>();
  EXPECT_SAME<
      required<apache::thrift::optionality::required>,
      fatal::get<traits::members, field3s, fatal::get_type::name>::optional>();
  EXPECT_SAME<
      required<apache::thrift::optionality::optional>,
      fatal::get<traits::members, field4s, fatal::get_type::name>::optional>();
  EXPECT_SAME<
      required<apache::thrift::optionality::required_of_writer>,
      fatal::get<traits::members, field5s, fatal::get_type::name>::optional>();

  EXPECT_EQ(
      98,
      (fatal::get<
          traits::members,
          traits::member::field0::name,
          fatal::get_type::name>::getter{}(pod)));
  EXPECT_EQ(
      "test",
      (fatal::get<
          traits::members,
          traits::member::field1::name,
          fatal::get_type::name>::getter{}(pod)));
  EXPECT_EQ(
      enum1::field0,
      (fatal::get<
          traits::members,
          traits::member::field2::name,
          fatal::get_type::name>::getter{}(pod)));
  EXPECT_EQ(
      enum2::field2_2,
      (fatal::get<
          traits::members,
          traits::member::field3::name,
          fatal::get_type::name>::getter{}(pod)));
  EXPECT_EQ(
      union1::Type::ui,
      (fatal::get<
           traits::members,
           traits::member::field4::name,
           fatal::get_type::name>::getter{}(pod)
           .getType()));
  EXPECT_EQ(
      56,
      (fatal::get<
           traits::members,
           traits::member::field4::name,
           fatal::get_type::name>::getter{}(pod)
           .get_ui()));
  EXPECT_EQ(
      union2::Type::ue_2,
      (fatal::get<
           traits::members,
           traits::member::field5::name,
           fatal::get_type::name>::getter{}(pod)
           .getType()));
  EXPECT_EQ(
      enum1::field1,
      (fatal::get<
           traits::members,
           traits::member::field5::name,
           fatal::get_type::name>::getter{}(pod)
           .get_ue_2()));

  EXPECT_SAME<
      apache::thrift::type_class::integral,
      fatal::get<traits::members, field0s, fatal::get_type::name>::
          type_class>();
  EXPECT_SAME<
      apache::thrift::type_class::string,
      fatal::get<traits::members, field1s, fatal::get_type::name>::
          type_class>();
  EXPECT_SAME<
      apache::thrift::type_class::enumeration,
      fatal::get<traits::members, field2s, fatal::get_type::name>::
          type_class>();
  EXPECT_SAME<
      apache::thrift::type_class::enumeration,
      fatal::get<traits::members, field3s, fatal::get_type::name>::
          type_class>();
  EXPECT_SAME<
      apache::thrift::type_class::variant,
      fatal::get<traits::members, field4s, fatal::get_type::name>::
          type_class>();
  EXPECT_SAME<
      apache::thrift::type_class::variant,
      fatal::get<traits::members, field5s, fatal::get_type::name>::
          type_class>();

  EXPECT_SAME<
      traits::member::field0,
      fatal::get<
          traits::members,
          traits::member::field0::name,
          fatal::get_type::name>>();
  EXPECT_SAME<
      traits::member::field1,
      fatal::get<
          traits::members,
          traits::member::field1::name,
          fatal::get_type::name>>();
  EXPECT_SAME<
      traits::member::field2,
      fatal::get<
          traits::members,
          traits::member::field2::name,
          fatal::get_type::name>>();
  EXPECT_SAME<
      traits::member::field3,
      fatal::get<
          traits::members,
          traits::member::field3::name,
          fatal::get_type::name>>();
  EXPECT_SAME<
      traits::member::field4,
      fatal::get<
          traits::members,
          traits::member::field4::name,
          fatal::get_type::name>>();
  EXPECT_SAME<
      traits::member::field5,
      fatal::get<
          traits::members,
          traits::member::field5::name,
          fatal::get_type::name>>();
}

FATAL_S(structB_annotation1k, "multi_line_annotation");
FATAL_S(structB_annotation1v, "line one\nline two");
FATAL_S(structB_annotation2k, "some.annotation");
FATAL_S(structB_annotation2v, "this is its value");
FATAL_S(structB_annotation3k, "some.other.annotation");
FATAL_S(structB_annotation3v, "this is its other value");

FATAL_S(struct3_annotation1k, "thrift.uri");
FATAL_S(
    struct3_annotation1v,
    "facebook.com/thrift/test/reflection/reflection/struct3");

TEST(fatal_struct, annotations) {
  EXPECT_SAME<
      fatal::list<>,
      apache::thrift::reflect_struct<struct1>::annotations::map>();

  EXPECT_SAME<
      fatal::list<>,
      apache::thrift::reflect_struct<struct2>::annotations::map>();

  EXPECT_SAME<
      fatal::list<apache::thrift::
                      annotation<struct3_annotation1k, struct3_annotation1v>>,
      apache::thrift::reflect_struct<struct3>::annotations::map>();

  EXPECT_SAME<
      fatal::list<>,
      apache::thrift::reflect_struct<structA>::annotations::map>();

  using structB_annotations =
      apache::thrift::reflect_struct<structB>::annotations;

  EXPECT_SAME<
      structB_annotation1k,
      structB_annotations::keys::multi_line_annotation>();
  EXPECT_SAME<
      structB_annotation1v,
      structB_annotations::values::multi_line_annotation>();
  EXPECT_SAME<
      structB_annotation2k,
      structB_annotations::keys::some_annotation>();
  EXPECT_SAME<
      structB_annotation2v,
      structB_annotations::values::some_annotation>();
  EXPECT_SAME<
      structB_annotation3k,
      structB_annotations::keys::some_other_annotation>();
  EXPECT_SAME<
      structB_annotation3v,
      structB_annotations::values::some_other_annotation>();
  EXPECT_SAME<
      fatal::list<
          apache::thrift::
              annotation<structB_annotation1k, structB_annotation1v>,
          apache::thrift::
              annotation<structB_annotation2k, structB_annotation2v>,
          apache::thrift::
              annotation<structB_annotation3k, structB_annotation3v>>,
      structB_annotations::map>();

  EXPECT_SAME<
      fatal::list<>,
      apache::thrift::reflect_struct<structC>::annotations::map>();
}

FATAL_S(structBd_annotation1k, "another.annotation");
FATAL_S(structBd_annotation1v, "another value");
FATAL_S(structBd_annotation2k, "some.annotation");
FATAL_S(structBd_annotation2v, "some value");

TEST(fatal_struct, member_annotations) {
  using info = apache::thrift::reflect_struct<structB>;

  EXPECT_SAME<fatal::list<>, info::members_annotations::c::map>();
  EXPECT_SAME<
      fatal::list<>,
      fatal::get<info::members, info::member::c::name, fatal::get_type::name>::
          annotations::map>();

  using annotations_d =
      fatal::get<info::members, info::member::d::name, fatal::get_type::name>::
          annotations;
  using expected_d_map = fatal::list<
      apache::thrift::annotation<structBd_annotation1k, structBd_annotation1v>,
      apache::thrift::annotation<structBd_annotation2k, structBd_annotation2v>>;

  EXPECT_SAME<expected_d_map, info::members_annotations::d::map>();
  EXPECT_SAME<expected_d_map, annotations_d::map>();

  EXPECT_SAME<
      structBd_annotation1k,
      info::members_annotations::d::keys::another_annotation>();
  EXPECT_SAME<
      structBd_annotation1v,
      info::members_annotations::d::values::another_annotation>();
  EXPECT_SAME<
      structBd_annotation2k,
      info::members_annotations::d::keys::some_annotation>();
  EXPECT_SAME<
      structBd_annotation2v,
      info::members_annotations::d::values::some_annotation>();

  EXPECT_SAME<structBd_annotation1k, annotations_d::keys::another_annotation>();
  EXPECT_SAME<
      structBd_annotation1v,
      annotations_d::values::another_annotation>();
  EXPECT_SAME<structBd_annotation2k, annotations_d::keys::some_annotation>();
  EXPECT_SAME<structBd_annotation2v, annotations_d::values::some_annotation>();
}

FATAL_S(annotated_s_b_false, "s_b_false");
FATAL_S(annotated_s_b_true, "s_b_true");
FATAL_S(annotated_s_int, "s_int");
FATAL_S(annotated_s_string, "s_string");
FATAL_S(annotated_s_int_list, "s_int_list");
FATAL_S(annotated_s_str_list, "s_str_list");
FATAL_S(annotated_s_mixed_list, "s_mixed_list");
FATAL_S(annotated_s_int_map, "s_int_map");
FATAL_S(annotated_s_str_map, "s_str_map");
FATAL_S(annotated_s_mixed_map, "s_mixed_map");

FATAL_S(annotated_v_s_b_false, "false");
FATAL_S(annotated_v_s_b_true, "true");
FATAL_S(annotated_v_s_int, "10");
FATAL_S(annotated_v_s_string, "\"hello\"");
FATAL_S(annotated_v_s_int_list, "[-1, 2, 3]");
FATAL_S(annotated_v_s_str_list, "[\"a\", \"b\", \"c\"]");
FATAL_S(annotated_v_s_mixed_list, "[\"a\", 1, \"b\", 2]");
FATAL_S(annotated_v_s_int_map, "{\"a\": 1, \"b\": -2, \"c\": -3}");
FATAL_S(annotated_v_s_str_map, "{\"a\": \"A\", \"b\": \"B\", \"c\": \"C\"}");
FATAL_S(annotated_v_s_mixed_map, "{\"a\": -2, \"b\": \"B\", \"c\": 3}");

TEST(fatal_struct, structured_annotations) {
  using info = apache::thrift::reflect_struct<annotated>::annotations;

  using bf =
      apache::thrift::annotation<annotated_s_b_false, annotated_v_s_b_false>;
  using bt =
      apache::thrift::annotation<annotated_s_b_true, annotated_v_s_b_true>;
  using i = apache::thrift::annotation<annotated_s_int, annotated_v_s_int>;
  using s =
      apache::thrift::annotation<annotated_s_string, annotated_v_s_string>;
  using il =
      apache::thrift::annotation<annotated_s_int_list, annotated_v_s_int_list>;
  using sl =
      apache::thrift::annotation<annotated_s_str_list, annotated_v_s_str_list>;
  using ml = apache::thrift::
      annotation<annotated_s_mixed_list, annotated_v_s_mixed_list>;
  using im =
      apache::thrift::annotation<annotated_s_int_map, annotated_v_s_int_map>;
  using sm =
      apache::thrift::annotation<annotated_s_str_map, annotated_v_s_str_map>;
  using mm = apache::thrift::
      annotation<annotated_s_mixed_map, annotated_v_s_mixed_map>;

  EXPECT_SAME<bf, fatal::get<info::map, info::keys::s_b_false>>();
  EXPECT_SAME<bt, fatal::get<info::map, info::keys::s_b_true>>();
  EXPECT_SAME<i, fatal::get<info::map, info::keys::s_int>>();
  EXPECT_SAME<s, fatal::get<info::map, info::keys::s_string>>();
  EXPECT_SAME<il, fatal::get<info::map, info::keys::s_int_list>>();
  EXPECT_SAME<sl, fatal::get<info::map, info::keys::s_str_list>>();
  EXPECT_SAME<ml, fatal::get<info::map, info::keys::s_mixed_list>>();
  EXPECT_SAME<im, fatal::get<info::map, info::keys::s_int_map>>();
  EXPECT_SAME<sm, fatal::get<info::map, info::keys::s_str_map>>();
  EXPECT_SAME<mm, fatal::get<info::map, info::keys::s_mixed_map>>();

  EXPECT_SAME<fatal::list<bf, bt, i, il, im, ml, mm, sl, sm, s>, info::map>();
}

FATAL_S(annotated_m_b_false, "m_b_false");
FATAL_S(annotated_m_b_true, "m_b_true");
FATAL_S(annotated_m_int, "m_int");
FATAL_S(annotated_m_string, "m_string");
FATAL_S(annotated_m_int_list, "m_int_list");
FATAL_S(annotated_m_str_list, "m_str_list");
FATAL_S(annotated_m_mixed_list, "m_mixed_list");
FATAL_S(annotated_m_int_map, "m_int_map");
FATAL_S(annotated_m_str_map, "m_str_map");
FATAL_S(annotated_m_mixed_map, "m_mixed_map");

FATAL_S(annotated_v_m_b_false, "false");
FATAL_S(annotated_v_m_b_true, "true");
FATAL_S(annotated_v_m_int, "10");
FATAL_S(annotated_v_m_string, "\"hello\"");
FATAL_S(annotated_v_m_int_list, "[-1, 2, 3]");
FATAL_S(annotated_v_m_str_list, "[\"a\", \"b\", \"c\"]");
FATAL_S(annotated_v_m_mixed_list, "[\"a\", 1, \"b\", 2]");
FATAL_S(annotated_v_m_int_map, "{\"a\": 1, \"b\": -2, \"c\": -3}");
FATAL_S(annotated_v_m_str_map, "{\"a\": \"A\", \"b\": \"B\", \"c\": \"C\"}");
FATAL_S(annotated_v_m_mixed_map, "{\"a\": -2, \"b\": \"B\", \"c\": 3}");

TEST(fatal_struct, structured_member_annotations) {
  using info =
      apache::thrift::reflect_struct<annotated>::members_annotations::a;

  using bf =
      apache::thrift::annotation<annotated_m_b_false, annotated_v_m_b_false>;
  using bt =
      apache::thrift::annotation<annotated_m_b_true, annotated_v_m_b_true>;
  using i = apache::thrift::annotation<annotated_m_int, annotated_v_m_int>;
  using s =
      apache::thrift::annotation<annotated_m_string, annotated_v_m_string>;
  using il =
      apache::thrift::annotation<annotated_m_int_list, annotated_v_m_int_list>;
  using sl =
      apache::thrift::annotation<annotated_m_str_list, annotated_v_m_str_list>;
  using ml = apache::thrift::
      annotation<annotated_m_mixed_list, annotated_v_m_mixed_list>;
  using im =
      apache::thrift::annotation<annotated_m_int_map, annotated_v_m_int_map>;
  using sm =
      apache::thrift::annotation<annotated_m_str_map, annotated_v_m_str_map>;
  using mm = apache::thrift::
      annotation<annotated_m_mixed_map, annotated_v_m_mixed_map>;

  EXPECT_SAME<bf, fatal::get<info::map, info::keys::m_b_false>>();
  EXPECT_SAME<bt, fatal::get<info::map, info::keys::m_b_true>>();
  EXPECT_SAME<i, fatal::get<info::map, info::keys::m_int>>();
  EXPECT_SAME<s, fatal::get<info::map, info::keys::m_string>>();
  EXPECT_SAME<il, fatal::get<info::map, info::keys::m_int_list>>();
  EXPECT_SAME<sl, fatal::get<info::map, info::keys::m_str_list>>();
  EXPECT_SAME<ml, fatal::get<info::map, info::keys::m_mixed_list>>();
  EXPECT_SAME<im, fatal::get<info::map, info::keys::m_int_map>>();
  EXPECT_SAME<sm, fatal::get<info::map, info::keys::m_str_map>>();
  EXPECT_SAME<mm, fatal::get<info::map, info::keys::m_mixed_map>>();

  EXPECT_SAME<fatal::list<bf, bt, i, il, im, ml, mm, sl, sm, s>, info::map>();
}

TEST(fatal_struct, set_methods) {
  using info = apache::thrift::reflect_struct<struct4>;
  using req_field = fatal::
      get<info::members, info::member::field0::name, fatal::get_type::name>;
  using opt_field = fatal::
      get<info::members, info::member::field1::name, fatal::get_type::name>;
  using def_field = fatal::
      get<info::members, info::member::field2::name, fatal::get_type::name>;

  using ref_field = fatal::
      get<info::members, info::member::field3::name, fatal::get_type::name>;

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

TEST(fatal_struct, field_ref_getter) {
  using info = apache::thrift::reflect_struct<struct4>;
  using req_field = fatal::
      get<info::members, info::member::field0::name, fatal::get_type::name>;
  using opt_field = fatal::
      get<info::members, info::member::field1::name, fatal::get_type::name>;
  using def_field = fatal::
      get<info::members, info::member::field2::name, fatal::get_type::name>;

  using ref_field = fatal::
      get<info::members, info::member::field3::name, fatal::get_type::name>;

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

TEST(fatal_struct, renamed_field) {
  using meta = apache::thrift::reflect_struct<struct_with_renamed_field>;
  using fmeta = meta::member::boring_cxx_name;
  auto fname = fatal::to_instance<std::string, fmeta::name>();
  EXPECT_EQ("fancy.idl.name", fname);
}

TEST(fatal_struct, StructWithAdaptedField) {
  using info = apache::thrift::reflect_struct<StructWithAdaptedField>;
  using type_adapted_field = fatal::get<
      info::members,
      info::member::typeAdapted::name,
      fatal::get_type::name>;
  using field_adapted_field = fatal::get<
      info::members,
      info::member::fieldAdapted::name,
      fatal::get_type::name>;
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

TEST(fatal_struct, Terse) {
  using traits = apache::thrift::reflect_struct<struct2>;
  EXPECT_SAME<
      required<apache::thrift::optionality::terse>,
      fatal::get<traits::members, fieldAs, fatal::get_type::name>::optional>();
}

} // namespace cpp_reflection
} // namespace test_cpp2
