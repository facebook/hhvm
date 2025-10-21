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

#include <thrift/test/reflection/gen-cpp2/reflection_fatal_types.h>

#include <thrift/lib/cpp2/reflection/internal/test_helpers.h>
#include <thrift/lib/cpp2/reflection/reflection.h>

#include <gtest/gtest.h>

#include <fatal/type/transform.h>

namespace test_cpp2 {
namespace cpp_reflection {

FATAL_S(union1s, "union1");
FATAL_S(uis, "ui");
FATAL_S(uds, "ud");
FATAL_S(uss, "us");
FATAL_S(ues, "ue");

using uii = std::integral_constant<union1::Type, union1::Type::ui>;
using udi = std::integral_constant<union1::Type, union1::Type::ud>;
using usi = std::integral_constant<union1::Type, union1::Type::us>;
using uei = std::integral_constant<union1::Type, union1::Type::ue>;

TEST(fatal_union, variants) {
  using traits = fatal::variant_traits<union1>;

  EXPECT_SAME<union1, traits::type>();
  EXPECT_SAME<union1s, traits::name>();
  EXPECT_SAME<union1::Type, traits::id>();

  EXPECT_SAME<uii, traits::ids::ui>();
  EXPECT_SAME<udi, traits::ids::ud>();
  EXPECT_SAME<usi, traits::ids::us>();
  EXPECT_SAME<uei, traits::ids::ue>();

  EXPECT_SAME<
      fatal::list<uii, udi, usi, uei>,
      fatal::transform<traits::descriptors, fatal::get_type::id>>();

  EXPECT_SAME<
      fatal::list<std::int32_t, double, std::string, enum1>,
      fatal::transform<traits::descriptors, fatal::get_type::type>>();
}

TEST(fatal_union, by_id) {
  using vtraits = fatal::variant_traits<union1>;
  using traits = vtraits::by_id;

  EXPECT_SAME<uii, traits::id<uii>>();
  EXPECT_SAME<udi, traits::id<udi>>();
  EXPECT_SAME<usi, traits::id<usi>>();
  EXPECT_SAME<uei, traits::id<uei>>();

  EXPECT_SAME<std::int32_t, traits::type<uii>>();
  EXPECT_SAME<double, traits::type<udi>>();
  EXPECT_SAME<std::string, traits::type<usi>>();
  EXPECT_SAME<enum1, traits::type<uei>>();

  union1 u;
  const union1& uc = u;
  union1& ul = u;
  union1&& ur = std::move(u);
  EXPECT_TRUE(vtraits::empty(u));
  EXPECT_TRUE(vtraits::empty(uc));
  EXPECT_TRUE(vtraits::empty(ul));
  EXPECT_TRUE(vtraits::empty(ur));

  traits::set<uii>(u, 10);
  EXPECT_FALSE(vtraits::empty(u));
  EXPECT_FALSE(vtraits::empty(uc));
  EXPECT_FALSE(vtraits::empty(ul));
  EXPECT_FALSE(vtraits::empty(ur));
  EXPECT_EQ(union1::Type::ui, u.getType());
  EXPECT_EQ(union1::Type::ui, vtraits::get_id(u));
  EXPECT_EQ(union1::Type::ui, vtraits::get_id(uc));
  EXPECT_EQ(union1::Type::ui, vtraits::get_id(ul));
  EXPECT_EQ(union1::Type::ui, vtraits::get_id(ur));
  EXPECT_EQ(10, u.get_ui());
  EXPECT_EQ(10, traits::get<uii>(ul));
  EXPECT_EQ(10, traits::get<uii>(uc));
  EXPECT_EQ(10, traits::get<uii>(ur));

  traits::set<udi>(u, 5.6);
  EXPECT_FALSE(vtraits::empty(u));
  EXPECT_FALSE(vtraits::empty(uc));
  EXPECT_FALSE(vtraits::empty(ul));
  EXPECT_FALSE(vtraits::empty(ur));
  EXPECT_EQ(union1::Type::ud, u.getType());
  EXPECT_EQ(union1::Type::ud, vtraits::get_id(u));
  EXPECT_EQ(union1::Type::ud, vtraits::get_id(uc));
  EXPECT_EQ(union1::Type::ud, vtraits::get_id(ul));
  EXPECT_EQ(union1::Type::ud, vtraits::get_id(ur));
  EXPECT_EQ(5.6, u.get_ud());
  EXPECT_EQ(5.6, traits::get<udi>(ul));
  EXPECT_EQ(5.6, traits::get<udi>(uc));
  EXPECT_EQ(5.6, traits::get<udi>(ur));

  traits::set<usi>(u, "hello");
  EXPECT_FALSE(vtraits::empty(u));
  EXPECT_FALSE(vtraits::empty(uc));
  EXPECT_FALSE(vtraits::empty(ul));
  EXPECT_FALSE(vtraits::empty(ur));
  EXPECT_EQ(union1::Type::us, u.getType());
  EXPECT_EQ(union1::Type::us, vtraits::get_id(u));
  EXPECT_EQ(union1::Type::us, vtraits::get_id(uc));
  EXPECT_EQ(union1::Type::us, vtraits::get_id(ul));
  EXPECT_EQ(union1::Type::us, vtraits::get_id(ur));
  EXPECT_EQ("hello", u.get_us());
  EXPECT_EQ("hello", traits::get<usi>(ul));
  EXPECT_EQ("hello", traits::get<usi>(uc));
  EXPECT_EQ("hello", traits::get<usi>(ur));

  traits::set<uei>(u, enum1::field1);
  EXPECT_FALSE(vtraits::empty(u));
  EXPECT_FALSE(vtraits::empty(uc));
  EXPECT_FALSE(vtraits::empty(ul));
  EXPECT_FALSE(vtraits::empty(ur));
  EXPECT_EQ(union1::Type::ue, u.getType());
  EXPECT_EQ(union1::Type::ue, vtraits::get_id(u));
  EXPECT_EQ(union1::Type::ue, vtraits::get_id(uc));
  EXPECT_EQ(union1::Type::ue, vtraits::get_id(ul));
  EXPECT_EQ(union1::Type::ue, vtraits::get_id(ur));
  EXPECT_EQ(enum1::field1, u.get_ue());
  EXPECT_EQ(enum1::field1, traits::get<uei>(ul));
  EXPECT_EQ(enum1::field1, traits::get<uei>(uc));
  EXPECT_EQ(enum1::field1, traits::get<uei>(ur));
}

TEST(fatal_union, by_type) {
  using vtraits = fatal::variant_traits<union1>;
  using traits = vtraits::by_type;

  EXPECT_SAME<uii, traits::id<std::int32_t>>();
  EXPECT_SAME<udi, traits::id<double>>();
  EXPECT_SAME<usi, traits::id<std::string>>();
  EXPECT_SAME<uei, traits::id<enum1>>();

  EXPECT_SAME<std::int32_t, traits::type<std::int32_t>>();
  EXPECT_SAME<double, traits::type<double>>();
  EXPECT_SAME<std::string, traits::type<std::string>>();
  EXPECT_SAME<enum1, traits::type<enum1>>();

  union1 u;
  const union1& uc = u;
  union1& ul = u;
  union1&& ur = std::move(u);
  EXPECT_TRUE(vtraits::empty(u));
  EXPECT_TRUE(vtraits::empty(uc));
  EXPECT_TRUE(vtraits::empty(ul));
  EXPECT_TRUE(vtraits::empty(ur));

  traits::set<std::int32_t>(u, 10);
  EXPECT_FALSE(vtraits::empty(u));
  EXPECT_FALSE(vtraits::empty(uc));
  EXPECT_FALSE(vtraits::empty(ul));
  EXPECT_FALSE(vtraits::empty(ur));
  EXPECT_EQ(union1::Type::ui, u.getType());
  EXPECT_EQ(union1::Type::ui, vtraits::get_id(u));
  EXPECT_EQ(union1::Type::ui, vtraits::get_id(uc));
  EXPECT_EQ(union1::Type::ui, vtraits::get_id(ul));
  EXPECT_EQ(union1::Type::ui, vtraits::get_id(ur));
  EXPECT_EQ(10, u.get_ui());
  EXPECT_EQ(10, traits::get<std::int32_t>(ul));
  EXPECT_EQ(10, traits::get<std::int32_t>(uc));
  EXPECT_EQ(10, traits::get<std::int32_t>(ur));

  traits::set<double>(u, 5.6);
  EXPECT_FALSE(vtraits::empty(u));
  EXPECT_FALSE(vtraits::empty(uc));
  EXPECT_FALSE(vtraits::empty(ul));
  EXPECT_FALSE(vtraits::empty(ur));
  EXPECT_EQ(union1::Type::ud, u.getType());
  EXPECT_EQ(union1::Type::ud, vtraits::get_id(u));
  EXPECT_EQ(union1::Type::ud, vtraits::get_id(uc));
  EXPECT_EQ(union1::Type::ud, vtraits::get_id(ul));
  EXPECT_EQ(union1::Type::ud, vtraits::get_id(ur));
  EXPECT_EQ(5.6, u.get_ud());
  EXPECT_EQ(5.6, traits::get<double>(ul));
  EXPECT_EQ(5.6, traits::get<double>(uc));
  EXPECT_EQ(5.6, traits::get<double>(ur));

  traits::set<std::string>(u, "hello");
  EXPECT_FALSE(vtraits::empty(u));
  EXPECT_FALSE(vtraits::empty(uc));
  EXPECT_FALSE(vtraits::empty(ul));
  EXPECT_FALSE(vtraits::empty(ur));
  EXPECT_EQ(union1::Type::us, u.getType());
  EXPECT_EQ(union1::Type::us, vtraits::get_id(u));
  EXPECT_EQ(union1::Type::us, vtraits::get_id(uc));
  EXPECT_EQ(union1::Type::us, vtraits::get_id(ul));
  EXPECT_EQ(union1::Type::us, vtraits::get_id(ur));
  EXPECT_EQ("hello", u.get_us());
  EXPECT_EQ("hello", traits::get<std::string>(ul));
  EXPECT_EQ("hello", traits::get<std::string>(uc));
  EXPECT_EQ("hello", traits::get<std::string>(ur));

  traits::set<enum1>(u, enum1::field1);
  EXPECT_FALSE(vtraits::empty(u));
  EXPECT_FALSE(vtraits::empty(uc));
  EXPECT_FALSE(vtraits::empty(ul));
  EXPECT_FALSE(vtraits::empty(ur));
  EXPECT_EQ(union1::Type::ue, u.getType());
  EXPECT_EQ(union1::Type::ue, vtraits::get_id(u));
  EXPECT_EQ(union1::Type::ue, vtraits::get_id(uc));
  EXPECT_EQ(union1::Type::ue, vtraits::get_id(ul));
  EXPECT_EQ(union1::Type::ue, vtraits::get_id(ur));
  EXPECT_EQ(enum1::field1, u.get_ue());
  EXPECT_EQ(enum1::field1, traits::get<enum1>(ul));
  EXPECT_EQ(enum1::field1, traits::get<enum1>(uc));
  EXPECT_EQ(enum1::field1, traits::get<enum1>(ur));
}

FATAL_S(unionA_annotation1k, "another.annotation");
FATAL_S(unionA_annotation1v, "some more text");
FATAL_S(unionA_annotation2k, "sample.annotation");
FATAL_S(unionA_annotation2v, "some text here");

TEST(fatal_union, annotations) {
  EXPECT_SAME<
      fatal::list<>,
      apache::thrift::reflect_variant<union2>::annotations::map>();

  EXPECT_SAME<
      fatal::list<>,
      apache::thrift::reflect_variant<union3>::annotations::map>();

  using actual_unionA = apache::thrift::reflect_variant<unionA>::annotations;

  EXPECT_SAME<unionA_annotation1k, actual_unionA::keys::another_annotation>();
  EXPECT_SAME<unionA_annotation1v, actual_unionA::values::another_annotation>();
  EXPECT_SAME<unionA_annotation2k, actual_unionA::keys::sample_annotation>();
  EXPECT_SAME<unionA_annotation2v, actual_unionA::values::sample_annotation>();

  EXPECT_SAME<
      fatal::list<
          apache::thrift::annotation<unionA_annotation1k, unionA_annotation1v>,
          apache::thrift::annotation<unionA_annotation2k, unionA_annotation2v>>,
      actual_unionA::map>();
}

TEST(fatal_union, by_name) {
  using id_traits = fatal::enum_traits<union1::Type>;
  using info = apache::thrift::reflect_variant<union1>;
  using member_info = info::by_name<id_traits::member::ui::name>;

  union1 u;
  u.set_ui(10);

  ASSERT_EQ(u.getType(), union1::Type::ui);
  EXPECT_EQ(10, member_info::get(u));
}

TEST(fatal_union, by_type_id) {
  using info = apache::thrift::reflect_variant<union1>;
  using member_info = info::by_type_id<union1::Type::ui>;

  union1 u;
  u.set_ui(10);

  ASSERT_EQ(u.getType(), union1::Type::ui);
  EXPECT_EQ(10, member_info::get(u));
}

TEST(fatal_union, by_field_id) {
  using info = apache::thrift::reflect_variant<union1>;
  using member_info = info::by_field_id<1>;

  union1 u;
  u.set_ui(10);

  ASSERT_EQ(u.getType(), union1::Type::ui);
  EXPECT_EQ(10, member_info::get(u));
}

TEST(fatal_struct, renamed_field) {
  using emeta = fatal::enum_traits<union_with_renamed_field::Type>;
  using vmeta = emeta::member::boring_cxx_name;
  auto vname = fatal::to_instance<std::string, vmeta::name>();
  EXPECT_EQ("fancy.idl.name", vname);

  using umeta = apache::thrift::reflect_variant<union_with_renamed_field>;
  using fmeta =
      umeta::by_type_id<union_with_renamed_field::Type::boring_cxx_name>;
  auto fname = fatal::to_instance<std::string, fmeta::metadata::name>();
  EXPECT_EQ("fancy.idl.name", fname);
}

} // namespace cpp_reflection
} // namespace test_cpp2
