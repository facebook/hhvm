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
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/annotations_metadata.h>

namespace apache::thrift::test {

std::vector<metadata::ThriftConstStruct> expectedAnnotations() {
  std::vector<metadata::ThriftConstStruct> ret{2};
  ret[0].type()->name() = "annotations.Annotation";
  ret[0].fields()["boolField"].cv_bool() = true;
  ret[0].fields()["i16Field"].cv_integer() = 16;
  ret[0].fields()["i32Field"].cv_integer() = 32;
  ret[0].fields()["floatField"].cv_double() = 10;
  ret[0].fields()["binaryField"].cv_string() = "binary";
  ret[0].fields()["doubleField"].cv_double() = 20;

  ret[0].fields()["structField"].cv_struct().emplace().type()->name() =
      "annotations.MyStruct";
  ret[0]
      .fields()["structField"]
      .cv_struct()
      ->fields()["stringField"]
      .cv_string() = "struct";
  ret[0].fields()["unionField"].cv_struct().emplace().type()->name() =
      "annotations.MyUnion";
  ret[0]
      .fields()["unionField"]
      .cv_struct()
      ->fields()["stringField"]
      .cv_string() = "union";

  ret[0].fields()["enumField"].cv_integer() = 2;
  ret[0].fields()["listField"].cv_list().emplace();
  ret[0].fields()["listField"].cv_list()->emplace_back().cv_integer() = 2;
  ret[0].fields()["listField"].cv_list()->emplace_back().cv_integer() = 1;
  ret[0].fields()["listField"].cv_list()->emplace_back().cv_integer() = 2;
  ret[0].fields()["setField"].cv_list().emplace();
  ret[0].fields()["setField"].cv_list()->emplace_back().cv_integer() = 2;
  ret[0].fields()["setField"].cv_list()->emplace_back().cv_integer() = 1;
  ret[0].fields()["mapField"].cv_map().emplace();
  ret[0].fields()["mapField"].cv_map()->emplace_back();
  ret[0].fields()["mapField"].cv_map()->back().key()->cv_integer() = 2;
  ret[0].fields()["mapField"].cv_map()->back().value()->cv_string() = "20";
  ret[0].fields()["mapField"].cv_map()->emplace_back();
  ret[0].fields()["mapField"].cv_map()->back().key()->cv_integer() = 1;
  ret[0].fields()["mapField"].cv_map()->back().value()->cv_string() = "10";

  ret[1].type()->name() = "annotations.Foo";
  ret[1].fields()["bar"].cv_struct().emplace().type()->name() =
      "annotations.Bar";
  ret[1].fields()["bar"].cv_struct()->fields()["baz"].cv_string() = "123";

  return ret;
}

metadata::ThriftEnum expectedEnum() {
  metadata::ThriftEnum ret;
  ret.name() = "annotations.TestEnum";
  ret.elements()[1] = "foo";
  ret.elements()[2] = "bar";
  auto annotations = expectedAnnotations();
  ret.structured_annotations().emplace().assign(
      annotations.begin(), annotations.end());
  return ret;
}

TEST(Annotations, Enum) {
  metadata::ThriftMetadata md;
  detail::md::EnumMetadata<TestEnum>::gen(md);
  EXPECT_EQ(md.enums()["annotations.TestEnum"], expectedEnum());
}

} // namespace apache::thrift::test
