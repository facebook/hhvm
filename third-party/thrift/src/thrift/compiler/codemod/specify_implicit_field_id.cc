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

#include <fmt/core.h>

#include <thrift/compiler/ast/ast_visitor.h>
#include <thrift/compiler/codemod/codemod.h>
#include <thrift/compiler/codemod/file_manager.h>

using namespace apache::thrift::compiler;

namespace {

const auto comment =
    "// @lint-ignore thrift-compiler-warning Negative field id is deprecated, "
    "don't add new ones.";

void specify_implicit_field_id(codemod::file_manager& fm, const t_field& f) {
  if (f.explicit_id()) {
    return;
  }
  auto loc = f.type().src_range().begin.offset();
  if (f.qualifier() == t_field_qualifier::optional ||
      f.qualifier() == t_field_qualifier::required) {
    loc -= std::string_view("optional ").size();
  }
  fm.add({loc, loc, fmt::format("{}\n    {}: ", comment, f.id())});
}

} // namespace

int main(int argc, char** argv) {
  return apache::thrift::compiler::run_codemod(
      argc, argv, [](source_manager& sm, t_program_bundle& pb) {
        t_program& program = *pb.root_program();
        basic_ast_visitor<true, codemod::file_manager&> visitor;
        visitor.add_field_visitor(specify_implicit_field_id);
        codemod::file_manager fm(sm, program);
        visitor(fm, program);
        fm.apply_replacements();
      });
}
