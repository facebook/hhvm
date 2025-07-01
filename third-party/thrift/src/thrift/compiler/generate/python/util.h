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

#include <map>
#include <string>
#include <string_view>
#include <unordered_set>

#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_type.h>
#include <thrift/compiler/ast/uri.h>

namespace apache::thrift::compiler {

/**
 * Currently just a string match for cpp.Type override matches folly::IOBuf or
 * unique_ptr<folly::IOBuf>. In future, this should be based on @python.IOBuf{}
 * annotation:
 *  1. avoids adding @cpp.Type annotation to thrift IDL that
 *     may not have any cpp2 usage.
 *  2. avoid any inconsistency due to top-level "::" usage.
 */
bool is_type_iobuf(std::string_view name);

bool is_type_iobuf(const t_type* type);

bool is_patch_program(const t_program* prog);

bool type_contains_patch(const t_type* type);

std::vector<std::string> get_py3_namespace(const t_program* prog);

std::string get_py3_namespace_with_name_and_prefix(
    const t_program* prog,
    const std::string& prefix,
    const std::string& sep = ".");

// List of programs that `root` needed by patch
std::unordered_set<const t_program*> needed_includes_by_patch(
    const t_program* root);

inline const std::unordered_set<std::string>& get_python_reserved_names() {
  static const std::unordered_set<std::string> keywords = {
      "False",  "None",    "True",    "and",      "as",       "assert", "async",
      "await",  "break",   "class",   "continue", "def",      "del",    "elif",
      "else",   "except",  "finally", "for",      "from",     "global", "if",
      "import", "in",      "is",      "lambda",   "nonlocal", "not",    "or",
      "pass",   "raise",   "return",  "try",      "while",    "with",   "yield",
      "cdef",   "cimport", "cpdef",   "cppclass", "ctypedef",
  };
  return keywords;
}

/**
 * strip comments and newlines off cpp annotation text
 */
void strip_cpp_comments_and_newlines(std::string& s);

namespace python {

class cached_properties {
 public:
  cached_properties(
      std::string cpp_template, std::string type, std::string flat_name);

  const std::string& cpp_template() const { return cpp_template_; }
  const std::string& cpp_type() const { return cpp_type_; }
  const std::string& flat_name() const { return flat_name_; }

  std::string to_cython_template() const;

  std::string to_cython_type() const;

  bool is_default_template(const t_type* type) const;

  void set_flat_name(
      const t_program* this_prog, const t_type* type, const std::string& extra);

 private:
  const std::string cpp_template_;
  std::string cpp_type_;
  std::string flat_name_;
};

template <class T>
std::string_view get_python_name_override(const T& node) {
  if (const t_const* annot =
          node.find_structured_annotation_or_null(kPythonNameUri)) {
    if (auto name =
            annot->get_value_from_structured_annotation_or_null("name")) {
      return name->get_string();
    }
  }
  if (const auto* name =
          node.find_unstructured_annotation_or_null("py3.name")) {
    return *name;
  }
  return {};
}

template <class T>
std::string get_py3_name(const T& node) {
  // Reserved Cython / Python keywords that are not blocked by thrift grammer
  // TODO: get rid of this list and force users to rename explicitly
  static const std::unordered_set<std::string> cython_keywords = {
      "DEF",
      "ELIF",
      "ELSE",
      "IF",
      "cdef",
      "cimport",
      "cpdef",
      "cppclass",
      "ctypedef",
  };

  auto name_override = get_python_name_override(node);
  if (!name_override.empty()) {
    return std::string(name_override);
  }

  const auto& name = node.get_name();
  const auto& python_keywords = get_python_reserved_names();
  if (cython_keywords.find(name) != cython_keywords.end() ||
      python_keywords.find(name) != python_keywords.end()) {
    return name + "_";
  }
  return name;
}

std::unordered_set<std::string_view> extract_modules_and_insert_into(
    std::string_view fully_qualified_name,
    std::unordered_set<std::string_view>& module_paths);

template <class T>
std::string get_py3_name_class_scope(
    const T& node, const std::string& parent_name) {
  auto name = get_py3_name(node);
  if (name.size() >= 2 && name[0] == '_' && name[1] == '_') {
    return fmt::format("_{}{}", parent_name, name);
  }
  return name;
}

// can put this back into python_capi when python_capi_converter removed from
// thrift-py3
std::string gen_capi_module_prefix_impl(const t_program* program);

} // namespace python
} // namespace apache::thrift::compiler
