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
#include <thrift/compiler/codemod/codemod.h>
#include <thrift/compiler/compiler.h>
#include <thrift/compiler/parse/parse_ast.h>
#include <thrift/compiler/sema/sema_context.h>

namespace apache::thrift::compiler {

int run_codemod(
    int argc,
    char** argv,
    const codemod_parsing_options& options,
    const std::function<void(source_manager&, t_program_bundle&)>& codemod) {
  if (argc <= 1) {
    fmt::print(stderr, "Usage: {} <thrift-file>\n", argv[0]);
    return 1;
  }

  // Parse command-line arguments.
  auto parsing_params = compiler::parsing_params();
  auto sema_params = compiler::sema_params();
  std::optional<std::string> filename = detail::parse_command_line_args(
      {argv, argv + argc}, parsing_params, sema_params);
  if (!filename) {
    return 1;
  }
  parsing_params.allow_missing_includes = true;
  sema_params.skip_lowering_cpp_type_annotations =
      options.skip_lowering_cpp_type_annotations;
  sema_params.skip_lowering_annotations = options.skip_lowering_annotations;

  // Parse the Thrift file.
  auto source_mgr = source_manager();
  diagnostics_engine diags = options.make_diagnostics_engine(source_mgr);
  auto program_bundle =
      parse_ast(source_mgr, diags, *filename, parsing_params, &sema_params);
  if (!program_bundle) {
    return 1;
  }

  // Run the codemod.
  codemod(source_mgr, *program_bundle);
  return 0;
}

} // namespace apache::thrift::compiler
