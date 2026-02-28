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

#include <boost/algorithm/string.hpp>
#include <thrift/compiler/ast/t_program.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace apache::thrift::compiler {

namespace {

/**
 * Determine if a node is eligible to have a URI.
 */
bool is_uri_eligible(const t_named& node) {
  if (dynamic_cast<const t_typedef*>(&node) == nullptr) {
    // Node is not a typedef => set uri.
    return true;
  }

  // Node is a typedef: only set URI if the @thrift.AllowLegacyTypedefUri
  // annotation is present.
  return node.find_structured_annotation_or_null(kAllowLegacyTypedefUriUri) !=
      nullptr;
}

} // namespace

void t_program::add_definition(std::unique_ptr<t_named> definition) {
  assert(definition != nullptr);

  program_scope_.add_definition(
      scope::scoped_id{definition->src_range(), name(), definition->name()},
      definition.get());

  // [TEMPORARY] Add global definition for <scope>.<name>
  global_scope_->add_definition(*definition, definition->name());

  if (!is_uri_eligible(*definition)) {
    // Not eligible for URIs - ensure that the URI value is empty
    definition->set_uri("", /*is_explicit=*/true);
  } else if (!definition->explicit_uri()) {
    // Resolve Thrift URI.
    if (auto* cnst = definition->find_structured_annotation_or_null(kUriUri)) {
      auto* val = cnst->get_value_from_structured_annotation_or_null("value");
      definition->set_uri(val ? val->get_string() : "", /*is_explicit=*/true);
    } else if (
        auto* uri =
            definition->find_unstructured_annotation_or_null("thrift.uri")) {
      definition->set_uri(*uri, /*is_explicit=*/true);
    } else { // Inherit from package.
      definition->set_uri(
          package_.get_uri(definition->name()), /*is_explicit=*/false);
    }
  }

  global_scope_->add_def_by_uri(*definition);

  // Index the node.
  auto* ptr = definition.get();
  if (auto* exception_type = dynamic_cast<t_exception*>(ptr)) {
    structured_definitions_.push_back(exception_type);
    exceptions_.push_back(exception_type);
  } else if (auto* union_type = dynamic_cast<t_union*>(ptr)) {
    structured_definitions_.push_back(union_type);
    structs_and_unions_.push_back(union_type);
  } else if (auto* struct_type = dynamic_cast<t_struct*>(ptr)) {
    structured_definitions_.push_back(struct_type);
    structs_and_unions_.push_back(struct_type);
  } else if (auto* interaction_type = dynamic_cast<t_interaction*>(ptr)) {
    interactions_.push_back(interaction_type);
  } else if (auto* service_type = dynamic_cast<t_service*>(ptr)) {
    services_.push_back(service_type);
  } else if (auto* enum_type = dynamic_cast<t_enum*>(ptr)) {
    enums_.push_back(enum_type);
  } else if (auto* typedef_type = dynamic_cast<t_typedef*>(ptr)) {
    typedefs_.push_back(typedef_type);
  } else if (auto* const_type = dynamic_cast<t_const*>(ptr)) {
    consts_.push_back(const_type);
  }

  // Transfer ownership of the definition.
  definitions_.push_back(std::move(definition));
}

void t_program::add_enum_definition(
    scope::enum_id id, const t_const& constant) {
  program_scope_.add_definition(id, &constant);

  // [TEMPORARY] Add global definition for <scope>.<enum_name>.<value_name>
  global_scope_->add_definition(constant, id.enum_name, id.value_name);
}

const std::string& t_program::get_namespace(const std::string& language) const {
  auto pos = namespaces_.find(language);
  static const auto& kEmpty = *new std::string();
  return (pos != namespaces_.end() ? pos->second->ns() : kEmpty);
}

std::vector<std::string> t_program::gen_namespace_or_default(
    const std::string& language, namespace_config config) const {
  std::vector<std::string> ret;

  auto pos = namespaces_.find(language);
  if (pos != namespaces_.end()) {
    if (!pos->second->ns().empty()) {
      split(ret, pos->second->ns(), boost::algorithm::is_any_of("."));
    }
    return ret;
  }

  // namespace is not defined explicitly. Generating it from package name

  if (package().empty()) {
    return ret;
  }

  if (!config.no_domain) {
    ret = package().domain();

    if (config.no_top_level_domain && !ret.empty()) {
      ret.pop_back();
    }

    // We use reverse domain name notation
    std::reverse(ret.begin(), ret.end());
  }

  const auto& path = package().path();
  ret.insert(ret.end(), path.begin(), path.end());
  if (config.no_filename && !ret.empty() && name() == ret.back()) {
    ret.pop_back();
  }

  return ret;
}

void t_program::set_include_prefix(std::string include_prefix) {
  include_prefix_ = std::move(include_prefix);

  const auto len = include_prefix_.size();
  if (len > 0 && include_prefix_[len - 1] != '/') {
    include_prefix_ += '/';
  }
}

std::string t_program::compute_name_from_file_path(std::string path) {
  std::string::size_type slash = path.find_last_of("/\\");
  if (slash != std::string::npos) {
    path = path.substr(slash + 1);
  }
  std::string::size_type dot = path.rfind('.');
  if (dot != std::string::npos) {
    path = path.substr(0, dot);
  }
  return path;
}

t_program::resolved_node t_program::find_by_id(scope::identifier id) const {
  return id.visit(
      [&](scope::unscoped_id&& id) {
        // Unscoped identifiers, e.g. `Bar` can only be local to the current
        // program.
        return resolved_node{program_scope_.find_by_name(id.name), false};
      },
      [&](scope::scoped_id&& id) {
        // Note: We can't distinguish between a scoped identifier e.g.
        // `foo.Bar` & an unscoped enum identifier `e.g. `Foo.Bar`

        // Check if the full identifier is available in the current program.
        // e.g. for enum values `Foo.Bar`
        if (const auto* local_node =
                program_scope_.find_by_name(id.scope, id.name)) {
          return resolved_node{local_node, false};
        }

        // Otherwise check for an include with the provided scope name, and if
        // so, use that to resolve the identifier.
        if (const auto scope_it = available_scopes_.find(id.scope);
            scope_it != available_scopes_.end()) {
          for (const scope_by_priority& scope_prio : scope_it->second) {
            if (auto* res = scope_prio.scope->find_by_name(id.name)) {
              return resolved_node{res, scope_prio.is_alias()};
            }

            // If this scope was an alias, don't fall back to checking other
            // scopes
            if (scope_prio.is_alias()) {
              break;
            }
          }
        }

        return resolved_node{nullptr, false};
      },
      [&](scope::enum_id&& id) {
        // Enum IDs always have a program scope, so if we can't find an
        // available program, fail the resolution.
        if (const auto scope_it = available_scopes_.find(id.scope);
            scope_it != available_scopes_.end()) {
          for (const scope_by_priority& scope_prio : scope_it->second) {
            if (auto* res = scope_prio.scope->find_by_name(
                    id.enum_name, id.value_name)) {
              return resolved_node{res, scope_prio.is_alias()};
            }
          }
        }

        return resolved_node{nullptr, false};
      });
}

const t_named* t_program::find_global_by_id(scope::identifier id) const {
  return id.visit(
      [&](scope::unscoped_id&& id) -> const t_named* {
        // [TEMPORARY] Find any unscoped identifiers as scoped by the local
        // program name. E.g. `common.Foo` can be resolved within
        // `common.thrift` as `Foo`, even if it's not defined in the local
        // program.
        return global_scope_->find(
            scope::identifier{scope::scoped_id{id.loc, name(), id.name}});
      },
      [&](scope::scoped_id&& id) -> const t_named* {
        // This could be a scoped external definition
        if (const auto* node = global_scope_->find(id)) {
          return node;
        }

        // Or an unscoped definition with the same scope name as the local
        // program.
        return global_scope_->find(
            scope::identifier{
                scope::enum_id{id.loc, name(), id.scope, id.name}});
      },
      [&](scope::enum_id&& id) -> const t_named* {
        return global_scope_->find(id);
      });
}

std::vector<t_program*> t_program::get_included_programs() const {
  std::vector<t_program*> included_programs;
  included_programs.reserve(includes_.size());
  for (const auto& include : includes_) {
    included_programs.push_back(include->get_program());
  }
  return included_programs;
}

std::vector<t_program*> t_program::get_includes_for_codegen() const {
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

void t_program::add_include(std::unique_ptr<t_include> include) {
  std::string_view scope_name =
      include->alias().value_or(include->get_program()->name());

  const auto global_priority = include->alias().has_value()
      ? scope::program_scope::ALIAS_PRIORITY
      : global_scope_->global_priority(*include->get_program());
  auto& defs = available_scopes_[scope_name];
  // TODO @sadroeck - Sort on insert for performance
  defs.push_back(
      scope_by_priority{
          &include->get_program()->program_scope(), global_priority});
  std::sort(defs.begin(), defs.end());
  includes_.push_back(include.get());
  nodes_.push_back(std::move(include));
}

const t_const* t_program::inherit_annotation_or_null(
    const t_named& node, const char* uri) const {
  if (const t_const* annot = node.find_structured_annotation_or_null(uri)) {
    return annot;
  } else if (node.generated()) { // Generated nodes do not inherit.
    return nullptr;
  }
  return find_structured_annotation_or_null(uri);
}

} // namespace apache::thrift::compiler
