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

#include <folly/portability/GTest.h>

#include <thrift/lib/cpp2/test/metadata/gen-cpp2/AnotherTestService.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/EnumTestService.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/ExceptionTestService.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/IncludeTestService.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/MyTestService.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/NestedStructsTestService.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/ParentService.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/RepeatedTestService.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/SimpleStructsTestService.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/StructUnionTestService.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/TypedefTestService.h>
#include <thrift/lib/thrift/gen-cpp2/metadata_types.h>

namespace {
using apache::thrift::metadata::ThriftConstStruct;
using apache::thrift::metadata::ThriftConstValue;
using apache::thrift::metadata::ThriftConstValuePair;
using apache::thrift::metadata::ThriftMetadata;
using apache::thrift::metadata::ThriftPrimitiveType;
using apache::thrift::metadata::ThriftServiceMetadataResponse;
using apache::thrift::metadata::ThriftType;

void check_unstructured_metadata(
    const std::map<std::string, std::string>& meta, const std::string& prefix) {
  EXPECT_EQ(meta.size(), 2);
  EXPECT_EQ(meta.at(prefix + "_foo"), "1");
  EXPECT_EQ(meta.at(prefix + "_bar"), prefix + "_baz");
}

auto cons(std::string data, std::optional<ThriftConstValue> next = {}) {
  ThriftConstStruct s;
  s.type_ref()->name_ref() = "simple_structs_test.Nat";
  ThriftConstValue cvData;
  cvData.cv_string_ref() = std::move(data);
  s.fields_ref()->emplace("data", std::move(cvData));
  if (next) {
    s.fields_ref()->emplace("next", std::move(*next));
  }
  ThriftConstValue cv;
  cv.cv_struct_ref() = std::move(s);
  return cv;
}

auto pair(int64_t i, std::string s) {
  ThriftConstValue k;
  k.cv_integer_ref() = i;
  ThriftConstValue v;
  v.cv_string_ref() = std::move(s);
  ThriftConstValuePair p;
  p.key_ref() = std::move(k);
  p.value_ref() = std::move(v);
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

TEST_F(ServiceMetadataTest, EnumTest) {
  auto& metadata = getMetadata<
      apache::thrift::ServiceHandler<metadata::test::enums::EnumTestService>>();
  EXPECT_EQ(
      response_.services_ref()->front().module_ref()->name_ref(), "enum_test");
  auto e = metadata.enums_ref()->at("enum_test.Continent");
  EXPECT_EQ(*e.name_ref(), "enum_test.Continent");
  EXPECT_EQ(e.elements_ref()->at(1), "NorthAmerica");
  EXPECT_EQ(e.elements_ref()->at(2), "SouthAmerica");
  EXPECT_EQ(e.elements_ref()->at(3), "Europe");
  EXPECT_EQ(e.elements_ref()->at(4), "Asia");
  EXPECT_EQ(e.elements_ref()->at(5), "Africa");
  EXPECT_EQ(e.elements_ref()->at(6), "Oceania");
  EXPECT_EQ(e.elements_ref()->at(7), "Antarctica");
}

TEST_F(ServiceMetadataTest, ExceptionTest) {
  auto& metadata = getMetadata<apache::thrift::ServiceHandler<
      metadata::test::exceptions::ExceptionTestService>>();
  EXPECT_EQ(
      response_.services_ref()->front().module_ref()->name_ref(),
      "exception_test");
  auto ex = metadata.exceptions_ref()->at("exception_test.RuntimeException");
  EXPECT_EQ(*ex.name_ref(), "exception_test.RuntimeException");
  EXPECT_EQ(*ex.fields_ref()[0].id_ref(), 1);
  EXPECT_EQ(*ex.fields_ref()[0].name_ref(), "reason");
  EXPECT_EQ(
      ex.fields_ref()[0].type_ref()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      ex.fields_ref()[0].type_ref()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_STRING_TYPE);
  EXPECT_EQ(*ex.fields_ref()[1].id_ref(), 2);
  EXPECT_EQ(*ex.fields_ref()[1].name_ref(), "level");
  EXPECT_EQ(
      ex.fields_ref()[1].type_ref()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      ex.fields_ref()[1].type_ref()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_I32_TYPE);
}

TEST_F(ServiceMetadataTest, SimpleStructsTest) {
  auto& metadata = getMetadata<apache::thrift::ServiceHandler<
      metadata::test::simple_structs::SimpleStructsTestService>>();
  EXPECT_EQ(
      response_.services_ref()->front().module_ref()->name_ref(),
      "simple_structs_test");

  auto s1 = metadata.structs_ref()->at("simple_structs_test.Country");
  EXPECT_EQ(*s1.name_ref(), "simple_structs_test.Country");
  EXPECT_EQ(*s1.fields_ref()[0].id_ref(), 1);
  EXPECT_EQ(*s1.fields_ref()[0].name_ref(), "name");
  EXPECT_EQ(
      s1.fields_ref()[0].type_ref()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s1.fields_ref()[0].type_ref()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_STRING_TYPE);
  EXPECT_EQ(*s1.fields_ref()[1].id_ref(), 3);
  EXPECT_EQ(*s1.fields_ref()[1].name_ref(), "capital");
  EXPECT_EQ(
      s1.fields_ref()[1].type_ref()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s1.fields_ref()[1].type_ref()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_STRING_TYPE);
  EXPECT_EQ(*s1.fields_ref()[2].id_ref(), 10);
  EXPECT_EQ(*s1.fields_ref()[2].name_ref(), "population");
  EXPECT_EQ(
      s1.fields_ref()[2].type_ref()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s1.fields_ref()[2].type_ref()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_DOUBLE_TYPE);

  EXPECT_EQ(*s1.fields_ref()[3].id_ref(), 4);
  EXPECT_EQ(*s1.fields_ref()[3].name_ref(), "abbreviations");
  EXPECT_EQ(
      s1.fields_ref()[3].type_ref()->getType(), ThriftType::Type::t_typedef);

  // structured annotation of the field
  EXPECT_EQ(
      s1.fields_ref()[3].structured_annotations()->at(0),
      *cons("Abbr_field").cv_struct_ref());
  EXPECT_EQ(s1.fields_ref()[3].structured_annotations()->size(), 1);

  // structured annotation of the typedef
  auto tf = s1.fields_ref()[3].type_ref()->get_t_typedef();
  EXPECT_EQ(tf.name_ref(), "simple_structs_test.Abbreviations");
  EXPECT_EQ(
      tf.structured_annotations()->at(0),
      *cons("Abbr_typedef").cv_struct_ref());
  EXPECT_EQ(tf.structured_annotations()->size(), 1);

  auto s2 = metadata.structs_ref()->at("simple_structs_test.City");
  EXPECT_EQ(*s2.name_ref(), "simple_structs_test.City");
  EXPECT_EQ(s2.structured_annotations()->size(), 1);
  EXPECT_EQ(
      s2.structured_annotations()->at(0), *cons("struct").cv_struct_ref());
  EXPECT_EQ(*s2.fields_ref()[0].id_ref(), 1);
  EXPECT_EQ(*s2.fields_ref()[0].name_ref(), "name");
  EXPECT_EQ(
      s2.fields_ref()[0].type_ref()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s2.fields_ref()[0].type_ref()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_STRING_TYPE);
  EXPECT_EQ(s2.fields_ref()[0].structured_annotations()->size(), 1);
  ThriftConstStruct cs;
  cs.type_ref()->name_ref() = "simple_structs_test.Map";
  ThriftConstValue m;
  m.cv_map_ref().ensure().push_back(pair(0, "0"));
  m.cv_map_ref()->push_back(pair(1, "1"));
  cs.fields_ref()->emplace("value", std::move(m));
  EXPECT_EQ(s2.fields_ref()[0].structured_annotations()->at(0), cs);

  EXPECT_EQ(*s2.fields_ref()[1].id_ref(), 2);
  EXPECT_EQ(*s2.fields_ref()[1].name_ref(), "country");
  EXPECT_EQ(
      s2.fields_ref()[1].type_ref()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s2.fields_ref()[1].type_ref()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_STRING_TYPE);
  EXPECT_EQ(s2.fields_ref()[1].structured_annotations()->size(), 1);
  EXPECT_EQ(
      s2.fields_ref()[1].structured_annotations()->at(0),
      *cons("2", cons("1", cons("0"))).cv_struct_ref());

  EXPECT_EQ(*s2.fields_ref()[2].id_ref(), 3);
  EXPECT_EQ(*s2.fields_ref()[2].name_ref(), "population");
  EXPECT_EQ(
      s2.fields_ref()[2].type_ref()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s2.fields_ref()[2].type_ref()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_DOUBLE_TYPE);
}

TEST_F(ServiceMetadataTest, StructUnionTest) {
  auto& metadata = getMetadata<apache::thrift::ServiceHandler<
      metadata::test::struct_union::StructUnionTestService>>();
  EXPECT_EQ(
      response_.services_ref()->front().module_ref()->name_ref(),
      "struct_union_test");

  auto s1 = metadata.structs_ref()->at("struct_union_test.Dog");
  EXPECT_EQ(*s1.name_ref(), "struct_union_test.Dog");
  EXPECT_EQ(*s1.is_union_ref(), false);

  auto s2 = metadata.structs_ref()->at("struct_union_test.Cat");
  EXPECT_EQ(*s2.name_ref(), "struct_union_test.Cat");
  EXPECT_EQ(*s2.is_union_ref(), false);

  auto u1 = metadata.structs_ref()->at("struct_union_test.Pet");
  EXPECT_EQ(*u1.name_ref(), "struct_union_test.Pet");
  EXPECT_EQ(*u1.is_union_ref(), true);
  EXPECT_EQ(*u1.fields_ref()[0].id_ref(), 1);
  EXPECT_EQ(*u1.fields_ref()[0].name_ref(), "dog");
  EXPECT_EQ(
      u1.fields_ref()[0].type_ref()->getType(), ThriftType::Type::t_struct);
  EXPECT_EQ(
      *u1.fields_ref()[0].type_ref()->get_t_struct().name_ref(),
      "struct_union_test.Dog");
  EXPECT_EQ(*u1.fields_ref()[1].id_ref(), 2);
  EXPECT_EQ(*u1.fields_ref()[1].name_ref(), "cat");
  EXPECT_EQ(
      u1.fields_ref()[1].type_ref()->getType(), ThriftType::Type::t_struct);
  EXPECT_EQ(
      *u1.fields_ref()[1].type_ref()->get_t_struct().name_ref(),
      "struct_union_test.Cat");
}

TEST_F(ServiceMetadataTest, NestedStructsTest) {
  auto& metadata = getMetadata<apache::thrift::ServiceHandler<
      metadata::test::nested_structs::NestedStructsTestService>>();
  EXPECT_EQ(
      response_.services_ref()->front().module_ref()->name_ref(),
      "nested_structs_test");
  auto e1 = metadata.enums_ref()->at("nested_structs_test.Continent");
  EXPECT_EQ(*e1.name_ref(), "nested_structs_test.Continent");
  EXPECT_EQ(e1.elements_ref()->at(1), "NorthAmerica");
  EXPECT_EQ(e1.elements_ref()->at(2), "SouthAmerica");
  EXPECT_EQ(e1.elements_ref()->at(3), "Europe");
  EXPECT_EQ(e1.elements_ref()->at(4), "Asia");
  EXPECT_EQ(e1.elements_ref()->at(5), "Africa");
  EXPECT_EQ(e1.elements_ref()->at(6), "Oceania");
  EXPECT_EQ(e1.elements_ref()->at(7), "Antarctica");

  auto s1 = metadata.structs_ref()->at("nested_structs_test.Country");
  EXPECT_EQ(*s1.name_ref(), "nested_structs_test.Country");
  EXPECT_EQ(*s1.fields_ref()[0].id_ref(), 1);
  EXPECT_EQ(*s1.fields_ref()[0].name_ref(), "name");
  EXPECT_EQ(
      s1.fields_ref()[0].type_ref()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s1.fields_ref()[0].type_ref()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_STRING_TYPE);
  check_unstructured_metadata(
      *s1.fields_ref()[0].unstructured_annotations(), "field");
  EXPECT_EQ(*s1.fields_ref()[1].id_ref(), 2);
  EXPECT_EQ(*s1.fields_ref()[1].name_ref(), "continent");
  EXPECT_EQ(s1.fields_ref()[1].type_ref()->getType(), ThriftType::Type::t_enum);
  EXPECT_EQ(
      *s1.fields_ref()[1].type_ref()->get_t_enum().name_ref(),
      "nested_structs_test.Continent");
  EXPECT_EQ(*s1.fields_ref()[2].id_ref(), 3);
  EXPECT_EQ(*s1.fields_ref()[2].name_ref(), "capital");
  EXPECT_EQ(
      s1.fields_ref()[2].type_ref()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s1.fields_ref()[2].type_ref()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_STRING_TYPE);
  EXPECT_EQ(*s1.fields_ref()[3].id_ref(), 4);
  EXPECT_EQ(*s1.fields_ref()[3].name_ref(), "population");
  EXPECT_EQ(
      s1.fields_ref()[3].type_ref()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s1.fields_ref()[3].type_ref()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_DOUBLE_TYPE);

  auto s2 = metadata.structs_ref()->at("nested_structs_test.City");
  EXPECT_EQ(*s2.name_ref(), "nested_structs_test.City");
  EXPECT_EQ(*s2.fields_ref()[0].id_ref(), 1);
  EXPECT_EQ(*s2.fields_ref()[0].name_ref(), "name");
  EXPECT_EQ(
      s2.fields_ref()[0].type_ref()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s2.fields_ref()[0].type_ref()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_STRING_TYPE);
  EXPECT_EQ(*s2.fields_ref()[1].id_ref(), 2);
  EXPECT_EQ(*s2.fields_ref()[1].name_ref(), "country");
  EXPECT_EQ(
      s2.fields_ref()[1].type_ref()->getType(), ThriftType::Type::t_struct);
  EXPECT_EQ(
      *s2.fields_ref()[1].type_ref()->get_t_struct().name_ref(),
      "nested_structs_test.Country");

  auto s3 = metadata.structs_ref()->at("nested_structs_test.Foo");
  EXPECT_EQ(*s3.name_ref(), "nested_structs_test.Foo");
  EXPECT_EQ(*s3.fields_ref()[0].id_ref(), 1);
  EXPECT_EQ(*s3.fields_ref()[0].name_ref(), "bar");
  EXPECT_EQ(
      s3.fields_ref()[0].type_ref()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s3.fields_ref()[0].type_ref()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_STRING_TYPE);
  EXPECT_EQ(*s3.fields_ref()[1].id_ref(), 2);
  EXPECT_EQ(*s3.fields_ref()[1].name_ref(), "foos");
  auto listType = *s3.fields_ref()[1].type_ref();
  EXPECT_EQ(listType.getType(), ThriftType::Type::t_list);
  auto elemType = listType.get_t_list().valueType_ref().get();
  auto ttypedef = elemType->get_t_typedef();
  EXPECT_EQ(
      *ttypedef.underlyingType_ref()->get_t_struct().name_ref(),
      "nested_structs_test.Foo");
}

TEST_F(ServiceMetadataTest, IncludeTest) {
  auto& metadata = getMetadata<apache::thrift::ServiceHandler<
      metadata::test::include::IncludeTestService>>();
  EXPECT_EQ(
      response_.services_ref()->front().module_ref()->name_ref(),
      "include_test");

  // In-file enums
  auto e1 = metadata.enums_ref()->at("include_test.Animal");
  EXPECT_EQ(*e1.name_ref(), "include_test.Animal");
  EXPECT_EQ(e1.elements_ref()[1], "Dog");
  EXPECT_EQ(e1.elements_ref()[2], "Cat");
  EXPECT_EQ(e1.elements_ref()[3], "Horse");
  EXPECT_EQ(e1.elements_ref()[4], "Cow");
  EXPECT_EQ(e1.elements_ref()[5], "Bear");

  // In-file structs
  auto s1 = metadata.structs_ref()->at("include_test.Example");
  EXPECT_EQ(*s1.name_ref(), "include_test.Example");
  EXPECT_EQ(*s1.fields_ref()[0].id_ref(), 1);
  EXPECT_EQ(*s1.fields_ref()[0].name_ref(), "includedEnum");
  EXPECT_EQ(s1.fields_ref()[0].type_ref()->getType(), ThriftType::Type::t_enum);
  EXPECT_EQ(
      *s1.fields_ref()[0].type_ref()->get_t_enum().name_ref(),
      "enum_test.Continent");
  EXPECT_EQ(*s1.fields_ref()[1].id_ref(), 2);
  EXPECT_EQ(*s1.fields_ref()[1].name_ref(), "includedException");
  EXPECT_EQ(
      s1.fields_ref()[1].type_ref()->getType(), ThriftType::Type::t_struct);
  EXPECT_EQ(
      *s1.fields_ref()[1].type_ref()->get_t_struct().name_ref(),
      "exception_test.RuntimeException");
  EXPECT_EQ(*s1.fields_ref()[2].id_ref(), 3);
  EXPECT_EQ(*s1.fields_ref()[2].name_ref(), "includedStruct");
  EXPECT_EQ(
      s1.fields_ref()[2].type_ref()->getType(), ThriftType::Type::t_struct);
  EXPECT_EQ(
      *s1.fields_ref()[2].type_ref()->get_t_struct().name_ref(),
      "simple_structs_test.Country");
  EXPECT_EQ(*s1.fields_ref()[3].id_ref(), 4);
  EXPECT_EQ(*s1.fields_ref()[3].name_ref(), "includedTypedef");
  EXPECT_EQ(
      s1.fields_ref()[3].type_ref()->getType(), ThriftType::Type::t_typedef);
  EXPECT_EQ(
      *s1.fields_ref()[3].type_ref()->get_t_typedef().name_ref(),
      "typedef_test.StringMap");
  auto utype1 =
      s1.fields_ref()[3].type_ref()->get_t_typedef().underlyingType_ref().get();
  EXPECT_EQ(utype1->getType(), ThriftType::Type::t_map);
  auto keyType1 = utype1->get_t_map().keyType_ref().get();
  EXPECT_EQ(keyType1->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      keyType1->get_t_primitive(), ThriftPrimitiveType::THRIFT_STRING_TYPE);
  auto valType1 = utype1->get_t_map().valueType_ref().get();
  EXPECT_EQ(valType1->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      valType1->get_t_primitive(), ThriftPrimitiveType::THRIFT_STRING_TYPE);
  EXPECT_EQ(*s1.fields_ref()[4].id_ref(), 5);
  EXPECT_EQ(*s1.fields_ref()[4].name_ref(), "coolString");
  EXPECT_EQ(
      s1.fields_ref()[4].type_ref()->getType(), ThriftType::Type::t_typedef);
  EXPECT_EQ(
      *s1.fields_ref()[4].type_ref()->get_t_typedef().name_ref(),
      "include_test.CoolString");
  auto utype2 =
      s1.fields_ref()[4].type_ref()->get_t_typedef().underlyingType_ref().get();
  EXPECT_EQ(utype2->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(utype2->get_t_primitive(), ThriftPrimitiveType::THRIFT_STRING_TYPE);

  // Included enums
  auto e2 = metadata.enums_ref()->at("enum_test.Continent");
  EXPECT_EQ(*e2.name_ref(), "enum_test.Continent");
  EXPECT_EQ(e2.elements_ref()[1], "NorthAmerica");
  EXPECT_EQ(e2.elements_ref()[2], "SouthAmerica");
  EXPECT_EQ(e2.elements_ref()[3], "Europe");
  EXPECT_EQ(e2.elements_ref()[4], "Asia");
  EXPECT_EQ(e2.elements_ref()[5], "Africa");
  EXPECT_EQ(e2.elements_ref()[6], "Oceania");
  EXPECT_EQ(e2.elements_ref()[7], "Antarctica");

  // Included structs
  auto s2 = metadata.structs_ref()->at("simple_structs_test.Country");
  EXPECT_EQ(*s2.name_ref(), "simple_structs_test.Country");
  EXPECT_EQ(*s2.fields_ref()[0].id_ref(), 1);
  EXPECT_EQ(*s2.fields_ref()[0].name_ref(), "name");
  EXPECT_EQ(
      s2.fields_ref()[0].type_ref()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s2.fields_ref()[0].type_ref()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_STRING_TYPE);
  EXPECT_EQ(*s2.fields_ref()[1].id_ref(), 3);
  EXPECT_EQ(*s2.fields_ref()[1].name_ref(), "capital");
  EXPECT_EQ(
      s2.fields_ref()[1].type_ref()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s2.fields_ref()[1].type_ref()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_STRING_TYPE);
  EXPECT_EQ(*s2.fields_ref()[2].id_ref(), 10);
  EXPECT_EQ(*s2.fields_ref()[2].name_ref(), "population");
  EXPECT_EQ(
      s2.fields_ref()[2].type_ref()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s2.fields_ref()[2].type_ref()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_DOUBLE_TYPE);

  auto s3 = metadata.structs_ref()->at("simple_structs_test.City");
  EXPECT_EQ(*s3.name_ref(), "simple_structs_test.City");
  EXPECT_EQ(*s3.fields_ref()[0].id_ref(), 1);
  EXPECT_EQ(*s3.fields_ref()[0].name_ref(), "name");
  EXPECT_EQ(
      s3.fields_ref()[0].type_ref()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s3.fields_ref()[0].type_ref()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_STRING_TYPE);
  EXPECT_EQ(*s3.fields_ref()[1].id_ref(), 2);
  EXPECT_EQ(*s3.fields_ref()[1].name_ref(), "country");
  EXPECT_EQ(
      s3.fields_ref()[1].type_ref()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s3.fields_ref()[1].type_ref()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_STRING_TYPE);
  EXPECT_EQ(*s3.fields_ref()[2].id_ref(), 3);
  EXPECT_EQ(*s3.fields_ref()[2].name_ref(), "population");
  EXPECT_EQ(
      s3.fields_ref()[2].type_ref()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      s3.fields_ref()[2].type_ref()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_DOUBLE_TYPE);

  // Included exceptions
  auto ex1 = metadata.exceptions_ref()->at("exception_test.RuntimeException");
  EXPECT_EQ(*ex1.name_ref(), "exception_test.RuntimeException");
  EXPECT_EQ(*ex1.fields_ref()[0].id_ref(), 1);
  EXPECT_EQ(*ex1.fields_ref()[0].name_ref(), "reason");
  EXPECT_EQ(
      ex1.fields_ref()[0].type_ref()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      ex1.fields_ref()[0].type_ref()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_STRING_TYPE);
  EXPECT_EQ(*ex1.fields_ref()[1].id_ref(), 2);
  EXPECT_EQ(*ex1.fields_ref()[1].name_ref(), "level");
  EXPECT_EQ(
      ex1.fields_ref()[1].type_ref()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      ex1.fields_ref()[1].type_ref()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_I32_TYPE);
}

TEST_F(ServiceMetadataTest, TypedefTest) {
  auto& metadata = getMetadata<apache::thrift::ServiceHandler<
      metadata::test::typedefs::TypedefTestService>>();
  EXPECT_EQ(
      response_.services_ref()->front().module_ref()->name_ref(),
      "typedef_test");

  auto s1 = metadata.structs_ref()->at("typedef_test.Types");
  EXPECT_EQ(*s1.name_ref(), "typedef_test.Types");

  // map<string,string>
  EXPECT_EQ(*s1.fields_ref()[0].id_ref(), 1);
  EXPECT_EQ(*s1.fields_ref()[0].name_ref(), "stringMap");
  EXPECT_EQ(
      s1.fields_ref()[0].type_ref()->getType(), ThriftType::Type::t_typedef);
  EXPECT_EQ(
      *s1.fields_ref()[0].type_ref()->get_t_typedef().name_ref(),
      "typedef_test.StringMap");
  auto utype1 =
      s1.fields_ref()[0].type_ref()->get_t_typedef().underlyingType_ref().get();
  EXPECT_EQ(utype1->getType(), ThriftType::Type::t_map);
  auto keyType1 = utype1->get_t_map().keyType_ref().get();
  EXPECT_EQ(keyType1->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      keyType1->get_t_primitive(), ThriftPrimitiveType::THRIFT_STRING_TYPE);
  auto valType1 = utype1->get_t_map().valueType_ref().get();
  EXPECT_EQ(valType1->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      valType1->get_t_primitive(), ThriftPrimitiveType::THRIFT_STRING_TYPE);

  // list<map<i32,string>>
  EXPECT_EQ(*s1.fields_ref()[1].id_ref(), 2);
  EXPECT_EQ(*s1.fields_ref()[1].name_ref(), "mapList");
  EXPECT_EQ(
      s1.fields_ref()[1].type_ref()->getType(), ThriftType::Type::t_typedef);
  EXPECT_EQ(
      *s1.fields_ref()[1].type_ref()->get_t_typedef().name_ref(),
      "typedef_test.MapList");
  auto utype2 =
      s1.fields_ref()[1].type_ref()->get_t_typedef().underlyingType_ref().get();
  EXPECT_EQ(utype2->getType(), ThriftType::Type::t_list);
  auto elemType = utype2->get_t_list().valueType_ref().get();
  EXPECT_EQ(elemType->getType(), ThriftType::Type::t_map);
  auto elemKey = elemType->get_t_map().keyType_ref().get();
  EXPECT_EQ(elemKey->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(elemKey->get_t_primitive(), ThriftPrimitiveType::THRIFT_I32_TYPE);
  auto elemValue = elemType->get_t_map().valueType_ref().get();
  EXPECT_EQ(elemValue->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      elemValue->get_t_primitive(), ThriftPrimitiveType::THRIFT_STRING_TYPE);

  // map<list<string>, map<i32, list<i64>>>
  EXPECT_EQ(*s1.fields_ref()[2].id_ref(), 3);
  EXPECT_EQ(*s1.fields_ref()[2].name_ref(), "veryComplex");
  EXPECT_EQ(
      s1.fields_ref()[2].type_ref()->getType(), ThriftType::Type::t_typedef);
  EXPECT_EQ(
      *s1.fields_ref()[2].type_ref()->get_t_typedef().name_ref(),
      "typedef_test.VeryComplex");
  auto utype3 =
      s1.fields_ref()[2].type_ref()->get_t_typedef().underlyingType_ref().get();
  EXPECT_EQ(utype3->getType(), ThriftType::Type::t_map);
  // Map key
  auto keyType3 = utype3->get_t_map().keyType_ref().get();
  EXPECT_EQ(keyType3->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      keyType3->get_t_primitive(), ThriftPrimitiveType::THRIFT_STRING_TYPE);
  // Map value (also a map)
  auto valueType3 = utype3->get_t_map().valueType_ref().get();
  EXPECT_EQ(valueType3->getType(), ThriftType::Type::t_map);
  // Inner map's key (i32)
  auto valueKeyType = valueType3->get_t_map().keyType_ref().get();
  EXPECT_EQ(valueKeyType->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      valueKeyType->get_t_primitive(), ThriftPrimitiveType::THRIFT_I32_TYPE);
  // Inner map's value (list<i64>)
  auto valueValType = valueType3->get_t_map().valueType_ref().get();
  EXPECT_EQ(valueValType->getType(), ThriftType::Type::t_list);
  auto valueValElemType = valueValType->get_t_list().valueType_ref().get();
  EXPECT_EQ(valueValElemType->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      valueValElemType->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_I64_TYPE);
}

TEST_F(ServiceMetadataTest, ServicesAndContextPopulated) {
  auto& metadata = getMetadata<apache::thrift::ServiceHandler<
      metadata::test::services::MyTestService>>();
  EXPECT_EQ(response_.services_ref()->size(), 2);
  auto& derived = response_.services_ref()->front();
  auto& base = response_.services_ref()->back();

  EXPECT_EQ(
      *derived.service_name_ref(),
      *response_.context_ref()->service_info_ref()->name_ref());
  EXPECT_EQ(
      *derived.module_ref()->name_ref(),
      *response_.context_ref()->module_ref()->name_ref());

  EXPECT_EQ(
      *metadata.services_ref()->at(*derived.service_name_ref()).name_ref(),
      "service_test.MyTestService");
  EXPECT_EQ(
      *metadata.services_ref()->at(*base.service_name_ref()).name_ref(),
      "service_test.ParentService");
}

TEST_F(ServiceMetadataTest, ServiceTest) {
  auto& metadata = getMetadata<apache::thrift::ServiceHandler<
      metadata::test::services::MyTestService>>();
  EXPECT_EQ(
      response_.services_ref()->front().module_ref()->name_ref(),
      "service_test");

  const auto& p = metadata.services_ref()->at("service_test.ParentService");
  EXPECT_EQ(*p.name_ref(), "service_test.ParentService");
  EXPECT_EQ(p.structured_annotations()->size(), 1);
  EXPECT_EQ(
      p.structured_annotations()->at(0), *cons("service").cv_struct_ref());
  EXPECT_EQ(p.functions_ref()->size(), 1);
  EXPECT_EQ(apache::thrift::get_pointer(p.parent()), nullptr);

  const auto& f = p.functions_ref()[0];
  EXPECT_EQ(*f.name_ref(), "parentFun");
  EXPECT_EQ(f.structured_annotations()->size(), 1);
  EXPECT_EQ(
      f.structured_annotations()->at(0), *cons("function").cv_struct_ref());
  EXPECT_EQ(f.return_type_ref()->getType(), ThriftType::Type::t_primitive);
  EXPECT_EQ(
      f.return_type_ref()->get_t_primitive(),
      ThriftPrimitiveType::THRIFT_I32_TYPE);
  EXPECT_EQ(f.arguments_ref()->size(), 0);
  EXPECT_EQ(f.exceptions_ref()->size(), 0);

  const auto& s = metadata.services_ref()->at(
      *response_.services_ref()->front().service_name_ref());
  EXPECT_EQ(*s.name_ref(), "service_test.MyTestService");
  EXPECT_EQ(s.functions_ref()->size(), 3);
  EXPECT_EQ(
      *apache::thrift::get_pointer(s.parent()), "service_test.ParentService");

  const auto& f0 = s.functions_ref()[0];
  EXPECT_EQ(*f0.name_ref(), "getAllTypes");
  EXPECT_EQ(f0.return_type_ref()->getType(), ThriftType::Type::t_list);
  auto retType1 = f0.return_type_ref()->get_t_list();
  auto elemType = retType1.valueType_ref().get();
  EXPECT_EQ(elemType->getType(), ThriftType::Type::t_struct);
  EXPECT_EQ(*elemType->get_t_struct().name_ref(), "typedef_test.Types");
  EXPECT_EQ(f0.arguments_ref()->size(), 0);
  EXPECT_EQ(f0.exceptions_ref()->size(), 0);
  EXPECT_FALSE(*f0.is_oneway_ref());

  const auto& f1 = s.functions_ref()[1];
  EXPECT_EQ(*f1.name_ref(), "getType");
  EXPECT_EQ(f1.return_type_ref()->getType(), ThriftType::Type::t_struct);
  EXPECT_EQ(
      *f1.return_type_ref()->get_t_struct().name_ref(), "typedef_test.Types");
  EXPECT_EQ(*f1.arguments_ref()[0].id_ref(), 1);
  EXPECT_EQ(*f1.arguments_ref()[0].name_ref(), "stringMap");
  EXPECT_EQ(*f1.arguments_ref()[0].is_optional_ref(), false);
  EXPECT_EQ(
      f1.arguments_ref()[0].type_ref()->getType(), ThriftType::Type::t_typedef);
  EXPECT_EQ(
      *f1.arguments_ref()[0].type_ref()->get_t_typedef().name_ref(),
      "typedef_test.StringMap");
  EXPECT_EQ(f1.arguments_ref()[0].structured_annotations()->size(), 1);
  EXPECT_EQ(
      f1.arguments_ref()[0].structured_annotations()->at(0),
      *cons("argument").cv_struct_ref());
  EXPECT_EQ(*f1.exceptions_ref()[0].id_ref(), 1);
  EXPECT_EQ(*f1.exceptions_ref()[0].name_ref(), "ex");
  EXPECT_EQ(*f1.exceptions_ref()[0].is_optional_ref(), false);
  EXPECT_EQ(
      f1.exceptions_ref()[0].type_ref()->getType(), ThriftType::Type::t_struct);
  EXPECT_EQ(
      *f1.exceptions_ref()[0].type_ref()->get_t_struct().name_ref(),
      "service_test.CutoffException");
  EXPECT_FALSE(*f1.is_oneway_ref());

  const auto& f2 = s.functions_ref()[2];
  EXPECT_EQ(*f2.name_ref(), "noReturn");
  EXPECT_TRUE(*f2.is_oneway_ref());
}

TEST_F(ServiceMetadataTest, RepeatedTest) {
  for (size_t i = 0; i < 3; i++) {
    resetResponse();
    auto& metadata = getMetadata<apache::thrift::ServiceHandler<
        metadata::test::repeated::RepeatedTestService>>();
    EXPECT_EQ(
        response_.services_ref()->front().module_ref()->name_ref(), "repeated");

    const auto& s = metadata.services_ref()->at(
        *response_.services_ref()->front().service_name_ref());
    EXPECT_EQ(*s.name_ref(), "repeated.RepeatedTestService");
    EXPECT_EQ(s.functions_ref()->size(), 1);
    EXPECT_EQ(*s.functions_ref()[0].name_ref(), "addValue");
    auto it = metadata.enums_ref()->find("repeated.ValueEnum");
    EXPECT_NE(it, metadata.enums_ref()->end());
    EXPECT_EQ(
        *s.functions_ref()
             ->at(0)
             .arguments_ref()
             ->at(0)
             .type_ref()
             ->get_t_struct()
             .name_ref(),
        "repeated.AddValueRequest");
    EXPECT_EQ(
        *metadata.structs_ref()
             ->at("repeated.AddValueRequest")
             .fields_ref()
             ->at(0)
             .type_ref()
             ->get_t_enum()
             .name_ref(),
        "repeated.ValueEnum");
  }
}

TEST_F(ServiceMetadataTest, NoNamespaceTest) {
  auto& metadata =
      getMetadata<apache::thrift::ServiceHandler<cpp2::AnotherTestService>>();
  EXPECT_EQ(
      response_.services_ref()->front().module_ref()->name_ref(),
      "no_namespace");

  auto s = metadata.structs_ref()->at("no_namespace.MyData");
  EXPECT_EQ(s.fields_ref()->size(), 1);
}
} // namespace
