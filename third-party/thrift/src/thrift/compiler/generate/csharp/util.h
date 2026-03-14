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

#include <cstdint>
#include <string>

#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_type.h>

namespace apache::thrift::compiler {
class diagnostics_engine;
}

namespace apache::thrift::compiler::csharp {

/**
 * Returns the property name for a C# field.
 * Fields whose name collides with the enclosing type (CS0542) should be
 * banned at the IDL level rather than silently mangled.
 */
std::string get_csharp_property_name(const std::string& name);

/**
 * Returns the TType wire type byte for a given Thrift type.
 * Examples:
 *   bool -> 2 (BOOL)
 *   i32 -> 8 (I32)
 *   string -> 11 (STRING)
 *   struct -> 12 (STRUCT)
 */
uint8_t get_csharp_ttype(const t_type* type);

/**
 * Gets the C# namespace for a program.
 * Checks for explicit "namespace csharp" annotation, then falls back to
 * deriving from the package. Emits a diagnostic error if neither is available.
 */
std::string get_csharp_namespace(
    const t_program& program, diagnostics_engine& diags);

/**
 * Escapes a string for use in a C# string literal (without surrounding quotes).
 * Handles common escape sequences like \t, \n, \r, \0, and uses Unicode escapes
 * for other non-printable characters.
 *
 * Examples:
 *   "hello"       -> "hello"
 *   "hello\tworld" -> "hello\\tworld"
 *   "line1\nline2" -> "line1\\nline2"
 *   "say \"hi\""   -> "say \\\"hi\\\""
 *   "\x01"         -> "\\u0001"
 */
std::string escape_csharp_string(const std::string& value);

/**
 * Quotes a string for use as a C# string literal.
 * Handles escaping of special characters.
 */
std::string quote_csharp_string(const std::string& value);

/**
 * Returns true if the C# type is nullable (reference type).
 */
bool is_csharp_nullable_type(const t_type* type);
} // namespace apache::thrift::compiler::csharp
