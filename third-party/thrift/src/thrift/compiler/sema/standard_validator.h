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

#include <thrift/compiler/sema/ast_validator.h>

namespace apache::thrift::compiler {
namespace detail {

/**
 * Checks if an initializer is compatible with a const or a field it
 * initializes.
 *
 * @return true if the initializer is compatible, in which case no errors were
 *         added to `diags` (although warnings may have been added). Otherwise,
 *         returns false.
 */
bool check_initializer(
    diagnostics_engine& diags,
    const t_named& node,
    const t_type* type,
    const t_const_value* initializer);

/**
 * Returns true iff the given initializer corresponds to the default value for
 * the given `type`.
 *
 * Precondition:
 * - `initializer` must be compatible with the given `type` (see
 *   `check_initializer()` above).
 */
bool is_initializer_default_value(
    const t_type& type, const t_const_value& initializer);

/**
 * Validates that any mapping value in a const initializer contains no duplicate
 * keys. Checks no duplicate keys in maps, sets, and struct initializers.
 */
void check_duplicate_keys(diagnostics_engine& diags, const t_const& const_);
void check_duplicate_keys(diagnostics_engine& diags, const t_field& field);

enum scope_check_type {
  // Default scope check, based on one-to-one mapping between AST node type and
  // scope URI
  default_scopes,
  function_parameter,
  thrown_exception,
};

template <scope_check_type check_type = scope_check_type::default_scopes>
void validate_annotation_scopes(sema_context& ctx, const t_named& node);

} // namespace detail

// The standard validator for Thrift.
ast_validator standard_validator();

} // namespace apache::thrift::compiler
