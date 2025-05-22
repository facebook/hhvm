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
#include <gtest/gtest.h>
#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_set.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/generate/cpp/orderable_type_utils.h>
#include <thrift/compiler/test/gen_testing.h>
#include <thrift/compiler/test/parser_test_helpers.h>

namespace apache::thrift::compiler::cpp2 {

namespace {
const t_structured& get_structured_named(
    const t_program& p, std::string_view name) {
  for (const t_structured* structured_definition : p.structured_definitions()) {
    if (structured_definition->name() == name) {
      return *structured_definition;
    }
  }
  throw std::logic_error("Foo not found");
}

const std::string kHeaderNoUri = R"(
include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"
)";

const std::string kHeaderWithUri =
    kHeaderNoUri + "package \"apache.org/thrift/test\"\n";

const std::string kOrderabilityTestProgram = R"(
// There are 3 categories:
// * Foo*: With custom set/map
// * Bar*: Without custom set/map
// * Baz*: Special cases

@cpp.Type{template = "std::unordered_set"}
typedef set<i32> CustomSet1;
@cpp.Type{name = "std::unordered_set<int32_t>"}
typedef set<i32> CustomSet2;
@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef set<i32> CustomSet3;
typedef set<i32> CustomSet4 (cpp.template = "std::unordered_set");
typedef set<i32> CustomSet5 (cpp.type = "std::unordered_set<int32_t>");

struct Foo1 { 1: CustomSet1 foo; }
struct Foo2 { 1: CustomSet2 foo; }
struct Foo3 { 1: CustomSet3 foo; }
struct Foo4 { 1: CustomSet4 foo; }
struct Foo5 { 1: CustomSet5 foo; }
struct Foo6 { 1: list<CustomSet1> foo; }
struct Foo7 {
  @cpp.Type{template = "std::unordered_set"}
  1: set<i32> foo;
}
struct Foo8 {
  @cpp.Type{template = "std::unordered_map"}
  1: map<i32, i32> foo;
}
struct Foo9 {
  1: i32 field1;
  2: CustomSet1 field2;
}
struct Foo10 {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: optional CustomSet1 foo;
}
struct Foo11 {
  @thrift.Box
  1: optional CustomSet1 foo;
}

@cpp.Type{template = "std::deque"}
typedef list<i32> CustomList1

struct Bar1 { 1: set<i32> foo; }
struct Bar2 { 1: CustomList1 foo; }
struct Bar3 { 1: Foo1 foo; }
struct Bar4 { 1: map<i32, Foo4> foo; }

// Weirdly field adapter doesn't count as custom type, probably a bug.
struct Baz1 {
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  1: set<i32> baz;
}

// Always unorderable due to the lack of URI
@thrift.Uri{value = ""}
struct Baz2 {
  @cpp.Type{template = "std::unordered_set"}
  1: set<i32> foo;
}

// Has URI, but still unorderable since it contains unorderable field.
@thrift.Uri{value = "apache.org/thrift/Baz3"}
struct Baz3 {
  1: Baz2 foo;
}

// Already enabled custom type ordering
@cpp.EnableCustomTypeOrdering
struct Baz4 {
  @cpp.Type{template = "std::unordered_set"}
  1: set<i32> foo;
}
)";
} // namespace

TEST(OrderableTypeUtilsTest, is_orderable_set_template) {
  t_set set_t(&t_primitive_type::t_double());
  set_t.set_unstructured_annotation("cpp2.template", "blah");
  t_program program_p("path/to/program.thrift", "path/to/program.thrift");
  t_struct struct_s(&program_p, "struct_name");
  struct_s.append(std::make_unique<t_field>(&set_t, "set_field", 1));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(struct_s, true));

  struct_s.set_uri("facebook.com/path/to/struct_name");
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(struct_s, true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(struct_s, false));

  struct_s.set_uri(""); // remove uri
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(struct_s, true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(struct_s, false));

  auto builder =
      gen::cpp_annotation_builder::EnableCustomTypeOrdering(program_p);
  struct_s.add_structured_annotation(builder.make());
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(struct_s, true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(struct_s, false));
}

TEST(OrderableTypeUtilsTest, is_orderable_struct) {
  t_program program_p("path/to/program.thrift", "path/to/program.thrift");
  t_struct struct_s(&program_p, "struct_name");
  struct_s.append(std::make_unique<t_field>(
      &t_primitive_type::t_string(), "field_name", 1));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(struct_s, true));
}

TEST(OrderableTypeUtilsTest, is_orderable_struct_self_reference) {
  t_program program_p("path/to/program.thrift", "path/to/program.thrift");

  t_set set_t(&t_primitive_type::t_double());
  set_t.set_unstructured_annotation("cpp2.template", "blah");

  t_struct struct_c(&program_p, "C");
  struct_c.append(std::make_unique<t_field>(&set_t, "set_field", 1));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(struct_c, true));

  t_struct struct_b(&program_p, "B");
  t_struct struct_a(&program_p, "A");

  struct_b.append(std::make_unique<t_field>(&struct_a, "struct_a", 1));
  struct_a.append(std::make_unique<t_field>(&struct_b, "struct_b", 1));
  struct_a.append(std::make_unique<t_field>(&struct_c, "struct_c", 2));

  EXPECT_FALSE(OrderableTypeUtils::is_orderable(struct_a, true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(struct_b, true));

  std::unordered_map<t_type const*, bool> memo;
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(memo, struct_a, true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(memo, struct_b, true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(memo, struct_c, true));
}

TEST(OrderableTypeUtilsTest, CustomSetOrderabilityWithoutUri) {
  source_manager source_mgr;
  std::shared_ptr<t_program> program = dedent_and_parse_to_program(
      source_mgr, kHeaderNoUri + kOrderabilityTestProgram);

  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo1"), true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo2"), true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo3"), true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo4"), true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo5"), true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo6"), true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo7"), true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo8"), true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo9"), true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo10"), true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo11"), true));

  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Bar1"), true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Bar2"), true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Bar3"), true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Bar4"), true));

  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Baz1"), true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Baz2"), true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Baz3"), true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Baz4"), true));
}

// The orderability should be the same as previous test case
TEST(
    OrderableTypeUtilsTest,
    CustomSetOrderabilityWithUriButPreservedOldBehavior) {
  source_manager source_mgr;
  std::shared_ptr<t_program> program = dedent_and_parse_to_program(
      source_mgr, kHeaderWithUri + kOrderabilityTestProgram);

  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo1"), false));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo2"), false));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo3"), false));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo4"), false));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo5"), false));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo6"), false));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo7"), false));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo8"), false));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo9"), false));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo10"), false));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo11"), false));

  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Bar1"), false));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Bar2"), false));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Bar3"), false));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Bar4"), false));

  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Baz1"), false));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Baz2"), false));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Baz3"), false));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Baz4"), false));
}

TEST(OrderableTypeUtilsTest, CustomSetOrderabilityWithUri) {
  source_manager source_mgr;
  std::shared_ptr<t_program> program = dedent_and_parse_to_program(
      source_mgr, kHeaderWithUri + kOrderabilityTestProgram);

  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo1"), true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo2"), true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo3"), true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo4"), true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo5"), true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo6"), true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo7"), true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo8"), true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo9"), true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo10"), true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Foo11"), true));

  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Bar1"), true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Bar2"), true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Bar3"), true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Bar4"), true));

  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Baz1"), true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Baz2"), true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Baz3"), true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(*program, "Baz4"), true));
}

TEST(OrderableTypeUtilsTest, get_orderable_condition) {
  source_manager source_mgr;
  std::shared_ptr<const t_program> program =
      dedent_and_parse_to_program(source_mgr, R"(
include "thrift/annotation/cpp.thrift"

struct TestStruct {
  1: i32 a;
  2: set<i32> b;
}

union TestUnion {
  1: i32 c;
  2: string d;
}

struct TestStructCustomTypeNoOrdering {
  @cpp.Type{template = "MyCustomType"}
  1: set<i32> e;
}

@cpp.EnableCustomTypeOrdering
struct TestStructCustomTypeOrderingExplicitlyEnabled {
  @cpp.Type{template = "MyCustomType"}
  1: set<i32> e;
}

@thrift.Uri{value = "apache.org/thrift/TestStructCustomTypeOrderingImplicitlyEnabled"}
struct TestStructCustomTypeOrderingImplicitlyEnabled {
  @cpp.Type{template = "MyCustomType"}
  1: set<i32> e;
}

typedef set<i32> NonCustomSet

@cpp.Type{template = "MyCustomType"}
typedef NonCustomSet CustomSet

typedef CustomSet AliasToCustomSet

struct TestStructTypedefNonCustomSet {
  1: NonCustomSet a;
}

struct TestStructTypedefCustomSet {
  1: CustomSet a;
}

struct TestStructTypedefAliasToCustomSet {
  1: AliasToCustomSet a;
}

struct TestStructTypedefNonCustomSetWithCppType {
  @cpp.Type{template = "MyCustomType"}
  1: NonCustomSet a;
}

@thrift.Uri{value = "apache.org/thrift/TestStructTypedefCustomSetWithUri"}
struct TestStructTypedefCustomSetWithUri {
  1: CustomSet a;
}

@cpp.EnableCustomTypeOrdering
struct TestStructTypedefCustomSetWithAnnotation {
  1: CustomSet a;
}
)");

  EXPECT_EQ(
      OrderableTypeUtils::get_orderable_condition(
          get_structured_named(*program, "TestStruct"),
          true /* enableCustomTypeOrderingIfStructureHasUri */
          ),
      OrderableTypeUtils::StructuredOrderableCondition::Always);

  EXPECT_EQ(
      OrderableTypeUtils::get_orderable_condition(
          get_structured_named(*program, "TestUnion"),
          true /* enableCustomTypeOrderingIfStructureHasUri */
          ),
      OrderableTypeUtils::StructuredOrderableCondition::Always);

  EXPECT_EQ(
      OrderableTypeUtils::get_orderable_condition(
          get_structured_named(*program, "TestStructCustomTypeNoOrdering"),
          true /* enableCustomTypeOrderingIfStructureHasUri */
          ),
      OrderableTypeUtils::StructuredOrderableCondition::
          NeedsCustomTypeOrderingEnabled);

  EXPECT_EQ(
      OrderableTypeUtils::get_orderable_condition(
          get_structured_named(
              *program, "TestStructCustomTypeOrderingExplicitlyEnabled"),
          true /* enableCustomTypeOrderingIfStructureHasUri */
          ),
      OrderableTypeUtils::StructuredOrderableCondition::
          OrderableByExplicitAnnotation);

  EXPECT_EQ(
      OrderableTypeUtils::get_orderable_condition(
          get_structured_named(
              *program, "TestStructCustomTypeOrderingImplicitlyEnabled"),
          true /* enableCustomTypeOrderingIfStructureHasUri */
          ),
      OrderableTypeUtils::StructuredOrderableCondition::
          OrderableByLegacyImplicitLogicEnabledByUri);

  EXPECT_EQ(
      OrderableTypeUtils::get_orderable_condition(
          get_structured_named(
              *program, "TestStructCustomTypeOrderingImplicitlyEnabled"),
          false /* enableCustomTypeOrderingIfStructureHasUri */
          ),
      OrderableTypeUtils::StructuredOrderableCondition::
          NeedsCustomTypeOrderingEnabled);

  EXPECT_EQ(
      OrderableTypeUtils::get_orderable_condition(
          get_structured_named(*program, "TestStructTypedefNonCustomSet"),
          true /* enableCustomTypeOrderingIfStructureHasUri */
          ),
      OrderableTypeUtils::StructuredOrderableCondition::Always);

  EXPECT_EQ(
      OrderableTypeUtils::get_orderable_condition(
          get_structured_named(*program, "TestStructTypedefCustomSet"),
          true /* enableCustomTypeOrderingIfStructureHasUri */
          ),
      OrderableTypeUtils::StructuredOrderableCondition::
          NeedsCustomTypeOrderingEnabled);

  EXPECT_EQ(
      OrderableTypeUtils::get_orderable_condition(
          get_structured_named(*program, "TestStructTypedefAliasToCustomSet"),
          true /* enableCustomTypeOrderingIfStructureHasUri */
          ),
      OrderableTypeUtils::StructuredOrderableCondition::
          NeedsCustomTypeOrderingEnabled);

  EXPECT_EQ(
      OrderableTypeUtils::get_orderable_condition(
          get_structured_named(
              *program, "TestStructTypedefNonCustomSetWithCppType"),
          true /* enableCustomTypeOrderingIfStructureHasUri */
          ),
      OrderableTypeUtils::StructuredOrderableCondition::
          NeedsCustomTypeOrderingEnabled);

  EXPECT_EQ(
      OrderableTypeUtils::get_orderable_condition(
          get_structured_named(*program, "TestStructTypedefCustomSetWithUri"),
          true /* enableCustomTypeOrderingIfStructureHasUri */
          ),
      OrderableTypeUtils::StructuredOrderableCondition::
          OrderableByLegacyImplicitLogicEnabledByUri);

  EXPECT_EQ(
      OrderableTypeUtils::get_orderable_condition(
          get_structured_named(*program, "TestStructTypedefCustomSetWithUri"),
          false /* enableCustomTypeOrderingIfStructureHasUri */
          ),
      OrderableTypeUtils::StructuredOrderableCondition::
          NeedsCustomTypeOrderingEnabled);

  EXPECT_EQ(
      OrderableTypeUtils::get_orderable_condition(
          get_structured_named(
              *program, "TestStructTypedefCustomSetWithAnnotation"),
          true /* enableCustomTypeOrderingIfStructureHasUri */
          ),
      OrderableTypeUtils::StructuredOrderableCondition::
          OrderableByExplicitAnnotation);
}

} // namespace apache::thrift::compiler::cpp2
