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
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/enum_test_metadata.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/exception_test_metadata.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/include_test_metadata.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/nested_structs_test_metadata.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/no_namespace_metadata.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/repeated_metadata.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/service_test_metadata.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/service_test_with_package_name_metadata.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/simple_structs_test_metadata.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/stream_test_metadata.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/struct_union_test_metadata.h>
#include <thrift/lib/cpp2/test/metadata/gen-cpp2/typedef_test_metadata.h>

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

template <typename T>
class ServiceComparisonTest : public testing::Test {};

// TYPED_TEST does not support incomplete types. We need to put service tag in
// folly::tag_t.
using Services = ::testing::Types<
    folly::tag_t<cpp2::AnotherTestService>,
    folly::tag_t<enums::EnumTestService>,
    folly::tag_t<exceptions::ExceptionTestService>,
    folly::tag_t<include::IncludeTestService>,
    folly::tag_t<nested_structs::NestedStructsTestService>,
    folly::tag_t<repeated::RepeatedTestService>,
    folly::tag_t<services::MyTestService>,
    folly::tag_t<services::MyTestServiceWithUri>,
    folly::tag_t<services::ParentService>,
    folly::tag_t<services::ParentServiceWithUri>,
    folly::tag_t<simple_structs::SimpleStructsTestService>,
    folly::tag_t<stream::StreamTestService>,
    folly::tag_t<typedefs::TypedefTestService>>;

TYPED_TEST_SUITE(ServiceComparisonTest, Services);

TYPED_TEST(ServiceComparisonTest, ServiceComparisonTest) {
  ThriftServiceMetadataResponse lhs, rhs;
  using Tag = folly::type_list_element_t<0, TypeParam>;
  ServiceMetadata<ServiceHandler<Tag>>::gen(lhs);
  genServiceMetadataResponse<Tag>(rhs);
  EXPECT_EQ(lhs, rhs);
  EXPECT_GE(lhs.services()->size(), 1); // sanity check
}
} // namespace apache::thrift::detail::md
