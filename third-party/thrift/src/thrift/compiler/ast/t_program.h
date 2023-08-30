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

#include <cassert>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <fmt/core.h>

#include <thrift/compiler/ast/node_list.h>
#include <thrift/compiler/ast/t_base_type.h>
#include <thrift/compiler/ast/t_const.h>
#include <thrift/compiler/ast/t_enum.h>
#include <thrift/compiler/ast/t_exception.h>
#include <thrift/compiler/ast/t_include.h>
#include <thrift/compiler/ast/t_interaction.h>
#include <thrift/compiler/ast/t_list.h>
#include <thrift/compiler/ast/t_map.h>
#include <thrift/compiler/ast/t_named.h>
#include <thrift/compiler/ast/t_package.h>
#include <thrift/compiler/ast/t_scope.h>
#include <thrift/compiler/ast/t_service.h>
#include <thrift/compiler/ast/t_set.h>
#include <thrift/compiler/ast/t_sink.h>
#include <thrift/compiler/ast/t_stream.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/ast/t_typedef.h>

namespace apache {
namespace thrift {
namespace compiler {

/**
 * Top level class representing an entire thrift program.
 */
class t_program : public t_named {
 public:
  // The value used when an offset is not specified/unknown.
  static constexpr auto noffset = static_cast<size_t>(-1);

  /**
   * Constructor for t_program
   *
   * @param path - A *.thrift file path.
   */
  explicit t_program(std::string path, const t_program* parent = nullptr)
      : t_program(
            std::move(path),
            parent ? parent->scope_ : std::make_shared<t_scope>()) {}

  void set_package(t_package package) { package_ = std::move(package); }
  const t_package& package() const { return package_; }

  // Defintions, in the order they were added.
  node_list_view<t_named> definitions() { return definitions_; }
  node_list_view<const t_named> definitions() const { return definitions_; }
  void add_definition(std::unique_ptr<t_named> definition);

  // A convience function that:
  //  - optionally sets the uri (overriding any set value or
  // inheritted default),
  //  - adds the definition to the program, and
  //  - returns a mutable reference to the stored value, so it can be
  //  additionally configured.
  template <typename T>
  T& add_def(std::unique_ptr<T> definition, fmt::string_view uri = {}) {
    auto* ptr = definition.get();
    if (uri.data() != nullptr) {
      definition->set_uri(std::string(uri.data(), uri.size()));
    }
    add_definition(std::move(definition));
    return *ptr;
  }
  template <typename T, typename... Args>
  T& create_def(Args&&... args) {
    return add_def(std::make_unique<T>(std::forward<Args>(args)...));
  }

  // Concrete instantiation of container types.
  node_list_view<t_container> type_instantiations() { return type_insts_; }
  node_list_view<const t_container> type_instantiations() const {
    return type_insts_;
  }
  t_type_ref add_type_instantiation(std::unique_ptr<t_container> type_inst) {
    assert(type_inst != nullptr);
    return *type_insts_.emplace_back(std::move(type_inst));
  }

  void add_unnamed_typedef(std::unique_ptr<t_typedef> td) {
    assert(td != nullptr);
    nodes_.push_back(std::move(td));
  }

  void add_unnamed_type(std::unique_ptr<t_type> ut) {
    assert(ut != nullptr);
    // Should use add_type_instantiation.
    assert(dynamic_cast<t_container*>(ut.get()) == nullptr);
    // Should use add_placeholder_typedef.
    assert(dynamic_cast<t_placeholder_typedef*>(ut.get()) == nullptr);
    // Should use add_unnamed_typedef.
    assert(dynamic_cast<t_typedef*>(ut.get()) == nullptr);
    nodes_.push_back(std::move(ut));
  }

  /**
   * Get program elements by kind.
   */
  const std::vector<t_typedef*>& typedefs() const { return typedefs_; }
  const std::vector<t_enum*>& enums() const { return enums_; }
  const std::vector<t_const*>& consts() const { return consts_; }
  const std::vector<t_struct*>& structs() const { return structs_; }
  const std::vector<t_exception*>& exceptions() const { return exceptions_; }
  const std::vector<t_struct*>& objects() const { return objects_; }
  const std::vector<t_service*>& services() const { return services_; }
  const std::vector<t_interaction*>& interactions() const {
    return interactions_;
  }

  void add_language_include(std::string language, std::string path) {
    language_includes_[std::move(language)].push_back(std::move(path));
  }

  /**
   * Language neutral namespace/packaging
   *
   * @param language - The target language (i.e. py, cpp) to generate code
   * @param name_space - //TODO add definition of name_space
   */
  void set_namespace(std::string language, std::string name_space) {
    namespaces_.emplace(language, name_space);
  }

  /**
   * t_program getters
   */
  const std::string& path() const { return path_; }

  const std::string& include_prefix() const { return include_prefix_; }

  /**
   * Returns a list of includes that the program contains. Each include is of
   * type t_include*, and contains information about the program included, as
   * well as the location of the include statement.
   */
  const std::vector<t_include*>& includes() const { return includes_; }

  /**
   * Returns a list of programs that are included by this program.
   */
  std::vector<t_program*> get_included_programs() const {
    std::vector<t_program*> included_programs;
    for (const auto& include : includes_) {
      included_programs.push_back(include->get_program());
    }
    return included_programs;
  }
  /**
   * As above, but excludes annotation files which shouldn't normally be
   * included.
   */
  std::vector<t_program*> get_includes_for_codegen() const {
    std::vector<t_program*> included_programs;
    for (const auto& include : includes_) {
      static const fmt::string_view prefix = "thrift/annotation/";
      auto path = include->raw_path();
      if (fmt::string_view(path.data(), std::min(path.size(), prefix.size())) ==
          prefix) {
        continue;
      }
      included_programs.push_back(include->get_program());
    }
    return included_programs;
  }

  t_scope* scope() const { return scope_.get(); }

  // Only used in py_frontend.tcc
  const std::map<std::string, std::string>& namespaces() const {
    return namespaces_;
  }

  const std::unordered_map<std::string, std::vector<std::string>>&
  language_includes() const {
    return language_includes_;
  }

  // TODO: replace all callsites and remove
  const std::vector<std::string>& cpp_includes() const {
    if (language_includes_.count("cpp")) {
      return language_includes_.at("cpp");
    }
    static const std::vector<std::string>& kEmpty =
        *new std::vector<std::string>;
    return kEmpty;
  }

  /**
   * Outputs a reference to the namespace corresponding to the
   * key(language) in the namespaces_ map.
   *
   * @param language - The target language (i.e. py, cpp) to generate code
   */
  const std::string& get_namespace(const std::string& language) const;

  struct namespace_config {
    bool no_top_level_domain = false;
    bool no_domain = false;
    bool no_filename = false;
  };

  std::vector<std::string> gen_namespace_or_default(
      const std::string& language, namespace_config config) const;

  void add_include(std::unique_ptr<t_include> include) {
    includes_.push_back(include.get());
    nodes_.push_back(std::move(include));
  }

  /**
   * This sets the directory path of the current thrift program,
   * adding checks to format it into a correct directory path
   *
   * @param include_prefix - The directory path of a thrift include statement
   */
  void set_include_prefix(std::string include_prefix);

  /**
   *  Obtains the name of a thrift file from the full file path
   *
   * @param path - A *.thrift file path
   */
  std::string compute_name_from_file_path(std::string path);

  // Helpers for constrcuting program scoped names.
  std::string scope_name(const std::string& defname) const {
    return name() + "." + defname;
  }
  std::string scope_name(const t_named& owner, const t_named& node) const {
    return name() + "." + owner.name() + "." + node.name();
  }
  std::string scope_name(const t_named& node) const {
    return scope_name(node.name());
  }

  enum class value_id : int64_t {};

  // Adds value to intern list and returns ID
  value_id intern_value(std::unique_ptr<t_const_value> val) {
    auto type = val->ttype();
    intern_list_.push_back(
        std::make_unique<t_const>(this, type, "", std::move(val)));
    return static_cast<value_id>(intern_list_.size());
  }
  const node_list<t_const>& intern_list() const { return intern_list_; }

 private:
  t_package package_;

  // All the elements owned by this program.
  node_list<t_node> nodes_;
  node_list<t_named> definitions_;
  node_list<t_container> type_insts_;

  /**
   * Components to generate code for
   */
  std::vector<t_typedef*> typedefs_;
  std::vector<t_enum*> enums_;
  std::vector<t_const*> consts_;
  std::vector<t_struct*> structs_;
  std::vector<t_exception*> exceptions_;
  std::vector<t_service*> services_;
  std::vector<t_include*> includes_;
  std::vector<t_interaction*> interactions_;
  std::vector<t_struct*> objects_; // structs_ + exceptions_
  node_list<t_const> intern_list_;

  std::string path_; // initialized in ctor init-list
  std::string include_prefix_;
  std::map<std::string, std::string> namespaces_;
  std::unordered_map<std::string, std::vector<std::string>> language_includes_;
  std::shared_ptr<t_scope> scope_;

  t_program(std::string path, std::shared_ptr<t_scope> scope)
      : path_(std::move(path)), scope_(std::move(scope)) {
    set_name(compute_name_from_file_path(path_));
  }

  // TODO(afuller): Remove everything below this comment. It is only provided
  // for backwards compatibility.
 public:
  void add_typedef(std::unique_ptr<t_typedef> node) {
    add_definition(std::move(node));
  }
  void add_enum(std::unique_ptr<t_enum> node) {
    add_definition(std::move(node));
  }
  void add_const(std::unique_ptr<t_const> node) {
    add_definition(std::move(node));
  }
  void add_struct(std::unique_ptr<t_struct> node) {
    add_definition(std::move(node));
  }
  void add_exception(std::unique_ptr<t_exception> node) {
    add_definition(std::move(node));
  }
  void add_service(std::unique_ptr<t_service> node) {
    add_definition(std::move(node));
  }
  void add_interaction(std::unique_ptr<t_interaction> node) {
    add_definition(std::move(node));
  }
  void add_xception(std::unique_ptr<t_exception> tx) {
    add_exception(std::move(tx));
  }
  const std::vector<t_exception*>& xceptions() const { return exceptions(); }

  // Looks for an annotation on the given node, then if not found, and the node
  // is not generated, looks for the same annotation on the program.
  const t_const* inherit_annotation_or_null(
      const t_named& node, const char* uri) const {
    if (const t_const* annot = node.find_structured_annotation_or_null(uri)) {
      return annot;
    } else if (node.generated()) { // Generated nodes do not inherit.
      return nullptr;
    }
    return find_structured_annotation_or_null(uri);
  }
};

} // namespace compiler
} // namespace thrift
} // namespace apache
