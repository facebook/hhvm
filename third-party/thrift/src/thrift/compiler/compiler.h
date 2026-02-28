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
#include <optional>
#include <string>
#include <vector>

#include <thrift/compiler/diagnostic.h>
#include <thrift/compiler/parse/parse_ast.h> // parsing_params

namespace apache::thrift::compiler {
namespace detail {

/**
 * Parses the Thrift compiler's command-line arguments and returns the input
 * file name if successful, or an empty optional on failure.
 *
 * See also: `parse_args()`
 *
 */
[[nodiscard]] std::optional<std::string> parse_command_line_args(
    const std::vector<std::string>& args,
    parsing_params& parsing_params,
    sema_params& sema_params);

} // namespace detail

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
 * Flags to control code generation
 */
struct gen_params {
  bool gen_recurse = false;
  std::string genfile;
  std::vector<std::string> targets;
  std::string out_path;
  bool add_gen_dir = true;

  // If true, code generation will be skipped (regardless of other parameters).
  // This is useful, for example, to parse and validate source Thrift IDL
  // without a particular target language in mind.
  bool skip_gen = false;

  bool inject_schema_const = false;
};

/**
 * Parses the Thrift compiler's command-line arguments and returns the input
 * file name if successful, or an empty string on failure.
 *
 * Updates the given parameter objects (parsing, generator, diagnostic and
 * semantic) accordingly.
 *
 * @param arguments Command-line arguments, as typically received by the
 *        `main()` method - i.e., including the program name (in the first
 *        position) and the input file name (last).
 */
std::string parse_args(
    const std::vector<std::string>& arguments,
    parsing_params& pparams,
    gen_params& gparams,
    diagnostic_params& dparams,
    sema_params& sparams);

/**
 * Parse with the given parameters, and dump all the diagnostic messages
 * returned.
 *
 * If the parsing fails, nullptr is returned.
 */
std::unique_ptr<t_program_bundle> parse_and_dump_diagnostics(
    const std::string& filename,
    source_manager& sm,
    const parsing_params& pparams,
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
    const parsing_params& params,
    diagnostic_params dparams = {});

/**
 * Compile with the given set of parameters.
 * The parameters can be parsed from the command line arguments using `compile`.
 */
compile_result compile_with_options(
    const parsing_params& pparams,
    const gen_params& gparams,
    diagnostic_params dparams,
    sema_params sparams,
    const std::string& input_filename,
    source_manager& source_mgr);

/**
 * Runs the Thrift compiler with the specified (command-line) arguments.
 */
compile_result compile(
    const std::vector<std::string>& arguments, source_manager& sm);

} // namespace apache::thrift::compiler
