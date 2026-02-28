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

#include <thrift/compiler/diagnostic.h>
#include <thrift/compiler/sema/ast_validator.h>

namespace apache::thrift::compiler {

inline constexpr std::string_view implicit_include_rule_name =
    "implicit-include";

/**
 * Transitive includes "work" in Thrift C++, but they invite vexing bugs if
 * multiple transitive includes have the same module name. In other languages
 * without transitive includes such as Rust, they cause compilation errors.
 * Thrift-python C API doesn't leak field types in headers, so transitive
 * includes cause confusing compilation errors re: template specialization not
 * found.
 *
 * This validator does not yet check any constants, e.g., as field default
 * values. Leaving this for a follow-on by a thrift language owner who would
 * benefit.
 */

void add_explicit_include_validators(
    ast_validator& validator,
    bool skip_annotations = false,
    bool skip_service_extends = false);

} // namespace apache::thrift::compiler
