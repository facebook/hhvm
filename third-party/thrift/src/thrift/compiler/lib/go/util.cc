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
#include <string>
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

// common_initialisms from https://github.com/golang/lint/blob/master/lint.go
static const std::set<std::string> common_initialisms = {
    "ACL",  "API",  "ASCII", "CPU",  "CSS",  "DNS",  "EOF", "GUID",
    "HTML", "HTTP", "HTTPS", "ID",   "IP",   "JSON", "LHS", "QPS",
    "RAM",  "RHS",  "RPC",   "SLA",  "SMTP", "SQL",  "SSH", "TCP",
    "TLS",  "TTL",  "UDP",   "UI",   "UID",  "URI",  "URL", "UTF8",
    "UUID", "VM",   "XML",   "XMPP", "XSRF", "XSS",
};

// To avoid conflict with methods (e.g. Error(), String())
static const std::set<std::string> reserved_field_names = {
    "Error",
    "String",
};

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
  auto base_name = go_package;
  if (parts.size() > 0) {
    base_name = parts.back();
  }

  // Avoid package base name collisions with reserved words
  if (is_go_reserved_word(base_name)) {
    base_name += "_";
  }

  return base_name;
}

// Convert snake_case to UpperCamelCase and captialize common initialisms.
std::string munge_ident(const std::string& ident, bool exported, bool compat) {
  assert(!ident.empty());

  std::ostringstream out;
  size_t word_start = 0;

  for (size_t i = 0; i < ident.size(); i++) {
    char cur_char = ident.at(i);
    bool eow = false;
    if (i + 1 == ident.size()) {
      eow = true;
    } else {
      char next_char = ident.at(i + 1);
      if ((next_char == '_') ||
          (islower(cur_char) && isupper(next_char) && !compat)) {
        eow = true;
      } else if (cur_char == '_') {
        word_start = i + 1;
        if (!islower(next_char)) {
          // Keep underscores, unless followed by a lowercase word.
          out << cur_char;
        }
      }
    }

    if (!eow) {
      continue;
    }

    size_t word_len = i - word_start + 1;
    std::string word = ident.substr(word_start, word_len);
    std::string upper = boost::algorithm::to_upper_copy(word);
    bool is_initialism = (common_initialisms.count(upper) > 0);
    bool is_first_word = (word_start == 0);
    size_t next_underscore_pos = ident.find('_', word_start);
    bool is_legacy_substr_bug =
        (next_underscore_pos != std::string::npos &&
         next_underscore_pos > word_len);

    if (is_initialism) {
      // Compat: legacy generator does not change whole-string
      // initialisms to uppercase.
      // Compat: legacy generator does not change initialisms
      // at the beginning of the string to uppercase.
      // Compat: legacy generator does not change initialisms
      // to uppercase if it hits a substring bug.
      if (!(compat && word_len == ident.size()) && !(compat && is_first_word) &&
          !(compat && is_legacy_substr_bug)) {
        boost::algorithm::to_upper(word);
      }
    }

    if (is_first_word) {
      if (exported) {
        word.at(0) = toupper(word.at(0));
      } else {
        if (is_initialism) {
          // Make the entire initialism lowercase
          boost::algorithm::to_lower(word);
        } else {
          word.at(0) = tolower(word.at(0));
        }
      }
    } else {
      word.at(0) = toupper(word.at(0));
    }

    out << word;

    // reset the word
    word_start = i + 1;
  }

  auto result = out.str();

  // Compat: legacy generator adds underscores to names ending with Arg/Result.
  // Compat: legacy generator adds underscores to names startng with New.
  // (to avoid name collisions with constructors and helper arg/result structs)
  bool starts_with_new = boost::algorithm::starts_with(result, "New");
  bool ends_with_args = boost::algorithm::ends_with(result, "Arg");
  bool ends_with_rslt = boost::algorithm::ends_with(result, "Result");
  if (compat && (starts_with_new || ends_with_args || ends_with_rslt)) {
    result += '_';
  }

  if (is_go_reserved_word(result)) {
    result += '_';
  }

  return result;
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
    std::map<std::string, int32_t>& name_collisions, const std::string& name) {
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
    name_collisions[name] = 0;
  } else {
    auto numSuffix = iter->second;
    unique_name = name + std::to_string(numSuffix);
    name_collisions[name] = numSuffix + 1;
  }
  return unique_name;
}

bool is_func_go_supported(const t_function* func) {
  return !func->returns_stream() && !func->returns_sink() &&
      !func->return_type()->is_service() &&
      func->returned_interaction().empty();
  ;
}

bool is_go_reserved_word(const std::string& value) {
  return go_reserved_words.count(value) > 0;
}

bool is_type_nilable(const t_type* type) {
  // Whether the underlying Go type can be set to 'nil'.
  return type->is_list() || type->is_map() || type->is_set() ||
      type->is_binary();
}

bool is_type_go_struct(const t_type* type) {
  // Whether the given Thrift type is represented by a Go struct in generated
  // Go code.
  return type->is_struct() || type->is_union() || type->is_exception();
}

std::string get_go_func_name(const t_function* func) {
  if (func->has_annotation("go.name")) {
    return func->get_annotation("go.name");
  }
  return munge_ident(func->name());
}

std::string get_field_name(const t_field* field) {
  auto name = munge_ident(field->name());
  if (reserved_field_names.count(name) > 0) {
    name += "_";
  }
  return name;
}

} // namespace go
} // namespace compiler
} // namespace thrift
} // namespace apache
