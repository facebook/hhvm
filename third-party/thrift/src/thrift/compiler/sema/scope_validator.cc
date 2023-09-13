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

#include <thrift/compiler/sema/scope_validator.h>

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>

#include <thrift/compiler/ast/t_exception.h>
#include <thrift/compiler/ast/t_service.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/ast/t_typedef.h>
#include <thrift/compiler/ast/t_union.h>
#include <thrift/compiler/lib/uri.h>

namespace apache {
namespace thrift {
namespace compiler {

namespace {

const std::unordered_map<std::string, std::type_index>& uri_map() {
  static const std::unordered_map<std::string, std::type_index> kUriMap = {
      {kScopeProgramUri, typeid(t_program)},
      {kScopeStructUri, typeid(t_struct)},
      {kScopeUnionUri, typeid(t_union)},
      {kScopeExceptionUri, typeid(t_exception)},
      {kScopeFieldUri, typeid(t_field)},
      {kScopeTypedefUri, typeid(t_typedef)},
      {kScopeServiceUri, typeid(t_service)},
      {kScopeInteractionUri, typeid(t_interaction)},
      {kScopeFunctionUri, typeid(t_function)},
      {kScopeEnumUri, typeid(t_enum)},
      {kScopeEnumValueUri, typeid(t_enum_value)},
      {kScopeConstUri, typeid(t_const)},
  };
  return kUriMap;
}

struct allowed_scopes {
  std::unordered_set<std::type_index> types;

  explicit allowed_scopes(node_metadata_cache& cache, const t_const& annot) {
    for (const auto& meta_annot : annot.type()->structured_annotations()) {
      if (is_transitive_annotation(*meta_annot.type())) {
        const auto& transitive_scopes =
            cache.get<allowed_scopes>(meta_annot).types;
        types.insert(transitive_scopes.begin(), transitive_scopes.end());
      }
      auto itr = uri_map().find(meta_annot.type()->uri());
      if (itr != uri_map().end()) {
        types.emplace(itr->second);
      }
    }
  }
};

} // namespace

void validate_annotation_scopes(diagnostic_context& ctx, const t_named& node) {
  // Ignore a transitive annotation definition because it is a collection of
  // annotations that apply at other scopes. For example:
  //
  //   @cpp.Ref{type = cpp.RefType.Unique}
  //   @scope.Transitive
  //   struct MyAnnotation {}
  //
  // Although @cpp.Ref is a field annotation we don't emit a diagnostic here
  // because it applies not to the definition of MyAnnotation but to its uses.
  if (is_transitive_annotation(node)) {
    return;
  }

  for (const t_const& annot : node.structured_annotations()) {
    const t_type* annot_type = &*annot.type();
    // Ignore scoping annotations themselves.
    if (uri_map().find(annot_type->uri()) != uri_map().end() ||
        annot_type->uri() == kSchemaAnnotationUri) {
      continue;
    }

    // Get the allowed set of node types.
    const auto& allowed = ctx.cache().get<allowed_scopes>(annot);
    if (allowed.types.empty()) {
      // Warn that the annotation isn't marked as such.
      ctx.warning_legacy_strict(
          annot.src_range().begin,
          "Using `{}` as an annotation, even though it has not been enabled "
          "for any annotation scope.",
          annot_type->name());
    } else if (allowed.types.find(typeid(node)) == allowed.types.end()) {
      // Type mismatch.
      ctx.error(
          annot, "`{}` cannot annotate `{}`", annot_type->name(), node.name());
    }
  }
}

} // namespace compiler
} // namespace thrift
} // namespace apache
