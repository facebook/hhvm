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

#include <thrift/compiler/generate/swift/util.h>

#include <unordered_set>

#include <fmt/core.h>

namespace apache::thrift::compiler::swift {

namespace {

const std::unordered_set<std::string>& swift_keywords() {
  // Reserved words that cannot be used as bare identifiers. Context-sensitive
  // keywords are omitted; escaping conservatively with backticks is always
  // safe, so we only need the hard reserved set here.
  static const std::unordered_set<std::string> kKeywords = {
      "associatedtype",
      "class",
      "deinit",
      "enum",
      "extension",
      "fileprivate",
      "func",
      "import",
      "init",
      "inout",
      "internal",
      "let",
      "open",
      "operator",
      "private",
      "protocol",
      "public",
      "rethrows",
      "static",
      "struct",
      "subscript",
      "typealias",
      "var",
      "break",
      "case",
      "continue",
      "default",
      "defer",
      "do",
      "else",
      "fallthrough",
      "for",
      "guard",
      "if",
      "in",
      "repeat",
      "return",
      "switch",
      "where",
      "while",
      "as",
      "catch",
      "false",
      "is",
      "nil",
      "super",
      "self",
      "Self",
      "throw",
      "throws",
      "true",
      "try",
      "Any",
      "Protocol",
      "Type",
  };
  return kKeywords;
}

} // namespace

bool is_swift_keyword(const std::string& name) {
  return swift_keywords().count(name) != 0;
}

std::string get_swift_name(const std::string& name) {
  if (is_swift_keyword(name)) {
    return "`" + name + "`";
  }
  return name;
}

// Field/property names are used verbatim (only keyword-escaped); user-defined
// names are not transformed, matching the C# backend's convention.
std::string get_swift_property_name(const std::string& name) {
  return get_swift_name(name);
}

namespace {

// Derives a module name from the package domain + path (strip the TLD),
// consistent with other language generators, joined with `_`. Falls back to the
// program name when the package is empty or yields nothing. The result is
// sanitized by the caller.
std::string derive_module_from_package(const t_program& program) {
  const t_package& pkg = program.package();
  if (pkg.empty()) {
    return std::string(program.name());
  }

  std::string ns;
  const auto& domain = pkg.domain();
  // Strip the TLD (last component) only when the domain has more than one
  // component. Sema rejects single-component domains before codegen, so this
  // branch is defensive: keep the sole component rather than dropping it and
  // silently falling back to the program name.
  const size_t domain_used =
      domain.size() > 1 ? domain.size() - 1 : domain.size();
  for (size_t i = 0; i < domain_used; ++i) {
    if (!ns.empty()) {
      ns += "_";
    }
    ns += domain[i];
  }
  for (std::string_view component : pkg.path()) {
    if (!ns.empty()) {
      ns += "_";
    }
    ns += std::string(component);
  }
  if (ns.empty()) {
    return std::string(program.name());
  }
  return ns;
}

// Rewrites a raw namespace into a single valid Swift module identifier: maps
// any non-identifier character to '_', prefixes a leading digit, and escapes
// reserved keywords. Swift module names cannot contain `.`/`-`, lead with a
// digit, or be a keyword (backticks are not accepted in `import` and
// `<Module>.<Name>` positions). ASCII checks only (avoid locale-dependent
// <cctype>).
std::string sanitize_swift_module(std::string ns) {
  for (char& c : ns) {
    const bool ident = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') || c == '_';
    if (!ident) {
      c = '_';
    }
  }
  if (!ns.empty() && ns.front() >= '0' && ns.front() <= '9') {
    ns = "_" + ns;
  }
  if (is_swift_keyword(ns)) {
    ns = "_" + ns;
  }
  return ns;
}

} // namespace

std::string get_swift_module(const t_program& program) {
  // An explicit `namespace swift` wins; otherwise derive from the package. Both
  // paths are sanitized to a valid Swift module identifier. Never errors: the
  // spike emits top-level types, so the module name is informational only.
  const std::string& swift_ns = program.get_namespace("swift");
  std::string ns =
      !swift_ns.empty() ? swift_ns : derive_module_from_package(program);
  return sanitize_swift_module(std::move(ns));
}

// Escapes a Thrift string value for embedding in a Swift string literal.
// Control characters (< 0x20) and Swift meta-characters are escaped; bytes with
// the high bit set (>= 0x80) are passed through verbatim, so the input is
// assumed to be valid UTF-8 (which Thrift string values are). Non-UTF-8 input
// would produce an invalid Swift literal; callers only feed Thrift string
// values here.
std::string escape_swift_string(const std::string& value) {
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
      default:
        if (c < 0x20) {
          // Swift Unicode scalar escape: \u{XX} (covers NUL as \u{0}).
          escaped += fmt::format("\\u{{{:x}}}", static_cast<int>(c));
        } else {
          escaped += static_cast<char>(c);
        }
        break;
    }
  }
  return escaped;
}

std::string quote_swift_string(const std::string& value) {
  return "\"" + escape_swift_string(value) + "\"";
}

} // namespace apache::thrift::compiler::swift
