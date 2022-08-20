/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <folly/portability/GTest.h>

namespace test_cpp2 {
namespace cpp_reflection {

FATAL_S(enum1s, "enum1");
FATAL_S(field0s, "field0");
FATAL_S(field1s, "field1");
FATAL_S(field2s, "field2");

TEST(fatal_enum, sanity_check) {
  using traits = fatal::enum_traits<enum1>;

  EXPECT_SAME<enum1, traits::type>();
  EXPECT_SAME<enum1s, traits::name>();
  EXPECT_SAME<std::underlying_type<enum1>::type, traits::int_type>();

  EXPECT_SAME<field0s, traits::member::field0::name>();
  EXPECT_SAME<field1s, traits::member::field1::name>();
  EXPECT_SAME<field2s, traits::member::field2::name>();

  EXPECT_SAME<
      std::integral_constant<enum1, enum1::field0>,
      traits::member::field0::value>();
  EXPECT_SAME<
      std::integral_constant<enum1, enum1::field1>,
      traits::member::field1::value>();
  EXPECT_SAME<
      std::integral_constant<enum1, enum1::field2>,
      traits::member::field2::value>();

  EXPECT_SAME<
      fatal::list<field0s, field1s, field2s>,
      fatal::transform<traits::fields, fatal::get_type::name>>();
  EXPECT_SAME<
      fatal::value_list<enum1, enum1::field0, enum1::field1, enum1::field2>,
      fatal::transform<traits::fields, fatal::get_type::value>>();

  EXPECT_SAME<field0s, traits::name_of<enum1::field0>>();
  EXPECT_SAME<field1s, traits::name_of<enum1::field1>>();
  EXPECT_SAME<field2s, traits::name_of<enum1::field2>>();

  EXPECT_SAME<
      std::integral_constant<enum1, enum1::field0>,
      traits::value_of<field0s>>();
  EXPECT_SAME<
      std::integral_constant<enum1, enum1::field1>,
      traits::value_of<field1s>>();
  EXPECT_SAME<
      std::integral_constant<enum1, enum1::field2>,
      traits::value_of<field2s>>();

  EXPECT_EQ("field0", fatal::try_enum_to_string(enum1::field0));
  EXPECT_EQ("field1", fatal::try_enum_to_string(enum1::field1));
  EXPECT_EQ("field2", fatal::try_enum_to_string(enum1::field2));
  EXPECT_EQ(nullptr, fatal::try_enum_to_string(static_cast<enum1>(99999)));

  EXPECT_EQ("field0", traits::to_string(enum1::field0));
  EXPECT_EQ("field1", traits::to_string(enum1::field1));
  EXPECT_EQ("field2", traits::to_string(enum1::field2));
  EXPECT_EQ(nullptr, traits::to_string(static_cast<enum1>(99999)));

  EXPECT_EQ(enum1::field0, traits::parse(std::string("field0")));
  EXPECT_EQ(enum1::field1, traits::parse(std::string("field1")));
  EXPECT_EQ(enum1::field2, traits::parse(std::string("field2")));
  EXPECT_THROW(traits::parse("not a field"), std::invalid_argument);

  enum1 e;
  EXPECT_TRUE(traits::try_parse(e, std::string("field0")));
  EXPECT_EQ(enum1::field0, e);
  EXPECT_TRUE(traits::try_parse(e, std::string("field1")));
  EXPECT_EQ(enum1::field1, e);
  EXPECT_TRUE(traits::try_parse(e, std::string("field2")));
  EXPECT_EQ(enum1::field2, e);
  EXPECT_FALSE(traits::try_parse(e, std::string("not a field")));
  EXPECT_EQ(enum1::field2, e);
}

FATAL_S(enum3_annotation1k, "_now.with.an.underscore");
FATAL_S(enum3_annotation1v, "_now.with.an.underscore");
FATAL_S(enum3_annotation2k, "another.there");
FATAL_S(enum3_annotation2v, ".");
FATAL_S(enum3_annotation3k, "duplicate_id_annotation_1");
FATAL_S(enum3_annotation3v, "duplicate id annotation");
FATAL_S(enum3_annotation4k, "duplicate_id_annotation_2");
FATAL_S(enum3_annotation4v, "duplicate.id.annotation");
FATAL_S(enum3_annotation5k, "one.here");
FATAL_S(enum3_annotation5v, "with some value associated");
FATAL_S(enum3_annotation6k, "yet.another");
FATAL_S(enum3_annotation6v, "and yet more text - it's that easy");

TEST(fatal_enum, annotations) {
  EXPECT_SAME<
      fatal::list<>,
      apache::thrift::reflect_enum<enum1>::annotations::map>();

  EXPECT_SAME<
      fatal::list<>,
      apache::thrift::reflect_enum<enum2>::annotations::map>();

  using enum3_annotations = apache::thrift::reflect_enum<enum3>::annotations;

  EXPECT_SAME<
      enum3_annotation1k,
      enum3_annotations::keys::_now_with_an_underscore>();
  EXPECT_SAME<
      enum3_annotation1v,
      enum3_annotations::values::_now_with_an_underscore>();
  EXPECT_SAME<enum3_annotation2k, enum3_annotations::keys::another_there>();
  EXPECT_SAME<enum3_annotation2v, enum3_annotations::values::another_there>();
  EXPECT_SAME<
      enum3_annotation3k,
      enum3_annotations::keys::duplicate_id_annotation_1>();
  EXPECT_SAME<
      enum3_annotation3v,
      enum3_annotations::values::duplicate_id_annotation_1>();
  EXPECT_SAME<
      enum3_annotation4k,
      enum3_annotations::keys::duplicate_id_annotation_2>();
  EXPECT_SAME<
      enum3_annotation4v,
      enum3_annotations::values::duplicate_id_annotation_2>();
  EXPECT_SAME<enum3_annotation5k, enum3_annotations::keys::one_here>();
  EXPECT_SAME<enum3_annotation5v, enum3_annotations::values::one_here>();
  EXPECT_SAME<enum3_annotation6k, enum3_annotations::keys::yet_another>();
  EXPECT_SAME<enum3_annotation6v, enum3_annotations::values::yet_another>();

  EXPECT_SAME<
      fatal::list<
          apache::thrift::annotation<enum3_annotation1k, enum3_annotation1v>,
          apache::thrift::annotation<enum3_annotation2k, enum3_annotation2v>,
          apache::thrift::annotation<enum3_annotation3k, enum3_annotation3v>,
          apache::thrift::annotation<enum3_annotation4k, enum3_annotation4v>,
          apache::thrift::annotation<enum3_annotation5k, enum3_annotation5v>,
          apache::thrift::annotation<enum3_annotation6k, enum3_annotation6v>>,
      enum3_annotations::map>();
}

FATAL_S(field_annotation, "field_annotation");
FATAL_S(field_annotated, "field annotated");
FATAL_S(field_structured_annotation, "field_structured_annotation");
FATAL_S(
    field_structured_annotation_text, R"({"a": "foo", "b": 567, "c": true})");
FATAL_S(some_other_text, "some other text");
FATAL_S(a, "a");
FATAL_S(b, "b");
FATAL_S(c, "c");
FATAL_S(foo, "foo");

TEST(fatal_enum, field_annotations) {
  using info = fatal::enum_traits<enum3>;
  using field0 = info::member::field0_3::annotations;
  using field1 = info::member::field1_3::annotations;
  using field2 = info::member::field2_3::annotations;

  EXPECT_SAME<field_annotation, field1::keys::field_annotation>();
  EXPECT_SAME<field_annotated, field1::values::field_annotation>();
  EXPECT_SAME<
      field_structured_annotation,
      field2::keys::field_structured_annotation>();
  EXPECT_SAME<
      field_structured_annotation_text,
      field2::values::field_structured_annotation>();
  EXPECT_SAME<field_annotation, field2::keys::field_annotation>();
  EXPECT_SAME<some_other_text, field2::values::field_annotation>();

  EXPECT_SAME<fatal::list<>, field0::map>();
  EXPECT_SAME<
      fatal::list<
          apache::thrift::annotation<field_annotation, field_annotated>>,
      field1::map>();
  EXPECT_SAME<
      fatal::list<
          apache::thrift::annotation<field_annotation, some_other_text>,
          apache::thrift::annotation<
              field_structured_annotation,
              field_structured_annotation_text>>,
      field2::map>();
}

TEST(fatal_struct, renamed_value) {
  using meta = fatal::enum_traits<enum_with_renamed_value>;
  using vmeta = meta::member::boring_cxx_name;
  auto vname = fatal::to_instance<std::string, vmeta::name>();
  EXPECT_EQ("fancy.idl.name", vname);
}

} // namespace cpp_reflection
} // namespace test_cpp2
