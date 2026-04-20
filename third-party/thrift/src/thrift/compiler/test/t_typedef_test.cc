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

#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/ast/t_structured.h>
#include <thrift/compiler/ast/t_type.h>
#include <thrift/compiler/ast/uri.h>
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
  auto diags = diagnostics_engine(source_mgr, [](const diagnostic&) {});
  auto programs = parse_ast(source_mgr, diags, "test.thrift", {});
  auto program = programs->root_program();

  const std::vector<t_structured*>& structs = program->structs_and_unions();

  EXPECT_EQ(structs.size(), 1);

  const t_structured* my_struct = structs[0];

  // Control case
  const t_field* first_field = my_struct->get_field_by_id(1);
  ASSERT_NE(first_field, nullptr);
  EXPECT_EQ(first_field->name(), "first");
  ASSERT_TRUE(first_field->type().resolved());
  EXPECT_NE(first_field->type()->get_true_type(), nullptr);

  // Missing type case
  const t_field* second_field = my_struct->get_field_by_id(2);
  ASSERT_NE(second_field, nullptr);
  EXPECT_EQ(second_field->name(), "second");
  EXPECT_FALSE(second_field->type().empty());
  EXPECT_TRUE(second_field->type().unresolved());
}

TEST(TypedefTest, inherited_annotations) {
  t_program program("test", "test");
  t_global_scope scope;
  t_typedef t1(&program, "t1", t_primitive_type::t_i32());
  t_typedef t2(&program, "t2", t1);
  t_typedef t3(&program, "t3", t2);
  const t_type* p1(&t1);
  const t_type* p2(&t2);
  const t_type* p3(&t3);

  EXPECT_EQ(p1->get_unstructured_annotation({"foo1", "foo2"}), "");
  EXPECT_EQ(p2->get_unstructured_annotation({"foo1", "foo2"}), "");
  EXPECT_EQ(p3->get_unstructured_annotation({"foo1", "foo2"}), "");
  EXPECT_EQ(
      t_typedef::get_first_unstructured_annotation(p1, {"foo1", "foo2"}), "");
  EXPECT_EQ(
      t_typedef::get_first_unstructured_annotation(p2, {"foo1", "foo2"}), "");
  EXPECT_EQ(
      t_typedef::get_first_unstructured_annotation(p3, {"foo1", "foo2"}), "");
  EXPECT_EQ(t_typedef::get_first_unstructured_annotation(p1, {"foo1"}), "");
  EXPECT_EQ(t_typedef::get_first_unstructured_annotation(p2, {"foo1"}), "");
  EXPECT_EQ(t_typedef::get_first_unstructured_annotation(p3, {"foo1"}), "");
  EXPECT_EQ(t_typedef::get_first_unstructured_annotation(p1, {"foo2"}), "");
  EXPECT_EQ(t_typedef::get_first_unstructured_annotation(p2, {"foo2"}), "");
  EXPECT_EQ(t_typedef::get_first_unstructured_annotation(p3, {"foo2"}), "");

  t2.set_unstructured_annotation("foo2", "a");
  EXPECT_EQ(p1->get_unstructured_annotation({"foo1", "foo2"}), "");
  EXPECT_EQ(p2->get_unstructured_annotation({"foo1", "foo2"}), "a");
  EXPECT_EQ(p3->get_unstructured_annotation({"foo1", "foo2"}), "");
  EXPECT_EQ(
      t_typedef::get_first_unstructured_annotation(p1, {"foo1", "foo2"}), "");
  EXPECT_EQ(
      t_typedef::get_first_unstructured_annotation(p2, {"foo1", "foo2"}), "a");
  EXPECT_EQ(
      t_typedef::get_first_unstructured_annotation(p3, {"foo1", "foo2"}), "a");
  EXPECT_EQ(t_typedef::get_first_unstructured_annotation(p1, {"foo1"}), "");
  EXPECT_EQ(t_typedef::get_first_unstructured_annotation(p2, {"foo1"}), "");
  EXPECT_EQ(t_typedef::get_first_unstructured_annotation(p3, {"foo1"}), "");
  EXPECT_EQ(t_typedef::get_first_unstructured_annotation(p1, {"foo2"}), "");
  EXPECT_EQ(t_typedef::get_first_unstructured_annotation(p2, {"foo2"}), "a");
  EXPECT_EQ(t_typedef::get_first_unstructured_annotation(p3, {"foo2"}), "a");

  t1.set_unstructured_annotation("foo1", "b");
  EXPECT_EQ(p1->get_unstructured_annotation({"foo1", "foo2"}), "b");
  EXPECT_EQ(p2->get_unstructured_annotation({"foo1", "foo2"}), "a");
  EXPECT_EQ(p3->get_unstructured_annotation({"foo1", "foo2"}), "");
  EXPECT_EQ(
      t_typedef::get_first_unstructured_annotation(p1, {"foo1", "foo2"}), "b");
  EXPECT_EQ(
      t_typedef::get_first_unstructured_annotation(p2, {"foo1", "foo2"}), "a");
  EXPECT_EQ(
      t_typedef::get_first_unstructured_annotation(p3, {"foo1", "foo2"}), "a");
  EXPECT_EQ(t_typedef::get_first_unstructured_annotation(p1, {"foo1"}), "b");
  EXPECT_EQ(t_typedef::get_first_unstructured_annotation(p2, {"foo1"}), "b");
  EXPECT_EQ(t_typedef::get_first_unstructured_annotation(p3, {"foo1"}), "b");
  EXPECT_EQ(t_typedef::get_first_unstructured_annotation(p1, {"foo2"}), "");
  EXPECT_EQ(t_typedef::get_first_unstructured_annotation(p2, {"foo2"}), "a");
  EXPECT_EQ(t_typedef::get_first_unstructured_annotation(p3, {"foo2"}), "a");

  t2.set_unstructured_annotation("foo1", "c");
  EXPECT_EQ(p1->get_unstructured_annotation({"foo1", "foo2"}), "b");
  EXPECT_EQ(p2->get_unstructured_annotation({"foo1", "foo2"}), "c");
  EXPECT_EQ(p3->get_unstructured_annotation({"foo1", "foo2"}), "");
  EXPECT_EQ(
      t_typedef::get_first_unstructured_annotation(p1, {"foo1", "foo2"}), "b");
  EXPECT_EQ(
      t_typedef::get_first_unstructured_annotation(p2, {"foo1", "foo2"}), "c");
  EXPECT_EQ(
      t_typedef::get_first_unstructured_annotation(p3, {"foo1", "foo2"}), "c");
  EXPECT_EQ(t_typedef::get_first_unstructured_annotation(p1, {"foo1"}), "b");
  EXPECT_EQ(t_typedef::get_first_unstructured_annotation(p2, {"foo1"}), "c");
  EXPECT_EQ(t_typedef::get_first_unstructured_annotation(p3, {"foo1"}), "c");
  EXPECT_EQ(t_typedef::get_first_unstructured_annotation(p1, {"foo2"}), "");
  EXPECT_EQ(t_typedef::get_first_unstructured_annotation(p2, {"foo2"}), "a");
  EXPECT_EQ(t_typedef::get_first_unstructured_annotation(p3, {"foo2"}), "a");

  t3.set_unstructured_annotation("foo2", "d");
  EXPECT_EQ(p1->get_unstructured_annotation({"foo1", "foo2"}), "b");
  EXPECT_EQ(p2->get_unstructured_annotation({"foo1", "foo2"}), "c");
  EXPECT_EQ(p3->get_unstructured_annotation({"foo1", "foo2"}), "d");
  EXPECT_EQ(
      t_typedef::get_first_unstructured_annotation(p1, {"foo1", "foo2"}), "b");
  EXPECT_EQ(
      t_typedef::get_first_unstructured_annotation(p2, {"foo1", "foo2"}), "c");
  EXPECT_EQ(
      t_typedef::get_first_unstructured_annotation(p3, {"foo1", "foo2"}), "d");
  EXPECT_EQ(t_typedef::get_first_unstructured_annotation(p1, {"foo1"}), "b");
  EXPECT_EQ(t_typedef::get_first_unstructured_annotation(p2, {"foo1"}), "c");
  EXPECT_EQ(t_typedef::get_first_unstructured_annotation(p3, {"foo1"}), "c");
  EXPECT_EQ(t_typedef::get_first_unstructured_annotation(p1, {"foo2"}), "");
  EXPECT_EQ(t_typedef::get_first_unstructured_annotation(p2, {"foo2"}), "a");
  EXPECT_EQ(t_typedef::get_first_unstructured_annotation(p3, {"foo2"}), "d");
}

TEST(TypedefTest, duva_annotations_are_exposed_through_t_node_accessors) {
  auto source_mgr = source_manager();
  auto program = dedent_and_parse_to_program(
      source_mgr,
      R"(
        package "facebook.com/thrift/test"
        include "thrift/annotation/thrift.thrift"

        @thrift.DeprecatedUnvalidatedAnnotations{items = {"foo": "bar"}}
        typedef i32 I32WithFoo

        typedef I32WithFoo Alias

        struct S {
          @thrift.DeprecatedUnvalidatedAnnotations{items = {"baz": "qux"}}
          1: Alias field;
        }
      )",
      {},
      {});

  const auto& typedefs = program->typedefs();
  ASSERT_EQ(typedefs.size(), 2);
  const auto& alias = *typedefs[1];
  EXPECT_EQ(
      t_typedef::get_first_unstructured_annotation(
          &alias.type().deref(), {"foo"}),
      "bar");

  const auto& fields = program->structs_and_unions()[0]->fields();
  ASSERT_EQ(fields.size(), 1);
  const auto& field = fields[0];
  EXPECT_TRUE(field.has_unstructured_annotation("baz"));
  EXPECT_EQ(field.get_unstructured_annotation("baz"), "qux");
}

TEST(TypedefTest, set_unstructured_annotation_populates_duva_items_field) {
  t_program program("test", "test");
  t_typedef alias(&program, "Alias", t_primitive_type::t_i32());

  alias.set_unstructured_annotation("foo", "bar");

  const auto* annotation = alias.find_structured_annotation_or_null(
      kDeprecatedUnvalidatedAnnotationsUri);
  ASSERT_NE(annotation, nullptr);

  const auto* items =
      annotation->get_value_from_structured_annotation_or_null("items");
  ASSERT_NE(items, nullptr);
  ASSERT_EQ(items->get_map().size(), 1);
  EXPECT_EQ(items->get_map()[0].first->get_string(), "foo");
  EXPECT_EQ(items->get_map()[0].second->get_string(), "bar");
}
