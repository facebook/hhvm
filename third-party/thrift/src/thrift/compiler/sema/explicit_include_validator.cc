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

#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/sema/explicit_include_validator.h>

namespace apache::thrift::compiler {

namespace {

std::string concatenate_relative_include(
    const t_program* include, const t_program* this_program) {
  // `\\` for WiNdOwS cOmPaTiBiLiTy
  size_t last_slash = this_program->path().find_last_of("/\\");
  return fmt::format(
      "{}/{}", this_program->path().substr(0, last_slash), include->path());
}

bool program_has_include(
    diagnostic_context& ctx, const t_program* type_program) {
  const t_program* this_program = &ctx.program();
  if (this_program == type_program) {
    return true;
  }
  return ctx.cache().get(*type_program, [this_program, type_program]() {
    for (const t_include* include : this_program->includes()) {
      // note: comparing pointers fails when the include is a relative path
      const t_program* include_program = include->get_program();
      if (include_program == type_program ||
          concatenate_relative_include(include_program, this_program) ==
              type_program->path()) {
        return std::make_unique<bool>(true);
      }
    }
    return std::make_unique<bool>(false);
  });
}

void report_missing(
    diagnostic_context& ctx,
    const t_named& src,
    const t_type& type,
    diagnostic_level level) {
  ctx.report(
      src,
      level,
      "Type `{}.{}` relies on a transitive include. Add `include \"{}\"` near the top of this file.",
      type.program()->name(),
      type.name(),
      type.program()->path());
}

void visit_type(
    diagnostic_context& ctx,
    const t_named& src,
    const t_type& type,
    diagnostic_level level) {
  if (type.program() != nullptr && !program_has_include(ctx, type.program())) {
    report_missing(ctx, src, type, level);
  } else if (type.is_list()) {
    const auto& list = *dynamic_cast<const t_list*>(&type);
    visit_type(ctx, src, *list.get_elem_type(), level);
  } else if (type.is_set()) {
    const auto& set = *dynamic_cast<const t_set*>(&type);
    visit_type(ctx, src, *set.get_elem_type(), level);
  } else if (type.is_map()) {
    const auto& map = *dynamic_cast<const t_map*>(&type);
    visit_type(ctx, src, *map.get_key_type(), level);
    visit_type(ctx, src, *map.get_val_type(), level);
  }
}
} // namespace

void validate_explicit_include(
    diagnostic_context& ctx,
    const t_named& src,
    const t_type& type,
    diagnostic_level level) {
  visit_type(ctx, src, type, level);
}

} // namespace apache::thrift::compiler
