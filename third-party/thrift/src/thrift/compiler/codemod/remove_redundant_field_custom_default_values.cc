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
#include <thrift/compiler/diagnostic.h>
#include <thrift/compiler/generate/cpp/util.h>
#include <thrift/compiler/sema/standard_validator.h>
#include <thrift/compiler/source_location.h>

using apache::thrift::compiler::source_manager;
using apache::thrift::compiler::t_program_bundle;

namespace apache::thrift::compiler {
namespace {

class RemoveRedundantFieldCustomDefaultValues final {
 public:
  RemoveRedundantFieldCustomDefaultValues(
      source_manager& src_manager, t_program& program)
      : source_manager_(src_manager),
        throwaway_diagnostic_engine_(
            source_manager_, [](const diagnostic&) { /* ignore */ }),
        program_(program),
        file_manager_(source_manager_, program) {}

  void run() {
    bool any_changed = false;

    const_ast_visitor visitor;
    visitor.add_field_visitor([&](const t_field& field) {
      any_changed |= maybe_change_field(field);
    });
    visitor(program_);

    if (any_changed) {
      file_manager_.apply_replacements();
    }
  }

 private:
  source_manager& source_manager_;
  diagnostics_engine throwaway_diagnostic_engine_;
  t_program& program_;
  std::unique_ptr<t_program_bundle> program_bundle_;
  codemod::file_manager file_manager_;

  bool maybe_change_field(const t_field& field) {
    if (!should_change_field(field)) {
      return false;
    }

    return attempt_change_field(field);
  }

  bool should_change_field(const t_field& field) {
    // NOTE: logic below mirrors `validate_field_default_value()` in
    // `standard_validator.cc`

    // No custom default value => nothing to do.
    if (field.default_value() == nullptr) {
      return false;
    }

    // Skip if the field type cannot be resolved.
    try {
      field.type().deref();
    } catch (const std::runtime_error&) {
      return false;
    }

    if (!detail::check_initializer(
            throwaway_diagnostic_engine_,
            field,
            &field.type().deref(),
            field.default_value())) {
      // If initializer is not valid to begin with, stop checks and return
      // error.
      return false;
    }

    return detail::is_initializer_default_value(
        field.type().deref(), *field.default_value());
  }

  bool attempt_change_field(const t_field& field) {
    // Syntax reminder:
    // field :=
    //   attributes
    //   field_id ":" [field_qualifier] type identifier
    //     [default_value] annotations [';'] [inline_doc]
    const source_range field_name_range = field.name_range().value();
    const source_range default_value_src_range =
        field.default_value()->src_range().value();
    file_manager_.add(
        {.begin_pos = field_name_range.end.offset(),
         .end_pos = default_value_src_range.end.offset(),
         .new_content = ""});

    return true;
  }
};

} // namespace
} // namespace apache::thrift::compiler

int main(int argc, char** argv) {
  return apache::thrift::compiler::run_codemod(
      argc, argv, [](source_manager& sm, t_program_bundle& pb) {
        apache::thrift::compiler::RemoveRedundantFieldCustomDefaultValues(
            sm, *pb.root_program())
            .run();
      });
}
