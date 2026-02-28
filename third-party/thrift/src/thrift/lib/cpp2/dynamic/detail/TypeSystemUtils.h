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

#include <concepts>

#include <thrift/lib/cpp2/dynamic/TypeSystem.h>

namespace apache::thrift::type_system {

template <typename F>
concept DefinitionVisitor = requires(F f, DefinitionRef ref) {
  { f(ref) } -> std::convertible_to<bool>;
};

namespace detail {

template <DefinitionVisitor F>
class DependencyWalker {
 public:
  DependencyWalker(const TypeSystem& source, F& visitor)
      : source_(source), visitor_(visitor) {}

  void visit(DefinitionRef root) { visitDefinition(root); }

 private:
  void visitDefinition(DefinitionRef ref) {
    if (!visitor_(ref)) {
      return;
    }

    ref.visit(
        [&](const StructNode& node) {
          visitAnnotations(node.annotations());
          for (const auto& field : node.fields()) {
            visitType(field.type());
            visitAnnotations(field.annotations());
          }
        },
        [&](const UnionNode& node) {
          visitAnnotations(node.annotations());
          for (const auto& field : node.fields()) {
            visitType(field.type());
            visitAnnotations(field.annotations());
          }
        },
        [&](const EnumNode& node) {
          visitAnnotations(node.annotations());
          for (const auto& v : node.values()) {
            visitAnnotations(v.annotations());
          }
        },
        [&](const OpaqueAliasNode& node) {
          visitAnnotations(node.annotations());
          visitType(node.targetType());
        });
  }

  void visitType(const TypeRef& ref) {
    ref.visit(
        [&](const StructNode& s) { visitDefinition(DefinitionRef(&s)); },
        [&](const UnionNode& u) { visitDefinition(DefinitionRef(&u)); },
        [&](const EnumNode& e) { visitDefinition(DefinitionRef(&e)); },
        [&](const OpaqueAliasNode& o) { visitDefinition(DefinitionRef(&o)); },
        [&](const TypeRef::List& l) { visitType(l.elementType()); },
        [&](const TypeRef::Set& s) { visitType(s.elementType()); },
        [&](const TypeRef::Map& m) {
          visitType(m.keyType());
          visitType(m.valueType());
        },
        [](const auto&) {});
  }

  void visitAnnotations(const AnnotationsMap& annotations) {
    for (const auto& [uri, _] : annotations) {
      if (uri.starts_with("facebook.com/thrift/annotation/")) {
        continue;
      }
      auto def = source_.getUserDefinedType(uri);
      if (!def.has_value()) {
        throw InvalidTypeError(
            fmt::format(
                "Type with URI '{}' is not defined in this TypeSystem.", uri));
      }
      visitDefinition(*def);
    }
  }

  const TypeSystem& source_;
  F& visitor_;
};

} // namespace detail

/**
 * Visits a DefinitionRef and all its transitively reachable dependencies
 * (field types, container elements, opaque alias targets, and non-standard
 * annotation types).
 *
 * The visitor is called with each DefinitionRef encountered. It must return
 * true if the definition is being visited for the first time (recurse into
 * it), or false if already visited (skip). This allows callers to control
 * their deduplication strategy.
 *
 * Standard annotations (facebook.com/thrift/annotation/) are skipped as they
 * are not currently bundled in TypeSystems.
 */
template <DefinitionVisitor F>
void forEachTransitiveDependency(
    const TypeSystem& source, DefinitionRef root, F&& visitor) {
  detail::DependencyWalker<std::remove_reference_t<F>> w(source, visitor);
  w.visit(root);
}

} // namespace apache::thrift::type_system
