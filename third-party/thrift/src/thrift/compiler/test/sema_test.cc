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
#include <thrift/compiler/sema/sema.h>

#include <gtest/gtest.h>
#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/diagnostic.h>
#include <thrift/compiler/sema/sema_context.h>
#include <thrift/compiler/test/gen_testing.h>

namespace apache::thrift::compiler {

class SemaTest : public ::testing::Test {
 protected:
  std::vector<diagnostic> mutate(
      diagnostic_params params = diagnostic_params::keep_all()) {
    source_manager source_mgr;
    diagnostic_results results;
    sema_context ctx(source_mgr, results, params);
    sema(false).run(ctx, program_bundle_);
    return std::move(results).diagnostics();
  }

  t_program* root_program() { return program_bundle_.root_program(); }

  t_program_bundle program_bundle_{std::make_unique<t_program>(
      "/path/to/file.thrift", "/root/path/to/file.thrift")};
};

TEST_F(SemaTest, terse_write_field) {
  t_program* program = root_program();
  auto terse_field =
      std::make_unique<t_field>(t_primitive_type::t_i64(), "terse_field", 1);
  auto strct = std::make_unique<t_struct>(program, "struct");

  // Store pointer for testing purpose.
  const auto* terse_field_ptr = terse_field.get();

  terse_field->add_structured_annotation(
      gen::thrift_annotation_builder::terse(*program).make());
  strct->append_field(std::move(terse_field));
  program->add_definition(std::move(strct));

  EXPECT_EQ(terse_field_ptr->qualifier(), t_field_qualifier::none);

  mutate();

  EXPECT_EQ(terse_field_ptr->qualifier(), t_field_qualifier::terse);
}

TEST_F(SemaTest, terse_write_struct) {
  t_program* program = root_program();
  auto terse_field =
      std::make_unique<t_field>(t_primitive_type::t_i64(), "terse_field", 1);
  auto optional_field =
      std::make_unique<t_field>(t_primitive_type::t_i64(), "optional_field", 2);
  auto required_field =
      std::make_unique<t_field>(t_primitive_type::t_i64(), "required_field", 3);
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
  program->add_definition(std::move(strct));

  EXPECT_EQ(terse_field_ptr->qualifier(), t_field_qualifier::none);
  EXPECT_EQ(optional_field_ptr->qualifier(), t_field_qualifier::optional);
  EXPECT_EQ(required_field_ptr->qualifier(), t_field_qualifier::required);

  mutate();

  EXPECT_EQ(terse_field_ptr->qualifier(), t_field_qualifier::terse);
  EXPECT_EQ(optional_field_ptr->qualifier(), t_field_qualifier::optional);
  EXPECT_EQ(required_field_ptr->qualifier(), t_field_qualifier::required);
}

TEST_F(SemaTest, transitive) {
  t_program* program = root_program();
  auto terse_field =
      std::make_unique<t_field>(t_primitive_type::t_i64(), "terse_field", 1);
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
  transitive_terse->add_structured_annotation(
      std::make_unique<t_const>(
          program, *transitive_annot, "", std::make_unique<t_const_value>()));
  transitive_terse->add_structured_annotation(
      gen::thrift_annotation_builder::terse(*program).make());
  transitive_terse->add_structured_annotation(
      gen::thrift_annotation_builder::transitive(*program).make());

  strct->append_field(std::move(terse_field));
  program->add_definition(std::move(strct));
  program->add_structured_annotation(
      std::make_unique<t_const>(
          program, *transitive_terse, "", std::make_unique<t_const_value>()));

  EXPECT_EQ(terse_field_ptr->qualifier(), t_field_qualifier::none);

  mutate();

  EXPECT_EQ(terse_field_ptr->qualifier(), t_field_qualifier::terse);
}

/**
 * Test that empty consts with mismatched const kind (i.e. mismatched
 * initializers) are normalized to the target type.
 * We warn on, but allow, empty list initializers for maps and empty map
 * initializers for lists/sets.
 */
TEST_F(SemaTest, empty_const_container_conversion) {
  // Test that empty list assigned to a map field is converted to CV_MAP
  // after post-validation mutators run
  t_program* program = root_program();
  t_map map_type{t_primitive_type::t_i32(), t_primitive_type::t_i32()};
  t_list list_type{t_primitive_type::t_i32()};
  t_set set_type{t_primitive_type::t_i32()};

  auto strct = std::make_unique<t_struct>(program, "TestStruct");
  t_field& map_field = strct->create_field(map_type, "map_field", /*id=*/1);
  map_field.set_default_value(t_const_value::make_list());

  t_field& list_field = strct->create_field(list_type, "list_field", /*id=*/2);
  list_field.set_default_value(t_const_value::make_map());

  t_field& set_field = strct->create_field(set_type, "set_field", /*id=*/3);
  set_field.set_default_value(t_const_value::make_map());

  program->add_definition(std::move(strct));

  auto constant = std::make_unique<t_const>(
      program, map_type, "map_const", t_const_value::make_list());
  const t_const_value* constant_value = constant->value();
  program->add_definition(std::move(constant));

  // Before mutation, values are mismatched
  EXPECT_EQ(map_field.default_value()->kind(), t_const_value::CV_LIST);
  EXPECT_TRUE(map_field.default_value()->is_empty());

  EXPECT_EQ(list_field.default_value()->kind(), t_const_value::CV_MAP);
  EXPECT_TRUE(list_field.default_value()->is_empty());

  EXPECT_EQ(set_field.default_value()->kind(), t_const_value::CV_MAP);
  EXPECT_TRUE(set_field.default_value()->is_empty());

  EXPECT_EQ(constant_value->kind(), t_const_value::CV_LIST);
  EXPECT_TRUE(constant_value->is_empty());

  mutate();

  // After mutation, values should be fixed
  EXPECT_EQ(map_field.default_value()->kind(), t_const_value::CV_MAP);
  EXPECT_EQ(list_field.default_value()->kind(), t_const_value::CV_LIST);
  EXPECT_EQ(set_field.default_value()->kind(), t_const_value::CV_LIST);
  EXPECT_EQ(constant_value->kind(), t_const_value::CV_MAP);
}

TEST_F(SemaTest, non_empty_containers_not_converted) {
  // Test that non-empty containers are not converted

  t_program* program = root_program();
  t_map map_type{t_primitive_type::t_i32(), t_primitive_type::t_i32()};
  t_list list_type{t_primitive_type::t_i32()};
  t_set set_type{t_primitive_type::t_i32()};

  std::unique_ptr<t_struct> strct =
      std::make_unique<t_struct>(program, "TestStruct");

  t_field& map_field = strct->create_field(map_type, "map_field", /*id=*/1);
  std::unique_ptr<t_const_value> map_default_value = t_const_value::make_list();
  map_default_value->add_list(std::make_unique<t_const_value>(42));
  map_field.set_default_value(std::move(map_default_value));

  t_field& list_field = strct->create_field(list_type, "list_field", /*id=*/2);
  std::unique_ptr<t_const_value> list_default_value = t_const_value::make_map();
  list_default_value->add_map(
      std::make_unique<t_const_value>(42), std::make_unique<t_const_value>(42));
  list_field.set_default_value(std::move(list_default_value));

  t_field& set_field = strct->create_field(set_type, "set_field", /*id=*/3);
  std::unique_ptr<t_const_value> set_default_value = t_const_value::make_map();
  set_default_value->add_map(
      std::make_unique<t_const_value>(42), std::make_unique<t_const_value>(42));
  set_field.set_default_value(std::move(set_default_value));

  program->add_definition(std::move(strct));

  auto constant = std::make_unique<t_const>(
      program, map_type, "map_const", t_const_value::make_list());
  t_const_value* constant_value = constant->value();
  constant_value->add_list(std::make_unique<t_const_value>(42));
  program->add_definition(std::move(constant));

  // Before mutation, values are mismatched
  EXPECT_EQ(map_field.default_value()->kind(), t_const_value::CV_LIST);
  EXPECT_FALSE(map_field.default_value()->is_empty());

  EXPECT_EQ(list_field.default_value()->kind(), t_const_value::CV_MAP);
  EXPECT_FALSE(list_field.default_value()->is_empty());

  EXPECT_EQ(set_field.default_value()->kind(), t_const_value::CV_MAP);
  EXPECT_FALSE(set_field.default_value()->is_empty());

  EXPECT_EQ(constant_value->kind(), t_const_value::CV_LIST);
  EXPECT_FALSE(constant_value->is_empty());

  mutate();

  // After mutation, values should be unchanged due to being non-empty
  EXPECT_EQ(map_field.default_value()->kind(), t_const_value::CV_LIST);
  EXPECT_EQ(list_field.default_value()->kind(), t_const_value::CV_MAP);
  EXPECT_EQ(set_field.default_value()->kind(), t_const_value::CV_MAP);
  EXPECT_EQ(constant_value->kind(), t_const_value::CV_LIST);
}

} // namespace apache::thrift::compiler
