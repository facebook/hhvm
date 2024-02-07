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

#include <thrift/compiler/gen/cpp/reference_type.h>
#include <thrift/compiler/test/gen_testing.h>

#include <folly/portability/GTest.h>
#include <thrift/compiler/ast/t_base_type.h>
#include <thrift/compiler/ast/t_field.h>

namespace apache::thrift::compiler::gen::cpp {
namespace {

class ReferenceTypeTest : public ::testing::Test {};

TEST_F(ReferenceTypeTest, None) {
  t_field tfield(&t_base_type::t_string(), "my_string");
  EXPECT_EQ(find_ref_type(tfield), reference_type::none);
}

TEST_F(ReferenceTypeTest, CppRef) {
  t_field tfield(&t_base_type::t_string(), "my_string");
  tfield.set_annotation("cpp.ref", "");
  EXPECT_EQ(find_ref_type(tfield), reference_type::unique);
}

TEST_F(ReferenceTypeTest, Cpp2Ref) {
  t_field tfield(&t_base_type::t_string(), "my_string");
  tfield.set_annotation("cpp2.ref", "");
  EXPECT_EQ(find_ref_type(tfield), reference_type::unique);
}

TEST_F(ReferenceTypeTest, CppRefType_Unique) {
  t_field tfield(&t_base_type::t_string(), "my_string");
  tfield.set_annotation("cpp.ref_type", "unique");
  EXPECT_EQ(find_ref_type(tfield), reference_type::unique);
}

TEST_F(ReferenceTypeTest, CppRefType_Shared) {
  t_field tfield(&t_base_type::t_string(), "my_string");
  tfield.set_annotation("cpp.ref_type", "shared");
  EXPECT_EQ(find_ref_type(tfield), reference_type::shared_mutable);
}

TEST_F(ReferenceTypeTest, CppRefType_SharedMutable) {
  t_field tfield(&t_base_type::t_string(), "my_string");
  tfield.set_annotation("cpp.ref_type", "shared_mutable");
  EXPECT_EQ(find_ref_type(tfield), reference_type::shared_mutable);
}

TEST_F(ReferenceTypeTest, CppRefType_SharedConst) {
  t_field tfield(&t_base_type::t_string(), "my_string");
  tfield.set_annotation("cpp.ref_type", "shared_const");
  EXPECT_EQ(find_ref_type(tfield), reference_type::shared_const);
}

TEST_F(ReferenceTypeTest, CppRefType_Unknown) {
  t_field tfield(&t_base_type::t_string(), "my_string");
  tfield.set_annotation("cpp.ref_type", "blah");
  EXPECT_THROW(find_ref_type(tfield), std::runtime_error);
}

TEST_F(ReferenceTypeTest, CppRefType_box) {
  t_field tfield(&t_base_type::t_string(), "my_string");
  tfield.set_annotation("thrift.box", "");
  EXPECT_EQ(find_ref_type(tfield), reference_type::boxed);
}

TEST_F(ReferenceTypeTest, ThriftBox) {
  t_program program{"path/to/program.thrift"};
  t_field tfield(&t_base_type::t_string(), "my_string");
  tfield.add_structured_annotation(
      thrift_annotation_builder::box(program).make());
  EXPECT_EQ(find_ref_type(tfield), reference_type::boxed);
}
} // namespace
} // namespace apache::thrift::compiler::gen::cpp
