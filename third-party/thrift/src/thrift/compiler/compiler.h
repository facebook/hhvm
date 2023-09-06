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

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <thrift/compiler/ast/diagnostic_context.h>
#include <thrift/compiler/parse/parse_ast.h> // parsing_params

namespace apache {
namespace thrift {
namespace compiler {

class t_program_bundle;

enum class compile_retcode {
  success = 0,
  failure = 1,
};

struct compile_result {
  compile_retcode retcode = compile_retcode::failure;
  diagnostic_results detail;
};

/**
 * Parse with the given parameters, and dump all the diagnostic messages
 * returned.
 *
 * If the parsing fails, nullptr is returned.
 */
std::unique_ptr<t_program_bundle> parse_and_dump_diagnostics(
    const std::string& filename,
    source_manager& sm,
    parsing_params pparams,
    diagnostic_params dparams = {});

/**
 * Parse and mutate with the given parameters
 *
 * If the parsing fails, nullptr is returned for the program bundle.
 */
std::pair<std::unique_ptr<t_program_bundle>, diagnostic_results>
parse_and_mutate_program(
    source_manager& sm,
    const std::string& filename,
    parsing_params params,
    diagnostic_params dparams = {});
std::unique_ptr<t_program_bundle> parse_and_mutate_program(
    source_manager& sm,
    diagnostic_context& ctx,
    const std::string& filename,
    parsing_params params,
    bool return_nullptr_on_failure = false,
    t_program_bundle* already_parsed = nullptr);

/**
 * Runs the Thrift parser with the specified (command-line) arguments and
 * returns the program bundle.
 * This does not run mutators and allows missing includes, as it is intended for
 * use by tooling like codemods and thrift2ast.
 */
std::unique_ptr<t_program_bundle> parse_and_get_program(
    source_manager& sm, const std::vector<std::string>& arguments);

/**
 * Runs the Thrift compiler with the specified (command-line) arguments.
 */
compile_result compile(
    const std::vector<std::string>& arguments, source_manager& sm);

} // namespace compiler
} // namespace thrift
} // namespace apache
