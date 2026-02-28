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

using apache::thrift::compiler::basic_ast_visitor;
using apache::thrift::compiler::run_codemod;
using apache::thrift::compiler::source_manager;
using apache::thrift::compiler::t_field;
using apache::thrift::compiler::t_program;
using apache::thrift::compiler::t_program_bundle;
using apache::thrift::compiler::codemod::file_manager;

namespace {

void specify_implicit_func_param_id(file_manager& fm, const t_field& f) {
  if (f.explicit_id()) {
    return;
  }
  auto loc = f.type().src_range().begin.offset();
  fm.add({loc, loc, fmt::format("{}: ", f.id())});
}

} // namespace

int main(int argc, char** argv) {
  return apache::thrift::compiler::run_codemod(
      argc, argv, [](source_manager& sm, t_program_bundle& pb) {
        t_program& program = *pb.root_program();
        basic_ast_visitor<true, file_manager&> visitor;
        visitor.add_function_param_visitor(specify_implicit_func_param_id);
        file_manager fm(sm, program);
        visitor(fm, program);
        fm.apply_replacements();
      });
}
