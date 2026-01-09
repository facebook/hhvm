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
#include <thrift/lib/cpp2/gen/module_metadata_h.h>
#include <thrift/lib/cpp2/runtime/SchemaRegistry.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/annotations_types.h>
#include <thrift/lib/cpp2/util/DebugTree.h>

namespace apache::thrift::test {

using metadata::detail::LimitedVector;
using namespace apache::thrift::detail::md;
using namespace apache::thrift::metadata;

std::vector<syntax_graph::TypeRef> getAnnotationTypes(
    folly::span<const syntax_graph::Annotation> annotations);

template <class T>
auto getAnnotationTypes() {
  return getAnnotationTypes(
      SchemaRegistry::get().getDefinitionNode<T>().annotations());
}

std::vector<syntax_graph::TypeRef> getFieldAnnotationTypes(
    const syntax_graph::StructuredNode& node, size_t position, std::int16_t id);

template <class T>
auto getFieldAnnotationTypes(size_t position, std::int16_t id) {
  return getFieldAnnotationTypes(
      SchemaRegistry::get().getNode<T>(), position, id);
}

std::vector<syntax_graph::TypeRef> getAnnotationTypes(
    folly::span<const syntax_graph::Annotation> annotations) {
  std::vector<syntax_graph::TypeRef> ret;
  ret.reserve(annotations.size());
  for (auto& annotation : annotations) {
    ret.push_back(annotation.type());
  }
  return ret;
}

std::vector<syntax_graph::TypeRef> getFieldAnnotationTypes(
    const syntax_graph::StructuredNode& node,
    size_t position,
    std::int16_t id) {
  DCHECK_LT(position, node.fields().size());
  const auto& field = node.fields()[position];
  DCHECK_EQ(static_cast<std::int16_t>(field.id()), id);
  return getAnnotationTypes(field.annotations());
}

// In ThriftConstValue, `set`/`map` are stored as list.
// This function sort `set`/`map` so that we can do equality comparison.
void normalizeThriftConstValue(
    ThriftConstValue& t, const syntax_graph::TypeRef& type);

void normalizeThriftConstStruct(
    ThriftConstStruct& t, const syntax_graph::TypeRef& type) {
  std::unordered_map<std::string, syntax_graph::TypeRef> fieldType;
  for (auto& field : type.trueType().asStructured().fields()) {
    fieldType.emplace(field.name(), field.type());
  }
  for (auto& [name, value] : *t.fields()) {
    normalizeThriftConstValue(value, fieldType.at(name));
  }
}
void normalizeThriftConstValue(
    ThriftConstValue& t, const syntax_graph::TypeRef& ref) {
  const auto& type = ref.trueType();
  if (type.isList()) {
    for (auto& i : *t.cv_list()) {
      normalizeThriftConstValue(i, type.asList().elementType());
    }
  }

  if (type.isSet()) {
    for (auto& i : *t.cv_list()) {
      normalizeThriftConstValue(i, type.asSet().elementType());
    }
    std::sort(t.cv_list()->begin(), t.cv_list()->end());
  }

  if (type.isMap()) {
    auto keyType = type.asMap().keyType();
    auto valueType = type.asMap().valueType();
    for (auto& i : *t.cv_map()) {
      normalizeThriftConstValue(*i.key(), keyType);
      normalizeThriftConstValue(*i.value(), valueType);
    }
    std::sort(t.cv_map()->begin(), t.cv_map()->end());
  }

  if (type.isStructured()) {
    normalizeThriftConstStruct(*t.cv_struct(), type);
  }
}

// This function will sort structured annotations, as well as sorting
// `set`/`map` inside annotations so that we can do equality comparison.
std::vector<ThriftConstStruct> normalizeStructuredAnnotations(
    std::vector<ThriftConstStruct> annotations,
    const std::unordered_map<std::string, syntax_graph::TypeRef>& nameToType) {
  for (auto& i : annotations) {
    normalizeThriftConstStruct(i, nameToType.at(*i.type()->name()));
  }
  std::sort(annotations.begin(), annotations.end());
  return annotations;
}

template <class Node>
static std::string getName(const Node& node) {
  const auto& def = node.definition();
  return fmt::format("{}.{}", def.program().name(), def.name());
}

// A Helper function to check whether two list of structured annotations have
// same data. We can not rely on `std::vector::operator==` directly since
// Annotations' order, as well as the order of `set`/`map` in the annotation
// fields might not be preserved.
bool structuredAnnotationsEquality(
    std::vector<ThriftConstStruct> lhsAnnotations,
    std::vector<ThriftConstStruct> rhsAnnotations,
    const std::vector<syntax_graph::TypeRef>& annotationTypes) {
  std::unordered_map<std::string, syntax_graph::TypeRef> nameToType;
  for (const auto& i : annotationTypes) {
    nameToType.emplace(getName(i.trueType().asStructured()), i);
  }
  return normalizeStructuredAnnotations(
             std::move(lhsAnnotations), nameToType) ==
      normalizeStructuredAnnotations(std::move(rhsAnnotations), nameToType);
}

LimitedVector<metadata::ThriftConstStruct> expectedAnnotations() {
  LimitedVector<metadata::ThriftConstStruct> ret;
  metadata::ThriftConstStruct* last = &ret.emplace_back();
  last->type()->name() = "annotations.Annotation";
  last->fields()["boolField"].cv_bool() = true;
  last->fields()["i16Field"].cv_integer() = 16;
  last->fields()["i32Field"].cv_integer() = 32;
  last->fields()["floatField"].cv_double() = 10;
  last->fields()["binaryField"].cv_string() = "binary";
  last->fields()["doubleField"].cv_double() = 20;

  last->fields()["structField"].cv_struct().emplace().type()->name() =
      "annotations.MyStruct";
  last->fields()["structField"]
      .cv_struct()
      ->fields()["stringField"]
      .cv_string() = "struct";
  last->fields()["unionField"].cv_struct().emplace().type()->name() =
      "annotations.MyUnion";
  last->fields()["unionField"]
      .cv_struct()
      ->fields()["stringField"]
      .cv_string() = "union";

  last->fields()["enumField"].cv_integer() = 2;
  last->fields()["listField"].cv_list().emplace();
  last->fields()["listField"].cv_list()->emplace_back().cv_integer() = 2;
  last->fields()["listField"].cv_list()->emplace_back().cv_integer() = 1;
  last->fields()["listField"].cv_list()->emplace_back().cv_integer() = 2;
  last->fields()["setField"].cv_list().emplace();
  last->fields()["setField"].cv_list()->emplace_back().cv_integer() = 2;
  last->fields()["setField"].cv_list()->emplace_back().cv_integer() = 1;
  last->fields()["mapField"].cv_map().emplace();
  last->fields()["mapField"].cv_map()->emplace_back();
  last->fields()["mapField"].cv_map()->back().key()->cv_integer() = 2;
  last->fields()["mapField"].cv_map()->back().value()->cv_string() = "20";
  last->fields()["mapField"].cv_map()->emplace_back();
  last->fields()["mapField"].cv_map()->back().key()->cv_integer() = 1;
  last->fields()["mapField"].cv_map()->back().value()->cv_string() = "10";

  last = &ret.emplace_back();
  last->type()->name() = "annotations.Foo";
  last->fields()["bar"].cv_struct().emplace().type()->name() =
      "annotations.Bar";
  last->fields()["bar"].cv_struct()->fields()["baz"].cv_string() = "123";
  return ret;
}

metadata::ThriftEnum expectedEnum() {
  metadata::ThriftEnum ret;
  ret.name() = "annotations.TestEnum";
  ret.elements()[1] = "foo";
  ret.elements()[2] = "bar";
  ret.structured_annotations() = expectedAnnotations();
  return ret;
}

TEST(Annotations, Enum) {
  metadata::ThriftMetadata md;
  EnumMetadata<TestEnum>::gen(md);
  auto actual = md.enums()->at("annotations.TestEnum");
  auto expected = expectedEnum();
  auto types = getAnnotationTypes<TestEnum>();

  // Annotations require a special function to check equality.
  EXPECT_TRUE(structuredAnnotationsEquality(
      *actual.structured_annotations(),
      *expected.structured_annotations(),
      types));

  actual.structured_annotations()->clear();
  expected.structured_annotations()->clear();

  // After excluding the annotations, other fields should be the same.
  EXPECT_EQ(actual, expected);
}

TEST(Annotations, Normalization) {
  auto annotationType =
      syntax_graph::TypeRef::of(SchemaRegistry::get().getNode<Annotation>());
  auto fooType =
      syntax_graph::TypeRef::of(SchemaRegistry::get().getNode<Foo>());
  {
    std::vector<metadata::ThriftConstStruct> lhs;
    lhs.emplace_back().type()->name() = "annotations.Annotation";
    lhs.emplace_back().type()->name() = "annotations.Foo";
    std::vector<metadata::ThriftConstStruct> rhs = {lhs[1], lhs[0]};
    std::vector<syntax_graph::TypeRef> types = {annotationType, fooType};

    EXPECT_TRUE(structuredAnnotationsEquality(lhs, rhs, types));
  }
  {
    metadata::ThriftConstStruct lhs;
    lhs.type()->name() = "annotations.Annotation";
    lhs.fields()["boolField"].cv_bool() = false;

    metadata::ThriftConstStruct rhs;
    rhs.type()->name() = "annotations.Annotation";
    rhs.fields()["boolField"].cv_bool() = true;

    EXPECT_FALSE(structuredAnnotationsEquality({lhs}, {rhs}, {annotationType}));
  }
  {
    metadata::ThriftConstStruct lhs;
    lhs.type()->name() = "annotations.Annotation";
    lhs.fields()["listField"].cv_list().emplace();
    lhs.fields()["listField"].cv_list()->emplace_back().cv_integer() = 2;
    lhs.fields()["listField"].cv_list()->emplace_back().cv_integer() = 1;

    metadata::ThriftConstStruct rhs;
    rhs.type()->name() = "annotations.Annotation";
    rhs.fields()["listField"].cv_list().emplace();
    rhs.fields()["listField"].cv_list()->emplace_back().cv_integer() = 1;
    rhs.fields()["listField"].cv_list()->emplace_back().cv_integer() = 2;

    EXPECT_FALSE(structuredAnnotationsEquality({lhs}, {rhs}, {annotationType}));
  }
  {
    metadata::ThriftConstStruct lhs;
    lhs.type()->name() = "annotations.Annotation";
    lhs.fields()["setField"].cv_list().emplace();
    lhs.fields()["setField"].cv_list()->emplace_back().cv_integer() = 2;
    lhs.fields()["setField"].cv_list()->emplace_back().cv_integer() = 1;

    metadata::ThriftConstStruct rhs;
    rhs.type()->name() = "annotations.Annotation";
    rhs.fields()["setField"].cv_list().emplace();
    rhs.fields()["setField"].cv_list()->emplace_back().cv_integer() = 1;
    rhs.fields()["setField"].cv_list()->emplace_back().cv_integer() = 2;

    EXPECT_TRUE(structuredAnnotationsEquality({lhs}, {rhs}, {annotationType}));
  }
  {
    metadata::ThriftConstStruct lhs;
    lhs.type()->name() = "annotations.Annotation";
    lhs.fields()["mapField"].cv_map().emplace();
    lhs.fields()["mapField"].cv_map()->emplace_back();
    lhs.fields()["mapField"].cv_map()->back().key()->cv_integer() = 2;
    lhs.fields()["mapField"].cv_map()->back().value()->cv_string() = "20";
    lhs.fields()["mapField"].cv_map()->emplace_back();
    lhs.fields()["mapField"].cv_map()->back().key()->cv_integer() = 1;
    lhs.fields()["mapField"].cv_map()->back().value()->cv_string() = "10";

    metadata::ThriftConstStruct rhs;
    rhs.type()->name() = "annotations.Annotation";
    rhs.fields()["mapField"].cv_map().emplace();
    rhs.fields()["mapField"].cv_map()->emplace_back();
    rhs.fields()["mapField"].cv_map()->back().key()->cv_integer() = 1;
    rhs.fields()["mapField"].cv_map()->back().value()->cv_string() = "10";
    rhs.fields()["mapField"].cv_map()->emplace_back();
    rhs.fields()["mapField"].cv_map()->back().key()->cv_integer() = 2;
    rhs.fields()["mapField"].cv_map()->back().value()->cv_string() = "20";

    EXPECT_TRUE(structuredAnnotationsEquality({lhs}, {rhs}, {annotationType}));
  }
}

TEST(Annotations, TestFloat) {
  metadata::ThriftMetadata md1, md2, md3;
  const auto& t1 = StructMetadata<TestFloat1>::gen(md1);
  const auto& t2 = StructMetadata<TestFloat2>::gen(md2);
  const auto& t3 = StructMetadata<TestFloat3>::gen(md3);
  EXPECT_EQ(t1.structured_annotations(), t2.structured_annotations());
  EXPECT_NE(t1.structured_annotations(), t3.structured_annotations());
}

metadata::ThriftStruct expectedStruct() {
  metadata::ThriftStruct ret;
  ret.name() = "annotations.TestStruct";
  ret.is_union() = false;
  metadata::ThriftField field;
  field.id() = 1;
  field.name() = "field_1";
  field.type()->t_primitive() =
      metadata::ThriftPrimitiveType::THRIFT_STRING_TYPE;
  ret.fields()->push_back(field);
  field.id() = 2;
  field.name() = "field_2";
  field.type()->t_primitive() = metadata::ThriftPrimitiveType::THRIFT_I32_TYPE;
  field.structured_annotations() = expectedAnnotations();
  ret.fields()->push_back(field);
  ret.structured_annotations() = expectedAnnotations();
  return ret;
}

metadata::ThriftException expectedException() {
  metadata::ThriftException ret;
  ret.name() = "annotations.TestException";
  ret.fields() = *expectedStruct().fields();
  ret.fields()[1].structured_annotations()->clear();
  ret.structured_annotations() = expectedAnnotations();
  return ret;
}

TEST(Annotations, Struct) {
  metadata::ThriftMetadata md;
  auto actual = StructMetadata<TestStruct>::gen(md);
  auto expected = expectedStruct();

  // Annotations require a special function to check equality.
  for (size_t i = 0; i < actual.fields()->size(); ++i) {
    EXPECT_TRUE(structuredAnnotationsEquality(
        *actual.fields()[i].structured_annotations(),
        *expected.fields()[i].structured_annotations(),
        getFieldAnnotationTypes<TestStruct>(i, *actual.fields()[i].id())));
    actual.fields()[i].structured_annotations()->clear();
    expected.fields()[i].structured_annotations()->clear();
  }

  EXPECT_TRUE(structuredAnnotationsEquality(
      *actual.structured_annotations(),
      *expected.structured_annotations(),
      getAnnotationTypes<TestStruct>()));

  actual.structured_annotations()->clear();
  expected.structured_annotations()->clear();

  // After excluding the annotations, other fields should be the same.
  EXPECT_EQ(actual, expected);
}

TEST(Annotations, Exception) {
  metadata::ThriftMetadata md;
  ExceptionMetadata<TestException>::gen(md);
  auto actual = md.exceptions()->at("annotations.TestException");
  auto expected = expectedException();

  // Annotations require a special function to check equality.
  EXPECT_TRUE(structuredAnnotationsEquality(
      *actual.structured_annotations(),
      *expected.structured_annotations(),
      getAnnotationTypes<TestException>()));

  actual.structured_annotations()->clear();
  expected.structured_annotations()->clear();

  // After excluding the annotations, other fields should be the same.
  EXPECT_EQ(actual, expected);
}

TEST(Annotations, Service) {
  metadata::ThriftMetadata md;
  auto service = genServiceMetadata<TestService>(md);
  EXPECT_EQ(
      service.structured_annotations()->begin()->fields()["baz"].cv_string(),
      "0");
  EXPECT_EQ(
      service.functions()[0]
          .structured_annotations()
          ->begin()
          ->fields()["baz"]
          .cv_string(),
      "1");
  EXPECT_EQ(
      service.functions()[0]
          .arguments()[0]
          .structured_annotations()
          ->begin()
          ->fields()["baz"]
          .cv_string(),
      "2");
  EXPECT_EQ(
      service.functions()[0]
          .exceptions()[0]
          .structured_annotations()
          ->begin()
          ->fields()["baz"]
          .cv_string(),
      "3");
}

} // namespace apache::thrift::test
