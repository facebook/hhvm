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

#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/ast/t_structured.h>
#include <thrift/compiler/ast/t_type.h>
#include <thrift/compiler/diagnostic.h>
#include <thrift/compiler/parse/parse_ast.h>
#include <thrift/compiler/test/parser_test_helpers.h>

using namespace apache::thrift::compiler;

// Confirm that we don't crash when getting the true type of an unresolved
// type name (MissingType in the example below)
TEST(TypedefTest, bad_true_type) {
  auto source_mgr = source_manager();
  source_mgr.add_virtual_file("test.thrift", R"(
    struct MyStruct {
      1: string first;
      2: MissingType second;
    }
  )");
  auto diags = diagnostics_engine(source_mgr, [](diagnostic) {});
  auto programs = parse_ast(source_mgr, diags, "test.thrift", {});
  auto program = programs->root_program();

  const std::vector<t_structured*>& structs = program->structs_and_unions();

  EXPECT_EQ(structs.size(), 1);

  const t_structured* my_struct = structs[0];

  // Control case
  const t_field* first_field = my_struct->get_field_by_id(1);
  ASSERT_NE(first_field, nullptr);
  EXPECT_EQ(first_field->name(), "first");
  const t_type* first_field_type = first_field->get_type();
  ASSERT_NE(first_field_type, nullptr);
  EXPECT_NE(first_field_type->get_true_type(), nullptr);

  // Missing type case
  const t_field* second_field = my_struct->get_field_by_id(2);
  ASSERT_NE(second_field, nullptr);
  EXPECT_EQ(second_field->name(), "second");
  const t_type* second_field_type = second_field->get_type();
  ASSERT_NE(second_field_type, nullptr);
  EXPECT_EQ(second_field_type->get_true_type(), nullptr);
}

TEST(TypedefTest, inherited_annotations) {
  t_program program("test");
  t_scope scope;
  t_typedef t1(&program, &t_base_type::t_i32(), "t1", &scope);
  t_typedef t2(&program, &t1, "t2", &scope);
  t_typedef t3(&program, &t2, "t3", &scope);
  const t_type* p1(&t1);
  const t_type* p2(&t2);
  const t_type* p3(&t3);

  EXPECT_EQ(p1->get_annotation({"foo1", "foo2"}), "");
  EXPECT_EQ(p2->get_annotation({"foo1", "foo2"}), "");
  EXPECT_EQ(p3->get_annotation({"foo1", "foo2"}), "");
  EXPECT_EQ(t_typedef::get_first_annotation(p1, {"foo1", "foo2"}), "");
  EXPECT_EQ(t_typedef::get_first_annotation(p2, {"foo1", "foo2"}), "");
  EXPECT_EQ(t_typedef::get_first_annotation(p3, {"foo1", "foo2"}), "");
  EXPECT_EQ(t_typedef::get_first_annotation(p1, {"foo1"}), "");
  EXPECT_EQ(t_typedef::get_first_annotation(p2, {"foo1"}), "");
  EXPECT_EQ(t_typedef::get_first_annotation(p3, {"foo1"}), "");
  EXPECT_EQ(t_typedef::get_first_annotation(p1, {"foo2"}), "");
  EXPECT_EQ(t_typedef::get_first_annotation(p2, {"foo2"}), "");
  EXPECT_EQ(t_typedef::get_first_annotation(p3, {"foo2"}), "");

  t2.set_annotation("foo2", "a");
  EXPECT_EQ(p1->get_annotation({"foo1", "foo2"}), "");
  EXPECT_EQ(p2->get_annotation({"foo1", "foo2"}), "a");
  EXPECT_EQ(p3->get_annotation({"foo1", "foo2"}), "");
  EXPECT_EQ(t_typedef::get_first_annotation(p1, {"foo1", "foo2"}), "");
  EXPECT_EQ(t_typedef::get_first_annotation(p2, {"foo1", "foo2"}), "a");
  EXPECT_EQ(t_typedef::get_first_annotation(p3, {"foo1", "foo2"}), "a");
  EXPECT_EQ(t_typedef::get_first_annotation(p1, {"foo1"}), "");
  EXPECT_EQ(t_typedef::get_first_annotation(p2, {"foo1"}), "");
  EXPECT_EQ(t_typedef::get_first_annotation(p3, {"foo1"}), "");
  EXPECT_EQ(t_typedef::get_first_annotation(p1, {"foo2"}), "");
  EXPECT_EQ(t_typedef::get_first_annotation(p2, {"foo2"}), "a");
  EXPECT_EQ(t_typedef::get_first_annotation(p3, {"foo2"}), "a");

  t1.set_annotation("foo1", "b");
  EXPECT_EQ(p1->get_annotation({"foo1", "foo2"}), "b");
  EXPECT_EQ(p2->get_annotation({"foo1", "foo2"}), "a");
  EXPECT_EQ(p3->get_annotation({"foo1", "foo2"}), "");
  EXPECT_EQ(t_typedef::get_first_annotation(p1, {"foo1", "foo2"}), "b");
  EXPECT_EQ(t_typedef::get_first_annotation(p2, {"foo1", "foo2"}), "a");
  EXPECT_EQ(t_typedef::get_first_annotation(p3, {"foo1", "foo2"}), "a");
  EXPECT_EQ(t_typedef::get_first_annotation(p1, {"foo1"}), "b");
  EXPECT_EQ(t_typedef::get_first_annotation(p2, {"foo1"}), "b");
  EXPECT_EQ(t_typedef::get_first_annotation(p3, {"foo1"}), "b");
  EXPECT_EQ(t_typedef::get_first_annotation(p1, {"foo2"}), "");
  EXPECT_EQ(t_typedef::get_first_annotation(p2, {"foo2"}), "a");
  EXPECT_EQ(t_typedef::get_first_annotation(p3, {"foo2"}), "a");

  t2.set_annotation("foo1", "c");
  EXPECT_EQ(p1->get_annotation({"foo1", "foo2"}), "b");
  EXPECT_EQ(p2->get_annotation({"foo1", "foo2"}), "c");
  EXPECT_EQ(p3->get_annotation({"foo1", "foo2"}), "");
  EXPECT_EQ(t_typedef::get_first_annotation(p1, {"foo1", "foo2"}), "b");
  EXPECT_EQ(t_typedef::get_first_annotation(p2, {"foo1", "foo2"}), "c");
  EXPECT_EQ(t_typedef::get_first_annotation(p3, {"foo1", "foo2"}), "c");
  EXPECT_EQ(t_typedef::get_first_annotation(p1, {"foo1"}), "b");
  EXPECT_EQ(t_typedef::get_first_annotation(p2, {"foo1"}), "c");
  EXPECT_EQ(t_typedef::get_first_annotation(p3, {"foo1"}), "c");
  EXPECT_EQ(t_typedef::get_first_annotation(p1, {"foo2"}), "");
  EXPECT_EQ(t_typedef::get_first_annotation(p2, {"foo2"}), "a");
  EXPECT_EQ(t_typedef::get_first_annotation(p3, {"foo2"}), "a");

  t3.set_annotation("foo2", "d");
  EXPECT_EQ(p1->get_annotation({"foo1", "foo2"}), "b");
  EXPECT_EQ(p2->get_annotation({"foo1", "foo2"}), "c");
  EXPECT_EQ(p3->get_annotation({"foo1", "foo2"}), "d");
  EXPECT_EQ(t_typedef::get_first_annotation(p1, {"foo1", "foo2"}), "b");
  EXPECT_EQ(t_typedef::get_first_annotation(p2, {"foo1", "foo2"}), "c");
  EXPECT_EQ(t_typedef::get_first_annotation(p3, {"foo1", "foo2"}), "d");
  EXPECT_EQ(t_typedef::get_first_annotation(p1, {"foo1"}), "b");
  EXPECT_EQ(t_typedef::get_first_annotation(p2, {"foo1"}), "c");
  EXPECT_EQ(t_typedef::get_first_annotation(p3, {"foo1"}), "c");
  EXPECT_EQ(t_typedef::get_first_annotation(p1, {"foo2"}), "");
  EXPECT_EQ(t_typedef::get_first_annotation(p2, {"foo2"}), "a");
  EXPECT_EQ(t_typedef::get_first_annotation(p3, {"foo2"}), "d");
}
