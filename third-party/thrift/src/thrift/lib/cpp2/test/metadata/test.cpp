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

#include <gtest/gtest.h>

#include <thrift/lib/cpp2/test/metadata/gen-cpp2/AnotherTestService.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/EnumTestService.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/ExceptionTestService.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/IncludeTestService.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/MyTestService.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/MyTestServiceWithUri.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/NestedStructsTestService.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/ParentService.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/RepeatedTestService.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/SimpleStructsTestService.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/StreamTestService.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/StructUnionTestService.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/TypedefTestService.h>
#include <thrift/lib/thrift/gen-cpp2/metadata_types.h>

namespace {
using apache::thrift::metadata::findStructuredAnnotationOrThrow;
using apache::thrift::metadata::ThriftConstStruct;
using apache::thrift::metadata::ThriftConstValue;
using apache::thrift::metadata::ThriftConstValuePair;
using apache::thrift::metadata::ThriftMetadata;
using apache::thrift::metadata::ThriftPrimitiveType;
using apache::thrift::metadata::ThriftServiceMetadataResponse;
using apache::thrift::metadata::ThriftType;

auto cons(std::string data, std::optional<ThriftConstValue> next = {}) {
  ThriftConstStruct s;
  s.type()->name() = "simple_structs_test.Nat";
  ThriftConstValue cvData;
  cvData.cv_string() = std::move(data);
  s.fields()->emplace("data", std::move(cvData));
  if (next) {
    s.fields()->emplace("next", std::move(*next));
  }
  ThriftConstValue cv;
  cv.cv_struct() = std::move(s);
  return cv;
}

auto pair(int64_t i, std::string s) {
  ThriftConstValue k;
  k.cv_integer() = i;
  ThriftConstValue v;
  v.cv_string() = std::move(s);
  ThriftConstValuePair p;
  p.key() = std::move(k);
  p.value() = std::move(v);
  return p;
}

class ServiceMetadataTest : public testing::Test {
 protected:
  template <typename T>
  const ThriftMetadata& getMetadata() {
    T svif;
    svif.getProcessor()->getServiceMetadata(response_);
    return *response_.metadata_ref();
  }
  ThriftServiceMetadataResponse response_;
  void resetResponse() { response_ = ThriftServiceMetadataResponse{}; }
};

TEST_F(ServiceMetadataTest, StreamTest) {
  auto& metadata = getMetadata<apache::thrift::ServiceHandler<
      metadata::test::stream::StreamTestService>>();
  EXPECT_EQ(response_.services()->front().module()->name(), "stream_test");
  auto s = metadata.services()->at("stream_test.StreamTestService");
  EXPECT_EQ(*s.name(), "stream_test.StreamTestService");
  EXPECT_EQ(*s.functions()->at(0).name(), "responseAndRange");
  EXPECT_EQ(
      s.functions()->at(0).return_type()->getType(),
      ThriftType::Type::t_stream);
  EXPECT_EQ(
      s.functions()
          ->at(0)
          .return_type()
          ->get_t_stream()
          .initialResponseType()
          ->getType(),
      ThriftType::Type::t_primitive);
}

TEST_F(ServiceMetadataTest, EnumTest) {
  auto& metadata = getMetadata<
      apache::thrift::ServiceHandler<metadata::test::enums::EnumTestService>>();
  EXPECT_EQ(response_.services()->front().module()->name(), "enum_test");
  auto e = metadata.enums()->at("enum_test.Continent");
  EXPECT_EQ(*e.name(), "enum_test.Continent");
  EXPECT_EQ(e.elements()->at(1), "NorthAmerica");
  EXPECT_EQ(e.elements()->at(2), "SouthAmerica");
  EXPECT_EQ(e.elements()->at(3), "Europe");
  EXPECT_EQ(e.elements()->at(4), "Asia");
  EXPECT_EQ(e.elements()->at(5), "Africa");
  EXPECT_EQ(e.elements()->at(6), "Oceania");
  EXPECT_EQ(e.elements()->at(7), "Antarctica");
}

TEST_F(ServiceMetadataTest, ExceptionTest) {
  auto& metadata = getMetadata<apache::thrift::ServiceHandler<
      metadata::test::exceptions::ExceptionTestService>>();
  EXPECT_EQ(response_.services()->front().module()->name(), "exception_test");
  auto ex = metadata.exceptions()->at("exception_test.RuntimeException");
  EXPECT_EQ(*ex.name(), "exception_test.RuntimeException");
  EXPECT_EQ(*ex.fields()[0].id(), 1);
  EXPECT_EQ(*ex.fields()[0].name(), "reason");
  EXPECT_EQ(ex.fields()[0].type()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      ex.fields()[0].type()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_STRING_TYPE);
  EXPECT_EQ(*ex.fields()[1].id(), 2);
  EXPECT_EQ(*ex.fields()[1].name(), "level");
  EXPECT_EQ(ex.fields()[1].type()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      ex.fields()[1].type()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_I32_TYPE);
}

TEST_F(ServiceMetadataTest, SimpleStructsTest) {
  auto& metadata = getMetadata<apache::thrift::ServiceHandler<
      metadata::test::simple_structs::SimpleStructsTestService>>();
  EXPECT_EQ(
      response_.services()->front().module()->name(), "simple_structs_test");

  auto s1 = metadata.structs()->at("simple_structs_test.Country");
  EXPECT_EQ(*s1.name(), "simple_structs_test.Country");
  EXPECT_EQ(*s1.fields()[0].id(), 1);
  EXPECT_EQ(*s1.fields()[0].name(), "name");
  EXPECT_EQ(s1.fields()[0].type()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s1.fields()[0].type()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_STRING_TYPE);
  EXPECT_EQ(*s1.fields()[1].id(), 3);
  EXPECT_EQ(*s1.fields()[1].name(), "capital");
  EXPECT_EQ(s1.fields()[1].type()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s1.fields()[1].type()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_STRING_TYPE);
  EXPECT_EQ(*s1.fields()[2].id(), 10);
  EXPECT_EQ(*s1.fields()[2].name(), "population");
  EXPECT_EQ(s1.fields()[2].type()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s1.fields()[2].type()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_DOUBLE_TYPE);

  EXPECT_EQ(*s1.fields()[3].id(), 4);
  EXPECT_EQ(*s1.fields()[3].name(), "abbreviations");
  EXPECT_EQ(s1.fields()[3].type()->getType(), ThriftType::Type::t_typedef);

  // structured annotation of the field
  EXPECT_EQ(
      findStructuredAnnotationOrThrow(
          *s1.fields()[3].structured_annotations(), "simple_structs_test.Nat"),
      *cons("Abbr_field").cv_struct());
  EXPECT_EQ(s1.fields()[3].structured_annotations()->size(), 1);

  // structured annotation of the typedef
  auto tf = s1.fields()[3].type()->get_t_typedef();
  EXPECT_EQ(tf.name(), "simple_structs_test.Abbreviations");
  EXPECT_EQ(
      findStructuredAnnotationOrThrow(
          *tf.structured_annotations(), "simple_structs_test.Nat"),
      *cons("Abbr_typedef").cv_struct());
  EXPECT_EQ(tf.structured_annotations()->size(), 1);

  auto s2 = metadata.structs()->at("simple_structs_test.City");
  EXPECT_EQ(*s2.name(), "simple_structs_test.City");
  EXPECT_EQ(s2.structured_annotations()->size(), 1);
  EXPECT_EQ(
      findStructuredAnnotationOrThrow(
          *s2.structured_annotations(), "simple_structs_test.Nat"),
      *cons("struct").cv_struct());
  EXPECT_EQ(*s2.fields()[0].id(), 1);
  EXPECT_EQ(*s2.fields()[0].name(), "name");
  EXPECT_EQ(s2.fields()[0].type()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s2.fields()[0].type()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_STRING_TYPE);
  EXPECT_EQ(s2.fields()[0].structured_annotations()->size(), 1);
  ThriftConstStruct cs;
  cs.type()->name() = "simple_structs_test.Map";
  ThriftConstValue m;
  m.cv_map().ensure().push_back(pair(0, "0"));
  m.cv_map()->push_back(pair(1, "1"));
  cs.fields()->emplace("value", std::move(m));
  EXPECT_EQ(
      findStructuredAnnotationOrThrow(
          *s2.fields()[0].structured_annotations(), "simple_structs_test.Map"),
      cs);

  EXPECT_EQ(*s2.fields()[1].id(), 2);
  EXPECT_EQ(*s2.fields()[1].name(), "country");
  EXPECT_EQ(s2.fields()[1].type()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s2.fields()[1].type()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_STRING_TYPE);
  EXPECT_EQ(s2.fields()[1].structured_annotations()->size(), 1);
  EXPECT_EQ(
      findStructuredAnnotationOrThrow(
          *s2.fields()[1].structured_annotations(), "simple_structs_test.Nat"),
      *cons("2", cons("1", cons("0"))).cv_struct());

  EXPECT_EQ(*s2.fields()[2].id(), 3);
  EXPECT_EQ(*s2.fields()[2].name(), "population");
  EXPECT_EQ(s2.fields()[2].type()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s2.fields()[2].type()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_DOUBLE_TYPE);
}

TEST_F(ServiceMetadataTest, StructUnionTest) {
  auto& metadata = getMetadata<apache::thrift::ServiceHandler<
      metadata::test::struct_union::StructUnionTestService>>();
  EXPECT_EQ(
      response_.services()->front().module()->name(), "struct_union_test");

  auto s1 = metadata.structs()->at("struct_union_test.Dog");
  EXPECT_EQ(*s1.name(), "struct_union_test.Dog");
  EXPECT_EQ(*s1.is_union(), false);

  auto s2 = metadata.structs()->at("struct_union_test.Cat");
  EXPECT_EQ(*s2.name(), "struct_union_test.Cat");
  EXPECT_EQ(*s2.is_union(), false);

  auto u1 = metadata.structs()->at("struct_union_test.Pet");
  EXPECT_EQ(*u1.name(), "struct_union_test.Pet");
  EXPECT_EQ(*u1.is_union(), true);
  EXPECT_EQ(*u1.fields()[0].id(), 1);
  EXPECT_EQ(*u1.fields()[0].name(), "dog");
  EXPECT_EQ(u1.fields()[0].type()->getType(), ThriftType::Type::t_struct);
  EXPECT_EQ(
      *u1.fields()[0].type()->get_t_struct().name(), "struct_union_test.Dog");
  EXPECT_EQ(*u1.fields()[1].id(), 2);
  EXPECT_EQ(*u1.fields()[1].name(), "cat");
  EXPECT_EQ(u1.fields()[1].type()->getType(), ThriftType::Type::t_struct);
  EXPECT_EQ(
      *u1.fields()[1].type()->get_t_struct().name(), "struct_union_test.Cat");
}

TEST_F(ServiceMetadataTest, NestedStructsTest) {
  auto& metadata = getMetadata<apache::thrift::ServiceHandler<
      metadata::test::nested_structs::NestedStructsTestService>>();
  EXPECT_EQ(
      response_.services()->front().module()->name(), "nested_structs_test");
  auto e1 = metadata.enums()->at("nested_structs_test.Continent");
  EXPECT_EQ(*e1.name(), "nested_structs_test.Continent");
  EXPECT_EQ(e1.elements()->at(1), "NorthAmerica");
  EXPECT_EQ(e1.elements()->at(2), "SouthAmerica");
  EXPECT_EQ(e1.elements()->at(3), "Europe");
  EXPECT_EQ(e1.elements()->at(4), "Asia");
  EXPECT_EQ(e1.elements()->at(5), "Africa");
  EXPECT_EQ(e1.elements()->at(6), "Oceania");
  EXPECT_EQ(e1.elements()->at(7), "Antarctica");

  auto s1 = metadata.structs()->at("nested_structs_test.Country");
  EXPECT_EQ(*s1.name(), "nested_structs_test.Country");
  EXPECT_EQ(*s1.fields()[0].id(), 1);
  EXPECT_EQ(*s1.fields()[0].name(), "name");
  EXPECT_EQ(s1.fields()[0].type()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s1.fields()[0].type()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_STRING_TYPE);
  EXPECT_EQ(*s1.fields()[1].id(), 2);
  EXPECT_EQ(*s1.fields()[1].name(), "continent");
  EXPECT_EQ(s1.fields()[1].type()->getType(), ThriftType::Type::t_enum);
  EXPECT_EQ(
      *s1.fields()[1].type()->get_t_enum().name(),
      "nested_structs_test.Continent");
  EXPECT_EQ(*s1.fields()[2].id(), 3);
  EXPECT_EQ(*s1.fields()[2].name(), "capital");
  EXPECT_EQ(s1.fields()[2].type()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s1.fields()[2].type()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_STRING_TYPE);
  EXPECT_EQ(*s1.fields()[3].id(), 4);
  EXPECT_EQ(*s1.fields()[3].name(), "population");
  EXPECT_EQ(s1.fields()[3].type()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s1.fields()[3].type()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_DOUBLE_TYPE);

  auto s2 = metadata.structs()->at("nested_structs_test.City");
  EXPECT_EQ(*s2.name(), "nested_structs_test.City");
  EXPECT_EQ(*s2.fields()[0].id(), 1);
  EXPECT_EQ(*s2.fields()[0].name(), "name");
  EXPECT_EQ(s2.fields()[0].type()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s2.fields()[0].type()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_STRING_TYPE);
  EXPECT_EQ(*s2.fields()[1].id(), 2);
  EXPECT_EQ(*s2.fields()[1].name(), "country");
  EXPECT_EQ(s2.fields()[1].type()->getType(), ThriftType::Type::t_struct);
  EXPECT_EQ(
      *s2.fields()[1].type()->get_t_struct().name(),
      "nested_structs_test.Country");

  auto s3 = metadata.structs()->at("nested_structs_test.Foo");
  EXPECT_EQ(*s3.name(), "nested_structs_test.Foo");
  EXPECT_EQ(*s3.fields()[0].id(), 1);
  EXPECT_EQ(*s3.fields()[0].name(), "bar");
  EXPECT_EQ(s3.fields()[0].type()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s3.fields()[0].type()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_STRING_TYPE);
  EXPECT_EQ(*s3.fields()[1].id(), 2);
  EXPECT_EQ(*s3.fields()[1].name(), "foos");
  auto listType = *s3.fields()[1].type();
  EXPECT_EQ(listType.getType(), ThriftType::Type::t_list);
  auto elemType = listType.get_t_list().valueType().get();
  EXPECT_EQ(*elemType->get_t_struct().name(), "nested_structs_test.Foo");
}

TEST_F(ServiceMetadataTest, IncludeTest) {
  auto& metadata = getMetadata<apache::thrift::ServiceHandler<
      metadata::test::include::IncludeTestService>>();
  EXPECT_EQ(response_.services()->front().module()->name(), "include_test");

  // In-file enums
  auto e1 = metadata.enums()->at("include_test.Animal");
  EXPECT_EQ(*e1.name(), "include_test.Animal");
  EXPECT_EQ(e1.elements()[1], "Dog");
  EXPECT_EQ(e1.elements()[2], "Cat");
  EXPECT_EQ(e1.elements()[3], "Horse");
  EXPECT_EQ(e1.elements()[4], "Cow");
  EXPECT_EQ(e1.elements()[5], "Bear");

  // In-file structs
  auto s1 = metadata.structs()->at("include_test.Example");
  EXPECT_EQ(*s1.name(), "include_test.Example");
  EXPECT_EQ(*s1.fields()[0].id(), 1);
  EXPECT_EQ(*s1.fields()[0].name(), "includedEnum");
  EXPECT_EQ(s1.fields()[0].type()->getType(), ThriftType::Type::t_enum);
  EXPECT_EQ(*s1.fields()[0].type()->get_t_enum().name(), "enum_test.Continent");
  EXPECT_EQ(*s1.fields()[1].id(), 2);
  EXPECT_EQ(*s1.fields()[1].name(), "includedException");
  EXPECT_EQ(s1.fields()[1].type()->getType(), ThriftType::Type::t_struct);
  EXPECT_EQ(
      *s1.fields()[1].type()->get_t_struct().name(),
      "exception_test.RuntimeException");
  EXPECT_EQ(*s1.fields()[2].id(), 3);
  EXPECT_EQ(*s1.fields()[2].name(), "includedStruct");
  EXPECT_EQ(s1.fields()[2].type()->getType(), ThriftType::Type::t_struct);
  EXPECT_EQ(
      *s1.fields()[2].type()->get_t_struct().name(),
      "simple_structs_test.Country");
  EXPECT_EQ(*s1.fields()[3].id(), 4);
  EXPECT_EQ(*s1.fields()[3].name(), "includedTypedef");
  EXPECT_EQ(s1.fields()[3].type()->getType(), ThriftType::Type::t_typedef);
  EXPECT_EQ(
      *s1.fields()[3].type()->get_t_typedef().name(), "typedef_test.StringMap");
  auto utype1 = s1.fields()[3].type()->get_t_typedef().underlyingType().get();
  EXPECT_EQ(utype1->getType(), ThriftType::Type::t_map);
  auto keyType1 = utype1->get_t_map().keyType().get();
  EXPECT_EQ(keyType1->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      keyType1->get_t_primitive(), ThriftPrimitiveType::THRIFT_STRING_TYPE);
  auto valType1 = utype1->get_t_map().valueType().get();
  EXPECT_EQ(valType1->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      valType1->get_t_primitive(), ThriftPrimitiveType::THRIFT_STRING_TYPE);
  EXPECT_EQ(*s1.fields()[4].id(), 5);
  EXPECT_EQ(*s1.fields()[4].name(), "coolString");
  EXPECT_EQ(s1.fields()[4].type()->getType(), ThriftType::Type::t_typedef);
  EXPECT_EQ(
      *s1.fields()[4].type()->get_t_typedef().name(),
      "include_test.CoolString");
  auto utype2 = s1.fields()[4].type()->get_t_typedef().underlyingType().get();
  EXPECT_EQ(utype2->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(utype2->get_t_primitive(), ThriftPrimitiveType::THRIFT_STRING_TYPE);

  // Included enums
  auto e2 = metadata.enums()->at("enum_test.Continent");
  EXPECT_EQ(*e2.name(), "enum_test.Continent");
  EXPECT_EQ(e2.elements()[1], "NorthAmerica");
  EXPECT_EQ(e2.elements()[2], "SouthAmerica");
  EXPECT_EQ(e2.elements()[3], "Europe");
  EXPECT_EQ(e2.elements()[4], "Asia");
  EXPECT_EQ(e2.elements()[5], "Africa");
  EXPECT_EQ(e2.elements()[6], "Oceania");
  EXPECT_EQ(e2.elements()[7], "Antarctica");

  // Included structs
  auto s2 = metadata.structs()->at("simple_structs_test.Country");
  EXPECT_EQ(*s2.name(), "simple_structs_test.Country");
  EXPECT_EQ(*s2.fields()[0].id(), 1);
  EXPECT_EQ(*s2.fields()[0].name(), "name");
  EXPECT_EQ(s2.fields()[0].type()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s2.fields()[0].type()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_STRING_TYPE);
  EXPECT_EQ(*s2.fields()[1].id(), 3);
  EXPECT_EQ(*s2.fields()[1].name(), "capital");
  EXPECT_EQ(s2.fields()[1].type()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s2.fields()[1].type()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_STRING_TYPE);
  EXPECT_EQ(*s2.fields()[2].id(), 10);
  EXPECT_EQ(*s2.fields()[2].name(), "population");
  EXPECT_EQ(s2.fields()[2].type()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s2.fields()[2].type()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_DOUBLE_TYPE);

  auto s3 = metadata.structs()->at("simple_structs_test.City");
  EXPECT_EQ(*s3.name(), "simple_structs_test.City");
  EXPECT_EQ(*s3.fields()[0].id(), 1);
  EXPECT_EQ(*s3.fields()[0].name(), "name");
  EXPECT_EQ(s3.fields()[0].type()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s3.fields()[0].type()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_STRING_TYPE);
  EXPECT_EQ(*s3.fields()[1].id(), 2);
  EXPECT_EQ(*s3.fields()[1].name(), "country");
  EXPECT_EQ(s3.fields()[1].type()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s3.fields()[1].type()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_STRING_TYPE);
  EXPECT_EQ(*s3.fields()[2].id(), 3);
  EXPECT_EQ(*s3.fields()[2].name(), "population");
  EXPECT_EQ(s3.fields()[2].type()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s3.fields()[2].type()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_DOUBLE_TYPE);

  // Included exceptions
  auto ex1 = metadata.exceptions()->at("exception_test.RuntimeException");
  EXPECT_EQ(*ex1.name(), "exception_test.RuntimeException");
  EXPECT_EQ(*ex1.fields()[0].id(), 1);
  EXPECT_EQ(*ex1.fields()[0].name(), "reason");
  EXPECT_EQ(ex1.fields()[0].type()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      ex1.fields()[0].type()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_STRING_TYPE);
  EXPECT_EQ(*ex1.fields()[1].id(), 2);
  EXPECT_EQ(*ex1.fields()[1].name(), "level");
  EXPECT_EQ(ex1.fields()[1].type()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      ex1.fields()[1].type()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_I32_TYPE);
}

TEST_F(ServiceMetadataTest, TypedefTest) {
  auto& metadata = getMetadata<apache::thrift::ServiceHandler<
      metadata::test::typedefs::TypedefTestService>>();
  EXPECT_EQ(response_.services()->front().module()->name(), "typedef_test");

  auto s1 = metadata.structs()->at("typedef_test.Types");
  EXPECT_EQ(*s1.name(), "typedef_test.Types");

  // map<string,string>
  EXPECT_EQ(*s1.fields()[0].id(), 1);
  EXPECT_EQ(*s1.fields()[0].name(), "stringMap");
  EXPECT_EQ(s1.fields()[0].type()->getType(), ThriftType::Type::t_typedef);
  EXPECT_EQ(
      *s1.fields()[0].type()->get_t_typedef().name(), "typedef_test.StringMap");
  auto utype1 = s1.fields()[0].type()->get_t_typedef().underlyingType().get();
  EXPECT_EQ(utype1->getType(), ThriftType::Type::t_map);
  auto keyType1 = utype1->get_t_map().keyType().get();
  EXPECT_EQ(keyType1->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      keyType1->get_t_primitive(), ThriftPrimitiveType::THRIFT_STRING_TYPE);
  auto valType1 = utype1->get_t_map().valueType().get();
  EXPECT_EQ(valType1->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      valType1->get_t_primitive(), ThriftPrimitiveType::THRIFT_STRING_TYPE);

  // list<map<i32,string>>
  EXPECT_EQ(*s1.fields()[1].id(), 2);
  EXPECT_EQ(*s1.fields()[1].name(), "mapList");
  EXPECT_EQ(s1.fields()[1].type()->getType(), ThriftType::Type::t_typedef);
  EXPECT_EQ(
      *s1.fields()[1].type()->get_t_typedef().name(), "typedef_test.MapList");
  auto utype2 = s1.fields()[1].type()->get_t_typedef().underlyingType().get();
  EXPECT_EQ(utype2->getType(), ThriftType::Type::t_list);
  auto elemType = utype2->get_t_list().valueType().get();
  EXPECT_EQ(elemType->getType(), ThriftType::Type::t_map);
  auto elemKey = elemType->get_t_map().keyType().get();
  EXPECT_EQ(elemKey->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(elemKey->get_t_primitive(), ThriftPrimitiveType::THRIFT_I32_TYPE);
  auto elemValue = elemType->get_t_map().valueType().get();
  EXPECT_EQ(elemValue->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      elemValue->get_t_primitive(), ThriftPrimitiveType::THRIFT_STRING_TYPE);

  // map<list<string>, map<i32, list<i64>>>
  EXPECT_EQ(*s1.fields()[2].id(), 3);
  EXPECT_EQ(*s1.fields()[2].name(), "veryComplex");
  EXPECT_EQ(s1.fields()[2].type()->getType(), ThriftType::Type::t_typedef);
  EXPECT_EQ(
      *s1.fields()[2].type()->get_t_typedef().name(),
      "typedef_test.VeryComplex");
  auto utype3 = s1.fields()[2].type()->get_t_typedef().underlyingType().get();
  EXPECT_EQ(utype3->getType(), ThriftType::Type::t_map);
  // Map key
  auto keyType3 = utype3->get_t_map().keyType().get();
  EXPECT_EQ(keyType3->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      keyType3->get_t_primitive(), ThriftPrimitiveType::THRIFT_STRING_TYPE);
  // Map value (also a map)
  auto valueType3 = utype3->get_t_map().valueType().get();
  EXPECT_EQ(valueType3->getType(), ThriftType::Type::t_map);
  // Inner map's key (i32)
  auto valueKeyType = valueType3->get_t_map().keyType().get();
  EXPECT_EQ(valueKeyType->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      valueKeyType->get_t_primitive(), ThriftPrimitiveType::THRIFT_I32_TYPE);
  // Inner map's value (list<i64>)
  auto valueValType = valueType3->get_t_map().valueType().get();
  EXPECT_EQ(valueValType->getType(), ThriftType::Type::t_list);
  auto valueValElemType = valueValType->get_t_list().valueType().get();
  EXPECT_EQ(valueValElemType->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      valueValElemType->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_I64_TYPE);
}

TEST_F(ServiceMetadataTest, ServicesAndContextPopulated) {
  auto& metadata = getMetadata<apache::thrift::ServiceHandler<
      metadata::test::services::MyTestService>>();
  EXPECT_EQ(response_.services()->size(), 2);
  auto& derived = response_.services()->front();
  auto& base = response_.services()->back();

  EXPECT_EQ(
      *derived.service_name(), *response_.context()->service_info()->name());
  EXPECT_EQ(*derived.module()->name(), *response_.context()->module()->name());

  EXPECT_EQ(
      *metadata.services()->at(*derived.service_name()).name(),
      "service_test.MyTestService");
  EXPECT_EQ(
      *metadata.services()->at(*base.service_name()).name(),
      "service_test.ParentService");
}

TEST_F(ServiceMetadataTest, ServiceTest) {
  auto& metadata = getMetadata<apache::thrift::ServiceHandler<
      metadata::test::services::MyTestService>>();
  EXPECT_EQ(response_.services()->front().module()->name(), "service_test");

  const auto& p = metadata.services()->at("service_test.ParentService");
  EXPECT_EQ(*p.name(), "service_test.ParentService");
  EXPECT_TRUE(p.uri()->empty());
  EXPECT_EQ(p.structured_annotations()->size(), 1);
  EXPECT_EQ(
      findStructuredAnnotationOrThrow(
          *p.structured_annotations(), "simple_structs_test.Nat"),
      *cons("service").cv_struct());
  EXPECT_EQ(p.functions()->size(), 1);
  EXPECT_EQ(apache::thrift::get_pointer(p.parent()), nullptr);

  const auto& f = p.functions()[0];
  EXPECT_EQ(*f.name(), "parentFun");
  EXPECT_EQ(f.structured_annotations()->size(), 1);
  EXPECT_EQ(
      findStructuredAnnotationOrThrow(
          *f.structured_annotations(), "simple_structs_test.Nat"),
      *cons("function").cv_struct());
  EXPECT_EQ(f.return_type()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      f.return_type()->get_t_primitive(), ThriftPrimitiveType::THRIFT_I32_TYPE);
  EXPECT_EQ(f.arguments()->size(), 0);
  EXPECT_EQ(f.exceptions()->size(), 0);

  const auto& s =
      metadata.services()->at(*response_.services()->front().service_name());
  EXPECT_EQ(*s.name(), "service_test.MyTestService");
  EXPECT_TRUE(p.uri()->empty());
  EXPECT_EQ(s.functions()->size(), 3);
  EXPECT_EQ(
      *apache::thrift::get_pointer(s.parent()), "service_test.ParentService");

  const auto& f0 = s.functions()[0];
  EXPECT_EQ(*f0.name(), "getAllTypes");
  EXPECT_EQ(f0.return_type()->getType(), ThriftType::Type::t_list);
  auto retType1 = f0.return_type()->get_t_list();
  auto elemType = retType1.valueType().get();
  EXPECT_EQ(elemType->getType(), ThriftType::Type::t_struct);
  EXPECT_EQ(*elemType->get_t_struct().name(), "typedef_test.Types");
  EXPECT_EQ(f0.arguments()->size(), 0);
  EXPECT_EQ(f0.exceptions()->size(), 0);
  EXPECT_FALSE(*f0.is_oneway());

  const auto& f1 = s.functions()[1];
  EXPECT_EQ(*f1.name(), "getType");
  EXPECT_EQ(f1.return_type()->getType(), ThriftType::Type::t_struct);
  EXPECT_EQ(*f1.return_type()->get_t_struct().name(), "typedef_test.Types");
  EXPECT_EQ(*f1.arguments()[0].id(), 1);
  EXPECT_EQ(*f1.arguments()[0].name(), "stringMap");
  EXPECT_EQ(*f1.arguments()[0].is_optional(), false);
  EXPECT_EQ(f1.arguments()[0].type()->getType(), ThriftType::Type::t_typedef);
  EXPECT_EQ(
      *f1.arguments()[0].type()->get_t_typedef().name(),
      "typedef_test.StringMap");
  EXPECT_EQ(f1.arguments()[0].structured_annotations()->size(), 1);
  EXPECT_EQ(
      findStructuredAnnotationOrThrow(
          *f1.arguments()[0].structured_annotations(),
          "simple_structs_test.Nat"),
      *cons("argument").cv_struct());
  EXPECT_EQ(*f1.exceptions()[0].id(), 1);
  EXPECT_EQ(*f1.exceptions()[0].name(), "ex");
  EXPECT_EQ(*f1.exceptions()[0].is_optional(), false);
  EXPECT_EQ(f1.exceptions()[0].type()->getType(), ThriftType::Type::t_struct);
  EXPECT_EQ(
      *f1.exceptions()[0].type()->get_t_struct().name(),
      "service_test.CutoffException");
  EXPECT_FALSE(*f1.is_oneway());

  const auto& f2 = s.functions()[2];
  EXPECT_EQ(*f2.name(), "noReturn");
  EXPECT_TRUE(*f2.is_oneway());
}

TEST_F(ServiceMetadataTest, ServiceNameTestWithUri) {
  auto& metadata = getMetadata<apache::thrift::ServiceHandler<
      metadata::test::services::MyTestServiceWithUri>>();
  EXPECT_EQ(
      response_.services()->front().module()->name(),
      "service_test_with_package_name");

  const auto& p = metadata.services()->at(
      "service_test_with_package_name.ParentServiceWithUri");
  EXPECT_EQ(*p.name(), "service_test_with_package_name.ParentServiceWithUri");
  EXPECT_FALSE(p.uri()->empty());
  EXPECT_EQ(*p.uri(), "meta.com/metadata/test/ParentServiceWithUri");
  EXPECT_EQ(p.functions()->size(), 1);
  EXPECT_EQ(apache::thrift::get_pointer(p.parent()), nullptr);

  const auto& f = p.functions()[0];
  EXPECT_EQ(*f.name(), "parentFun");
  EXPECT_EQ(f.return_type()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      f.return_type()->get_t_primitive(), ThriftPrimitiveType::THRIFT_I32_TYPE);
  EXPECT_EQ(f.arguments()->size(), 0);
  EXPECT_EQ(f.exceptions()->size(), 0);

  const auto& s =
      metadata.services()->at(*response_.services()->front().service_name());
  EXPECT_EQ(*s.name(), "service_test_with_package_name.MyTestServiceWithUri");
  EXPECT_FALSE(s.uri()->empty());
  EXPECT_EQ(*s.uri(), "meta.com/metadata/test/MyTestServiceWithUri");
  EXPECT_EQ(s.functions()->size(), 3);
  EXPECT_EQ(
      *apache::thrift::get_pointer(s.parent()),
      "service_test_with_package_name.ParentServiceWithUri");

  const auto& f0 = s.functions()[0];
  EXPECT_EQ(*f0.name(), "getAllTypes");
  EXPECT_EQ(f0.return_type()->getType(), ThriftType::Type::t_list);
  auto retType1 = f0.return_type()->get_t_list();
  auto elemType = retType1.valueType().get();
  EXPECT_EQ(elemType->getType(), ThriftType::Type::t_struct);
  EXPECT_EQ(*elemType->get_t_struct().name(), "typedef_test.Types");
  EXPECT_EQ(f0.arguments()->size(), 0);
  EXPECT_EQ(f0.exceptions()->size(), 0);
  EXPECT_FALSE(*f0.is_oneway());

  const auto& f1 = s.functions()[1];
  EXPECT_EQ(*f1.name(), "getStr");
  EXPECT_EQ(f1.return_type()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      f1.return_type()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_STRING_TYPE);
  EXPECT_EQ(*f1.arguments()[0].id(), 1);
  EXPECT_EQ(*f1.arguments()[0].name(), "input");
  EXPECT_EQ(*f1.arguments()[0].is_optional(), false);
  EXPECT_EQ(f1.arguments()[0].type()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      f1.arguments()[0].type()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_I32_TYPE);
  EXPECT_FALSE(*f1.is_oneway());

  const auto& f2 = s.functions()[2];
  EXPECT_EQ(*f2.name(), "noReturn");
  EXPECT_TRUE(*f2.is_oneway());
}

TEST_F(ServiceMetadataTest, RepeatedTest) {
  for (size_t i = 0; i < 3; i++) {
    resetResponse();
    auto& metadata = getMetadata<apache::thrift::ServiceHandler<
        metadata::test::repeated::RepeatedTestService>>();
    EXPECT_EQ(response_.services()->front().module()->name(), "repeated");

    const auto& s =
        metadata.services()->at(*response_.services()->front().service_name());
    EXPECT_EQ(*s.name(), "repeated.RepeatedTestService");
    EXPECT_EQ(s.functions()->size(), 1);
    EXPECT_EQ(*s.functions()[0].name(), "addValue");
    auto it = metadata.enums()->find("repeated.ValueEnum");
    EXPECT_NE(it, metadata.enums()->end());
    EXPECT_EQ(
        *s.functions()->at(0).arguments()->at(0).type()->get_t_struct().name(),
        "repeated.AddValueRequest");
    EXPECT_EQ(
        *metadata.structs()
             ->at("repeated.AddValueRequest")
             .fields()
             ->at(0)
             .type()
             ->get_t_enum()
             .name(),
        "repeated.ValueEnum");
  }
}

TEST_F(ServiceMetadataTest, NoNamespaceTest) {
  auto& metadata =
      getMetadata<apache::thrift::ServiceHandler<cpp2::AnotherTestService>>();
  EXPECT_EQ(response_.services()->front().module()->name(), "no_namespace");

  auto s = metadata.structs()->at("no_namespace.MyData");
  EXPECT_EQ(s.fields()->size(), 1);
}
} // namespace
