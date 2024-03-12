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
#include <boost/algorithm/string.hpp>
#include <fmt/core.h>
#include <thrift/compiler/lib/go/util.h>

namespace apache {
namespace thrift {
namespace compiler {
namespace go {

// Name of the field of the response helper struct where
// the return value is stored (if function call is not void).
const std::string DEFAULT_RETVAL_FIELD_NAME = "success";

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

void codegen_data::set_current_program(const t_program* program) {
  current_program_ = program;

  // Prevent collisions with *this* program's package name
  auto pkg_name = go::get_go_package_base_name(program, package_override);
  go_package_name_collisions_[pkg_name] = 0;
}

std::string codegen_data::make_go_package_name_unique(const std::string& name) {
  // Uses go_package_name_collisions_ map to keep track of name collisions in
  // order to uniquify package names. When a collision is detected, i.e. package
  // or program with the same name - an incrementing numeric suffix is added.
  auto unique_name = name;
  if (is_go_reserved_word(name)) {
    // Emplaces only if not already in map.
    go_package_name_collisions_.try_emplace(name, 0);
  }
  auto iter = go_package_name_collisions_.find(name);
  if (iter == go_package_name_collisions_.end()) {
    go_package_name_collisions_[name] = 0;
  } else {
    auto numSuffix = iter->second;
    unique_name = name + std::to_string(numSuffix);
    go_package_name_collisions_[name] = numSuffix + 1;
  }
  return unique_name;
}

void codegen_data::compute_go_package_aliases() {
  for (auto include : current_program_->get_includes_for_codegen()) {
    auto package = go::get_go_package_name(include);
    auto package_base_name = go::get_go_package_base_name(include);
    auto unique_package_name = make_go_package_name_unique(
        go::munge_ident(package_base_name, /*exported*/ false));

    go_package_map_.emplace(package, unique_package_name);
  }
}

void codegen_data::compute_struct_to_field_names() {
  for (t_structured* struct_ : current_program_->structs_and_unions()) {
    struct_to_field_names[struct_->name()] =
        go::get_struct_go_field_names(struct_);
  }
}

void codegen_data::compute_service_to_req_resp_structs() {
  for (auto service : current_program_->services()) {
    std::vector<t_struct*> req_resp_structs =
        go::get_service_req_resp_structs(service);
    service_to_req_resp_structs[service->name()] = req_resp_structs;
    for (auto struct_ : req_resp_structs) {
      req_resp_struct_names.insert(struct_->name());
      struct_to_field_names[struct_->name()] =
          go::get_struct_go_field_names(struct_);
    }
  }
}

void codegen_data::add_to_thrift_metadata_types(
    const t_type* type, std::set<std::string>& visited_type_names) {
  // skip over typedefs that are not "defined"
  auto type_name = type->get_full_name();
  if (visited_type_names.count(type_name) > 0) {
    return; // Already visited
  }

  // skip over a chain of non-defined typedefs
  if (type->is_typedef()) {
    auto typedef_ = dynamic_cast<const t_typedef*>(type);
    if (typedef_->typedef_kind() != t_typedef::kind::defined) {
      auto underlying_type = typedef_->get_type();
      add_to_thrift_metadata_types(underlying_type, visited_type_names);
      return;
    }
  }

  visited_type_names.insert(type_name);

  // The recursion below is equivalent to post-order tree traversal.
  // It ensures that the types are recorded in the dependency order.

  if (type->is_typedef()) {
    auto typedef_ = dynamic_cast<const t_typedef*>(type);
    auto underlying_type = typedef_->get_type();
    add_to_thrift_metadata_types(underlying_type, visited_type_names);
  } else if (type->is_list()) {
    auto list_type = dynamic_cast<const t_list*>(type);
    auto elem_type = list_type->elem_type().get_type();
    add_to_thrift_metadata_types(elem_type, visited_type_names);
  } else if (type->is_set()) {
    auto set_type = dynamic_cast<const t_set*>(type);
    auto elem_type = set_type->elem_type().get_type();
    add_to_thrift_metadata_types(elem_type, visited_type_names);
  } else if (type->is_map()) {
    auto map_type = dynamic_cast<const t_map*>(type);
    auto key_type = map_type->key_type().get_type();
    auto val_type = map_type->val_type().get_type();
    add_to_thrift_metadata_types(key_type, visited_type_names);
    add_to_thrift_metadata_types(val_type, visited_type_names);
  }

  thrift_metadata_types.push_back(type);
}

void codegen_data::compute_thrift_metadata_types() {
  // The following items need metadata generated for them:
  //   * Struct/union/exception field types
  //   * Typedef underlying types
  //   * Function return types
  //   * Function argument types
  //   * Function exception types

  std::set<std::string> visited_type_names;

  for (auto const& struct_ : current_program_->structs_and_unions()) {
    for (auto const& field : struct_->fields()) {
      auto type = field.type().get_type();
      add_to_thrift_metadata_types(type, visited_type_names);
    }
  }
  for (auto const& exception : current_program_->exceptions()) {
    for (auto const& field : exception->get_members()) {
      auto type = field->get_type();
      add_to_thrift_metadata_types(type, visited_type_names);
    }
  }
  for (auto const& typedef_ : current_program_->typedefs()) {
    auto type = typedef_->get_type();
    add_to_thrift_metadata_types(type, visited_type_names);
  }

  for (auto const& service : current_program_->services()) {
    for (const auto& func : service->functions()) {
      if (!is_func_go_supported(&func)) {
        continue; // Skip unsupported functions
      }

      auto return_type = func.return_type().get_type();
      add_to_thrift_metadata_types(return_type, visited_type_names);

      for (const auto& parameter : func.params().get_members()) {
        auto type = parameter->get_type();
        add_to_thrift_metadata_types(type, visited_type_names);
      }
      if (func.exceptions() != nullptr) {
        for (const auto& exception : func.exceptions()->get_members()) {
          auto type = exception->get_type();
          add_to_thrift_metadata_types(type, visited_type_names);
        }
      }
    }
  }
}

bool codegen_data::is_current_program(const t_program* program) {
  return (program == current_program_);
}

std::string_view codegen_data::maybe_munge_ident_and_cache(
    const t_named* named, bool exported, bool compact) {
  assert(named);

  if (auto name = get_go_name_annotation(named)) {
    return *name;
  }

  go_munged_names_cache_key_ key{named->name(), exported, compact};
  auto& item = go_munged_names_cache_[key];
  if (item.view.data() != nullptr) {
    return item.view;
  }

  return item.view = item.ownership =
             munge_ident(named->name(), exported, compact);
}

std::string codegen_data::get_go_package_alias(const t_program* program) {
  if (is_current_program(program)) {
    return "";
  }

  auto package = go::get_go_package_name(program, package_override);
  auto iter = go_package_map_.find(package);
  if (iter != go_package_map_.end()) {
    return iter->second;
  }
  throw std::runtime_error(
      fmt::format("unable to determine Go package alias '{}'", package));
}

std::string codegen_data::go_package_alias_prefix(const t_program* program) {
  auto alias = get_go_package_alias(program);
  if (alias == "") {
    return "";
  } else {
    return alias + ".";
  }
}

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

  // Compat: legacy generator adds underscores to names ending with
  // Args/Result. Compat: legacy generator adds underscores to names startng
  // with New. (to avoid name collisions with constructors and helper
  // arg/result structs)
  bool starts_with_new = boost::algorithm::starts_with(result, "New");
  if (compat && starts_with_new) {
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

bool is_func_go_supported(const t_function* func) {
  return !func->sink_or_stream() && !func->return_type()->is_service() &&
      !func->interaction();
}

bool is_go_reserved_word(const std::string& value) {
  return go_reserved_words.count(value) > 0;
}

bool is_type_go_struct(const t_type* type) {
  // Whether the given Thrift type is represented by a Go struct:
  //   * Thrift struct    - represented by Go struct pointer
  //   * Thrift union     - represented by Go struct pointer
  //   * Thrift exception - represented by Go struct pointer
  return type->is_struct() || type->is_union() || type->is_exception();
}

bool is_type_go_nilable(const t_type* type) {
  // Whether the underlying Go type can be set to 'nil':
  //   * Thrift list      - represented by Go slice  - nilable
  //   * Thrift set       - represented by Go slice  - nilable
  //   * Thrift binary    - represented by Go slice  - nilable
  //   * Thrift map       - represented by Go map    - nilable
  // Go struct backed types (see is_type_go_struct above):
  //   * Thrift struct    - represented by Go struct pointer - nilable
  //   * Thrift union     - represented by Go struct pointer - nilable
  //   * Thrift exception - represented by Go struct pointer - nilable
  return type->is_list() || type->is_set() || type->is_map() ||
      type->is_binary() || is_type_go_struct(type);
}

bool is_type_go_comparable(
    const t_type* type, std::map<std::string, int> visited_type_names) {
  // Whether the underlying Go type is comparable.
  // As per: https://go.dev/ref/spec#Comparison_operators
  //   > Slice, map, and function types are not comparable.
  // (By extension - structs with slice of map fields are incomparable.)

  // Struct hierarchy can sometime be recursive.
  // Check if we have already visited this type.
  auto type_name = type->get_full_name();
  auto iter = visited_type_names.find(type_name);
  if (iter != visited_type_names.end() && iter->second > 1) {
    return true;
  }

  // All of the types below are represented by either slice or a map.
  auto real_type = type->get_true_type();
  if (real_type->is_list() || real_type->is_map() || real_type->is_set() ||
      real_type->is_binary()) {
    return false;
  }

  if (real_type->is_struct()) {
    auto as_struct = dynamic_cast<const t_struct*>(real_type);
    if (as_struct != nullptr) {
      for (auto member : as_struct->get_members()) {
        auto member_type = member->type().get_type();
        auto member_name = member_type->get_full_name();
        // Insert 0 if member_name is not yet in the map.
        auto emplace_pair = visited_type_names.emplace(member_name, 0);
        emplace_pair.first->second += 1;
        if (!is_type_go_comparable(member_type, visited_type_names)) {
          return false;
        }
      }
    }
  }
  return true;
}

bool is_type_metadata_primitive(const t_type* type) {
  // Whether this type is primitive from metadata.thrift perspective.
  // i.e. see ThriftPrimitiveType enum in metadata.thrift
  return type->is_bool() || type->is_byte() || type->is_i16() ||
      type->is_i32() || type->is_i64() || type->is_float() ||
      type->is_double() || type->is_binary() || type->is_string() ||
      type->is_void();
}

std::string get_go_func_name(const t_function* func) {
  auto name_override = get_go_name_annotation(func);
  if (name_override != nullptr) {
    return *name_override;
  }
  return munge_ident(func->name());
}

std::string get_go_field_name(const t_field* field) {
  auto name_override = get_go_name_annotation(field);
  if (name_override != nullptr) {
    return *name_override;
  }

  auto name = munge_ident(field->name());
  if (reserved_field_names.count(name) > 0) {
    name += "_";
  }
  return name;
}

std::set<std::string> get_struct_go_field_names(const t_structured* tstruct) {
  // Returns a set of Go field names from the given struct.
  std::set<std::string> field_names;
  for (const t_field& field : tstruct->fields()) {
    field_names.insert(go::get_go_field_name(&field));
  }
  return field_names;
}

std::vector<t_struct*> get_service_req_resp_structs(const t_service* service) {
  std::vector<t_struct*> req_resp_structs;
  auto svcGoName = go::munge_ident(service->name());
  for (auto func : service->get_functions()) {
    if (!go::is_func_go_supported(func)) {
      continue;
    }

    auto funcGoName = go::get_go_func_name(func);

    auto req_struct_name =
        go::munge_ident("req" + svcGoName + funcGoName, false);
    auto req_struct = new t_struct(service->program(), req_struct_name);
    for (auto member : func->params().get_members()) {
      req_struct->append_field(std::unique_ptr<t_field>(member));
    }
    req_resp_structs.push_back(req_struct);

    auto resp_struct_name =
        go::munge_ident("resp" + svcGoName + funcGoName, false);
    auto resp_struct = new t_struct(service->program(), resp_struct_name);
    if (!func->return_type()->is_void()) {
      auto resp_field = std::make_unique<t_field>(
          func->return_type(), DEFAULT_RETVAL_FIELD_NAME, 0);
      resp_field->set_qualifier(t_field_qualifier::optional);
      resp_struct->append_field(std::move(resp_field));
    }
    if (func->exceptions() != nullptr) {
      for (const auto& xs : func->exceptions()->get_members()) {
        auto xc_ptr = std::unique_ptr<t_field>(xs);
        xc_ptr->set_qualifier(t_field_qualifier::optional);
        resp_struct->append_field(std::move(xc_ptr));
      }
    }
    req_resp_structs.push_back(resp_struct);
  }
  return req_resp_structs;
}

const std::string* get_go_name_annotation(const t_named* node) {
  auto name_annotation = node->find_structured_annotation_or_null(kGoNameUri);
  if (name_annotation != nullptr) {
    return &(name_annotation->get_value_from_structured_annotation("name")
                 .get_string());
  }
  return nullptr;
}

const std::string* get_go_tag_annotation(const t_named* node) {
  auto tag_annotation = node->find_structured_annotation_or_null(kGoTagUri);
  if (tag_annotation != nullptr) {
    return &(tag_annotation->get_value_from_structured_annotation("tag")
                 .get_string());
  }
  return nullptr;
}

} // namespace go
} // namespace compiler
} // namespace thrift
} // namespace apache
