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

#include <memory>
#include <thrift/compiler/sema/standard_mutator.h>

#include <folly/portability/GTest.h>
#include <thrift/compiler/ast/diagnostic_context.h>
#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/diagnostic.h>
#include <thrift/compiler/gen/testing.h>

namespace apache::thrift::compiler {
namespace {

class StandardMutatorTest : public ::testing::Test {
 protected:
  std::vector<diagnostic> mutate(
      diagnostic_params params = diagnostic_params::keep_all()) {
    source_manager source_mgr;
    diagnostic_results results;
    diagnostic_context ctx(source_mgr, results, std::move(params));
    standard_mutators(false)(ctx, program_bundle_);
    return std::move(results).diagnostics();
  }

  t_program* root_program() { return program_bundle_.root_program(); }

  t_program_bundle program_bundle_{
      std::make_unique<t_program>("/path/to/file.thrift")};
};

TEST_F(StandardMutatorTest, Empty) {
  mutate();
}

TEST_F(StandardMutatorTest, TerseWriteField) {
  t_program* program = root_program();
  auto terse_field =
      std::make_unique<t_field>(t_base_type::t_i64(), "terse_field", 1);
  auto strct = std::make_unique<t_struct>(program, "struct");

  // Store pointer for testing purpose.
  const auto* terse_field_ptr = terse_field.get();

  terse_field->add_structured_annotation(
      gen::thrift_annotation_builder::terse(*program).make());
  strct->append_field(std::move(terse_field));
  program->add_struct(std::move(strct));

  EXPECT_EQ(terse_field_ptr->qualifier(), t_field_qualifier::none);

  mutate();

  EXPECT_EQ(terse_field_ptr->qualifier(), t_field_qualifier::terse);
}

TEST_F(StandardMutatorTest, TerseWriteStruct) {
  t_program* program = root_program();
  auto terse_field =
      std::make_unique<t_field>(t_base_type::t_i64(), "terse_field", 1);
  auto optional_field =
      std::make_unique<t_field>(t_base_type::t_i64(), "optional_field", 2);
  auto required_field =
      std::make_unique<t_field>(t_base_type::t_i64(), "required_field", 3);
  auto strct = std::make_unique<t_struct>(program, "struct");

  optional_field->set_qualifier(t_field_qualifier::optional);
  required_field->set_qualifier(t_field_qualifier::required);

  // Store pointers for testing purpose.
  const auto* terse_field_ptr = terse_field.get();
  const auto* optional_field_ptr = optional_field.get();
  const auto* required_field_ptr = required_field.get();

  strct->add_structured_annotation(
      gen::thrift_annotation_builder::terse(*program).make());
  strct->append_field(std::move(terse_field));
  strct->append_field(std::move(optional_field));
  strct->append_field(std::move(required_field));
  program->add_struct(std::move(strct));

  EXPECT_EQ(terse_field_ptr->qualifier(), t_field_qualifier::none);
  EXPECT_EQ(optional_field_ptr->qualifier(), t_field_qualifier::optional);
  EXPECT_EQ(required_field_ptr->qualifier(), t_field_qualifier::required);

  mutate();

  EXPECT_EQ(terse_field_ptr->qualifier(), t_field_qualifier::terse);
  EXPECT_EQ(optional_field_ptr->qualifier(), t_field_qualifier::optional);
  EXPECT_EQ(required_field_ptr->qualifier(), t_field_qualifier::required);
}

TEST_F(StandardMutatorTest, Transitive) {
  t_program* program = root_program();
  auto terse_field =
      std::make_unique<t_field>(t_base_type::t_i64(), "terse_field", 1);
  auto strct = std::make_unique<t_struct>(program, "struct");

  // Store pointer for testing purpose.
  const auto* terse_field_ptr = terse_field.get();

  // IDL Representation:
  //
  //     @scope.Transitive
  //     struct TransitiveAnnot {}
  //
  //     @TransitiveAnnot
  //     @thrift.TerseWrite
  //     @scope.Transitive
  //     struct TransitiveTerse {}
  auto transitive_annot =
      std::make_unique<t_struct>(program, "TransitiveAnnot");
  auto transitive_terse =
      std::make_unique<t_struct>(program, "TransitiveTerse");
  transitive_annot->add_structured_annotation(
      gen::thrift_annotation_builder::transitive(*program).make());
  transitive_terse->add_structured_annotation(std::make_unique<t_const>(
      program, *transitive_annot, "", std::make_unique<t_const_value>()));
  transitive_terse->add_structured_annotation(
      gen::thrift_annotation_builder::terse(*program).make());
  transitive_terse->add_structured_annotation(
      gen::thrift_annotation_builder::transitive(*program).make());

  strct->append_field(std::move(terse_field));
  program->add_struct(std::move(strct));
  program->add_structured_annotation(std::make_unique<t_const>(
      program, *transitive_terse, "", std::make_unique<t_const_value>()));

  EXPECT_EQ(terse_field_ptr->qualifier(), t_field_qualifier::none);

  mutate();

  EXPECT_EQ(terse_field_ptr->qualifier(), t_field_qualifier::terse);
}

} // namespace
} // namespace apache::thrift::compiler
