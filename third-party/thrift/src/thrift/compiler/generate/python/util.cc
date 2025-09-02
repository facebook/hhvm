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

#include <thrift/compiler/generate/python/util.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <thrift/compiler/ast/ast_visitor.h>
#include <thrift/compiler/ast/uri.h>
#include <thrift/compiler/generate/cpp/util.h>

namespace apache::thrift::compiler {

bool is_type_iobuf(std::string_view name) {
  return name == "folly::IOBuf" || name == "std::unique_ptr<folly::IOBuf>";
}

bool is_type_iobuf(const t_type* type) {
  return is_type_iobuf(cpp2::get_type(type));
}

bool is_patch_program(const t_program* prog) {
  return prog ? (boost::algorithm::starts_with(prog->name(), "gen_patch_") ||
                 boost::algorithm::starts_with(prog->name(), "gen_safe_patch_"))
              : false;
}

static void get_needed_includes_by_patch_impl(
    const t_program* root,
    const t_type& type,
    std::unordered_set<const t_type*>& seen,
    std::unordered_set<const t_program*>& result) {
  if (!seen.insert(&type).second) {
    return;
  }
  if (const t_typedef* asTypedef = type.try_as<t_typedef>()) {
    // We don't generate patch struct for typedef. We need to skip the chain.
    get_needed_includes_by_patch_impl(
        root, *asTypedef->get_type(), seen, result);
    return;
  }
  if (type.is<t_primitive_type>()) {
    // We don't need to import module to get the primitive types.
    return;
  }
  if (type.program() && type.program() != root) {
    // If type is not in root, root needs type's program in runtime.
    result.insert(type.program());
    return;
  }
  if (const t_list* asList = type.try_as<t_list>()) {
    get_needed_includes_by_patch_impl(
        root, *asList->get_elem_type(), seen, result);
  }
  if (const t_set* asSet = type.try_as<t_set>()) {
    get_needed_includes_by_patch_impl(
        root, *asSet->get_elem_type(), seen, result);
  }
  if (const t_map* asMap = type.try_as<t_map>()) {
    get_needed_includes_by_patch_impl(
        root, asMap->key_type().deref(), seen, result);
    get_needed_includes_by_patch_impl(
        root, asMap->val_type().deref(), seen, result);
  }
  if (const t_structured* asStructured = type.try_as<t_structured>()) {
    if (!should_generate_patch(asStructured)) {
      return;
    }
    for (auto& field : asStructured->fields()) {
      get_needed_includes_by_patch_impl(
          root, *field.type().get_type(), seen, result);
    }
  }
}

std::unordered_set<const t_program*> needed_includes_by_patch(
    const t_program* root) {
  std::unordered_set<const t_program*> programs;
  std::unordered_set<const t_type*> seen;
  const_ast_visitor visitor;
  visitor.add_root_definition_visitor([&](const t_named& def) {
    if (auto type = dynamic_cast<const t_type*>(&def)) {
      get_needed_includes_by_patch_impl(root, *type, seen, programs);
    }
  });
  visitor(*root);
  return programs;
}

bool type_contains_patch(const t_type* type) {
  if (auto typedf = dynamic_cast<t_typedef const*>(type)) {
    return type_contains_patch(typedf->get_type());
  }

  if (auto map = dynamic_cast<t_map const*>(type)) {
    return type_contains_patch(&map->key_type().deref()) ||
        type_contains_patch(&map->val_type().deref());
  }

  if (auto set = dynamic_cast<t_set const*>(type)) {
    return type_contains_patch(set->get_elem_type());
  }

  if (auto list = dynamic_cast<t_list const*>(type)) {
    return type_contains_patch(list->get_elem_type());
  }

  return is_patch_program(type->program());
}

bool has_generate_patch_new_annotation(const t_structured* type) {
  return type->find_structured_annotation_or_null(kGeneratePatchNewUri);
}

bool has_structured_with_generate_patch_new_annotation(const t_program* root) {
  for (const auto* t : root->structured_definitions()) {
    if (has_generate_patch_new_annotation(t)) {
      return true;
    }
  }
  return false;
}

bool should_generate_patch(const t_structured* type) {
  return !has_structured_with_generate_patch_new_annotation(type->program()) ||
      has_generate_patch_new_annotation(type);
}

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

  // strip cpp-style comments - '//' followed by 0+ non-newline characters
  fr = s.find("//");
  while (fr != std::string::npos) {
    auto to = s.find('\n', fr + 2);
    // Erase from '//' through to the following newline
    // If to == npos (last line commented without final line break), we erase to
    // the end of s
    s.erase(fr, to - fr);

    fr = s.find("//", fr);
  }

  // strip newlines
  boost::algorithm::replace_all(s, "\n", " ");
}

namespace python {
cached_properties::cached_properties(
    std::string cpp_template, std::string type, std::string flat_name)
    : cpp_template_(std::move(cpp_template)),
      cpp_type_(std::move(type)),
      flat_name_(std::move(flat_name)) {
  strip_cpp_comments_and_newlines(cpp_type_);
}

std::string cached_properties::to_cython_template() const {
  // handle special built-ins first:
  if (cpp_template_ == "std::vector") {
    return "vector";
  } else if (cpp_template_ == "std::set") {
    return "cset";
  } else if (cpp_template_ == "std::map") {
    return "cmap";
  }
  // then default handling:
  return boost::algorithm::replace_all_copy(cpp_template_, "::", "_");
}

std::string cached_properties::to_cython_type() const {
  if (cpp_type_ == "") {
    return "";
  }
  std::string cython_type = cpp_type_;
  boost::algorithm::replace_all(cython_type, "::", "_");
  boost::algorithm::replace_all(cython_type, "<", "_");
  boost::algorithm::replace_all(cython_type, ">", "");
  boost::algorithm::replace_all(cython_type, " ", "");
  boost::algorithm::replace_all(cython_type, ", ", "_");
  boost::algorithm::replace_all(cython_type, ",", "_");
  return cython_type;
}

bool cached_properties::is_default_template(
    const apache::thrift::compiler::t_type* type) const {
  return (!type->is<t_container>() && cpp_template_ == "") ||
      (type->is<t_list>() && cpp_template_ == "std::vector") ||
      (type->is<t_set>() && cpp_template_ == "std::set") ||
      (type->is<t_map>() && cpp_template_ == "std::map");
}

void cached_properties::set_flat_name(
    const apache::thrift::compiler::t_program* this_prog,
    const apache::thrift::compiler::t_type* type,
    const std::string& extra) {
  std::string custom_prefix;
  if (!is_default_template(type)) {
    custom_prefix = to_cython_template() + "__";
  } else if (cpp_type_ != "") {
    custom_prefix = to_cython_type() + "__";
  }
  const t_program* type_program = type->program();
  if (type_program && type_program != this_prog) {
    custom_prefix += type_program->name() + "_";
  }
  custom_prefix += extra;
  flat_name_ = std::move(custom_prefix);
}

std::unordered_set<std::string_view> extract_modules_and_insert_into(
    std::string_view fully_qualified_name,
    std::unordered_set<std::string_view>& module_paths) {
  size_t start = 0;
  size_t end = 0;
  while (end != std::string_view::npos) {
    end = fully_qualified_name.find('[', start);
    size_t last_dot = fully_qualified_name.rfind('.', end);
    if (last_dot != std::string_view::npos && last_dot > start) {
      module_paths.insert(fully_qualified_name.substr(start, last_dot - start));
    }
    start = end + 1;
  }
  return module_paths;
}

std::string to_python_string_literal(std::string val) {
  std::string quotes = R"(""")";
  boost::algorithm::replace_all(val, "\\", "\\\\");
  boost::algorithm::replace_all(val, "\"", "\\\"");
  return quotes + val + quotes;
}

std::string gen_capi_module_prefix_impl(const t_program* program) {
  std::string prefix =
      get_py3_namespace_with_name_and_prefix(program, "", "__");
  // kebab is not kosher in cpp fn names
  std::replace(prefix.begin(), prefix.end(), '-', '_');
  return prefix;
}
} // namespace python
} // namespace apache::thrift::compiler
