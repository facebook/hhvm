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

#include <thrift/compiler/sema/standard_validator.h>

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>

#include <thrift/compiler/ast/t_exception.h>
#include <thrift/compiler/ast/t_service.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/ast/t_typedef.h>
#include <thrift/compiler/ast/t_union.h>
#include <thrift/compiler/ast/uri.h>

namespace apache::thrift::compiler {

namespace {

// The set of annotations that are used to define the scope of other
// annotations.
const std::unordered_set<std::string>& scope_annotation_uris_set() {
  static const std::unordered_set<std::string> kScopeAnnotationUrisSet = {
      kScopeProgramUri,
      kScopeStructUri,
      kScopeUnionUri,
      kScopeExceptionUri,
      kScopeThrownExceptionUri,
      kScopeFieldUri,
      kScopeTypedefUri,
      kScopeServiceUri,
      kScopeInteractionUri,
      kScopeFunctionUri,
      kScopeFunctionParameterUri,
      kScopeEnumUri,
      kScopeEnumValueUri,
      kScopeConstUri,
  };
  return kScopeAnnotationUrisSet;
}

// Mapping of node types that have a one-to-one mapping to a corresponding
// scope URI, to be used by the generic scope validator. The only exception is
// `t_field`, which is overloaded to be also used for function parameters. Scope
// validation for function parameters is handled separately, and does not
// consume this map.
const std::unordered_map<std::type_index, std::string>& type_scope_uri_map() {
  static const std::unordered_map<std::type_index, std::string>
      kTypeScopeUriMap = {
          {typeid(t_program), kScopeProgramUri},
          {typeid(t_struct), kScopeStructUri},
          {typeid(t_union), kScopeUnionUri},
          {typeid(t_exception), kScopeExceptionUri},
          {typeid(t_field), kScopeFieldUri},
          {typeid(t_typedef), kScopeTypedefUri},
          {typeid(t_service), kScopeServiceUri},
          {typeid(t_interaction), kScopeInteractionUri},
          {typeid(t_function), kScopeFunctionUri},
          {typeid(t_enum), kScopeEnumUri},
          {typeid(t_enum_value), kScopeEnumValueUri},
          {typeid(t_const), kScopeConstUri},
      };
  return kTypeScopeUriMap;
}

struct allowed_scopes {
  std::unordered_set<std::string> scope_uris;

  explicit allowed_scopes(node_metadata_cache& cache, const t_const& annot) {
    for (const auto& meta_annot : annot.type()->structured_annotations()) {
      if (is_transitive_annotation(*meta_annot.type())) {
        const auto& transitive_scopes =
            cache.get<allowed_scopes>(meta_annot).scope_uris;
        scope_uris.insert(transitive_scopes.begin(), transitive_scopes.end());
      }

      const auto& scope = meta_annot.type()->uri();
      if (scope_annotation_uris_set().count(scope)) {
        scope_uris.emplace(scope);
      }
    }
  }
};

} // namespace

template <detail::scope_check_type check_type>
void detail::validate_annotation_scopes(
    sema_context& ctx, const t_named& node) {
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
    if (scope_annotation_uris_set().count(annot_type->uri()) ||
        annot_type->uri() == kSchemaAnnotationUri) {
      continue;
    }

    // Get the accepted set of scopes for this annotation.
    const auto& allowed = ctx.cache().get<allowed_scopes>(annot);
    if (allowed.scope_uris.empty()) {
      // Warn that the annotation isn't marked as such.
      ctx.warning_legacy_strict(
          annot.src_range().begin,
          "Using `{}` as an annotation, even though it has not been enabled "
          "for any annotation scope.",
          annot_type->name());
      continue;
    }

    auto is_valid_scope = false;
    if constexpr (check_type == detail::scope_check_type::function_parameter) {
      // Checking function parameters is a special case, because they are
      // modelled in the AST as t_field but do not accept field-scoped
      // annotations.
      is_valid_scope = allowed.scope_uris.count(kScopeFunctionParameterUri) > 0;

      if (!is_valid_scope && allowed.scope_uris.count(kScopeFieldUri)) {
        // Legacy behaviour - field annotation without a function parameter
        // scope applied to a function parameter. Warn until migration complete.
        ctx.warning(
            annot.src_range().begin,
            "Using field-scoped annotation `{}` to annotate parameter `{}` - "
            "add @scope.FunctionParameter for function parameters",
            annot_type->name(),
            node.name());

        // Prevent duplicate error for field annotation on function parameter
        is_valid_scope = true;
      }
    } else {
      // For non-function parameter nodes, we expect a one-to-one mapping of AST
      // node type to accepted annotation scope, so type_scope_uri_map can be
      // used
      const auto& type_scope = type_scope_uri_map().find(typeid(node));
      is_valid_scope = type_scope != type_scope_uri_map().end() &&
          allowed.scope_uris.count(type_scope->second);
    }

    if (!is_valid_scope) {
      ctx.error(
          annot, "`{}` cannot annotate `{}`", annot_type->name(), node.name());
    }
  }
}

template void
detail::validate_annotation_scopes<detail::scope_check_type::default_scopes>(
    sema_context& ctx, const t_named& node);
template void detail::validate_annotation_scopes<
    detail::scope_check_type::function_parameter>(
    sema_context& ctx, const t_named& node);

} // namespace apache::thrift::compiler
