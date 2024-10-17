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

#include <regex>

#include <boost/algorithm/string/replace.hpp>
#include <thrift/compiler/generate/py3/util.h>

namespace apache::thrift::compiler {

std::vector<std::string> get_py3_namespace(const t_program* prog) {
  t_program::namespace_config conf;
  conf.no_top_level_domain = true;
  conf.no_filename = true;
  return prog->gen_namespace_or_default("py3", conf);
}

std::string get_py3_namespace_with_name_and_prefix(
    const t_program* prog, const std::string& prefix, const std::string& sep) {
  std::ostringstream ss;
  if (!prefix.empty()) {
    ss << prefix << sep;
  }
  for (const auto& name : get_py3_namespace(prog)) {
    ss << name << sep;
  }
  ss << prog->name();
  return ss.str();
}

void strip_cpp_comments_and_newlines(std::string& s) {
  // strip c-style comments
  auto fr = s.find("/*");
  while (fr != std::string::npos) {
    auto to = s.find("*/", fr + 2);
    if (to == std::string::npos) {
      throw std::runtime_error{"no matching */ for annotation comments"};
    }
    s.erase(fr, to - fr + 2);
    fr = s.find("/*", fr);
  }
  // strip cpp-style comments
  s.replace(
      s.begin(),
      s.end(),
      std::regex_replace(
          s,
          std::regex("//.*(?=$|\\n)"), /* simulate multiline regex */
          ""));

  // strip newlines
  boost::algorithm::replace_all(s, "\n", " ");
}

namespace py3 {
CachedProperties::CachedProperties(
    std::string _template, std::string type, std::string flatName)
    : cppTemplate_(std::move(_template)),
      cppType_(std::move(type)),
      flatName_(std::move(flatName)) {
  strip_cpp_comments_and_newlines(cppType_);
}

std::string CachedProperties::to_cython_template() const {
  // handle special built-ins first:
  if (cppTemplate_ == "std::vector") {
    return "vector";
  } else if (cppTemplate_ == "std::set") {
    return "cset";
  } else if (cppTemplate_ == "std::map") {
    return "cmap";
  }
  // then default handling:
  return boost::algorithm::replace_all_copy(cppTemplate_, "::", "_");
}

std::string CachedProperties::to_cython_type() const {
  if (cppType_ == "") {
    return "";
  }
  std::string cython_type = cppType_;
  boost::algorithm::replace_all(cython_type, "::", "_");
  boost::algorithm::replace_all(cython_type, "<", "_");
  boost::algorithm::replace_all(cython_type, ">", "");
  boost::algorithm::replace_all(cython_type, " ", "");
  boost::algorithm::replace_all(cython_type, ", ", "_");
  boost::algorithm::replace_all(cython_type, ",", "_");
  return cython_type;
}

bool CachedProperties::is_default_template(
    const apache::thrift::compiler::t_type* type) const {
  return (!type->is_container() && cppTemplate_ == "") ||
      (type->is_list() && cppTemplate_ == "std::vector") ||
      (type->is_set() && cppTemplate_ == "std::set") ||
      (type->is_map() && cppTemplate_ == "std::map");
}

void CachedProperties::set_flat_name(
    const apache::thrift::compiler::t_program* thisProg,
    const apache::thrift::compiler::t_type* type,
    const std::string& extra) {
  std::string custom_prefix;
  if (!is_default_template(type)) {
    custom_prefix = to_cython_template() + "__";
  } else if (cppType_ != "") {
    custom_prefix = to_cython_type() + "__";
  }
  const t_program* typeProgram = type->program();
  if (typeProgram && typeProgram != thisProg) {
    custom_prefix += typeProgram->name() + "_";
  }
  custom_prefix += extra;
  flatName_ = std::move(custom_prefix);
}

} // namespace py3
} // namespace apache::thrift::compiler
