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
#include <string_view>
#include <utility>
#include <vector>

#include <fmt/core.h>

#include <thrift/compiler/ast/node_list.h>
#include <thrift/compiler/ast/program_scope.h>
#include <thrift/compiler/ast/t_const.h>
#include <thrift/compiler/ast/t_enum.h>
#include <thrift/compiler/ast/t_exception.h>
#include <thrift/compiler/ast/t_global_scope.h>
#include <thrift/compiler/ast/t_include.h>
#include <thrift/compiler/ast/t_interaction.h>
#include <thrift/compiler/ast/t_list.h>
#include <thrift/compiler/ast/t_map.h>
#include <thrift/compiler/ast/t_named.h>
#include <thrift/compiler/ast/t_package.h>
#include <thrift/compiler/ast/t_primitive_type.h>
#include <thrift/compiler/ast/t_service.h>
#include <thrift/compiler/ast/t_set.h>
#include <thrift/compiler/ast/t_sink.h>
#include <thrift/compiler/ast/t_stream.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/ast/t_typedef.h>
#include <thrift/compiler/ast/t_union.h>

namespace apache::thrift::compiler {

/**
 * Top level class representing an entire thrift program.
 */
class t_program : public t_named {
 private:
  // A combination of a program scope & the order in which it was added
  // to the global scope.
  struct scope_by_priority {
    const scope::program_scope* scope;
    // The priority is the order in which the program was added to the global
    // scope. An alias include has the highest priority, followed by the root
    // program, followed by regular includes.
    scope::program_scope::ScopePriority priority;

    bool is_alias() const {
      return priority == scope::program_scope::ALIAS_PRIORITY;
    }

    bool operator==(const scope_by_priority& other) const {
      return priority == other.priority;
    }

    bool operator<(const scope_by_priority& other) const {
      // Reverse order, so later definitions have higher priority.
      return priority > other.priority;
    }
  };

  using scopes_by_priority = std::vector<scope_by_priority>;

  // The value used when an offset is not specified/unknown.
  static constexpr auto noffset = static_cast<size_t>(-1);

 public:
  /**
   * Constructor for t_program
   *
   * @param path - A *.thrift file path.
   */
  explicit t_program(
      std::string path,
      std::string full_path,
      const t_program* parent = nullptr)
      : t_program(
            std::move(path),
            std::move(full_path),
            parent ? parent->global_scope_
                   : std::make_shared<t_global_scope>()) {}

  void set_typedef_uri_requires_annotation(bool val) {
    typedef_uri_requires_annotation_ = val;
  }
  void set_use_global_resolution(bool val) { use_global_resolution_ = val; }
  void set_package(t_package package) { package_ = std::move(package); }
  const t_package& package() const { return package_; }

  // Defintions, in the order they were added.
  node_list_view<t_named> definitions() { return definitions_; }
  node_list_view<const t_named> definitions() const { return definitions_; }
  void add_definition(std::unique_ptr<t_named> definition);
  void add_enum_definition(scope::enum_id id, const t_const& constant);

  // A convience function that:
  //  - optionally sets the uri (overriding any set value or
  // inheritted default),
  //  - adds the definition to the program, and
  //  - returns a mutable reference to the stored value, so it can be
  //  additionally configured.
  template <typename T>
  T& add_def(std::unique_ptr<T> definition, std::string_view uri = {}) {
    auto* ptr = definition.get();
    if (uri.data() != nullptr) {
      definition->set_uri(std::string(uri.data(), uri.size()));
    }
    add_definition(std::move(definition));
    return *ptr;
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

  const std::vector<t_structured*>& structs_and_unions() const {
    return structs_and_unions_;
  }

  const std::vector<t_exception*>& exceptions() const { return exceptions_; }

  const std::vector<t_structured*>& structured_definitions() const {
    return structured_definitions_;
  }
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
  void set_namespace(
      const std::string& language, const std::string& name_space) {
    namespaces_.emplace(language, name_space);
  }

  /**
   * t_program getters
   */
  const std::string& path() const { return path_; }

  const std::string& full_path() const { return full_path_; }

  const std::string& include_prefix() const { return include_prefix_; }

  /**
   * Returns a list of includes that the program contains. Each include is of
   * type t_include*, and contains information about the program included, as
   * well as the location of the include statement.
   */
  const std::vector<t_include*>& includes() const { return includes_; }
  std::vector<t_include*>& includes() { return includes_; }

  /**
   * Returns a list of programs that are included by this program.
   */
  std::vector<t_program*> get_included_programs() const {
    std::vector<t_program*> included_programs;
    included_programs.reserve(includes_.size());
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
      static const std::string_view prefix = "thrift/annotation/";
      auto path = include->raw_path();
      if (std::string_view(path.data(), std::min(path.size(), prefix.size())) ==
          prefix) {
        continue;
      }
      included_programs.push_back(include->get_program());
    }
    return included_programs;
  }

  t_global_scope* global_scope() const { return global_scope_.get(); }

  // Only used in py_frontend.tcc
  const std::map<std::string, std::string>& namespaces() const {
    return namespaces_;
  }

  const std::unordered_map<std::string, std::vector<std::string>>&
  language_includes() const {
    return language_includes_;
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
    std::string_view scope_name =
        include->alias().value_or(include->get_program()->name());

    const auto global_priority = include->alias().has_value()
        ? scope::program_scope::ALIAS_PRIORITY
        : global_scope_->global_priority(*include->get_program());
    auto& defs = available_scopes_[scope_name];
    // TODO @sadroeck - Sort on insert for performance
    defs.push_back(scope_by_priority{
        &include->get_program()->program_scope(), global_priority});
    std::sort(defs.begin(), defs.end());
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
  std::string scoped_name(const t_named& node) const {
    return fmt::format("{}.{}", name(), node.name());
  }

  const scope::program_scope& program_scope() const { return program_scope_; }

  // Returns the definition of the identifier or nullptr if there is no such
  // definition.
  template <typename Node = t_named>
  const Node* find(scope::identifier id) const {
    const auto [local_node, resolved_via_alias] = find_by_id(id);
    if (!use_global_resolution_ || resolved_via_alias) {
      return dynamic_cast<const Node*>(local_node);
    }

    const auto* global_node = find_global_by_id(id);

    if (local_node != global_node) {
      // If the local and global nodes are different, then there is a
      // resolution mismatch.
      if (local_node || global_node) {
        global_scope_->add_resolution_mismatch(
            id, *this, local_node, global_node);
      }

      // [TEMPORARY] For the time being
      // we'll return the "old" global resolution.
      return dynamic_cast<const Node*>(global_node);
    }

    // Local and global resolution are the same.
    return dynamic_cast<const Node*>(local_node);
  }

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

  // This includes both structs and unions (but no other derived type of
  // t_struct, i.e. no exceptions, t_paramlist, etc.).
  std::vector<t_structured*> structs_and_unions_;

  std::vector<t_exception*> exceptions_;

  // t_structs + t_unions + t_exceptions
  std::vector<t_structured*> structured_definitions_;

  std::vector<t_service*> services_;
  std::vector<t_include*> includes_;
  std::vector<t_interaction*> interactions_;

  std::string path_; // initialized in ctor init-list
  std::string full_path_;
  std::string include_prefix_;
  std::map<std::string, std::string> namespaces_;
  std::unordered_map<std::string, std::vector<std::string>> language_includes_;
  std::shared_ptr<t_global_scope> global_scope_;
  scope::program_scope program_scope_;

  // A map from scope name to the scope object. This is used to resolve
  // references to definitions in other scopes.
  //
  // TEMPORARY: A scope name can refer to multiple scopes, ordered by the order
  // in which they were added to the global scope.
  std::unordered_map<std::string_view, scopes_by_priority> available_scopes_;
  bool use_global_resolution_;
  // DO_BEFORE(hchok,20250901): Clean up typedef URI flag once rolled out
  bool typedef_uri_requires_annotation_ = true;

  t_program(
      std::string path,
      std::string full_path,
      std::shared_ptr<t_global_scope> global_scope)
      : path_(std::move(path)),
        full_path_(std::move(full_path)),
        global_scope_(std::move(global_scope)),
        program_scope_{},
        available_scopes_{},
        use_global_resolution_{true} {
    set_name(compute_name_from_file_path(path_));
  }

  // [TEMPORARY] This is an annotation to identify when a node was resolved via
  // an include alias. Include alias resolution has absolute priority over
  // global resolution. In all other scenarios, (currently) the global
  // resolution is preferred.
  struct resolved_node {
    const t_named* node;
    bool via_include_alias;
  };

  resolved_node find_by_id(scope::identifier id) const;
  const t_named* find_global_by_id(scope::identifier id) const;
};

} // namespace apache::thrift::compiler
