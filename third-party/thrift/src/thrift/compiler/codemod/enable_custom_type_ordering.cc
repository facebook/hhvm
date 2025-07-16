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

#include <glog/logging.h>
#include <thrift/compiler/ast/ast_visitor.h>
#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/codemod/codemod.h>
#include <thrift/compiler/codemod/file_manager.h>
#include <thrift/compiler/generate/cpp/orderable_type_utils.h>

namespace apache::thrift::compiler {
namespace {

using apache::thrift::compiler::cpp2::OrderableTypeUtils;

void codemod_main(source_manager& sm, t_program_bundle& bundle) {
  const t_program& program = *bundle.root_program();
  codemod::file_manager fm(sm, program);
  const_ast_visitor visitor;
  visitor.add_structured_definition_visitor([&](const t_structured&
                                                    structured_type) {
    const bool hasUri = !structured_type.uri().empty();
    const bool isAnnotated = structured_type.find_structured_annotation_or_null(
        kCppEnableCustomTypeOrdering);

    switch (OrderableTypeUtils::get_orderable_condition(
        structured_type,
        true /* enableCustomTypeOrderingIfStructureHasUri */)) {
      case OrderableTypeUtils::StructuredOrderableCondition::Always:
      case OrderableTypeUtils::StructuredOrderableCondition::
          OrderableByNestedLegacyImplicitLogicEnabledByUri:
        CHECK(!isAnnotated)
            << "Unnecessary @cpp.EnableCustomTypeOrdering: type does not "
            << "contain custom types: " << structured_type.name();
        return;

      case OrderableTypeUtils::StructuredOrderableCondition::NotOrderable:
        CHECK(!isAnnotated)
            << "Misleading @cpp.EnableCustomTypeOrdering: type is not "
            << "orderable: " << structured_type.name();
        return;

      case OrderableTypeUtils::StructuredOrderableCondition::
          OrderableByExplicitAnnotation:
      case OrderableTypeUtils::StructuredOrderableCondition::
          OrderableByExplicitAnnotationAndNestedLegacyImplicitLogic:
        // structured_type is already made orderable by explicit annotation
        // (@cpp.EnableCustomTypeOrdering). Nothing to do here.
        CHECK(isAnnotated)
            << "Logic error: OrderableByExplicitAnnotation implies the type "
            << "is annotated: " << structured_type.name();
        return;

      case OrderableTypeUtils::StructuredOrderableCondition::
          OrderableByLegacyImplicitLogicEnabledByUri:
        // structured_type currently relies on legacy (URI-enabled) logic to
        // be orderable. The goal of this codemod is to make this explicit
        // by adding an annotation
        CHECK(!isAnnotated)
            << "Logic error: if type is orderable due to the legacy "
            << "(URI-enabled) logic, it cannot have the explicit annotation: "
            << structured_type.name();
        CHECK(hasUri) << "Logic error: if type is orderable due to the legacy "
                      << "(URI-enabled) logic, it must have a URI: "
                      << structured_type.name();

        fm.add_include("thrift/annotation/cpp.thrift");
        size_t offset = fm.to_offset(structured_type.src_range().begin);
        fm.add({offset, offset, "@cpp.EnableCustomTypeOrdering\n"});
    }
  });
  visitor(program);
  fm.apply_replacements();
}
} // namespace
} // namespace apache::thrift::compiler

int main(int argc, char** argv) {
  return run_codemod(
      argc,
      argv,
      {.skip_lowering_cpp_type_annotations = false},
      apache::thrift::compiler::codemod_main);
}
