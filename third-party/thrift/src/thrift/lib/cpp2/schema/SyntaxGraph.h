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

#include <folly/CppAttributes.h>
#include <folly/Overload.h>
#include <folly/Synchronized.h>
#include <folly/Traits.h>
#include <folly/Utility.h>
#include <folly/container/F14Map.h>
#include <folly/container/span.h>
#include <folly/json/dynamic.h>
#include <folly/lang/Exception.h>
#include <folly/memory/not_null.h>

#include <cstdint>
#include <functional>
#include <iosfwd>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include <thrift/common/tree_printer.h>
#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/cpp2/schema/gen-cpp2/syntax_graph_types.h>

namespace apache::thrift::syntax_graph {

/**
 * A `SyntaxGraph` object allows syntactic inspection of Thrift schema, i.e.
 * the "AST" of a set of source .thrift files. In reality, the "AST" is a graph
 * and there may be cycles (for example, a self-referential struct).
 *
 * The "AST" view of Thrift schema does not completely match the type system
 * described in the schema. Consider a struct `Foo` is moved from `a.thrift` to
 * `b.thrift` and its URI is held unchanged...
 *   - From the perspective of Thrift's type system, such a move has no effect.
 *     `Foo` has the same URI and definition and therefore, it IS the same exact
 *     struct. The type-graph (relationship between types) is unchanged.
 *   - From the persepective of `SyntaxGraph`, the structure of the graph has
 *     changed. `Foo` is now defined in `b.thrift` rather than `a.thrift`.
 *     Thrift files are irrelevant to the type system but *are* relevant to the
 *     `SyntaxGraph`.
 *
 * `SyntaxGraph` exposes "compiler" concepts such as Thrift files, struct &
 * service definitions, struct fields, etc. as nodes in a graph. For example, a
 * `FieldNode` and `StructNode` share an edge iff a Thrift struct contains the
 * field.
 *
 * `SyntaxGraph` nodes are (mostly) undirected. That is, A `StructNode` contains
 * a list of `FieldNode`s but also each `FieldNode` has a pointer back to the
 * `StructNode` that "owns" it. This means that each `FieldNode` contains the
 * information necessarily to traverse the entire `SyntaxGraph`.
 *
 * The `SyntaxGraph` object owns all nodes in the graph. Therefore, it must be
 * kept alive while traversing nodes in the graph. The lifetimes of each
 * individual nodes are, otherwise, managed entirely by this API.
 *
 * `SyntaxGraph` is read-only. It is not possible to mutate the schema, nor
 * create new schema from scratch using this API.
 */
class SyntaxGraph;

/**
 * A ".thrift" file.
 */
class ProgramNode;

/**
 * A top-level Thrift user definition in a Thrift file.
 *
 * This node represents the source of truth for the description of one of the
 * following Thrift IDL concepts:
 *   - struct
 *   - union
 *   - exception
 *   - enum
 *   - typedef
 *   - const
 *   - service
 *   - interaction
 *
 * Each of these concepts have their own nodes types (see below), which have a
 * 1-to-1 connection with a node of this type.
 *
 * Examples:
 *
 *     const DefinitionNode& d = ...;
 *     if (d.isStruct()) {
 *       assert(d.kind() == DefinitionNode::Kind::Struct);
 *       const StructNode& s = d.asStruct();
 *     }
 *
 *     d.visit(
 *       [](const StructNode& s) {}
 *       [](const UnionNode& u) {}
 *       [](auto&&) {}
 *     );
 */
class DefinitionNode;

/**
 * A definition representing a Thrift struct:
 *   https://github.com/facebook/fbthrift/blob/main/thrift/doc/idl/index.md#structs
 */
class StructNode;
/**
 * A definition representing a Thrift union:
 * https://github.com/facebook/fbthrift/blob/main/thrift/doc/idl/index.md#unions
 */
class UnionNode;
/**
 * A definition representing a Thrift exception:
 *   https://github.com/facebook/fbthrift/blob/main/thrift/doc/idl/index.md#exceptions
 */
class ExceptionNode;
/**
 * The base class for all structured types in Thrift.
 *   https://github.com/facebook/fbthrift/blob/main/thrift/doc/idl/index.md#named-types
 *
 * All structured types contain a set of fields. This node is one of:
 *   - struct
 *   - union
 *   - exception
 */
class StructuredNode;
/**
 * A field within a structured type.
 */
class FieldNode;
using FieldId = apache::thrift::type::FieldId;

/**
 * A definition representing a Thrift enum:
 *   https://github.com/facebook/fbthrift/blob/main/thrift/doc/idl/index.md#enums
 *
 * All enums have a corresponding `DefinitionNode`.
 */
class EnumNode;

/**
 * A definition representing a Thrift typedef:
 *   https://github.com/facebook/fbthrift/blob/main/thrift/doc/idl/index.md#typedefs
 */
class TypedefNode;

/**
 * A definition representing a constant Thrift value:
 *   https://github.com/facebook/fbthrift/blob/main/thrift/doc/idl/index.md#constants
 */
class ConstantNode;

/**
 * A definition representing a Thrift service:
 *   https://github.com/facebook/fbthrift/blob/main/thrift/doc/idl/index.md#services
 */
class ServiceNode;
/**
 * A definition representing a Thrift interaction:
 *   https://github.com/facebook/fbthrift/blob/main/thrift/doc/idl/index.md#interactions
 */
class InteractionNode;
/**
 * A base class for Thrift constructs that define functions for RPC.
 *
 * This node is of one:
 *  - service
 *  - interaction
 */
class RpcInterfaceNode;

/**
 * A function defined in an RPC interface.
 */
class FunctionNode;
/**
 * A sink, which flows client → server uni-directionally:
 *    https://github.com/facebook/fbthrift/blob/main/thrift/doc/idl/index.md#streaming
 */
class FunctionSink;
/**
 * A stream, which flows server → client uni-directionally:
 *    https://github.com/facebook/fbthrift/blob/main/thrift/doc/idl/index.md#streaming
 */
class FunctionStream;
/**
 * A function response, which includes:
 *   - a data type, void, or an interaction
 *   - a stream or sink (optional)
 */
class FunctionResponse;
/**
 * A parameter that is passed to a Thrift RPC.
 */
class FunctionParam;
/**
 * An exception in the throws clause of a Thrift RPC.
 */
class FunctionException;

/**
 * A descriptor of a data type within Thrift. A `TypeRef` is one of:
 *   - primitive Thrift type (i32, string etc.)
 *   - user definitions (structured, enum, or typedef)
 *   - container type (list, set, map)
 *
 * This class is recursive since container types are generic and refer to other
 * types.
 *
 * `TypeRef` models both Copyable and Movable.
 *
 * NOTE: `TypeRef` objects are only valid if the `SyntaxGraph` object that it
 * originates from is still alive.
 *
 * Examples:
 *
 *     const FieldNode& f = ...;
 *     if (f.type() == TypeRef::of(Primitive::I32)) {
 *       assert(f.kind() == TypeRef::Kind::PRIMITIVE);
 *       assert(f.as<Primitive>() == Primitive::I32);
 *     }
 *
 *     f.type().visit(
 *       [](const List& l) {}
 *       [](const StructNode& s) {}
 *       [](auto&&) {}
 *     );
 */
class TypeRef;
/**
 * A generic Thrift list type:
 *   https://github.com/facebook/fbthrift/blob/main/thrift/doc/idl/index.md#container-types
 */
class List;
/**
 * A generic Thrift set type:
 *   https://github.com/facebook/fbthrift/blob/main/thrift/doc/idl/index.md#container-types
 */
class Set;
/**
 * A generic Thrift map type:
 *   https://github.com/facebook/fbthrift/blob/main/thrift/doc/idl/index.md#container-types
 */
class Map;
/**
 * A primitive Thrift type:
 *   https://github.com/facebook/fbthrift/blob/main/thrift/doc/idl/index.md#primitive-types
 */
enum class Primitive;
std::string_view toString(Primitive);

/**
 * A class representing a structured annotation in Thrift
 */
class Annotation;

/**
 * Exception type that is thrown if there an attempt to form a `SyntaxGraph`
 * from invalid schema information. Some common scenario that result in this
 * exception being thrown:
 *   - Schema contains a reference to definition (DefinitionKey) without a
 *     corresponding concrete Definition for that key.
 *   - Schema contains reference to a file which is missing from the schema.
 */
class InvalidSyntaxGraphError : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

namespace detail {

using DefinitionKeyRef =
    std::reference_wrapper<const apache::thrift::type::DefinitionKey>;

/**
 * An opaque type whose implementation "resolves" the nodes at the end of edges
 * in the `SyntaxGraph` graph. For example, a `FunctionNode` contains an edge
 * back to its parent `ServiceNode`. The `Resolver` is responsible for mapping
 * the `type::DefinitionKey` (edge) into a `const ServiceNode&`.
 *
 * This type is hidden from the user but is the "brain" of the graph. Most graph
 * nodes will carry a reference to this type internally.
 *
 * The Resolver has two very crucial responsibilities:
 *   1. Manage the lifetime of all graph nodes. This allows most graph traversal
 *      operations to return by reference and hide lifetime management from the
 *      user.
 *   2. Enable lazy resolution of nodes in the graph. This is necessary
 *      to support forward references and self-references (recursive struct).
 *      For example, a `FieldNode` may refer to a `StructNode` that is defined
 *      later in the file. In this case, the `FieldNode` will not have a
 *      reference to the `StructNode` at the time of construction. Instead the
 *      `FieldNode` will contain a `DefinitionKey` that can be used to lazily
 *      resolve the `StructNode` at a later time.
 */
class Resolver;
/**
 * Base class for graph nodes that need lazy resolution internally to traverse
 * the graph.
 */
class WithResolver {
 protected:
  explicit WithResolver(const Resolver& resolver) : resolver_(&resolver) {}
  const Resolver& resolver() const { return *resolver_; }

 private:
  folly::not_null<const Resolver*> resolver_;
};
/**
 * Lazily resolves the graph node representing a Thrift definition, backed by a
 * lookup key in raw runtime schema struct.
 */
const DefinitionNode& lazyResolve(
    const Resolver&, const apache::thrift::type::DefinitionKey&);

/**
 * A class representing a (potentially) lazily-resolved definition graph node of
 * the specified type.
 *
 * Lazy resolution is needed because:
 *   - Thrift types can refer to types defined later in a file (forward
 *     references).
 *   - Thrift types can be self-referential.
 *
 * Lazy resolution is also used to implement "back-references" — e.g. a
 * `FieldNode` referring back to the containing `StructNode`.
 */
template <typename T>
class Lazy {
 public:
  /**
   * An unresolved reference to a definition. This reference will be resolved
   * only upon access via the provided resolver.
   */
  struct Unresolved {
    folly::not_null<const Resolver*> resolver;
    detail::DefinitionKeyRef definitionKey;

    Unresolved(
        const Resolver& resolver,
        const apache::thrift::type::DefinitionKey& key)
        : resolver(&resolver), definitionKey(key) {}
  };
  /**
   * An already-resolved definition. Upon access, no resolution is performed.
   * This is useful in scenarios where the definition is already known to avoid
   * redundant resolution.
   */
  struct Resolved {
    folly::not_null<const T*> value;

    explicit Resolved(const T& value) : value(std::addressof(value)) {}
  };

  /* implicit */ Lazy(Unresolved&& unresolved)
      : state_(std::move(unresolved)) {}
  /* implicit */ Lazy(Resolved&& resolved) : state_(std::move(resolved)) {}

  /**
   * Resolves the definition (if necessary).
   */
  const T& operator*() const;

 private:
  std::variant<Unresolved, Resolved> state_;
};

/**
 * Base class for all graph nodes that have names.
 */
class WithName {
 protected:
  // Requirement: `name` is backed by a null-terminated string (e.g. static
  // string or std::string)
  explicit WithName(std::string_view name);

  /**
   * The scoped name of this graph node.
   * Can be assumed to be null-terminated
   */
  std::string_view name() const { return name_; }

 private:
  std::string_view name_;
};

/**
 * Base class for all graph nodes that have URIs.
 */
class WithUri {
 protected:
  explicit WithUri(std::string_view uri) : uri_(uri) {}

  /**
   * The URI name of this graph node. This may be inferred from the package or
   * explicit provided with an annotation.
   */
  std::string_view uri() const { return uri_; }

 private:
  std::string_view uri_;
};

/**
 * Base class for all graph nodes that have a corresponding `DefinitionNode`.
 */
class WithDefinition : public WithResolver {
 protected:
  explicit WithDefinition(
      const Resolver& resolver, const type::DefinitionKey& definitionKey)
      : WithResolver(resolver), definitionKey_(definitionKey) {}

  const DefinitionNode& definition() const {
    return lazyResolve(resolver(), definitionKey_);
  }

  DefinitionKeyRef definitionKey_;
};

/**
 * Base class for all graph nodes that can have structured annotations applied
 * to them.
 */
class WithAnnotations {
 protected:
  folly::span<const Annotation> annotations() const;

  explicit WithAnnotations(std::vector<Annotation>&& annotations);
  ~WithAnnotations() noexcept;

  WithAnnotations(WithAnnotations&&) noexcept;
  WithAnnotations& operator=(WithAnnotations&&) noexcept;

 private:
  std::vector<Annotation> annotations_;
};

// Helper to get the index of a type in a variant at compile-time
template <typename Variant, typename T>
struct IndexOfImpl;
template <typename T, typename... Types>
struct IndexOfImpl<std::variant<Types...>, T> {
  static constexpr std::size_t value = folly::type_pack_find_v<T, Types...>;
};
template <typename Variant, typename T>
inline constexpr std::size_t IndexOf = IndexOfImpl<Variant, T>::value;

/**
 * In most cases, using SyntaxGraph completely abstracts away DefinitionKeys.
 * This is because SyntaxGraph only creates one DefinitionNode per key. So
 * comparing two nodes by their address is equivalent to comparing their keys.
 *
 * However, some APIs that predate SyntaxGraph expose the raw schema.thrift,
 * along with relevant DefinitionKeys. In such cases, there is no way easy
 * migration path without an escape hatch like this.
 *
 * Throws std::out_of_range if the key is not found in the graph.
 */
const DefinitionNode& lookUpDefinition(
    const SyntaxGraph&, const apache::thrift::type::DefinitionKey&);

/**
 * This class tracks the set of nodes that have already been visited when debug
 * printing the graph. The graph may include cycles, so this is necessary to
 * avoid infinite recursion.
 *
 * Since every node has a unique identity (and thus address), we store and
 * compare against the memory address of nodes. Once a node has been visited, it
 * may not be "un-visited".
 */
class VisitationTracker {
 public:
  struct MarkResult {
    /**
     * True if the node has been marked already before this call.
     */
    bool already;
  };

  MarkResult mark(const DefinitionNode& definition) {
    auto [_, inserted] = visited_.insert(addressOf(definition));
    return {!inserted};
  }
  MarkResult mark(const ProgramNode& program) {
    auto [_, inserted] = visited_.insert(addressOf(program));
    return {!inserted};
  }

 private:
  template <typename T>
  static std::uintptr_t addressOf(const T& object) {
    return reinterpret_cast<std::uintptr_t>(std::addressof(object));
  }

  std::unordered_set<std::uintptr_t> visited_;
};

/**
 * CRTP base class to implement debug printing for node types.
 *
 * The "Self" type is expected to have a member function with the following
 * signature:
 *
 *     void printTo(tree_printer::scope&, VisitationTracker&) const;
 */
template <typename Self>
class WithDebugPrinting {
 public:
  /**
   * Produces a string representation of this node that is useful for debugging
   * ONLY.
   *
   * This string is not guaranteed to be stable, nor is it guaranteed to include
   * complete information.
   *
   * The printTo() function includes an overload that includes a set of
   * already-visited nodes since the graph may contain cycles.
   */
  void printTo(tree_printer::scope& scope) const {
    detail::VisitationTracker visited;
    static_cast<const Self&>(*this).printTo(scope, visited);
  }

  std::string toDebugString() const {
    auto scope = tree_printer::scope::make_root();
    this->printTo(scope);
    return tree_printer::to_string(scope);
  }

  friend std::ostream& operator<<(std::ostream& out, const Self& self) {
    auto scope = tree_printer::scope::make_root();
    detail::VisitationTracker visited;
    self.printTo(scope, visited);
    return out << scope;
  }
};

} // namespace detail

class FieldNode final : folly::MoveOnly,
                        detail::WithResolver,
                        detail::WithName,
                        detail::WithAnnotations,
                        public detail::WithDebugPrinting<FieldNode> {
 public:
  /**
   * The presence qualification for a field. If a field is not marked optional,
   * it is always present.
   *
   * This does not mean that the serialized binary will include the field. For
   * example, a terse field set to the intrinsic default will be skipped during
   * serialization. However, such fields still "has a value" according to
   * Thrift's type system.
   */
  using PresenceQualifier = syntax_graph::FieldPresenceQualifier;

  using detail::WithAnnotations::annotations;
  using detail::WithName::name;
  FieldId id() const { return id_; }
  TypeRef type() const;
  PresenceQualifier presence() const { return presence_; }
  const apache::thrift::protocol::Value* FOLLY_NULLABLE customDefault() const;
  /**
   * A reference to the user-defined type that contains this field.
   */
  const StructuredNode& parent() const;

  FieldNode(
      const detail::Resolver& resolver,
      const apache::thrift::type::DefinitionKey& parent,
      std::vector<Annotation>&& annotations,
      FieldId id,
      PresenceQualifier presence,
      std::string_view name,
      folly::not_null_unique_ptr<TypeRef> type,
      std::optional<apache::thrift::type::ValueId> customDefaultId)
      : detail::WithResolver(resolver),
        detail::WithName(name),
        detail::WithAnnotations(std::move(annotations)),
        parent_(parent),
        id_(id),
        presence_(presence),
        type_(std::move(type)),
        customDefaultId_(std::move(customDefaultId)) {}
  ~FieldNode() noexcept;

  FieldNode(FieldNode&&) noexcept;
  FieldNode& operator=(FieldNode&&) noexcept;

  void printTo(
      tree_printer::scope& scope, detail::VisitationTracker& visited) const;

 private:
  detail::DefinitionKeyRef parent_;
  FieldId id_;
  PresenceQualifier presence_;
  folly::not_null_unique_ptr<TypeRef> type_;
  std::optional<apache::thrift::type::ValueId> customDefaultId_;
};

class StructuredNode : detail::WithDefinition, detail::WithUri {
 public:
  using detail::WithDefinition::definition;
  using detail::WithUri::uri;
  folly::span<const FieldNode> fields() const { return fields_; }

  /**
   * Looks up a field by ID.
   *
   * Throws:
   *   - std::out_of_range if the field ID is not present.
   */
  const FieldNode& at(FieldId id) const { return at(fieldHandleFor(id)); }
  /**
   * Looks up a field by name.
   *
   * Throws:
   *   - std::out_of_range if the field ID is not present.
   */
  const FieldNode& at(std::string_view name) const {
    return at(fieldHandleFor(name));
  }

 protected:
  StructuredNode(
      const detail::Resolver& resolver,
      const apache::thrift::type::DefinitionKey& definitionKey,
      std::string_view uri,
      std::vector<FieldNode> fields)
      : detail::WithDefinition(resolver, definitionKey),
        detail::WithUri(uri),
        fields_(std::move(fields)) {
    std::uint16_t ordinal = 1;
    for (const FieldNode& field : fields_) {
      fieldHandleById_.emplace(
          field.id(), type_system::FastFieldHandle{ordinal});
      fieldHandleByName_.emplace(
          field.name(), type_system::FastFieldHandle{ordinal});
      ++ordinal;
    }
  }

 private:
  /**
   * Looks up a field by a fast field handle, previously obtained from
   * fieldHandleFor(...);
   *
   * Preconditions:
   *   - The provided handle was obtained by calling fieldHandleFor(...) on this
   *     instance.
   *
   * Throws:
   *   - std::out_of_range if the field handle is invalid or out of range.
   */
  const FieldNode& at(type_system::FastFieldHandle handle) const {
    if (!handle.valid() || handle.ordinal > fields_.size()) {
      folly::throw_exception<std::out_of_range>(
          fmt::format("invalid field handle: {}", handle.ordinal));
    }
    return fields_.at(handle.ordinal - 1);
  }

  /**
   * Returns a field handle for the given field ID, if it exists, returning
   * `FastFieldHandle::invalid()` otherwise.
   */
  type_system::FastFieldHandle fieldHandleFor(FieldId id) const noexcept {
    if (const type_system::FastFieldHandle* handle =
            folly::get_ptr(fieldHandleById_, id)) {
      return *handle;
    }
    return type_system::FastFieldHandle::invalid();
  }
  /**
   * Returns a field handle for the given field name, if it exists, returning
   * `FastFieldHandle::invalid()` otherwise.
   */
  type_system::FastFieldHandle fieldHandleFor(
      std::string_view name) const noexcept {
    if (const type_system::FastFieldHandle* handle =
            folly::get_ptr(fieldHandleByName_, name)) {
      return *handle;
    }
    return type_system::FastFieldHandle::invalid();
  }

  std::vector<FieldNode> fields_;
  folly::F14FastMap<FieldId, type_system::FastFieldHandle> fieldHandleById_;
  folly::F14FastMap<std::string_view, type_system::FastFieldHandle>
      fieldHandleByName_;
};

class StructNode final : folly::MoveOnly,
                         public StructuredNode,
                         public detail::WithDebugPrinting<StructNode> {
 public:
  StructNode(
      const detail::Resolver& resolver,
      const apache::thrift::type::DefinitionKey& definitionKey,
      std::string_view uri,
      std::vector<FieldNode> fields)
      : StructuredNode(resolver, definitionKey, uri, std::move(fields)) {}

  void printTo(
      tree_printer::scope& scope, detail::VisitationTracker& visited) const;
};

class UnionNode final : folly::MoveOnly,
                        public StructuredNode,
                        public detail::WithDebugPrinting<UnionNode> {
 public:
  UnionNode(
      const detail::Resolver& resolver,
      const apache::thrift::type::DefinitionKey& definitionKey,
      std::string_view uri,
      std::vector<FieldNode> fields)
      : StructuredNode(resolver, definitionKey, uri, std::move(fields)) {}

  void printTo(
      tree_printer::scope& scope, detail::VisitationTracker& visited) const;
};

class ExceptionNode final : folly::MoveOnly,
                            public StructuredNode,
                            public detail::WithDebugPrinting<ExceptionNode> {
 public:
  ExceptionNode(
      const detail::Resolver& resolver,
      const apache::thrift::type::DefinitionKey& definitionKey,
      std::string_view uri,
      std::vector<FieldNode> fields)
      : StructuredNode(resolver, definitionKey, uri, std::move(fields)) {}

  void printTo(
      tree_printer::scope& scope, detail::VisitationTracker& visited) const;
};

class EnumNode final : folly::MoveOnly,
                       detail::WithDefinition,
                       detail::WithUri,
                       public detail::WithDebugPrinting<EnumNode> {
 public:
  /**
   * A mapping of enum name to its i32 value.
   */
  // TODO: these can also have annotations
  class Value : detail::WithName, detail::WithAnnotations {
   public:
    Value(
        std::string_view name,
        std::int32_t i32,
        std::vector<Annotation>&& annotations)
        : detail::WithName(name),
          detail::WithAnnotations(std::move(annotations)),
          i32_(i32) {}

    using detail::WithAnnotations::annotations;
    using detail::WithName::name;
    /**
     * All enums values in Thrift have underlying type of i32
     */
    std::int32_t i32() const { return i32_; }

    friend bool operator==(const Value& lhs, const Value& rhs) {
      return std::tuple(lhs.name(), lhs.i32()) ==
          std::tuple(rhs.name(), rhs.i32());
    }

   private:
    std::int32_t i32_;
  };

  using detail::WithDefinition::definition;
  using detail::WithUri::uri;
  folly::span<const Value> values() const { return values_; }

  EnumNode(
      const detail::Resolver& resolver,
      const apache::thrift::type::DefinitionKey& definitionKey,
      std::string_view uri,
      std::vector<Value> values)
      : detail::WithDefinition(resolver, definitionKey),
        detail::WithUri(uri),
        values_(std::move(values)) {}

  void printTo(
      tree_printer::scope& scope, detail::VisitationTracker& visited) const;

 private:
  std::vector<Value> values_;
};

class TypedefNode final : folly::MoveOnly,
                          detail::WithDefinition,
                          public detail::WithDebugPrinting<TypedefNode> {
 public:
  using detail::WithDefinition::definition;
  /**
   * A reference to the underlying type, which itself could be another typedef.
   */
  const TypeRef& targetType() const { return *targetType_; }

  TypedefNode(
      const detail::Resolver& resolver,
      const apache::thrift::type::DefinitionKey& definitionKey,
      TypeRef&& targetType);

  void printTo(
      tree_printer::scope& scope, detail::VisitationTracker& visited) const;

 private:
  // Heap usage prevents mutual recursion with `TypeRef` and `DefinitionNode`.
  folly::not_null_unique_ptr<TypeRef> targetType_;
};

class ConstantNode final : folly::MoveOnly,
                           detail::WithDefinition,
                           public detail::WithDebugPrinting<ConstantNode> {
 public:
  using detail::WithDefinition::definition;
  const TypeRef& type() const { return *type_; }
  const apache::thrift::protocol::Value& value() const;

  ConstantNode(
      const detail::Resolver& resolver,
      const apache::thrift::type::DefinitionKey& definitionKey,
      TypeRef&& type,
      apache::thrift::type::ValueId valueId);

  void printTo(
      tree_printer::scope& scope, detail::VisitationTracker& visited) const;

 private:
  // Heap usage prevents mutual recursion with `TypeRef` and `Definition`.
  folly::not_null_unique_ptr<TypeRef> type_;
  apache::thrift::type::ValueId valueId_;
};

class List final : public detail::WithDebugPrinting<List> {
 public:
  const TypeRef& elementType() const { return *elementType_; }

  ~List() noexcept = default;
  List(const List&);
  List& operator=(const List&);
  List(List&&) noexcept = default;
  List& operator=(List&&) noexcept = default;

  friend bool operator==(const List& lhs, const List& rhs);

  static List of(TypeRef elementType);

  explicit List(TypeRef&& elementType);

  void printTo(
      tree_printer::scope& scope, detail::VisitationTracker& visited) const;

 private:
  // Heap usage prevents mutual recursion with `TypeRef`.
  folly::not_null_unique_ptr<TypeRef> elementType_;
};

class Set final : public detail::WithDebugPrinting<Set> {
 public:
  const TypeRef& elementType() const { return *elementType_; }

  ~Set() noexcept = default;
  Set(const Set&);
  Set& operator=(const Set&);
  Set(Set&&) noexcept = default;
  Set& operator=(Set&&) noexcept = default;

  friend bool operator==(const Set& lhs, const Set& rhs);

  static Set of(TypeRef elementType);

  explicit Set(TypeRef&& elementType);

  void printTo(
      tree_printer::scope& scope, detail::VisitationTracker& visited) const;

 private:
  // Heap usage prevents mutual recursion with `TypeRef`.
  folly::not_null_unique_ptr<TypeRef> elementType_;
};

class Map final : public detail::WithDebugPrinting<Map> {
 public:
  const TypeRef& keyType() const { return *keyType_; }
  const TypeRef& valueType() const { return *valueType_; }

  ~Map() noexcept = default;
  Map(const Map&);
  Map& operator=(const Map&);
  Map(Map&&) noexcept = default;
  Map& operator=(Map&&) noexcept = default;

  friend bool operator==(const Map& lhs, const Map& rhs);

  static Map of(TypeRef keyType, TypeRef valueType);

  Map(TypeRef&& keyType, TypeRef&& valueType);

  void printTo(
      tree_printer::scope& scope, detail::VisitationTracker& visited) const;

 private:
  // Heap usage prevents mutual recursion with `TypeRef`.
  folly::not_null_unique_ptr<TypeRef> keyType_;
  folly::not_null_unique_ptr<TypeRef> valueType_;
};

class FunctionException final
    : folly::MoveOnly,
      detail::WithResolver,
      detail::WithName,
      public detail::WithDebugPrinting<FunctionException> {
 public:
  using detail::WithName::name;
  FieldId id() const { return id_; }
  TypeRef type() const;

  FunctionException(
      const detail::Resolver& resolver,
      FieldId id,
      std::string_view name,
      folly::not_null_unique_ptr<TypeRef> type)
      : detail::WithResolver(resolver),
        detail::WithName(name),
        id_(id),
        type_(std::move(type)) {}

  void printTo(
      tree_printer::scope& scope, detail::VisitationTracker& visited) const;

 private:
  FieldId id_;
  folly::not_null_unique_ptr<TypeRef> type_;
};

class FunctionStream final : folly::MoveOnly,
                             public detail::WithDebugPrinting<FunctionStream> {
 public:
  // A Thrift stream in IDL takes the form:
  //     stream<{payloadType} throws (... {exceptions} ...)>
  const TypeRef& payloadType() const { return *payloadType_; }
  folly::span<const FunctionException> exceptions() const;

  FunctionStream(
      TypeRef&& payloadType, std::vector<FunctionException>&& exceptions);

  void printTo(
      tree_printer::scope& scope, detail::VisitationTracker& visited) const;

 private:
  // Heap usage prevents mutual recursion with `DefinitionNode`.
  folly::not_null_unique_ptr<TypeRef> payloadType_;
  std::vector<FunctionException> exceptions_;
};

class TypeRef final : public detail::WithDebugPrinting<TypeRef> {
 public:
  using Alternative = std::variant<
      Primitive,
      detail::Lazy<StructNode>,
      detail::Lazy<UnionNode>,
      detail::Lazy<ExceptionNode>,
      detail::Lazy<EnumNode>,
      detail::Lazy<TypedefNode>,
      List,
      Set,
      Map>;
  static_assert(std::is_copy_constructible_v<Alternative>);

  enum class Kind {
    PRIMITIVE = detail::IndexOf<Alternative, Primitive>,
    STRUCT = detail::IndexOf<Alternative, detail::Lazy<StructNode>>,
    UNION = detail::IndexOf<Alternative, detail::Lazy<UnionNode>>,
    EXCEPTION = detail::IndexOf<Alternative, detail::Lazy<ExceptionNode>>,
    ENUM = detail::IndexOf<Alternative, detail::Lazy<EnumNode>>,
    TYPEDEF = detail::IndexOf<Alternative, detail::Lazy<TypedefNode>>,
    LIST = detail::IndexOf<Alternative, List>,
    SET = detail::IndexOf<Alternative, Set>,
    MAP = detail::IndexOf<Alternative, Map>,
  };
  Kind kind() const { return static_cast<Kind>(type_.index()); }

  bool isPrimitive() const noexcept { return kind() == Kind::PRIMITIVE; }
  bool isStruct() const noexcept { return kind() == Kind::STRUCT; }
  bool isUnion() const noexcept { return kind() == Kind::UNION; }
  bool isException() const noexcept { return kind() == Kind::EXCEPTION; }
  bool isEnum() const noexcept { return kind() == Kind::ENUM; }
  bool isTypedef() const noexcept { return kind() == Kind::TYPEDEF; }
  bool isList() const noexcept { return kind() == Kind::LIST; }
  bool isSet() const noexcept { return kind() == Kind::SET; }
  bool isMap() const noexcept { return kind() == Kind::MAP; }

  const Primitive& asPrimitive() const { return as<Primitive>(); }
  const StructNode& asStruct() const { return as<StructNode>(); }
  const UnionNode& asUnion() const { return as<UnionNode>(); }
  const ExceptionNode& asException() const { return as<ExceptionNode>(); }
  const EnumNode& asEnum() const { return as<EnumNode>(); }
  const TypedefNode& asTypedef() const { return as<TypedefNode>(); }
  const List& asList() const { return as<List>(); }
  const Set& asSet() const { return as<Set>(); }
  const Map& asMap() const { return as<Map>(); }

  bool isStructured() const {
    switch (kind()) {
      case Kind::STRUCT:
      case Kind::UNION:
      case Kind::EXCEPTION:
        return true;
      case Kind::PRIMITIVE:
      case Kind::ENUM:
      case Kind::TYPEDEF:
      case Kind::LIST:
      case Kind::SET:
      case Kind::MAP:
      default:
        return false;
    }
  }
  /**
   * Returns the `StructuredNode` object reference, assuming the active variant
   * alternative is a structured type.
   *
   * Pre-conditions:
   *   - kind() is one of {STRUCT, UNION, EXCEPTION}.
   */
  const StructuredNode& asStructured() const {
    switch (kind()) {
      case Kind::STRUCT:
        return asStruct();
      case Kind::UNION:
        return asUnion();
      case Kind::EXCEPTION:
        return asException();
      case Kind::PRIMITIVE:
      case Kind::ENUM:
      case Kind::TYPEDEF:
      case Kind::LIST:
      case Kind::SET:
      case Kind::MAP:
      default:
        break;
    }
    folly::throw_exception<std::bad_variant_access>();
  }

  bool isContainer() const {
    switch (kind()) {
      case Kind::LIST:
      case Kind::SET:
      case Kind::MAP:
        return true;
      case Kind::PRIMITIVE:
      case Kind::ENUM:
      case Kind::TYPEDEF:
      case Kind::STRUCT:
      case Kind::UNION:
      case Kind::EXCEPTION:
      default:
        return false;
    }
  }

  /**
   * Returns a new `TypeRef` object by recursively resolving typedefs.
   *
   * If this type is a typedef, then this function returns the trueType() of the
   * underlying type.
   *
   * If this type does not represent a typedef, then this function returns a
   * reference to `*this`.
   *
   * Post-conditions:
   *   - kind() != Kind::TYPEDEF
   */
  const TypeRef& trueType() const {
    if (kind() == Kind::TYPEDEF) {
      return asTypedef().targetType().trueType();
    }

    return *this;
  }

  /**
   * An `std::visit`-like API for pattern-matching on the active variant
   * alternative of the underlying type.
   */
  template <typename... F>
  decltype(auto) visit(F&&... visitors) const {
    auto overloaded = folly::overload(std::forward<F>(visitors)...);
    return folly::variant_match(
        type_,
        [&](const Primitive& primitive) -> decltype(auto) {
          return overloaded(primitive);
        },
        [&](const List& list) -> decltype(auto) { return overloaded(list); },
        [&](const Set& set) -> decltype(auto) { return overloaded(set); },
        [&](const Map& map) -> decltype(auto) { return overloaded(map); },
        [&](auto&& lazy) -> decltype(auto) { return overloaded(*lazy); });
  }

  /**
   * `as<T>` produces the contained T, assuming it is the currently active
   * variant alternative.
   *
   * Pre-conditions:
   *   - T is the active variant alternative, else throws
   *     `std::bad_variant_access`
   */
  template <typename T>
  const T& as() const {
    return visit(
        [](const T& value) -> const T& { return value; },
        [](auto&&) -> const T& {
          folly::throw_exception<std::bad_variant_access>();
        });
  }

  friend bool operator==(const TypeRef&, const TypeRef&);
  friend bool operator==(const TypeRef&, const DefinitionNode&);
  friend bool operator==(const DefinitionNode& lhs, const TypeRef& rhs) {
    return rhs == lhs;
  }

  static TypeRef of(Primitive);
  static TypeRef of(const StructNode&);
  static TypeRef of(const UnionNode&);
  static TypeRef of(const ExceptionNode&);
  static TypeRef of(const EnumNode&);
  static TypeRef of(const TypedefNode&);
  static TypeRef of(const List&);
  static TypeRef of(const Set&);
  static TypeRef of(const Map&);

  explicit TypeRef(Alternative&& type) : type_(std::move(type)) {}

  void printTo(
      tree_printer::scope& scope, detail::VisitationTracker& visited) const;

 private:
  Alternative type_;
};
// `TypeRef` is a value type and should behave like one.
static_assert(std::is_copy_constructible_v<TypeRef>);
static_assert(std::is_move_constructible_v<TypeRef>);
static_assert(std::is_copy_assignable_v<TypeRef>);
static_assert(std::is_move_assignable_v<TypeRef>);

class FunctionSink final : folly::MoveOnly,
                           detail::WithDebugPrinting<FunctionSink> {
 public:
  // A Thrift sink in IDL takes the form:
  //     sink<{payloadType} throws (... {clientExceptions} ...),
  //          {finalResponseType} throws (... {serverExceptions} ...)>
  const TypeRef& payloadType() const { return *payloadType_; }
  const TypeRef& finalResponseType() const { return *finalResponseType_; }
  folly::span<const FunctionException> clientExceptions() const;
  folly::span<const FunctionException> serverExceptions() const;

  FunctionSink(
      TypeRef&& payloadType,
      TypeRef&& finalResponseType,
      std::vector<FunctionException>&& clientExceptions,
      std::vector<FunctionException>&& serverExceptions);

  void printTo(
      tree_printer::scope& scope, detail::VisitationTracker& visited) const;

 private:
  // Heap usage prevents mutual recursion with `DefinitionNode`.
  folly::not_null_unique_ptr<TypeRef> payloadType_;
  folly::not_null_unique_ptr<TypeRef> finalResponseType_;
  std::vector<FunctionException> clientExceptions_;
  std::vector<FunctionException> serverExceptions_;
};

class FunctionResponse final
    : folly::MoveOnly,
      public detail::WithDebugPrinting<FunctionResponse> {
 public:
  /**
   * Returns the initial response data type of an RPC, or nullptr if the
   * response is `void`.
   */
  const TypeRef* FOLLY_NULLABLE type() const { return type_.get(); }

  /**
   * Returns the interaction type created by the RPC, or nullptr if there is no
   * interaction.
   */
  const InteractionNode* FOLLY_NULLABLE interaction() const {
    if (interaction_.has_value()) {
      return std::addressof(*interaction_.value());
    }
    return nullptr;
  }

  /**
   * Returns the sink opened by the RPC, or nullptr if there is no sink.
   *
   * This is mutually exclusive with streams.
   */
  const FunctionSink* FOLLY_NULLABLE sink() const {
    return std::get_if<FunctionSink>(&sinkOrStream_);
  }
  /**
   * Returns the sink opened by the RPC, or nullptr if there is no sink.
   *
   * This is mutually exclusive with sinks.
   */
  const FunctionStream* FOLLY_NULLABLE stream() const {
    return std::get_if<FunctionStream>(&sinkOrStream_);
  }

  using SinkOrStream =
      std::variant<std::monostate, FunctionSink, FunctionStream>;
  FunctionResponse(
      std::unique_ptr<TypeRef>&& type,
      std::optional<detail::Lazy<InteractionNode>>&& interaction,
      SinkOrStream&& sinkOrStream)
      : type_(std::move(type)),
        interaction_(std::move(interaction)),
        sinkOrStream_(std::move(sinkOrStream)) {}

  void printTo(
      tree_printer::scope& scope, detail::VisitationTracker& visited) const;

 private:
  std::unique_ptr<TypeRef> type_;
  std::optional<detail::Lazy<InteractionNode>> interaction_;
  SinkOrStream sinkOrStream_;
};

class FunctionParam final : folly::MoveOnly,
                            detail::WithResolver,
                            detail::WithName,
                            public detail::WithDebugPrinting<FunctionParam> {
 public:
  using detail::WithName::name;
  FieldId id() const { return id_; }
  TypeRef type() const;

  FunctionParam(
      const detail::Resolver& resolver,
      FieldId id,
      std::string_view name,
      folly::not_null_unique_ptr<TypeRef> type)
      : detail::WithResolver(resolver),
        detail::WithName(name),
        id_(id),
        type_(std::move(type)) {}

  void printTo(
      tree_printer::scope& scope, detail::VisitationTracker& visited) const;

 private:
  FieldId id_;
  folly::not_null_unique_ptr<TypeRef> type_;
};

class FunctionNode final : folly::MoveOnly,
                           detail::WithResolver,
                           detail::WithName,
                           detail::WithAnnotations,
                           public detail::WithDebugPrinting<FunctionNode> {
 public:
  using detail::WithAnnotations::annotations;
  using detail::WithName::name;
  /**
   * A reference to the service or interaction that contains this function.
   */
  const RpcInterfaceNode& parent() const;

  using Sink = FunctionSink;
  using Stream = FunctionStream;
  using Response = FunctionResponse;
  using Param = FunctionParam;
  using Exception = FunctionException;

  const Response& response() const { return response_; }
  folly::span<const Param> params() const { return params_; }
  folly::span<const Exception> exceptions() const;

  FunctionNode(
      const detail::Resolver& resolver,
      const apache::thrift::type::DefinitionKey& parent,
      std::vector<Annotation>&& annotations,
      Response&& response,
      std::string_view name,
      std::vector<Param>&& params,
      std::vector<Exception>&& exceptions);

  void printTo(
      tree_printer::scope& scope, detail::VisitationTracker& visited) const;

 private:
  detail::DefinitionKeyRef parent_;
  Response response_;
  std::vector<Param> params_;
  std::vector<Exception> exceptions_;
};

class RpcInterfaceNode : detail::WithDefinition, detail::WithUri {
 public:
  using detail::WithDefinition::definition;
  using detail::WithUri::uri;
  folly::span<const FunctionNode> functions() const { return functions_; }

  // We outline this destructor because `FunctionNode` is incomplete at the
  // time of declaration.
  // https://eel.is/c++draft/vector#overview-4 states that the type must be
  // complete when any of vector's members are referenced.
  ~RpcInterfaceNode();

  // We need to explicitly declare a default move constructor here because
  // https://eel.is/c++draft/class.copy.ctor#8.4 states that the move
  // constructor is implicitly declared as defaulted only if there's no user
  // declared destructor, even if we default the destructor later.
  RpcInterfaceNode(RpcInterfaceNode&&) = default;
  RpcInterfaceNode& operator=(RpcInterfaceNode&&) = default;

  void printTo(
      tree_printer::scope& scope, detail::VisitationTracker& visited) const;

 protected:
  RpcInterfaceNode(
      const detail::Resolver& resolver,
      const apache::thrift::type::DefinitionKey& definitionKey,
      std::string_view uri,
      std::vector<FunctionNode>&& functions);

  using detail::WithDefinition::WithResolver::resolver;

 private:
  std::vector<FunctionNode> functions_;
};

class ServiceNode final : folly::MoveOnly,
                          public RpcInterfaceNode,
                          public detail::WithDebugPrinting<ServiceNode> {
 public:
  const ServiceNode* FOLLY_NULLABLE baseService() const;

  ServiceNode(
      const detail::Resolver& resolver,
      const apache::thrift::type::DefinitionKey& definitionKey,
      std::string_view uri,
      std::vector<FunctionNode>&& functions,
      std::optional<detail::DefinitionKeyRef> baseServiceKey)
      : RpcInterfaceNode(resolver, definitionKey, uri, std::move(functions)),
        baseServiceKey_(std::move(baseServiceKey)) {}

  void printTo(
      tree_printer::scope& scope, detail::VisitationTracker& visited) const;

 private:
  std::optional<detail::DefinitionKeyRef> baseServiceKey_;
};

class InteractionNode final
    : folly::MoveOnly,
      public RpcInterfaceNode,
      public detail::WithDebugPrinting<InteractionNode> {
 public:
  InteractionNode(
      const detail::Resolver& resolver,
      const apache::thrift::type::DefinitionKey& definitionKey,
      std::string_view uri,
      std::vector<FunctionNode>&& functions)
      : RpcInterfaceNode(resolver, definitionKey, uri, std::move(functions)) {}

  void printTo(
      tree_printer::scope& scope, detail::VisitationTracker& visited) const;
};

class DefinitionNode final : folly::MoveOnly,
                             detail::WithResolver,
                             detail::WithName,
                             detail::WithAnnotations,
                             public detail::WithDebugPrinting<DefinitionNode> {
 public:
  using Alternative = std::variant<
      StructNode,
      UnionNode,
      ExceptionNode,
      EnumNode,
      TypedefNode,
      ConstantNode,
      ServiceNode,
      InteractionNode>;
  static_assert(std::is_move_constructible_v<Alternative>);

  const ProgramNode& program() const;
  using detail::WithAnnotations::annotations;
  using detail::WithName::name;

  enum class Kind {
    STRUCT = detail::IndexOf<Alternative, StructNode>,
    UNION = detail::IndexOf<Alternative, UnionNode>,
    EXCEPTION = detail::IndexOf<Alternative, ExceptionNode>,
    ENUM = detail::IndexOf<Alternative, EnumNode>,
    TYPEDEF = detail::IndexOf<Alternative, TypedefNode>,
    CONSTANT = detail::IndexOf<Alternative, ConstantNode>,
    SERVICE = detail::IndexOf<Alternative, ServiceNode>,
    INTERACTION = detail::IndexOf<Alternative, InteractionNode>,
  };
  Kind kind() const { return static_cast<Kind>(definition_.index()); }

  bool isStruct() const noexcept { return kind() == Kind::STRUCT; }
  bool isUnion() const noexcept { return kind() == Kind::UNION; }
  bool isException() const noexcept { return kind() == Kind::EXCEPTION; }
  bool isEnum() const noexcept { return kind() == Kind::ENUM; }
  bool isTypedef() const noexcept { return kind() == Kind::TYPEDEF; }
  bool isConstant() const noexcept { return kind() == Kind::CONSTANT; }
  bool isService() const noexcept { return kind() == Kind::SERVICE; }
  bool isInteraction() const noexcept { return kind() == Kind::INTERACTION; }

  const StructNode& asStruct() const { return as<StructNode>(); }
  const UnionNode& asUnion() const { return as<UnionNode>(); }
  const ExceptionNode& asException() const { return as<ExceptionNode>(); }
  const EnumNode& asEnum() const { return as<EnumNode>(); }
  const TypedefNode& asTypedef() const { return as<TypedefNode>(); }
  const ConstantNode& asConstant() const { return as<ConstantNode>(); }
  const ServiceNode& asService() const { return as<ServiceNode>(); }
  const InteractionNode& asInteraction() const { return as<InteractionNode>(); }

  bool isStructured() const {
    switch (kind()) {
      case Kind::STRUCT:
      case Kind::UNION:
      case Kind::EXCEPTION:
        return true;
      case Kind::ENUM:
      case Kind::TYPEDEF:
      case Kind::CONSTANT:
      case Kind::SERVICE:
      case Kind::INTERACTION:
      default:
        return false;
    }
  }
  /**
   * Returns the `StructuredNode` object reference, assuming the active variant
   * alternative is a structured type.
   *
   * Pre-conditions:
   *   - kind() is one of {STRUCT, UNION, EXCEPTION}, else throws
   *     `std::bad_variant_access`.
   */
  const StructuredNode& asStructured() const {
    switch (kind()) {
      case Kind::STRUCT:
        return asStruct();
      case Kind::UNION:
        return asUnion();
      case Kind::EXCEPTION:
        return asException();
      case Kind::ENUM:
      case Kind::TYPEDEF:
      case Kind::CONSTANT:
      case Kind::SERVICE:
      case Kind::INTERACTION:
      default:
        break;
    }
    folly::throw_exception<std::bad_variant_access>();
  }

  bool isRpcInterface() const {
    switch (kind()) {
      case Kind::SERVICE:
      case Kind::INTERACTION:
        return true;
      case Kind::STRUCT:
      case Kind::UNION:
      case Kind::EXCEPTION:
      case Kind::ENUM:
      case Kind::TYPEDEF:
      case Kind::CONSTANT:
      default:
        return false;
    }
  }
  /**
   * Returns the `RpcInterfaceNode` object reference, assuming the active
   * variant alternative is an interface type.
   *
   * Pre-conditions:
   *   - kind() is one of {SERVICE, INTERACTION}, else throws
   *     `std::bad_variant_access`.
   */
  const RpcInterfaceNode& asRpcInterface() const {
    switch (kind()) {
      case Kind::SERVICE:
        return asService();
      case Kind::INTERACTION:
        return asInteraction();
      case Kind::STRUCT:
      case Kind::UNION:
      case Kind::EXCEPTION:
      case Kind::ENUM:
      case Kind::TYPEDEF:
      case Kind::CONSTANT:
      default:
        break;
    }
    folly::throw_exception<std::bad_variant_access>();
  }

  /**
   * An `std::visit`-like API for pattern-matching on the active variant
   * alternative of the underlying definition.
   */
  template <typename... F>
  decltype(auto) visit(F&&... visitors) const {
    return folly::variant_match(definition_, std::forward<F>(visitors)...);
  }

  /**
   * `as<T>` produces the contained T, assuming it is the currently active
   * variant alternative.
   *
   * Pre-conditions:
   *   - T is the active variant alternative, else throws
   *     `std::bad_variant_access`.
   */
  template <typename T>
  const T& as() const {
    return visit(
        [](const T& value) -> const T& { return value; },
        [](auto&&) -> const T& {
          folly::throw_exception<std::bad_variant_access>();
        });
  }

  DefinitionNode(
      const detail::Resolver& resolver,
      apache::thrift::type::ProgramId programId,
      std::vector<Annotation>&& annotations,
      std::string_view name,
      Alternative&& definition);

  void printTo(
      tree_printer::scope& scope, detail::VisitationTracker& visited) const;

 private:
  apache::thrift::type::ProgramId programId_;
  Alternative definition_;
};

class Annotation final : folly::MoveOnly {
 public:
  const TypeRef& type() const { return *type_; }

  /**
   * Returns the value of the annotation as a folly::dynamic.
   *
   * For Thrift struct, values are stored field-name based where keys are the
   * field names and values are the corresponding field values.
   *
   * Example:
   *
   *     struct MyAnnot {
   *       1: i32 field1;
   *     }
   *
   *     Annotation a = ...;
   *     const folly::dynamic& value = a.value();
   *     value["field1"].asInt();
   */
  const folly::dynamic& value() const { return value_; }

  Annotation(
      TypeRef&& type,
      const std::map<std::string, apache::thrift::protocol::Value>& fields);

  void printTo(
      tree_printer::scope& scope, detail::VisitationTracker& visited) const;

 private:
  folly::not_null_unique_ptr<TypeRef> type_;
  folly::dynamic value_;
};

class ProgramNode final : folly::MoveOnly,
                          detail::WithResolver,
                          detail::WithName,
                          public detail::WithDebugPrinting<ProgramNode> {
 public:
  using detail::WithName::name;

  std::string_view path() const { return path_; }

  using IncludesList = std::vector<folly::not_null<const ProgramNode*>>;
  IncludesList includes() const;

  using Definitions = std::vector<folly::not_null<const DefinitionNode*>>;
  Definitions definitions() const { return definitions_; }

  using DefinitionsByName = folly::
      F14FastMap<std::string_view, folly::not_null<const DefinitionNode*>>;
  DefinitionsByName definitionsByName() const;

  using Namespaces = std::map<std::string, std::string>;
  const Namespaces& namespaces() const { return namespaces_; }

  ProgramNode(
      const detail::Resolver& resolver,
      std::string_view path,
      std::string_view name,
      std::vector<apache::thrift::type::ProgramId> includes,
      Definitions definitions,
      Namespaces namespaces)
      : detail::WithResolver(resolver),
        detail::WithName(name),
        path_(path),
        includes_(std::move(includes)),
        definitions_(std::move(definitions)),
        namespaces_(std::move(namespaces)) {}

  void printTo(
      tree_printer::scope& scope, detail::VisitationTracker& visited) const;

 private:
  std::string_view path_;
  std::vector<apache::thrift::type::ProgramId> includes_;
  Definitions definitions_;
  Namespaces namespaces_;
};

class SyntaxGraph final : public detail::WithDebugPrinting<SyntaxGraph> {
 public:
  SyntaxGraph(const SyntaxGraph&) = delete;
  SyntaxGraph& operator=(const SyntaxGraph&) = delete;
  SyntaxGraph(SyntaxGraph&&) noexcept;
  SyntaxGraph& operator=(SyntaxGraph&&) noexcept;
  ~SyntaxGraph() noexcept;

  /**
   * Creates a definition graph from raw schema.thrift representation.
   *
   * This overload assumes that the caller will manage the lifetime of the raw
   * `Schema` object. The `Schema` object must outlive the returned
   * `SyntaxGraph` object.
   *
   * Example:
   *
   *     const type::Schema& s = ...;
   *     SyntaxGraph graph = SyntaxGraph::fromSchema(&s);
   *     // I can only use `graph` while `s` is alive.
   *
   * Throws:
   *   - `InvalidSyntaxGraphError` if there are unresolved references within
   *     the provided schema.
   */
  static SyntaxGraph fromSchema(
      folly::not_null<const apache::thrift::type::Schema*>);
  /**
   * Creates a definition graph from raw schema.thrift representation.
   *
   * This overload takes ownership of the provided `Schema` object.
   *
   * Examples:
   *
   *     type::Schema s = ...;
   *     SyntaxGraph graph1 = SyntaxGraph::fromSchema(folly::copy(s));
   *     SyntaxGraph graph2 = SyntaxGraph::fromSchema(std::move(s));
   *     // Both `graph1` and `graph2` are valid.
   *
   * Throws:
   *   - `InvalidSyntaxGraphError` if there are unresolved references within
   *     the provided schema.
   */
  static SyntaxGraph fromSchema(apache::thrift::type::Schema&&);

  /**
   * List of all unique .thrift files that are accessible in the schema.
   */
  ProgramNode::IncludesList programs() const;

  /**
   * Find program by name.
   */
  const ProgramNode& findProgramByName(std::string_view name) const;

  /**
   * Provides a view of all definitions in the schema as a TypeSystem.
   *
   * This object must be kept alive for the lifetime of the returned object.
   *
   * Throws `std::runtime_error` if this instance does not support URI-based
   * lookup (which does not happen with any official Resolver implementations).
   */
  const type_system::TypeSystem& asTypeSystem() const;

  /**
   * Allows converting a SyntaxGraph node into its corresponding TypeSystem
   * node. Equivalent to `asTypeSystem().getUserDefinedType(node.uri())`, but
   * more efficient and supports nodes without URIs.
   */
  type_system::DefinitionRef asTypeSystemDefinitionRef(
      const DefinitionNode& node) const;
  const type_system::StructNode& asTypeSystemStructNode(
      const StructNode& node) const;
  const type_system::UnionNode& asTypeSystemUnionNode(
      const UnionNode& node) const;
  const type_system::EnumNode& asTypeSystemEnumNode(const EnumNode& node) const;

  /**
   * Allows converting a TypeSystem node into its corresponding SyntaxGraph. A
   * TypeSystem node can only be converted if it was initially converted from
   * SyntaxGraph node.
   *
   * Throws `std::runtime_error` if the provided node was not originally
   * converted from a SyntaxGraph node.
   */
  const DefinitionNode& asSyntaxGraphDefinition(
      const type_system::DefinitionNode& node) const;
  const StructNode& asSyntaxGraphStructNode(
      const type_system::StructNode& node) const;
  const UnionNode& asSyntaxGraphUnionNode(
      const type_system::UnionNode& node) const;
  const EnumNode& asSyntaxGraphEnumNode(
      const type_system::EnumNode& node) const;

  explicit SyntaxGraph(std::unique_ptr<detail::Resolver> resolver);

  void printTo(
      tree_printer::scope& scope, detail::VisitationTracker& visited) const;

 private:
  folly::not_null_unique_ptr<const detail::Resolver> resolver_;
  mutable folly::Synchronized<std::unique_ptr<type_system::TypeSystem>>
      typeSystemFacade_;

  friend const DefinitionNode& detail::lookUpDefinition(
      const SyntaxGraph&, const apache::thrift::type::DefinitionKey&);
};
static_assert(std::is_move_constructible_v<SyntaxGraph>);
static_assert(std::is_move_assignable_v<SyntaxGraph>);

inline RpcInterfaceNode::RpcInterfaceNode(
    const detail::Resolver& resolver,
    const apache::thrift::type::DefinitionKey& definitionKey,
    std::string_view uri,
    std::vector<FunctionNode>&& functions)
    : detail::WithDefinition(resolver, definitionKey),
      detail::WithUri(uri),
      functions_(std::move(functions)) {}

inline RpcInterfaceNode::~RpcInterfaceNode() = default;

inline FieldNode::~FieldNode() noexcept = default;
inline FieldNode::FieldNode(FieldNode&&) noexcept = default;
inline FieldNode& FieldNode::operator=(FieldNode&&) noexcept = default;

namespace detail {

inline WithAnnotations::~WithAnnotations() noexcept = default;
inline WithAnnotations::WithAnnotations(WithAnnotations&&) noexcept = default;
inline WithAnnotations& WithAnnotations::operator=(WithAnnotations&&) noexcept =
    default;

template <typename T>
const T& Lazy<T>::operator*() const {
  return folly::variant_match(
      state_,
      [](const Unresolved& unresolved) -> const T& {
        const DefinitionNode& definition =
            lazyResolve(*unresolved.resolver, unresolved.definitionKey);
        return definition.visit(
            [](const T& value) -> const T& { return value; },
            [](auto&&) -> const T& {
              folly::throw_exception<std::bad_variant_access>();
            });
      },
      [](const Resolved& resolved) -> const T& { return *resolved.value; });
}

} // namespace detail

} // namespace apache::thrift::syntax_graph

#endif // THRIFT_SCHEMA_AVAILABLE
