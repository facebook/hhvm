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
  const t_program& program = *bundle.get_root_program();
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
        // structured_type is always orderable, i.e. does not need any
        // annotation. Nothing to do (besides, optionally, checking that no
        // unnecessary annotation exists).
        assert(
            (void("Unnecessary @cpp.EnableCustomTypeOrdering: type does not "
                  "contain custom types."),
             !isAnnotated));
        return;

      case OrderableTypeUtils::StructuredOrderableCondition::
          NeedsCustomTypeOrderingEnabled:
        // structured_type could be orderable, if it was annotated or custom
        // type ordering was enable by the (legacy) URI logic.
        // If this codemod is running, both of these conditions should be
        // false:
        assert(
            (void("Logic error: if type is annotated, "
                  "NeedsCustomTypeOrderingEnabled is impossible."),
             !isAnnotated));
        assert(
            (void("Logic error: in this codemod, (legacy) implicit custom "
                  "type enabling via URI is assumed to be enabled, so "
                  "NeedsCustomTypeOrderingEnabled implies there is no URI."),
             !hasUri));
        return;

      case OrderableTypeUtils::StructuredOrderableCondition::
          OrderableByExplicitAnnotation:
        // structured_type is already made orderable by explicit annotation
        // (@cpp.EnableCustomTypeOrdering). Nothing to do here.
        assert(
            (void("Logic error: OrderableByExplicitAnnotation implies the type "
                  "is annotated."),
             isAnnotated));
        return;

      case OrderableTypeUtils::StructuredOrderableCondition::
          OrderableByLegacyImplicitLogicEnabledByUri:
        // structured_type currently relies on legacy (URI-enabled) logic to
        // be orderable. The goal of this codemod is to make this explicit
        // by adding an annotation
        assert((
            void(
                "Logic error: if type is orderable due to the legacy "
                "(URI-enabled) logic, it cannot have the explicit annotation!"),
            !isAnnotated));
        assert(
            (void("Logic error: if type is orderable due to the legacy "
                  "(URI-enabled) logic, it must have a URI!"),
             hasUri));

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
      {.skip_lowering_annotations = false},
      apache::thrift::compiler::codemod_main);
}
