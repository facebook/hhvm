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

#include <string>

#include <thrift/compiler/ast/t_program.h>

namespace apache::thrift::compiler::swift {

/**
 * Returns true if `name` is a Swift reserved keyword that must be escaped with
 * backticks when used as an identifier.
 */
bool is_swift_keyword(const std::string& name);

/**
 * Escapes an identifier for use in Swift source: wraps reserved keywords in
 * backticks, otherwise returns the name unchanged. Used for type and enum-case
 * names (which keep their original Thrift spelling).
 */
std::string get_swift_name(const std::string& name);

/**
 * Returns the Swift property name for a Thrift field: the field's Thrift name
 * used verbatim (not case-transformed), escaped with backticks if it is a
 * reserved keyword. Matches the C# backend's convention of not transforming
 * user-defined names.
 */
std::string get_swift_property_name(const std::string& name);

/**
 * Resolves the Swift module name for a program. Prefers an explicit
 * `namespace swift`, then derives a name from the package. Either way the
 * result is sanitized to a valid Swift module identifier (no `.`/`-`, no
 * leading digit). This spike emits types at the top level of the generated file
 * (Swift has no namespace keyword), so the module name is informational only.
 * Never errors.
 */
std::string get_swift_module(const t_program& program);

/**
 * Escapes a string for a Swift string literal (without surrounding quotes).
 */
std::string escape_swift_string(const std::string& value);

/**
 * Quotes a string as a Swift string literal.
 */
std::string quote_swift_string(const std::string& value);

} // namespace apache::thrift::compiler::swift
