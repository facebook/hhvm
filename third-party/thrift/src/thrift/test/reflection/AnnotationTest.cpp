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

} // namespace apache::thrift::test
