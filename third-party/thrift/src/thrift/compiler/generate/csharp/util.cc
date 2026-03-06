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

#include <thrift/compiler/generate/csharp/util.h>

#include <fmt/core.h>

#include <thrift/compiler/ast/t_container.h>
#include <thrift/compiler/ast/t_enum.h>
#include <thrift/compiler/ast/t_primitive_type.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/ast/t_union.h>
#include <thrift/compiler/diagnostic.h>

namespace apache::thrift::compiler::csharp {

std::string get_csharp_property_name(const std::string& name) {
  return name;
}

uint8_t get_csharp_ttype(const t_type* type) {
  type = type->get_true_type();

  if (const auto* prim_type = dynamic_cast<const t_primitive_type*>(type)) {
    switch (prim_type->primitive_type()) {
      case t_primitive_type::type::t_void:
        return 0; // STOP
      case t_primitive_type::type::t_bool:
        return 2; // BOOL
      case t_primitive_type::type::t_byte:
        return 3; // BYTE
      case t_primitive_type::type::t_i16:
        return 6; // I16
      case t_primitive_type::type::t_i32:
        return 8; // I32
      case t_primitive_type::type::t_i64:
        return 10; // I64
      case t_primitive_type::type::t_float:
        return 19; // FLOAT
      case t_primitive_type::type::t_double:
        return 4; // DOUBLE
      case t_primitive_type::type::t_string:
      case t_primitive_type::type::t_binary:
        return 11; // STRING
      default:
        return 0; // STOP
    }
  }

  if (dynamic_cast<const t_list*>(type)) {
    return 15; // LIST
  }

  if (dynamic_cast<const t_set*>(type)) {
    return 14; // SET
  }

  if (dynamic_cast<const t_map*>(type)) {
    return 13; // MAP
  }

  if (dynamic_cast<const t_enum*>(type)) {
    return 8; // I32 (enums are serialized as i32)
  }

  if (dynamic_cast<const t_struct*>(type) ||
      dynamic_cast<const t_union*>(type)) {
    return 12; // STRUCT
  }

  return 0; // STOP as default
}

std::string get_csharp_namespace(
    const t_program& program, diagnostics_engine& diags) {
  const std::string& csharp_ns = program.get_namespace("csharp");
  if (!csharp_ns.empty()) {
    return csharp_ns;
  }

  // For included programs (e.g., annotation files) that lack an explicit
  // namespace csharp, derive a fallback from the package or program name.
  // The root program is validated separately by the generator entry point.
  const t_package& pkg = program.package();
  if (pkg.empty()) {
    diags.error(
        program,
        "No namespace 'csharp' in `{}`. Please add "
        "'namespace csharp <YourNamespace>' to the .thrift file.",
        program.name());
    return program.name();
  }

  // Convert package path (e.g., "facebook.com/thrift/annotation/hack")
  // to C# namespace (e.g., "Facebook.Thrift.Annotation.Hack")
  // Note: std::toupper is safe here because package components are ASCII-only.
  std::string ns;
  for (std::string_view component : pkg.domain()) {
    if (!ns.empty()) {
      ns += ".";
    }
    std::string part(component);
    if (!part.empty()) {
      part[0] = std::toupper(static_cast<unsigned char>(part[0]));
    }
    ns += part;
  }
  for (std::string_view component : pkg.path()) {
    if (!ns.empty()) {
      ns += ".";
    }
    std::string part(component);
    if (!part.empty()) {
      part[0] = std::toupper(static_cast<unsigned char>(part[0]));
    }
    ns += part;
  }
  if (!ns.empty()) {
    return ns;
  }

  diags.error(
      program,
      "No namespace 'csharp' in `{}`. Please add "
      "'namespace csharp <YourNamespace>' to the .thrift file.",
      program.name());
  return program.name();
}

std::string escape_csharp_string(const std::string& value) {
  std::string escaped;
  escaped.reserve(value.size());
  for (unsigned char c : value) {
    switch (c) {
      case '\\':
        escaped += "\\\\";
        break;
      case '"':
        escaped += "\\\"";
        break;
      case '\n':
        escaped += "\\n";
        break;
      case '\r':
        escaped += "\\r";
        break;
      case '\t':
        escaped += "\\t";
        break;
      case '\0':
        escaped += "\\0";
        break;
      default:
        if (c < 0x20) {
          // Escape control characters (except those handled above)
          escaped += fmt::format("\\u{:04x}", static_cast<int>(c));
        } else {
          escaped += c;
        }
        break;
    }
  }
  return escaped;
}

std::string quote_csharp_string(const std::string& value) {
  return "\"" + escape_csharp_string(value) + "\"";
}

} // namespace apache::thrift::compiler::csharp
