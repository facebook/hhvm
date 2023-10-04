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

#include <thrift/compiler/sema/scope_validator.h>

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <thrift/compiler/ast/diagnostic_context.h>
#include <thrift/compiler/ast/t_const.h>
#include <thrift/compiler/ast/t_const_value.h>
#include <thrift/compiler/ast/t_enum.h>
#include <thrift/compiler/ast/t_enum_value.h>
#include <thrift/compiler/ast/t_exception.h>
#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/ast/t_function.h>
#include <thrift/compiler/ast/t_interaction.h>
#include <thrift/compiler/ast/t_service.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/ast/t_typedef.h>
#include <thrift/compiler/ast/t_union.h>
#include <thrift/compiler/diagnostic.h>
#include <thrift/compiler/lib/uri.h>

namespace apache::thrift::compiler {
namespace {

class ScopeValidatorTest : public ::testing::Test {
 public:
  ScopeValidatorTest()
      : loc(source_mgr.add_virtual_file("path/to/file.thrift", "").start),
        program{"path/to/file.thrift"} {
    program.set_name("MyProgram");
    scopeProgram.set_uri(kScopeProgramUri);
    scopeStruct.set_uri(kScopeStructUri);
    scopeUnion.set_uri(kScopeUnionUri);
    scopeException.set_uri(kScopeExceptionUri);
    scopeField.set_uri(kScopeFieldUri);
    scopeTypedef.set_uri(kScopeTypedefUri);
    scopeService.set_uri(kScopeServiceUri);
    scopeInteraction.set_uri(kScopeInteractionUri);
    scopeFunction.set_uri(kScopeFunctionUri);
    scopeEnum.set_uri(kScopeEnumUri);
    scopeEnumValue.set_uri(kScopeEnumValueUri);
    scopeConst.set_uri(kScopeConstUri);
    metaTransitive.set_uri(kTransitiveUri);
  }

  void SetUp() override {
    annotProgram.add_structured_annotation(inst(&scopeProgram));
    annotStruct.add_structured_annotation(inst(&scopeStruct));
    annotUnion.add_structured_annotation(inst(&scopeUnion));
    annotException.add_structured_annotation(inst(&scopeException));
    annotField.add_structured_annotation(inst(&scopeField));
    annotTypedef.add_structured_annotation(inst(&scopeTypedef));
    annotService.add_structured_annotation(inst(&scopeService));
    annotInteraction.add_structured_annotation(inst(&scopeInteraction));
    annotFunction.add_structured_annotation(inst(&scopeFunction));
    annotEnum.add_structured_annotation(inst(&scopeEnum));
    annotEnumValue.add_structured_annotation(inst(&scopeEnumValue));
    annotConst.add_structured_annotation(inst(&scopeConst));

    metaTransitive.add_structured_annotation(inst(&scopeStruct));
    annotStructured.add_structured_annotation(inst(&scopeStruct));
    annotStructured.add_structured_annotation(inst(&scopeUnion));
    annotStructured.add_structured_annotation(inst(&scopeException));
    annotStructured.add_structured_annotation(inst(&metaTransitive));
    annotNonTransitiveStructured.add_structured_annotation(inst(&scopeStruct));
    annotNonTransitiveStructured.add_structured_annotation(inst(&scopeUnion));
    annotNonTransitiveStructured.add_structured_annotation(
        inst(&scopeException));
    annotMyStructured.add_structured_annotation(inst(&annotStructured));
    annotMyStructured.add_structured_annotation(inst(&metaTransitive));
    annotMyNonTransitiveStructured.add_structured_annotation(
        inst(&annotNonTransitiveStructured));
    annotMyNestedStructured.add_structured_annotation(inst(&annotMyStructured));

    all_annots.emplace_back(&annotProgram);
    all_annots.emplace_back(&annotStruct);
    all_annots.emplace_back(&annotUnion);
    all_annots.emplace_back(&annotException);
    all_annots.emplace_back(&annotField);
    all_annots.emplace_back(&annotTypedef);
    all_annots.emplace_back(&annotService);
    all_annots.emplace_back(&annotInteraction);
    all_annots.emplace_back(&annotFunction);
    all_annots.emplace_back(&annotEnum);
    all_annots.emplace_back(&annotEnumValue);
    all_annots.emplace_back(&annotConst);
  }

  void annotateWithAll(t_named& node) {
    for (const auto* annot : all_annots) {
      node.add_structured_annotation(inst(annot));
    }
    node.add_structured_annotation(inst(&annotUnscoped));
  }

 protected:
  t_struct scopeProgram{nullptr, "Program"};
  t_struct scopeStruct{nullptr, "Struct"};
  t_struct scopeUnion{nullptr, "Union"};
  t_struct scopeException{nullptr, "Exception"};
  t_struct scopeField{nullptr, "Field"};
  t_struct scopeTypedef{nullptr, "Typedef"};
  t_struct scopeService{nullptr, "Service"};
  t_struct scopeInteraction{nullptr, "Interaction"};
  t_struct scopeFunction{nullptr, "Function"};
  t_struct scopeEnum{nullptr, "Enum"};
  t_struct scopeEnumValue{nullptr, "EnumValue"};
  t_struct scopeConst{nullptr, "Const"};
  t_struct metaTransitive{nullptr, "Transitive"};

  t_struct annotProgram{nullptr, "ProgramAnnot"};
  t_struct annotStruct{nullptr, "StructAnnot"};
  t_struct annotUnion{nullptr, "UnionAnnot"};
  t_struct annotException{nullptr, "ExceptionAnnot"};
  t_struct annotField{nullptr, "FieldAnnot"};
  t_struct annotTypedef{nullptr, "TypedefAnnot"};
  t_struct annotService{nullptr, "ServiceAnnot"};
  t_struct annotInteraction{nullptr, "InteractionAnnot"};
  t_struct annotFunction{nullptr, "FunctionAnnot"};
  t_struct annotEnum{nullptr, "EnumAnnot"};
  t_struct annotEnumValue{nullptr, "EnumValueAnnot"};
  t_struct annotConst{nullptr, "ConstAnnot"};
  t_struct annotUnscoped{nullptr, "UnscopedAnnot"};
  t_struct annotStructured{nullptr, "StructuredAnnot"};
  t_struct annotMyStructured{nullptr, "MyStructuredAnnot"};
  t_struct annotNonTransitiveStructured{
      nullptr, "NonTransitiveStructuredAnnot"};
  t_struct annotMyNonTransitiveStructured{
      nullptr, "MyNonTransitiveStructuredAnnot"};
  t_struct annotMyNestedStructured{nullptr, "MyNestedStructuredAnnot"};
  std::vector<t_struct*> all_annots;

  source_manager source_mgr;
  source_location loc;
  t_program program{"path/to/file.thrift"};

  std::unique_ptr<t_const> inst(const t_type* ttype) {
    auto value = std::make_unique<t_const_value>();
    value->set_map();
    value->set_ttype(t_type_ref::from_ptr(ttype));
    auto result =
        std::make_unique<t_const>(&program, ttype, "", std::move(value));
    result->set_src_range({loc, loc});
    return result;
  }

  diagnostic_results validate(const t_named& node) {
    diagnostic_results results;
    diagnostic_context ctx(source_mgr, results, diagnostic_params::keep_all());
    ctx.begin_visit(program);
    validate_annotation_scopes(ctx, node);
    return results;
  }

  void runTest(t_named&& node, std::string scope) {
    annotateWithAll(node);
    auto result = validate(node);
    std::vector<diagnostic> expected;
    std::string matching_name = scope + "Annot";
    for (const auto* annot : all_annots) {
      if (matching_name == annot->name()) {
        continue;
      }
      expected.emplace_back(
          diagnostic_level::error,
          "`" + annot->name() + "` cannot annotate `" + node.name() + "`",
          "path/to/file.thrift",
          1);
    }
    expected.emplace_back(
        diagnostic_level::warning,
        "Using `UnscopedAnnot` as an annotation, even though it has not been "
        "enabled for any annotation scope.",
        "path/to/file.thrift",
        1);
    EXPECT_THAT(result.diagnostics(), ::testing::ContainerEq(expected));
  }
};

TEST_F(ScopeValidatorTest, Program) {
  runTest(std::move(program), "Program");
}

TEST_F(ScopeValidatorTest, Struct) {
  runTest(t_struct{&program, "MyStruct"}, "Struct");
}

TEST_F(ScopeValidatorTest, Union) {
  runTest(t_union{&program, "MyUnion"}, "Union");
}

TEST_F(ScopeValidatorTest, Exception) {
  runTest(t_exception{&program, "MyException"}, "Exception");
}

TEST_F(ScopeValidatorTest, Field) {
  runTest(t_field{t_base_type::t_i32(), "my_field"}, "Field");
}

TEST_F(ScopeValidatorTest, Typedef) {
  runTest(t_typedef{&program, "MyTypedef", t_base_type::t_void()}, "Typedef");
}

TEST_F(ScopeValidatorTest, Service) {
  runTest(t_service{&program, "MyService"}, "Service");
}

TEST_F(ScopeValidatorTest, Interaction) {
  runTest(t_interaction{&program, "MyInteraction"}, "Interaction");
}

TEST_F(ScopeValidatorTest, Function) {
  runTest(
      t_function(
          &program, t_type_ref::from_req_ptr(&t_base_type::t_i32()), "my_func"),
      "Function");
}

TEST_F(ScopeValidatorTest, Enum) {
  runTest(t_enum{&program, "MyEnum"}, "Enum");
}

TEST_F(ScopeValidatorTest, EnumValue) {
  runTest(t_enum_value{"MyEnumValue"}, "EnumValue");
}

TEST_F(ScopeValidatorTest, Const) {
  runTest(t_const{&program, t_base_type::t_i32(), "MyConst", nullptr}, "Const");
}

TEST_F(ScopeValidatorTest, StructWithTransitiveStructuredScope) {
  t_struct strct{&program, "MyStruct"};
  strct.add_structured_annotation(inst(&annotMyStructured));
  strct.add_structured_annotation(inst(&annotMyNestedStructured));
  auto result = validate(strct);
  EXPECT_TRUE(result.diagnostics().empty());
}

TEST_F(ScopeValidatorTest, FieldWithTransitiveStructuredScope) {
  t_field field{&t_base_type::t_i32(), "MyField"};
  field.add_structured_annotation(inst(&annotMyStructured));
  field.add_structured_annotation(inst(&annotMyNestedStructured));
  auto result = validate(field);
  std::vector<diagnostic> expected{
      {diagnostic_level::error,
       "`MyStructuredAnnot` cannot annotate `" + field.name() + "`",
       "path/to/file.thrift",
       1},
      {diagnostic_level::error,
       "`MyNestedStructuredAnnot` cannot annotate `" + field.name() + "`",
       "path/to/file.thrift",
       1}};
  EXPECT_THAT(result.diagnostics(), ::testing::ContainerEq(expected));
}

TEST_F(ScopeValidatorTest, StructWithNonTransitiveStructuredScope) {
  t_struct strct{&program, "MyStruct"};
  strct.add_structured_annotation(inst(&annotMyNonTransitiveStructured));
  auto result = validate(strct);
  std::vector<diagnostic> expected{
      {diagnostic_level::warning,
       "Using `MyNonTransitiveStructuredAnnot` as an annotation, even though "
       "it has not been enabled for any annotation scope.",
       "path/to/file.thrift",
       1}};
  EXPECT_THAT(result.diagnostics(), ::testing::ContainerEq(expected));
}

TEST_F(ScopeValidatorTest, FieldWithNonTransitiveStructuredScope) {
  t_field field{&t_base_type::t_i32(), "MyField"};
  field.add_structured_annotation(inst(&annotMyNonTransitiveStructured));
  auto result = validate(field);
  std::vector<diagnostic> expected{
      {diagnostic_level::warning,
       "Using `MyNonTransitiveStructuredAnnot` as an annotation, even though "
       "it has not been enabled for any annotation scope.",
       "path/to/file.thrift",
       1}};
  EXPECT_THAT(result.diagnostics(), ::testing::ContainerEq(expected));
}

TEST_F(ScopeValidatorTest, StructWithTypedefedScope) {
  t_struct strct{&program, "MyStruct"};
  t_typedef typedf{&program, "AnnotationTypedef", annotStruct};
  strct.add_structured_annotation(inst(&typedf));
  auto result = validate(strct);
  std::vector<diagnostic> expected{
      {diagnostic_level::warning,
       "Using `AnnotationTypedef` as an annotation, even though "
       "it has not been enabled for any annotation scope.",
       "path/to/file.thrift",
       1}};
  EXPECT_THAT(result.diagnostics(), ::testing::ContainerEq(expected));
}

} // namespace
} // namespace apache::thrift::compiler
