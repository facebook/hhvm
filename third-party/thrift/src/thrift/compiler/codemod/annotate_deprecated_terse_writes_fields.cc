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

#include <string>
#include <string_view>
#include <fmt/core.h>
#include <thrift/compiler/ast/ast_visitor.h>
#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/codemod/codemod.h>
#include <thrift/compiler/codemod/file_manager.h>
#include <thrift/compiler/generate/cpp/util.h>

using apache::thrift::compiler::source_manager;
using apache::thrift::compiler::t_program_bundle;

namespace apache::thrift::compiler {
namespace {

constexpr std::string_view kAnnotation = "@cpp.DeprecatedTerseWrite";
constexpr const char* const kAnnotationUri = kCppDeprecatedTerseWriteUri;

bool should_annotate_field(const t_field& field) {
  if (field.qualifier() != t_field_qualifier::none) {
    return false;
  }

  if (field.has_structured_annotation(kAnnotationUri)) {
    return false;
  }

  if (cpp2::deprecated_terse_writes(&field)) {
    return true;
  }

  return false;
}

class AnnonateDeprecatedTerseWriteFields final {
 public:
  AnnonateDeprecatedTerseWriteFields(
      source_manager& src_manager, t_program& program)
      : source_manager_(src_manager),
        program_(program),
        file_manager_(source_manager_, program) {}

  void run() {
    bool any_annotated = false;

    const_ast_visitor visitor;
    visitor.add_structured_definition_visitor([&](const t_structured& strct) {
      if (strct.is<t_union>()) {
        return;
      }
      for (const t_field& field : strct.fields()) {
        any_annotated |= maybe_annotate_field(field);
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

  bool maybe_annotate_field(const t_field& field) {
    if (!should_annotate_field(field)) {
      return false;
    }
    // First, make sure the include is present
    file_manager_.add_include("thrift/annotation/cpp.thrift");

    // Second, add a line above the field (with the correct indentation), with
    // the annotation.
    const source_range line_leading_whitespace =
        file_manager_.get_line_leading_whitespace(
            field.type().src_range().begin);

    const uint_least32_t start_of_line_offset =
        line_leading_whitespace.begin.offset();
    file_manager_.add(
        {.begin_pos = start_of_line_offset,
         .end_pos = start_of_line_offset,
         .new_content = fmt::format(
             "{}{}\n",
             source_manager_.get_text_range(line_leading_whitespace),
             kAnnotation)});
    return true;
  }
};

} // namespace
} // namespace apache::thrift::compiler

int main(int argc, char** argv) {
  return apache::thrift::compiler::run_codemod(
      argc, argv, [](source_manager& sm, t_program_bundle& pb) {
        apache::thrift::compiler::AnnonateDeprecatedTerseWriteFields(
            sm, *pb.get_root_program())
            .run();
      });
}
