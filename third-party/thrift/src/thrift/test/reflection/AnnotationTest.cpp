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
#include <thrift/test/reflection/gen-cpp2/AnnotationTest_types.h>

namespace apache::thrift::test {

TEST(AnnotationTest, GetAnnotationValue) {
  Oncall expected;
  expected.name() = "thrift";

  // Checking whether a field has a given annotation. If so, use it.
  if (auto* oncall = get_field_annotation<Oncall, MyStruct, ident::field>()) {
    EXPECT_EQ(*oncall, expected);
  } else {
    ADD_FAILURE();
  }

  // `Sensitive` annotation exists on MyStruct.field
  EXPECT_TRUE((get_field_annotation<Sensitive, MyStruct, ident::field>()));

  // `Doc` annotation does not exist on MyStruct.field
  EXPECT_FALSE((get_field_annotation<Doc, MyStruct, ident::field>()));
}

TEST(AnnotationTest, GetStructAnnotationValue) {
  Doc expected;
  expected.text() = "I am a struct";

  // Checking whether a struct has a given annotation. If so, use it.
  if (auto* doc = get_struct_annotation<Doc, MyStruct>()) {
    EXPECT_EQ(*doc, expected);
  } else {
    ADD_FAILURE();
  }

  // `Doc` annotation exists on MyStruct
  EXPECT_TRUE((get_struct_annotation<Doc, MyStruct>()));

  // `Oncall` annotation does not exist on MyStruct
  EXPECT_FALSE((get_struct_annotation<Oncall, MyStruct>()));
}

TEST(AnnotationTest, GetUnionAnnotationValue) {
  Doc expected;
  expected.text() = "I am a union";

  // Checking whether a union has a given annotation. If so, use it.
  if (auto* doc = get_struct_annotation<Doc, MyUnion>()) {
    EXPECT_EQ(*doc, expected);
  } else {
    ADD_FAILURE();
  }

  // `Doc` annotation exists on MyUnion
  EXPECT_TRUE((get_struct_annotation<Doc, MyUnion>()));

  // `Oncall` annotation does not exist on MyUnion
  EXPECT_FALSE((get_struct_annotation<Oncall, MyUnion>()));
}

TEST(AnnotationTest, GetExceptionAnnotationValue) {
  Doc expected;
  expected.text() = "I am an exception";

  // Checking whether an exception has a given annotation. If so, use it.
  if (auto* doc = get_struct_annotation<Doc, MyException>()) {
    EXPECT_EQ(*doc, expected);
  } else {
    ADD_FAILURE();
  }

  // `Doc` annotation exists on MyException
  EXPECT_TRUE((get_struct_annotation<Doc, MyException>()));

  // `Oncall` annotation does not exist on MyException
  EXPECT_FALSE((get_struct_annotation<Oncall, MyException>()));
}

} // namespace apache::thrift::test
