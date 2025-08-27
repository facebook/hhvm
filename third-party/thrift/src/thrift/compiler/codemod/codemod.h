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

#include <functional>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/source_location.h>

namespace apache::thrift::compiler {

struct codemod_parsing_options {
  // Thrift compiler will convert the structured annotaton `@cpp.Type` to
  // `cpp.type`/`cpp.template` (lowering annotation from structured to
  // unstructured). In codemod program, by default we don't do such conversion
  // and this option changes it.
  bool skip_lowering_cpp_type_annotations = true;
  bool skip_lowering_annotations = true;
};

// Parses a Thrift file specified in the command-line arguments and runs
// `codemod`, a function implementing the codemod, on the AST representation of
// this file.
[[nodiscard]] int run_codemod(
    int argc,
    char** argv,
    codemod_parsing_options,
    std::function<void(source_manager&, t_program_bundle&)> codemod);

// DO_BEFORE(aristidis,20250609) Consolidate run_codemod overloads.
[[nodiscard]] inline int run_codemod(
    int argc,
    char** argv,
    std::function<void(source_manager&, t_program_bundle&)> codemod) {
  return run_codemod(argc, argv, {}, std::move(codemod));
}

} // namespace apache::thrift::compiler
