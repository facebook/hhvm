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
@cpp.Type{template = "std::unordered_set"}
typedef set<i32> CustomSetCppTemplateTypedef;

@cpp.Type{name = "std::unordered_set<int32_t>"}
typedef set<i32> CustomSetCppTypeTypedef;

// Adapter on typedef should be orderable.
@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef set<i32> CustomSetAdapterTypedef;

typedef set<i32> CustomSetUnstructuredCppTemplateTypedef (cpp.template = "std::unordered_set");

typedef set<i32> CustomSetUnstructuredCppTypeTypedef (cpp.type = "std::unordered_set<int32_t>");

// ----------------------------------------------------------------------------
// The following structs all have custom sets/maps fields (either directly or
// via typedefs).

struct StructWithCustomSetCppTemplate { 1: CustomSetCppTemplateTypedef foo; }

struct StructWithCustomSetCppType { 1: CustomSetCppTypeTypedef foo; }

struct StructWithCustomSetAdapter { 1: CustomSetAdapterTypedef foo; }

struct StructWithCustomSetUnstructuredCppTemplate {
  1: CustomSetUnstructuredCppTemplateTypedef foo;
}

struct StructWithCustomSetUnstructuredCppType {
  1: CustomSetUnstructuredCppTypeTypedef foo;
}

struct StructWithListOfCustomSetCppTemplate {
  1: list<CustomSetCppTemplateTypedef> foo;
}

struct StructWithCustomSetCppTemplateField {
  @cpp.Type{template = "std::unordered_set"}
  1: set<i32> foo;
}

struct StructWithCustomMapCppTemplateField {
  @cpp.Type{template = "std::unordered_map"}
  1: map<i32, i32> foo;
}

struct StructWithI32AndCustomSetCppTemplate {
  1: i32 field1;
  2: CustomSetCppTemplateTypedef field2;
}

struct StructWithCustomSetCppTemplateRef {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: optional CustomSetCppTemplateTypedef foo;
}

struct StructWithCustomSetCppTemplateBox {
  @thrift.Box
  1: optional CustomSetCppTemplateTypedef foo;
}

// ----------------------------------------------------------------------------
// The following structs do NOT have custom sets/maps (even if they have struct
// fields, that have custom sets/maps).

@cpp.Type{template = "std::deque"}
typedef list<i32> CustomListCppTemplateTypedef


struct StructWithSet {
  1: set<i32> foo;
}

struct StructWithCustomListCppTemplate {
  1: CustomListCppTemplateTypedef foo;
}

struct StructWithStructWithCustomSetCppTemplate {
  1: StructWithCustomSetCppTemplate foo;
}

struct StructWithMapToStructWithCustomSetUnstructuredCppTemplate {
  1: map<i32, StructWithCustomSetUnstructuredCppTemplate> foo;
}
// ----------------------------------------------------------------------------

struct StructWithSetAdapterField {
  // Adapter on a field should be orderable.
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  1: set<i32> baz;
}

// Always unorderable due to the lack of URI
@thrift.Uri{value = ""}
struct StructWithCustomSetCppTypeFieldNoUri {
  @cpp.Type{template = "std::unordered_set"}
  1: set<i32> foo;
}

// Has URI, but still unorderable since it contains unorderable field.
@thrift.Uri{value = "apache.org/thrift/StructWithUriAndStructWithCustomSetCppTypeFieldNoUri"}
struct StructWithUriAndStructWithCustomSetCppTypeFieldNoUri {
  1: StructWithCustomSetCppTypeFieldNoUri foo;
}

// Already enabled custom type ordering
@cpp.EnableCustomTypeOrdering
struct StructWithCustomSetCppTypeWithEnableCustomTypeOrdering {
  @cpp.Type{template = "std::unordered_set"}
  1: set<i32> foo;
}
)";
} // namespace

TEST(OrderableTypeUtilsTest, is_orderable_set_template) {
  t_set set_t(t_primitive_type::t_double());
  set_t.set_unstructured_annotation("cpp2.template", "blah");
  t_program program_p("path/to/program.thrift", "path/to/program.thrift");
  t_struct struct_s(&program_p, "struct_name");
  struct_s.append(std::make_unique<t_field>(set_t, "set_field", 1));
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
  struct_s.append(
      std::make_unique<t_field>(t_primitive_type::t_string(), "field_name", 1));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(struct_s, true));
}

TEST(OrderableTypeUtilsTest, is_orderable_struct_self_reference) {
  t_program program_p("path/to/program.thrift", "path/to/program.thrift");

  t_set set_t(t_primitive_type::t_double());
  set_t.set_unstructured_annotation("cpp2.template", "blah");

  t_struct struct_c(&program_p, "C");
  struct_c.append(std::make_unique<t_field>(set_t, "set_field", 1));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(struct_c, true));

  t_struct struct_b(&program_p, "B");
  t_struct struct_a(&program_p, "A");

  struct_b.append(std::make_unique<t_field>(struct_a, "struct_a", 1));
  struct_a.append(std::make_unique<t_field>(struct_b, "struct_b", 1));
  struct_a.append(std::make_unique<t_field>(struct_c, "struct_c", 2));

  EXPECT_FALSE(OrderableTypeUtils::is_orderable(struct_a, true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(struct_b, true));

  std::unordered_map<t_type const*, bool> memo;
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(memo, struct_a, true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(memo, struct_b, true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(memo, struct_c, true));
}

void checkCustomSetOrderabilityWithoutUri(const t_program& program) {
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithCustomSetCppTemplate"), true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithCustomSetCppType"), true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithCustomSetAdapter"), true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(
          program, "StructWithCustomSetUnstructuredCppTemplate"),
      true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithCustomSetUnstructuredCppType"),
      true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithListOfCustomSetCppTemplate"),
      true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithCustomSetCppTemplateField"),
      true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithCustomMapCppTemplateField"),
      true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithI32AndCustomSetCppTemplate"),
      true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithCustomSetCppTemplateRef"),
      true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithCustomSetCppTemplateBox"),
      true));

  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithSet"), true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithCustomListCppTemplate"), true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithStructWithCustomSetCppTemplate"),
      true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(
          program, "StructWithMapToStructWithCustomSetUnstructuredCppTemplate"),
      true));

  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithSetAdapterField"), true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithCustomSetCppTypeFieldNoUri"),
      true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(
          program, "StructWithUriAndStructWithCustomSetCppTypeFieldNoUri"),
      true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(
          program, "StructWithCustomSetCppTypeWithEnableCustomTypeOrdering"),
      true));
}

TEST(OrderableTypeUtilsTest, CustomSetOrderabilityWithoutUri) {
  source_manager source_mgr;
  std::shared_ptr<t_program> program = dedent_and_parse_to_program(
      source_mgr,
      kHeaderNoUri + kOrderabilityTestProgram,
      parsing_params{},
      sema_params{.skip_lowering_cpp_type_annotations = false});
  checkCustomSetOrderabilityWithoutUri(*program);
}

TEST(
    OrderableTypeUtilsTest,
    CustomSetOrderabilityWithoutUriSkipLoweringCppTypeAnnotations) {
  source_manager source_mgr;
  std::shared_ptr<t_program> program = dedent_and_parse_to_program(
      source_mgr,
      kHeaderNoUri + kOrderabilityTestProgram,
      parsing_params{},
      sema_params{.skip_lowering_cpp_type_annotations = true});
  checkCustomSetOrderabilityWithoutUri(*program);
}

// The orderability should be the same as previous test case
void checkCustomSetOrderabilityWithUriButPreservedOldBehavior(
    const t_program& program) {
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithCustomSetCppTemplate"), false));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithCustomSetCppType"), false));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithCustomSetAdapter"), false));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(
          program, "StructWithCustomSetUnstructuredCppTemplate"),
      false));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithCustomSetUnstructuredCppType"),
      false));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithListOfCustomSetCppTemplate"),
      false));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithCustomSetCppTemplateField"),
      false));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithCustomMapCppTemplateField"),
      false));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithI32AndCustomSetCppTemplate"),
      false));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithCustomSetCppTemplateRef"),
      false));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithCustomSetCppTemplateBox"),
      false));

  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithSet"), false));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithCustomListCppTemplate"), false));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithStructWithCustomSetCppTemplate"),
      false));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(
          program, "StructWithMapToStructWithCustomSetUnstructuredCppTemplate"),
      false));

  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithSetAdapterField"), false));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithCustomSetCppTypeFieldNoUri"),
      false));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(
          program, "StructWithUriAndStructWithCustomSetCppTypeFieldNoUri"),
      false));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(
          program, "StructWithCustomSetCppTypeWithEnableCustomTypeOrdering"),
      false));
}
TEST(
    OrderableTypeUtilsTest,
    CustomSetOrderabilityWithUriButPreservedOldBehavior) {
  source_manager source_mgr;
  std::shared_ptr<t_program> program = dedent_and_parse_to_program(
      source_mgr,
      kHeaderWithUri + kOrderabilityTestProgram,
      parsing_params{},
      sema_params{.skip_lowering_cpp_type_annotations = false});
  checkCustomSetOrderabilityWithUriButPreservedOldBehavior(*program);
}

TEST(
    OrderableTypeUtilsTest,
    CustomSetOrderabilityWithUriButPreservedOldBehaviorSkipLoweringCppTypeAnnotations) {
  source_manager source_mgr;
  std::shared_ptr<t_program> program = dedent_and_parse_to_program(
      source_mgr,
      kHeaderWithUri + kOrderabilityTestProgram,
      parsing_params{},
      sema_params{.skip_lowering_cpp_type_annotations = true});
  checkCustomSetOrderabilityWithUriButPreservedOldBehavior(*program);
}

void checkCustomSetOrderabilityWithUri(const t_program& program) {
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithCustomSetCppTemplate"), true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithCustomSetCppType"), true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithCustomSetAdapter"), true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(
          program, "StructWithCustomSetUnstructuredCppTemplate"),
      true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithCustomSetUnstructuredCppType"),
      true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithListOfCustomSetCppTemplate"),
      true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithCustomSetCppTemplateField"),
      true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithCustomMapCppTemplateField"),
      true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithI32AndCustomSetCppTemplate"),
      true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithCustomSetCppTemplateRef"),
      true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithCustomSetCppTemplateBox"),
      true));

  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithSet"), true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithCustomListCppTemplate"), true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithStructWithCustomSetCppTemplate"),
      true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(
          program, "StructWithMapToStructWithCustomSetUnstructuredCppTemplate"),
      true));

  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithSetAdapterField"), true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(program, "StructWithCustomSetCppTypeFieldNoUri"),
      true));
  EXPECT_FALSE(OrderableTypeUtils::is_orderable(
      get_structured_named(
          program, "StructWithUriAndStructWithCustomSetCppTypeFieldNoUri"),
      true));
  EXPECT_TRUE(OrderableTypeUtils::is_orderable(
      get_structured_named(
          program, "StructWithCustomSetCppTypeWithEnableCustomTypeOrdering"),
      true));
}
TEST(OrderableTypeUtilsTest, CustomSetOrderabilityWithUri) {
  source_manager source_mgr;
  std::shared_ptr<t_program> program = dedent_and_parse_to_program(
      source_mgr,
      kHeaderWithUri + kOrderabilityTestProgram,
      parsing_params{},
      sema_params{.skip_lowering_cpp_type_annotations = false});
  checkCustomSetOrderabilityWithUri(*program);
}
TEST(
    OrderableTypeUtilsTest,
    CustomSetOrderabilityWithUriSkipLoweringCppTypeAnnotations) {
  source_manager source_mgr;
  std::shared_ptr<t_program> program = dedent_and_parse_to_program(
      source_mgr,
      kHeaderWithUri + kOrderabilityTestProgram,
      parsing_params{},
      sema_params{.skip_lowering_cpp_type_annotations = true});
  checkCustomSetOrderabilityWithUri(*program);
}

namespace {
const std::string kOrderabilityTestProgram2 = R"(
include "thrift/annotation/cpp.thrift"

struct TestStructAlwaysOrderable {
  1: i32 a;
  2: set<i32> b;
}

union TestUnionAlwaysOrderable {
  1: i32 c;
  2: string d;
}

// This struct contains a field with a custom set, but has neither implicit nor
// explicit ordering are enable, and therefore is not orderable.
struct TestStructCustomTypeNoOrdering {
  @cpp.Type{template = "MyCustomType"}
  1: set<i32> e;
}

// This struct contains a field with a custom set, and
// @cpp.EnableCustomTypeOrdering is explicitly enabled, so it is orderable.
@cpp.EnableCustomTypeOrdering
struct TestStructCustomTypeOrderingExplicitlyEnabled {
  @cpp.Type{template = "MyCustomType"}
  1: set<i32> e;
}

// This struct contains a field with a custom set, and a URI, so it is
// orderable iff implicit orderability of types with URIs is enabled (this is
// being deprecated).
@thrift.Uri{value = "apache.org/thrift/TestStructCustomTypeOrderingImplicitlyEnabled"}
struct TestStructCustomTypeOrderingImplicitlyEnabled {
  @cpp.Type{template = "MyCustomType"}
  1: set<i32> e;
}

// This struct is orderable iff the nested struct is orderable, i.e. if
// implicit ordering via URI is enabled.
struct TestStructWithNestedImplicitlyOrderableStruct {
  1: TestStructCustomTypeOrderingImplicitlyEnabled a;
}

// ----------------------------------------------------------------------------
// Same structs as above, but with typedef fields

typedef set<i32> NonCustomSetTypedef

@cpp.Type{template = "MyCustomType"}
typedef NonCustomSetTypedef CustomSetTypedef

typedef CustomSetTypedef AliasToCustomSetTypedef

struct TestStructTypedefNonCustomSet {
  1: NonCustomSetTypedef a;
}

struct TestStructTypedefCustomSet {
  1: CustomSetTypedef a;
}

struct TestStructTypedefAliasToCustomSet {
  1: AliasToCustomSetTypedef a;
}

struct TestStructTypedefNonCustomSetWithCppType {
  @cpp.Type{template = "MyCustomType"}
  1: NonCustomSetTypedef a;
}

@thrift.Uri{value = "apache.org/thrift/TestStructTypedefCustomSetWithUri"}
struct TestStructTypedefCustomSetWithUri {
  1: CustomSetTypedef a;
}

@cpp.EnableCustomTypeOrdering
struct TestStructTypedefCustomSetWithAnnotation {
  1: CustomSetTypedef a;
}
)";
} // namespace

void checkGetOrderableCondition(const t_program& program) {
  EXPECT_EQ(
      OrderableTypeUtils::get_orderable_condition(
          get_structured_named(program, "TestStructAlwaysOrderable"),
          true /* enableCustomTypeOrderingIfStructureHasUri */
          ),
      OrderableTypeUtils::StructuredOrderableCondition::Always);

  EXPECT_EQ(
      OrderableTypeUtils::get_orderable_condition(
          get_structured_named(program, "TestUnionAlwaysOrderable"),
          true /* enableCustomTypeOrderingIfStructureHasUri */
          ),
      OrderableTypeUtils::StructuredOrderableCondition::Always);

  EXPECT_EQ(
      OrderableTypeUtils::get_orderable_condition(
          get_structured_named(program, "TestStructTypedefNonCustomSet"),
          true /* enableCustomTypeOrderingIfStructureHasUri */
          ),
      OrderableTypeUtils::StructuredOrderableCondition::Always);

  EXPECT_EQ(
      OrderableTypeUtils::get_orderable_condition(
          get_structured_named(program, "TestStructTypedefCustomSet"),
          true /* enableCustomTypeOrderingIfStructureHasUri */
          ),
      OrderableTypeUtils::StructuredOrderableCondition::NotOrderable);

  EXPECT_EQ(
      OrderableTypeUtils::get_orderable_condition(
          get_structured_named(program, "TestStructTypedefAliasToCustomSet"),
          true /* enableCustomTypeOrderingIfStructureHasUri */
          ),
      OrderableTypeUtils::StructuredOrderableCondition::NotOrderable);

  EXPECT_EQ(
      OrderableTypeUtils::get_orderable_condition(
          get_structured_named(program, "TestStructTypedefCustomSetWithUri"),
          true /* enableCustomTypeOrderingIfStructureHasUri */
          ),
      OrderableTypeUtils::StructuredOrderableCondition::
          OrderableByLegacyImplicitLogicEnabledByUri);

  EXPECT_EQ(
      OrderableTypeUtils::get_orderable_condition(
          get_structured_named(program, "TestStructTypedefCustomSetWithUri"),
          false /* enableCustomTypeOrderingIfStructureHasUri */
          ),
      OrderableTypeUtils::StructuredOrderableCondition::NotOrderable);

  EXPECT_EQ(
      OrderableTypeUtils::get_orderable_condition(
          get_structured_named(
              program, "TestStructTypedefCustomSetWithAnnotation"),
          true /* enableCustomTypeOrderingIfStructureHasUri */
          ),
      OrderableTypeUtils::StructuredOrderableCondition::
          OrderableByExplicitAnnotation);
  EXPECT_EQ(
      OrderableTypeUtils::get_orderable_condition(
          get_structured_named(program, "TestStructCustomTypeNoOrdering"),
          true /* enableCustomTypeOrderingIfStructureHasUri */
          ),
      OrderableTypeUtils::StructuredOrderableCondition::NotOrderable);
  EXPECT_EQ(
      OrderableTypeUtils::get_orderable_condition(
          get_structured_named(
              program, "TestStructCustomTypeOrderingExplicitlyEnabled"),
          true /* enableCustomTypeOrderingIfStructureHasUri */
          ),
      OrderableTypeUtils::StructuredOrderableCondition::
          OrderableByExplicitAnnotation);
  EXPECT_EQ(
      OrderableTypeUtils::get_orderable_condition(
          get_structured_named(
              program, "TestStructCustomTypeOrderingImplicitlyEnabled"),
          true /* enableCustomTypeOrderingIfStructureHasUri */
          ),
      OrderableTypeUtils::StructuredOrderableCondition::
          OrderableByLegacyImplicitLogicEnabledByUri);
  EXPECT_EQ(
      OrderableTypeUtils::get_orderable_condition(
          get_structured_named(
              program, "TestStructCustomTypeOrderingImplicitlyEnabled"),
          false /* enableCustomTypeOrderingIfStructureHasUri */
          ),
      OrderableTypeUtils::StructuredOrderableCondition::NotOrderable);
  EXPECT_EQ(
      OrderableTypeUtils::get_orderable_condition(
          get_structured_named(
              program, "TestStructTypedefNonCustomSetWithCppType"),
          true /* enableCustomTypeOrderingIfStructureHasUri */
          ),
      OrderableTypeUtils::StructuredOrderableCondition::NotOrderable);
  EXPECT_EQ(
      OrderableTypeUtils::get_orderable_condition(
          get_structured_named(
              program, "TestStructWithNestedImplicitlyOrderableStruct"),
          true /* enableCustomTypeOrderingIfStructureHasUri */
          ),
      OrderableTypeUtils::StructuredOrderableCondition::
          OrderableByNestedLegacyImplicitLogicEnabledByUri);
  EXPECT_EQ(
      OrderableTypeUtils::get_orderable_condition(
          get_structured_named(
              program, "TestStructWithNestedImplicitlyOrderableStruct"),
          false /* enableCustomTypeOrderingIfStructureHasUri */
          ),
      OrderableTypeUtils::StructuredOrderableCondition::NotOrderable);
}

TEST(OrderableTypeUtilsTest, GetOrderableCondition) {
  source_manager source_mgr;
  std::shared_ptr<const t_program> program = dedent_and_parse_to_program(
      source_mgr,
      kOrderabilityTestProgram2,
      parsing_params{},
      sema_params{.skip_lowering_cpp_type_annotations = false});
  checkGetOrderableCondition(*program);
}

TEST(
    OrderableTypeUtilsTest,
    GetOrderableConditionSkipLoweringCppTypeAnnotations) {
  source_manager source_mgr;
  std::shared_ptr<const t_program> program = dedent_and_parse_to_program(
      source_mgr,
      kOrderabilityTestProgram2,
      parsing_params{},
      sema_params{.skip_lowering_cpp_type_annotations = true});
  checkGetOrderableCondition(*program);
}

} // namespace apache::thrift::compiler::cpp2
