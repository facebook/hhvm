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
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/test/FieldInterceptorTest.h>
#include <thrift/test/gen-cpp2/field_interceptor_types.h>

namespace apache::thrift::test {

template <class T>
struct InterceptedFieldsTest : ::testing::Test {
  void SetUp() override { TestFieldInterceptor::count = 0; }
};

TYPED_TEST_CASE_P(InterceptedFieldsTest);

TYPED_TEST_P(InterceptedFieldsTest, intercept) {
  InterceptedFields obj;
  auto&& ref = static_cast<TypeParam>(obj);
  EXPECT_EQ(TestFieldInterceptor::count, 0);
  ref.access_field();
  EXPECT_EQ(TestFieldInterceptor::count, 1);
  ref.access_shared_field();
  EXPECT_EQ(TestFieldInterceptor::count, 2);
  ref.access_optional_shared_field();
  EXPECT_EQ(TestFieldInterceptor::count, 3);
  ref.access_shared_const_field_ref();
  EXPECT_EQ(TestFieldInterceptor::count, 4);
  ref.access_optional_shared_const_field_ref();
  EXPECT_EQ(TestFieldInterceptor::count, 5);
  ref.access_optional_boxed_field();
  EXPECT_EQ(TestFieldInterceptor::count, 6);
}

REGISTER_TYPED_TEST_CASE_P(InterceptedFieldsTest, intercept);

using InterceptedFieldsRefs = ::testing::Types<
    InterceptedFields&,
    InterceptedFields&&,
    const InterceptedFields&,
    const InterceptedFields&&>;

INSTANTIATE_TYPED_TEST_CASE_P(
    TerseWriteTest, InterceptedFieldsTest, InterceptedFieldsRefs);

} // namespace apache::thrift::test
