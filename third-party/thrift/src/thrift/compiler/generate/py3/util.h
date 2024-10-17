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

#include <map>
#include <string>
#include <unordered_set>

#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/lib/uri.h>

namespace apache::thrift::compiler {

std::vector<std::string> get_py3_namespace(const t_program* prog);

std::string get_py3_namespace_with_name_and_prefix(
    const t_program* prog,
    const std::string& prefix,
    const std::string& sep = ".");

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

namespace py3 {

class CachedProperties {
 public:
  CachedProperties(
      std::string _template, std::string type, std::string flatName);

  const std::string& cppTemplate() const { return cppTemplate_; }
  const std::string& cppType() const { return cppType_; }
  const std::string& flatName() const { return flatName_; }

  std::string to_cython_template() const;

  std::string to_cython_type() const;

  bool is_default_template(const apache::thrift::compiler::t_type* type) const;

  void set_flat_name(
      const apache::thrift::compiler::t_program* thisProg,
      const apache::thrift::compiler::t_type* type,
      const std::string& extra);

 private:
  const std::string cppTemplate_;
  std::string cppType_;
  std::string flatName_;
};

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

  if (const t_const* annot =
          node.find_structured_annotation_or_null(kPythonNameUri)) {
    if (auto name =
            annot->get_value_from_structured_annotation_or_null("name")) {
      return name->get_string();
    }
  }
  if (const auto* name = node.find_annotation_or_null("py3.name")) {
    return *name;
  }
  const auto& name = node.get_name();
  const auto& python_keywords = get_python_reserved_names();
  if (cython_keywords.find(name) != cython_keywords.end() ||
      python_keywords.find(name) != python_keywords.end()) {
    return name + "_";
  }
  return name;
}

} // namespace py3
} // namespace apache::thrift::compiler
