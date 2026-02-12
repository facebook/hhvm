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

#include <optional>
#include <string_view>
#include <fmt/core.h>
#include <thrift/compiler/ast/ast_visitor.h>
#include <thrift/compiler/ast/t_map.h>
#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/ast/t_set.h>
#include <thrift/compiler/ast/uri.h>
#include <thrift/compiler/codemod/codemod.h>
#include <thrift/compiler/codemod/file_manager.h>

using apache::thrift::compiler::source_manager;
using apache::thrift::compiler::t_program_bundle;

namespace apache::thrift::compiler {
namespace {

constexpr std::string_view kAnnotation = "@thrift.AllowUnsafeNonSealedKeyType";
constexpr const char* const kAnnotationUri = kAllowUnsafeNonSealedKeyTypeUri;

/**
 * Returns `true` if `type` is a map with a non-sealed key type or a set with a
 * non-sealed element type, and `node` does not already have the
 * `@thrift.AllowUnsafeNonSealedKeyType` annotation.
 */
bool needs_annotation(const t_type& type, const t_named& node) {
  if (node.has_structured_annotation(kAnnotationUri)) {
    return false;
  }

  if (const t_map* map = type.try_as<t_map>()) {
    return !map->key_type().deref().is_sealed();
  }
  if (const t_set* set = type.try_as<t_set>()) {
    return !set->elem_type().deref().is_sealed();
  }

  return false;
}

class AnnotateAllowUnsafeNonSealedKeyType final {
 public:
  AnnotateAllowUnsafeNonSealedKeyType(
      source_manager& src_manager, t_program& program)
      : source_manager_(src_manager),
        program_(program),
        file_manager_(source_manager_, program) {}

  void run() {
    bool any_annotated = false;

    const_ast_visitor visitor;

    visitor.add_field_visitor([&](const t_field& field) {
      const t_type& type = field.type().deref();
      if (needs_annotation(type, field)) {
        annotate_node(field.type().src_range().begin);
        any_annotated = true;
      }
    });

    visitor.add_typedef_visitor([&](const t_typedef& node) {
      const t_type& type = node.type().deref();
      if (needs_annotation(type, node)) {
        annotate_node(node.src_range().begin);
        any_annotated = true;
      }
    });

    visitor.add_function_visitor([&](const t_function& func) {
      for (const t_field& param : func.params().fields()) {
        const t_type& type = param.type().deref();
        if (needs_annotation(type, param)) {
          annotate_node(param.type().src_range().begin);
          any_annotated = true;
        }
      }
    });

    visitor(program_);

    if (any_annotated) {
      file_manager_.apply_replacements();
    }
  }

 private:
  source_manager& source_manager_;
  t_program& program_;
  codemod::file_manager file_manager_;

  /**
   * Adds any (thrift) include required for the annotation type to be available.
   */
  void maybe_add_include() {
    // In www, the include path is sometimes preceded by "fbcode", handle that
    // special case here to avoid introducing a duplicate.
    if (file_manager_.has_thrift_include(
            "fbcode/thrift/annotation/thrift.thrift")) {
      return;
    }

    file_manager_.add_include("thrift/annotation/thrift.thrift");
  }

  /**
   * Adds the `@thrift.AllowUnsafeNonSealedKeyType` annotation on the line
   * before the node at the given source location, preserving indentation.
   */
  void annotate_node(source_location loc) {
    maybe_add_include();

    const source_range line_leading_whitespace =
        file_manager_.get_line_leading_whitespace(loc);

    const uint_least32_t offset = line_leading_whitespace.begin.offset();
    file_manager_.add(
        {.begin_pos = offset,
         .end_pos = offset,
         .new_content = fmt::format(
             "{}{}\n",
             source_manager_.get_text_range(line_leading_whitespace),
             kAnnotation)});
  }
};

} // namespace
} // namespace apache::thrift::compiler

int main(int argc, char** argv) {
  return apache::thrift::compiler::run_codemod(
      argc, argv, [](source_manager& sm, t_program_bundle& pb) {
        apache::thrift::compiler::AnnotateAllowUnsafeNonSealedKeyType(
            sm, *pb.root_program())
            .run();
      });
}
