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

#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/diagnostic.h>
#include <thrift/compiler/source_location.h>

namespace apache::thrift::compiler {

struct sema_params;

struct parsing_params {
  parsing_params() noexcept {} // Disable aggregate initialization.

  /**
   * Strictness level.
   */
  int strict = 127;

  /**
   * Whether or not 64-bit constants will generate a warning.
   *
   * Some languages don't support 64-bit constants, but many do, so we can
   * suppress this warning for projects that don't use any non-64-bit-safe
   * languages.
   */
  bool allow_64bit_consts = false;

  /**
   * Whether or not a missing include file will end parsing.
   *
   * The resulting program won't be generatable, but this is
   * useful for codemod tooling.
   */
  bool allow_missing_includes = false;

  /**
   * Whether (Thrift) `include` paths should be resolved against the directory
   * of the parent (i.e., current) file, prior to the explicitly provided
   * include paths.
   */
  bool allow_self_relative_includes = true;

  /**
   * Whether to use the legacy type ref resolution behavior, which produces a
   * worse AST. Do not use in new code. Only enabled for plugins.
   */
  bool use_legacy_type_ref_resolution = false;

  /**
   * Whether to use the global resolution (~ via t_global_scope) for identifiers
   * or rely only on the local resolution (~ via program_scope).
   */
  bool use_global_resolution = true;

  /**
   * Search path for includes.
   */
  std::vector<std::string> incl_searchpath;

  /**
   * Allow parsing Bidi and simple sink while under development.
   * TODO(iahs): Remove this flag once the features are released.
   */
  bool allow_unreleased_streaming = false;
};

/**
 * Parses a program, performs semantic analysis and returns the resulting AST.
 * Diagnostics such as warnings and errors are reported via `diags`.
 */
std::unique_ptr<t_program_bundle> parse_ast(
    source_manager& sm,
    diagnostics_engine& diags,
    const std::string& path,
    const parsing_params& params,
    const sema_params* sparams = nullptr,
    t_program_bundle* already_parsed = nullptr);

} // namespace apache::thrift::compiler
