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

#include <thrift/lib/cpp2/gen/module_metadata_cpp.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/nested_structs_test_metadata.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/simple_structs_test_metadata.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/struct_union_test_metadata.h>

using namespace metadata::test;
namespace apache::thrift::detail::md {

template <typename T>
class StructComparisonTest : public testing::Test {};

using Structureds = ::testing::Types<
    simple_structs::City,
    simple_structs::Country,
    nested_structs::City,
    nested_structs::Country,
    nested_structs::Foo,
    struct_union::Dog,
    struct_union::Cat,
    struct_union::Pet,
    struct_union::ComplexUnion>;

TYPED_TEST_SUITE(StructComparisonTest, Structureds);

TYPED_TEST(StructComparisonTest, StructComparisonTest) {
  metadata::ThriftMetadata lhs, rhs;
  StructMetadata<TypeParam>::gen(lhs);
  genStructMetadata<TypeParam>(
      rhs, {.genAnnotations = true, .genNestedTypes = true});
  EXPECT_EQ(lhs, rhs);
  EXPECT_GE(lhs.structs()->size(), 1); // sanity check
}
} // namespace apache::thrift::detail::md
