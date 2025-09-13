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

#include <thrift/lib/cpp2/schema/SchemaV1.h>

#ifdef THRIFT_SCHEMA_AVAILABLE

#include <unordered_set>

#include <folly/synchronization/DelayedInit.h>
#include <folly/synchronization/RelaxedAtomic.h>

#include <thrift/lib/cpp/util/EnumUtils.h>
#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/cpp2/runtime/BaseSchemaRegistry.h>
#include <thrift/lib/cpp2/schema/SyntaxGraph.h>
#include <thrift/lib/cpp2/schema/detail/SchemaBackedResolver.h>
#include <thrift/lib/thrift/gen-cpp2/schema_types.h>

namespace apache::thrift {
namespace detail {
template <typename T>
struct NodeTag {
  using type = T;
};

template <typename T>
constexpr auto getSyntaxGraphNodeTypeFor() {
  if constexpr (is_thrift_service_tag_v<T>) {
    return NodeTag<syntax_graph::ServiceNode>{};
  } else if constexpr (is_thrift_struct_v<T>) {
    return NodeTag<syntax_graph::StructNode>{};
  } else if constexpr (is_thrift_union_v<T>) {
    return NodeTag<syntax_graph::UnionNode>{};
  } else if constexpr (is_thrift_exception_v<T>) {
    return NodeTag<syntax_graph::ExceptionNode>{};
  } else if constexpr (util::is_thrift_enum_v<T>) {
    return NodeTag<syntax_graph::EnumNode>{};
  } else {
    static_assert(folly::always_false<T>, "Unsupported Thrift type");
  }
  // It's unclear how to include typedefs and constants here.
  // TODO: interactions
}

template <typename T>
constexpr auto getTypeSystemNodeTypeFor() {
  if constexpr (is_thrift_struct_v<T>) {
    return NodeTag<type_system::StructNode>{};
  } else if constexpr (is_thrift_union_v<T>) {
    return NodeTag<type_system::UnionNode>{};
  } else if constexpr (util::is_thrift_enum_v<T>) {
    return NodeTag<type_system::EnumNode>{};
  } else {
    static_assert(folly::always_false<T>, "Unsupported Thrift type");
  }
}

template <typename T>
using SyntaxGraphNodeTypeFor =
    typename decltype(getSyntaxGraphNodeTypeFor<T>())::type;

template <typename T>
using TypeSystemNodeTypeFor =
    typename decltype(getTypeSystemNodeTypeFor<T>())::type;
} // namespace detail
namespace test {
struct SchemaTest;
}

class SchemaRegistry : public type_system::SourceIndexedTypeSystem {
 public:
  // Access the global registry.
  static SchemaRegistry& get();

  /**
   * Gets node for given definition, or throws `std::out_of_range` if not
   * present in schema.
   */
  template <typename T>
  const syntax_graph::DefinitionNode& getDefinitionNode() const {
    return resolver_->getDefinitionNode<T>();
  }

  /**
   * Gets node for given definition, or throws `std::out_of_range` if not
   * present in schema.
   * Returns most-derived type (e.g. StructNode) that matches the template
   * parameter.
   */
  template <typename T>
  const detail::SyntaxGraphNodeTypeFor<T>& getNode() const {
    return resolver_->getDefinitionNode<T>()
        .template as<detail::SyntaxGraphNodeTypeFor<T>>();
  }

  /**
   * Gets TypeSystem node for given URI, or nullopt if not found.
   * Only types defined in a file with the `any` cpp2 compiler option enabled
   * can be found using this method.
   */
  std::optional<type_system::DefinitionRef> getTypeSystemDefinitionRefByUri(
      const std::string_view uri) const;
  /**
   * Gets the exact set of URIs for which getTypeSystemDefinitionRefByUri will
   * succeed.
   *
   * This includes only types defined in a file with the `any` cpp2 compiler
   * option enabled.
   */
  const folly::F14FastSet<type_system::Uri>& getTypeSystemUris() const;

  /**
   * Gets TypeSystem node for given definition, or throws `std::out_of_range` if
   * not present in schema.
   */
  template <typename T>
  type_system::DefinitionRef getTypeSystemDefinitionRef() const {
    const syntax_graph::DefinitionNode& sgNode = getDefinitionNode<T>();
    return syntaxGraph_->asTypeSystemDefinitionRef(sgNode);
  }

  /**
   * Gets TypeSystem node for given definition, or throws `std::out_of_range` if
   * not present in schema. Returns most-derived type (e.g. StructNode) that
   * matches the template parameter.
   */
  template <typename T>
  const detail::TypeSystemNodeTypeFor<T>& getTypeSystemNode() const {
    return getTypeSystemDefinitionRef<T>()
        .template asType<detail::TypeSystemNodeTypeFor<T>>();
  }

  /**
   * Gets SyntaxGraph node for given TypeSystem node, or throws
   * `std::runtime_error` if the TypeSystem node was originated from the same
   * SchemaRegistry.
   */
  const syntax_graph::DefinitionNode& getSyntaxGraphNode(
      const type_system::DefinitionNode& node) const {
    return syntaxGraph_->asSyntaxGraphDefinition(node);
  }

  /**
   * Gets TypeSystem node for given source identifier, or nullopt if not found.
   */
  std::optional<type_system::DefinitionRef>
  getTypeSystemDefinitionRefBySourceIdentifier(
      type_system::SourceIdentifierView sourceIdentifier) const {
    return getUserDefinedTypeBySourceIdentifier(sourceIdentifier);
  }

  explicit SchemaRegistry(BaseSchemaRegistry& base);
  ~SchemaRegistry() override;

 private:
  // TypeSystem API defined as private to encourage use of more specific method
  // names.
  std::optional<type_system::DefinitionRef> getUserDefinedType(
      type_system::UriView uri) const override {
    return getTypeSystemDefinitionRefByUri(uri);
  }
  std::optional<folly::F14FastSet<type_system::Uri>> getKnownUris()
      const override {
    // We pessimitically return an empty optional even though we can enumerate
    // all URIs for files with the `any` cpp2 compiler option enabled.
    return std::nullopt;
  }
  std::optional<type_system::DefinitionRef>
  getUserDefinedTypeBySourceIdentifier(
      type_system::SourceIdentifierView sourceIdentifier) const override {
    if (auto* node =
            resolver_->getDefinitionNodeBySourceIdentifier(sourceIdentifier)) {
      return syntaxGraph_->asTypeSystemDefinitionRef(*node);
    }
    return std::nullopt;
  }
  std::optional<type_system::SourceIdentifierView>
  getSourceIdentiferForUserDefinedType(
      type_system::DefinitionRef) const override {
    throw std::runtime_error("not implemented");
  }
  type_system::SourceIndexedTypeSystem::NameToDefinitionsMap
  getUserDefinedTypesAtLocation(std::string_view) const override {
    throw std::runtime_error("not implemented");
  }

  using Ptr = std::shared_ptr<type::Schema>;

  // Access all data registered
  Ptr getMergedSchema();

  // Look up a definition by URI. Returns nullptr if not found.
  // The definition must be defined in a file with the `any` cpp2 compiler
  // option enabled.
  const syntax_graph::DefinitionNode* getSyntaxGraphDefinitionNodeByUri(
      const std::string_view uri) const;

  BaseSchemaRegistry& base_;

  Ptr mergedSchema_;
  folly::relaxed_atomic<bool> mergedSchemaAccessed_{false};
  std::unordered_set<type::ProgramId> includedPrograms_;

  std::unique_ptr<syntax_graph::SyntaxGraph> syntaxGraph_;
  mutable folly::DelayedInit<folly::F14FastSet<type_system::Uri>> knownUris_;
  syntax_graph::detail::IncrementalResolver* resolver_;

  friend struct test::SchemaTest;
};

} // namespace apache::thrift

#endif
