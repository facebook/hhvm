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

#include <cctype>
#include <set>
#include <boost/algorithm/string.hpp>
#include <thrift/compiler/lib/go/util.h>

namespace apache {
namespace thrift {
namespace compiler {
namespace go {

// keywords
// https://go.dev/ref/spec#Keywords
static const std::set<std::string> go_keywords = {
    "break",  "case",   "chan",        "const", "continue", "default", "defer",
    "else",   "error",  "fallthrough", "for",   "func",     "go",      "goto",
    "if",     "import", "interface",   "map",   "package",  "range",   "return",
    "select", "struct", "switch",      "type",  "var",
};
// types
// https://go.dev/ref/spec#Types
static const std::set<std::string> go_types = {
    "complex128",
    "complex64",
    "float32",
    "float64",
    "int",
    "int16",
    "int32",
    "int64",
    "int8",
    "rune",
    "uint",
    "uint16",
    "uint32",
    "uint64",
    "uintptr",
    "uint8",
};
// predelcared identifiers
// https://go.dev/ref/spec#Predeclared_identifiers
static const std::set<std::string> go_predeclared = {
    "any",   "append", "cap",     "close", "complex", "copy", "delete",
    "false", "imag",   "iota",    "len",   "make",    "new",  "nil",
    "panic", "print",  "println", "real",  "recover", "true",
};
static const std::set<std::string> go_reserved_words = []() {
  std::set<std::string> set;
  set.insert(go_keywords.cbegin(), go_keywords.cend());
  set.insert(go_types.cbegin(), go_types.cend());
  set.insert(go_predeclared.cbegin(), go_predeclared.cend());
  return set;
}();

std::string get_go_package_name(
    const t_program* program, std::string name_override) {
  if (!name_override.empty()) {
    return name_override;
  }

  std::string real_package = program->get_namespace("go");
  if (!real_package.empty()) {
    return real_package;
  }

  return boost::algorithm::to_lower_copy(program->name());
}

std::string get_go_package_dir(
    const t_program* program, std::string name_override) {
  auto go_package = get_go_package_name(program, name_override);
  return boost::replace_all_copy(go_package, ".", "/");
}

std::string get_go_package_base_name(
    const t_program* program, std::string name_override) {
  auto go_package = get_go_package_name(program, name_override);
  std::vector<std::string> parts;
  boost::split(parts, go_package, boost::is_any_of("."));
  if (parts.size() > 0) {
    return parts.back();
  }
  return go_package;
}

// Convert snake_case to UpperCamelCase and captialize common initialisms.
std::string munge_ident(const std::string& ident, bool exported, bool compat) {
  // common_initialisms from https://github.com/golang/lint/blob/master/lint.go
  static const std::set<std::string> common_initialisms = {
      "ACL",  "API",  "ASCII", "CPU",  "CSS",  "DNS",  "EOF", "GUID",
      "HTML", "HTTP", "HTTPS", "ID",   "IP",   "JSON", "LHS", "QPS",
      "RAM",  "RHS",  "RPC",   "SLA",  "SMTP", "SQL",  "SSH", "TCP",
      "TLS",  "TTL",  "UDP",   "UI",   "UID",  "URI",  "URL", "UTF8",
      "UUID", "VM",   "XML",   "XMPP", "XSRF", "XSS",
  };

  bool allUpper = std::all_of(ident.begin(), ident.end(), [](unsigned char c) {
    return !std::isalpha(c) || std::isupper(c);
  });
  if (allUpper) {
    return ident;
  }

  std::ostringstream out;
  size_t word_start = 0;
  size_t word_len = 1;
  size_t i = 0;

  while (i + 1 <= ident.size()) {
    bool eow = false;
    if (i + 1 == ident.size()) {
      eow = true;
    } else if (ident.at(i + 1) == '_') {
      eow = true;
      i++;
    } else if (islower(ident.at(i)) && !islower(ident.at(i + 1)) && !compat) {
      eow = true;
    }
    i++;

    if (!eow) {
      word_len++;
      continue;
    }

    auto word = ident.substr(word_start, word_len);

    // check for initialism
    std::string upper = boost::algorithm::to_upper_copy(word);
    bool is_initialism = (common_initialisms.count(upper) > 0);

    // Compatibility workarounds for the legacy generator (i.e. bugs):
    //  * word_start != 0
    //    * Legacy generator does not change initialisms at the beginning of the
    //    string to uppercase.
    //  * word_len != ident.size()
    //    * Legacy generator does not change whole-string initialisms to
    //    uppercase.
    if (is_initialism && compat && word_len != ident.size() &&
        word_start != 0) {
      word = upper;
      if (word_start == 0 && !exported) {
        boost::algorithm::to_lower(word);
      }
    } else if (exported || word_start > 0) {
      word.at(0) = toupper(word.at(0));
    } else {
      word.at(0) = tolower(word.at(0));
    }
    out << word;

    // reset the word
    word_len = 1;
    word_start = i;
  }

  auto result = out.str();

  // Workaround for a bug in the old Go generator.
  // It would add an underscore before "Arg" and "Result".
  if (compat) {
    bool ends_with_args = boost::algorithm::ends_with(result, "Arg");
    bool ends_with_rslt = boost::algorithm::ends_with(result, "Result");
    if (ends_with_args || ends_with_rslt) {
      result += '_';
    }
  }

  return result;
}

std::string munge_arg(const std::string& ident) {
  // we should never get an empty identifier
  assert(!ident.empty());

  // fast path / reserved name check if all lower
  bool all_lower = std::all_of(ident.begin(), ident.end(), [](unsigned char c) {
    return std::islower(c);
  });

  if (all_lower) {
    // append an _ if identifier make conflict with a reserved go word
    // (keywords, types, predelcared identifiers)
    if (is_go_reserved_word(ident)) {
      return ident + "_";
    } else {
      return ident;
    }
  }

  return munge_ident(ident, false);
}

std::string quote(const std::string& data) {
  std::ostringstream quoted;
  quoted << '"';

  for (auto ch : data) {
    if (ch == '\t') {
      quoted << '\\' << 't';
    } else if (ch == '\r') {
      quoted << '\\' << 'r';
    } else if (ch == '\n') {
      quoted << '\\' << 'n';
    } else if (ch == '\\' || ch == '"') {
      quoted << '\\' << ch;
    } else if (ch < '\x7f') {
      quoted << ch;
    } else {
      throw std::runtime_error("Non-ASCII string literal not implemented");
    }
  }

  quoted << '"';
  return quoted.str();
}

// Convert CamelCase to snake_case.
std::string snakecase(const std::string& name) {
  std::ostringstream snake;

  char last = '_';
  for (auto ch : name) {
    if (isupper(ch)) {
      if (last != '_') {
        // Don't insert '_' after an existing one, such as in `Sample_CalcRs`.
        // Also don't put a '_' right at the front.
        snake << '_';
      }
      last = (char)tolower(ch);
    } else {
      last = ch;
    }
    snake << last;
  }

  return snake.str();
}

std::string make_unique_name(
    std::map<std::string, int32_t>& name_collisions, std::string name) {
  // Uses name_collisions map provided by the caller to keep track of name
  // collisions in order to uniquify names. When a collision is detected,
  // i.e. package or program with the same name - an incrementing numeric
  // suffix is added.
  auto unique_name = name;
  if (is_go_reserved_word(name)) {
    // Emplaces only if not already in map.
    name_collisions.try_emplace(name, 0);
  }
  auto iter = name_collisions.find(name);
  if (iter == name_collisions.end()) {
    name_collisions.emplace(name, 0);
  } else {
    auto numSuffix = iter->second;
    unique_name = name + std::to_string(numSuffix);
    name_collisions.emplace(name, numSuffix + 1);
  }
  return unique_name;
}

bool is_func_go_supported(const t_function* func) {
  return !func->returns_stream() && !func->returns_sink();
}

bool is_go_reserved_word(const std::string& value) {
  return go_reserved_words.count(value) > 0;
}

} // namespace go
} // namespace compiler
} // namespace thrift
} // namespace apache
