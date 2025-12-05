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

#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <fmt/core.h>
#include <glog/logging.h>
#include <thrift/compiler/ast/ast_visitor.h>
#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/ast/uri.h>
#include <thrift/compiler/codemod/codemod.h>
#include <thrift/compiler/codemod/file_manager.h>
#include <thrift/compiler/sema/ast_uri_utils.h>

using apache::thrift::compiler::source_manager;
using apache::thrift::compiler::t_program_bundle;

namespace apache::thrift::compiler {
namespace {

constexpr std::string_view kAnnotation = "@thrift.AllowLegacyMissingUris";
constexpr const char* const kAnnotationUri = kAllowLegacyMissingUris;

/**
 * Returns `true` if the given program needs the
 * `@thrift.AllowLegacyMissingThriftUris` annotation to be added at the package
 * level.
 */
bool needs_new_package_level_annotation(const t_program& program) {
  const t_package& package = program.package();

  // A non-empty package cannot require a (new) annotation, since all types will
  // have a URI by definition.
  if (!package.empty()) {
    return false;
  }

  // package is empty - it is either missing or explicitly set to ""

  // If the package (i.e., program) already has the annotation => nothing to do.
  if (program.has_structured_annotation(kAnnotationUri)) {
    return false;
  }

  // Check if any root definition is missing a URI and does not have the
  // annotation

  bool needs_annotation = false;
  const_ast_visitor visitor;
  visitor.add_root_definition_visitor([&](const t_named& node) {
    if (needs_annotation) {
      // Already found that the annotation is needed, short-circuit subsequent
      // checks.
      return;
    }

    if (!AstUriUtils::shouldHaveUri(node)) {
      // Node does not require a URI => skip
      return;
    }

    if (!node.uri().empty()) {
      // Node has a URI => skip
      return;
    }

    // Node requires a URI, but does not have one.

    if (node.has_structured_annotation(kAnnotationUri)) {
      // Node is already annotated with @thrift.AllowLegacyMissingUris => skip
      return;
    }

    needs_annotation = true;
  });
  visitor(program);

  return needs_annotation;
}

/**
 * Applies this codemod to the given Thrift file (`program`).
 *
 * The logic of this codemod is as follows:
 *
 * 1. First, determine if a package-level @thrift.AllowLegacyMissingThriftUris
 *    must be added, using the following logic:
 *    a. If there is an (explicit) `package` directive:
 *      i. If it is not empty => false
 *      b. If (it is empty, and) already has the annotation => false
 *    b. If any of the definitions in `program` that require a URI (struct,
 *       union, exception, enum) do not have one, and are NOT annotated with
 *       @thrift.AllogLegacyMissingThriftUris => return true c. (Else, ) =>
 *       return false
 *
 * 2. If the answer above is false => return (nothing to do)
 *
 * 3. (Else) If the answer above is true (i.e., the annotation must be added),
 *    do the followinmg:
 *    a. If there is no package directive, add one with an explicitly empty
 *       name (i.e., `package;`)
 *    b. Annotate the package directive with the annotation (including the
 *       thrift.thrift file as needed).
 *    c. Remve the annotation from any root definition that has it.
 */
class AnnotateAllowLegacyMissingUris final {
 public:
  AnnotateAllowLegacyMissingUris(
      source_manager& src_manager, t_program& program)
      : source_manager_(src_manager),
        program_(program),
        file_manager_(source_manager_, program) {}

  void run() {
    if (!needs_new_package_level_annotation(program_)) {
      return;
    }

    annotate_package();
    file_manager_.apply_replacements();
  }

 private:
  source_manager& source_manager_;
  t_program& program_;
  codemod::file_manager file_manager_;

  /**
   * Adds the `@thrift.AllowLegacyMissingUris` annotation at the package level.
   *
   * If the file has an explicit package declaration (including empty package
   * `package;`), the annotation is added before it. Otherwise, a new empty
   * package with the annotation is created.
   *
   * @return `true` if the annotation was added, `false` otherwise
   */
  bool annotate_package() {
    const std::optional<size_t> maybe_new_include_offset =
        file_manager_.add_include("thrift/annotation/thrift.thrift");

    const t_package& package = program_.package();
    if (package.is_explicit()) {
      // There is already a `package` directive: pre-pend annotation.
      const uint_least32_t package_offset = package.src_range().begin.offset();
      file_manager_.add(
          {.begin_pos = package_offset,
           .end_pos = package_offset,
           .new_content = fmt::format("{}\n", kAnnotation)});
    } else {
      // No explicit package - create empty package with annotation

      // New package should be immediately after the includes - either the newly
      // added one above, or after the last include

      const uint_least32_t new_package_offset =
          maybe_new_include_offset.has_value()
          ? maybe_new_include_offset.value()
          : file_manager_.to_offset(
                program_.includes().back()->src_range().end) +
              1;

      file_manager_.add(
          {.begin_pos = new_package_offset,
           .end_pos = new_package_offset,
           .new_content = fmt::format("\n{}\npackage;\n\n", kAnnotation)});
    }
    return true;
  }
};

} // namespace
} // namespace apache::thrift::compiler

int main(int argc, char** argv) {
  return apache::thrift::compiler::run_codemod(
      argc, argv, [](source_manager& sm, t_program_bundle& pb) {
        apache::thrift::compiler::AnnotateAllowLegacyMissingUris(
            sm, *pb.root_program())
            .run();
      });
}
