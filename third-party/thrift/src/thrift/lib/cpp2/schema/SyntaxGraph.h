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

#include <thrift/lib/cpp2/async/SchemaV1.h>

#include <folly/CppAttributes.h>
#include <folly/Overload.h>
#include <folly/Traits.h>
#include <folly/Utility.h>
#include <folly/container/F14Map.h>
#include <folly/container/span.h>
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
#include <utility>
#include <variant>
#include <vector>

#ifdef THRIFT_SCHEMA_AVAILABLE

namespace apache::thrift::schema {

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
  explicit WithName(std::string_view name) : name_(name) {}

  /**
   * The scoped name of this graph node.
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
 * A storage type that either:
 *   - owns an object of `T` or,
 *   - keeps a non-owning reference to `T`.
 */
template <typename T>
struct MaybeManaged {
 public:
  explicit MaybeManaged(T&& object) : storage_(std::move(object)) {}
  explicit MaybeManaged(const T& ref) : storage_(std::addressof(ref)) {}

  const T& operator*() const {
    return folly::variant_match(
        storage_,
        [](const T& object) -> const T& { return object; },
        [](const T* ref) -> const T& { return *ref; });
  }
  const T* operator->() const { return std::addressof(**this); }

 private:
  std::variant<T, const T*> storage_;
};

} // namespace detail

enum class Primitive {
  BOOL,
  BYTE,
  I16,
  I32,
  I64,
  FLOAT,
  DOUBLE,
  STRING,
  BINARY,
};

class FieldNode final : folly::MoveOnly,
                        detail::WithResolver,
                        detail::WithName {
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
  enum class PresenceQualifier { UNQUALIFIED, OPTIONAL };

  using detail::WithName::name;
  FieldId id() const { return id_; }
  TypeRef type() const;
  PresenceQualifier presence() const { return presence_; }
  const apache::thrift::protocol::Value* FOLLY_NULLABLE customDefault() const;
  /**
   * A reference to the user-defined type that contains this field.
   */
  const StructuredNode& parent() const;

 private:
  detail::DefinitionKeyRef parent_;
  FieldId id_;
  PresenceQualifier presence_;
  folly::not_null<const apache::thrift::type::Type*> type_;
  std::optional<apache::thrift::type::ValueId> customDefaultId_;

  FieldNode(
      const detail::Resolver& resolver,
      const apache::thrift::type::DefinitionKey& parent,
      FieldId id,
      PresenceQualifier presence,
      std::string_view name,
      const apache::thrift::type::Type& type,
      std::optional<apache::thrift::type::ValueId> customDefaultId)
      : detail::WithResolver(resolver),
        detail::WithName(name),
        parent_(parent),
        id_(id),
        presence_(presence),
        type_(&type),
        customDefaultId_(std::move(customDefaultId)) {}

  friend class detail::Resolver;
};

class StructuredNode : detail::WithDefinition, detail::WithUri {
 public:
  using detail::WithDefinition::definition;
  using detail::WithUri::uri;
  folly::span<const FieldNode> fields() const { return fields_; }

 protected:
  StructuredNode(
      const detail::Resolver& resolver,
      const apache::thrift::type::DefinitionKey& definitionKey,
      std::string_view uri,
      std::vector<FieldNode> fields)
      : detail::WithDefinition(resolver, definitionKey),
        detail::WithUri(uri),
        fields_(std::move(fields)) {}

 private:
  std::vector<FieldNode> fields_;
};

class StructNode final : folly::MoveOnly, public StructuredNode {
 public:
  /**
   * Produces a string representation of this Struct that is useful for
   * debugging ONLY.
   *
   * This string is not guaranteed to be stable, nor is it guaranteed to include
   * complete information.
   */
  std::string toDebugString() const;
  friend std::ostream& operator<<(std::ostream&, const StructNode&);

 private:
  using StructuredNode::StructuredNode;

  friend class detail::Resolver;
};

class UnionNode final : folly::MoveOnly, public StructuredNode {
 public:
  /**
   * Produces a string representation of this Union that is useful for
   * debugging ONLY.
   *
   * This string is not guaranteed to be stable, nor is it guaranteed to include
   * complete information.
   */
  std::string toDebugString() const;
  friend std::ostream& operator<<(std::ostream&, const UnionNode&);

 private:
  using StructuredNode::StructuredNode;

  friend class detail::Resolver;
};

class ExceptionNode final : folly::MoveOnly, public StructuredNode {
 public:
  /**
   * Produces a string representation of this Exception that is useful for
   * debugging ONLY.
   *
   * This string is not guaranteed to be stable, nor is it guaranteed to include
   * complete information.
   */
  std::string toDebugString() const;
  friend std::ostream& operator<<(std::ostream&, const ExceptionNode&);

 private:
  using StructuredNode::StructuredNode;

  friend class detail::Resolver;
};

class EnumNode final : folly::MoveOnly,
                       detail::WithDefinition,
                       detail::WithUri {
 public:
  /**
   * A mapping of enum name to its i32 value.
   */
  class Value : detail::WithName {
   public:
    Value(std::string_view name, std::int32_t i32)
        : WithName(name), i32_(i32) {}

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

  /**
   * Produces a string representation of this Enum that is useful for
   * debugging ONLY.
   *
   * This string is not guaranteed to be stable, nor is it guaranteed to include
   * complete information.
   */
  std::string toDebugString() const;
  friend std::ostream& operator<<(std::ostream&, const EnumNode&);

 private:
  std::vector<Value> values_;

  EnumNode(
      const detail::Resolver& resolver,
      const apache::thrift::type::DefinitionKey& definitionKey,
      std::string_view uri,
      std::vector<Value> values)
      : detail::WithDefinition(resolver, definitionKey),
        detail::WithUri(uri),
        values_(std::move(values)) {}

  friend class detail::Resolver;
};

class TypedefNode final : folly::MoveOnly, detail::WithDefinition {
 public:
  using detail::WithDefinition::definition;
  /**
   * A reference to the underlying type, which itself could be another typedef.
   */
  const TypeRef& targetType() const { return *targetType_; }

  /**
   * Produces a string representation of this `TypedefNode` that is useful for
   * debugging ONLY.
   *
   * This string is not guaranteed to be stable, nor is it guaranteed to include
   * complete information.
   */
  std::string toDebugString() const;
  friend std::ostream& operator<<(std::ostream&, const TypedefNode&);

 private:
  // Heap usage prevents mutual recursion with `TypeRef` and `DefinitionNode`.
  folly::not_null_unique_ptr<TypeRef> targetType_;

  TypedefNode(
      const detail::Resolver& resolver,
      const apache::thrift::type::DefinitionKey& definitionKey,
      TypeRef&& targetType);

  friend class detail::Resolver;
};

class ConstantNode final : folly::MoveOnly, detail::WithDefinition {
 public:
  using detail::WithDefinition::definition;
  const TypeRef& type() const { return *type_; }
  const apache::thrift::protocol::Value& value() const;

 private:
  // Heap usage prevents mutual recursion with `TypeRef` and `Definition`.
  folly::not_null_unique_ptr<TypeRef> type_;
  apache::thrift::type::ValueId valueId_;

  ConstantNode(
      const detail::Resolver& resolver,
      const apache::thrift::type::DefinitionKey& definitionKey,
      TypeRef&& type,
      apache::thrift::type::ValueId valueId);

  friend class detail::Resolver;
};

class List final {
 public:
  const TypeRef& elementType() const { return *elementType_; }

  ~List() noexcept = default;
  List(const List&);
  List& operator=(const List&);
  List(List&&) noexcept = default;
  List& operator=(List&&) noexcept = default;

  friend bool operator==(const List& lhs, const List& rhs);

  /**
   * Produces a string representation of the List type that is useful for
   * debugging ONLY.
   *
   * This string is not guaranteed to be stable, nor is it guaranteed to include
   * complete information.
   */
  std::string toDebugString() const;
  friend std::ostream& operator<<(std::ostream&, const List&);

  static List of(TypeRef elementType);

 private:
  // Heap usage prevents mutual recursion with `TypeRef`.
  folly::not_null_unique_ptr<TypeRef> elementType_;

  explicit List(TypeRef&& elementType);

  friend class detail::Resolver;
};

class Set final {
 public:
  const TypeRef& elementType() const { return *elementType_; }

  ~Set() noexcept = default;
  Set(const Set&);
  Set& operator=(const Set&);
  Set(Set&&) noexcept = default;
  Set& operator=(Set&&) noexcept = default;

  friend bool operator==(const Set& lhs, const Set& rhs);

  /**
   * Produces a string representation of the Set type that is useful for
   * debugging ONLY.
   *
   * This string is not guaranteed to be stable, nor is it guaranteed to include
   * complete information.
   */
  std::string toDebugString() const;
  friend std::ostream& operator<<(std::ostream&, const Set&);

  static Set of(TypeRef elementType);

 private:
  // Heap usage prevents mutual recursion with `TypeRef`.
  folly::not_null_unique_ptr<TypeRef> elementType_;

  explicit Set(TypeRef&& elementType);

  friend class detail::Resolver;
};

class Map final {
 public:
  const TypeRef& keyType() const { return *keyType_; }
  const TypeRef& valueType() const { return *valueType_; }

  ~Map() noexcept = default;
  Map(const Map&);
  Map& operator=(const Map&);
  Map(Map&&) noexcept = default;
  Map& operator=(Map&&) noexcept = default;

  friend bool operator==(const Map& lhs, const Map& rhs);

  /**
   * Produces a string representation of the Map type that is useful for
   * debugging ONLY.
   *
   * This string is not guaranteed to be stable, nor is it guaranteed to include
   * complete information.
   */
  std::string toDebugString() const;
  friend std::ostream& operator<<(std::ostream&, const Map&);

  static Map of(TypeRef keyType, TypeRef valueType);

 private:
  // Heap usage prevents mutual recursion with `TypeRef`.
  folly::not_null_unique_ptr<TypeRef> keyType_;
  folly::not_null_unique_ptr<TypeRef> valueType_;

  Map(TypeRef&& keyType, TypeRef&& valueType);

  friend class detail::Resolver;
};

class FunctionStream final : folly::MoveOnly {
 public:
  // A Thrift stream in IDL takes the form:
  //     stream<{payloadType} throws (... {exceptions} ...)>
  const TypeRef& payloadType() const { return *payloadType_; }
  folly::span<const TypeRef> exceptions() const;

 private:
  // Heap usage prevents mutual recursion with `DefinitionNode`.
  folly::not_null_unique_ptr<TypeRef> payloadType_;
  std::vector<TypeRef> exceptions_;

  FunctionStream(TypeRef&& payloadType, std::vector<TypeRef>&& exceptions);

  friend class detail::Resolver;
};

class FunctionSink final : folly::MoveOnly {
 public:
  // A Thrift sink in IDL takes the form:
  //     sink<{payloadType} throws (... {clientExceptions} ...),
  //          {finalResponseType} throws (... {serverExceptions} ...)>
  const TypeRef& payloadType() const { return *payloadType_; }
  const TypeRef& finalResponseType() const { return *finalResponseType_; }
  folly::span<const TypeRef> clientExceptions() const;
  folly::span<const TypeRef> serverExceptions() const;

 private:
  // Heap usage prevents mutual recursion with `DefinitionNode`.
  folly::not_null_unique_ptr<TypeRef> payloadType_;
  folly::not_null_unique_ptr<TypeRef> finalResponseType_;
  std::vector<TypeRef> clientExceptions_;
  std::vector<TypeRef> serverExceptions_;

  FunctionSink(
      TypeRef&& payloadType,
      TypeRef&& finalResponseType,
      std::vector<TypeRef>&& clientExceptions,
      std::vector<TypeRef>&& serverExceptions);

  friend class detail::Resolver;
};

class FunctionResponse final : folly::MoveOnly {
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

 private:
  using SinkOrStream =
      std::variant<std::monostate, FunctionSink, FunctionStream>;

  std::unique_ptr<TypeRef> type_;
  std::optional<detail::Lazy<InteractionNode>> interaction_;
  SinkOrStream sinkOrStream_;

  FunctionResponse(
      std::unique_ptr<TypeRef>&& type,
      std::optional<detail::Lazy<InteractionNode>>&& interaction,
      SinkOrStream&& sinkOrStream)
      : type_(std::move(type)),
        interaction_(std::move(interaction)),
        sinkOrStream_(std::move(sinkOrStream)) {}

  friend class detail::Resolver;
};

class FunctionParam final : folly::MoveOnly,
                            detail::WithResolver,
                            detail::WithName {
 public:
  using detail::WithName::name;
  FieldId id() const { return id_; }
  TypeRef type() const;

 private:
  FieldId id_;
  folly::not_null<const apache::thrift::type::Type*> type_;

  FunctionParam(
      const detail::Resolver& resolver,
      FieldId id,
      std::string_view name,
      const apache::thrift::type::Type& type)
      : detail::WithResolver(resolver),
        detail::WithName(name),
        id_(id),
        type_(&type) {}

  friend class detail::Resolver;
};

class FunctionNode final : folly::MoveOnly,
                           detail::WithResolver,
                           detail::WithName,
                           detail::WithAnnotations {
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

  const Response& response() const { return response_; }
  folly::span<const Param> params() const { return params_; }
  folly::span<const TypeRef> exceptions() const;

 private:
  detail::DefinitionKeyRef parent_;
  Response response_;
  std::vector<Param> params_;
  std::vector<TypeRef> exceptions_;

  FunctionNode(
      const detail::Resolver& resolver,
      const apache::thrift::type::DefinitionKey& parent,
      std::vector<Annotation>&& annotations,
      Response&& response,
      std::string_view name,
      std::vector<Param>&& params,
      std::vector<TypeRef>&& exceptions);

  friend class detail::Resolver;
};

class RpcInterfaceNode : detail::WithDefinition, detail::WithUri {
 public:
  using detail::WithDefinition::definition;
  using detail::WithUri::uri;
  folly::span<const FunctionNode> functions() const { return functions_; }

 protected:
  RpcInterfaceNode(
      const detail::Resolver& resolver,
      const apache::thrift::type::DefinitionKey& definitionKey,
      std::string_view uri,
      std::vector<FunctionNode>&& functions)
      : detail::WithDefinition(resolver, definitionKey),
        detail::WithUri(uri),
        functions_(std::move(functions)) {}

  using detail::WithDefinition::WithResolver::resolver;

 private:
  std::vector<FunctionNode> functions_;
};

class ServiceNode final : folly::MoveOnly, public RpcInterfaceNode {
 public:
  const ServiceNode* FOLLY_NULLABLE baseService() const;

 private:
  std::optional<detail::DefinitionKeyRef> baseServiceKey_;

  ServiceNode(
      const detail::Resolver& resolver,
      const apache::thrift::type::DefinitionKey& definitionKey,
      std::string_view uri,
      std::vector<FunctionNode>&& functions,
      std::optional<detail::DefinitionKeyRef> baseServiceKey)
      : RpcInterfaceNode(resolver, definitionKey, uri, std::move(functions)),
        baseServiceKey_(std::move(baseServiceKey)) {}

  friend class detail::Resolver;
};

class InteractionNode final : folly::MoveOnly, public RpcInterfaceNode {
 private:
  using RpcInterfaceNode::RpcInterfaceNode;

  friend class detail::Resolver;
};

class DefinitionNode final : folly::MoveOnly,
                             detail::WithResolver,
                             detail::WithName,
                             detail::WithAnnotations {
 private:
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

 public:
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

  /**
   * Produces a string representation of the definition that is useful for
   * debugging ONLY.
   *
   * This string is not guaranteed to be stable, nor is it guaranteed to include
   * complete information.
   */
  std::string toDebugString() const;
  friend std::ostream& operator<<(std::ostream&, const DefinitionNode&);

 private:
  apache::thrift::type::ProgramId programId_;
  Alternative definition_;

  DefinitionNode(
      const detail::Resolver& resolver,
      apache::thrift::type::ProgramId programId,
      std::vector<Annotation>&& annotations,
      std::string_view name,
      Alternative&& definition);

  friend class detail::Resolver;
};

class TypeRef final {
 private:
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

 public:
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
   * copy of `*this`.
   *
   * Post-conditions:
   *   - kind() != Kind::TYPEDEF
   */
  TypeRef trueType() const {
    switch (kind()) {
      case Kind::TYPEDEF:
        return asTypedef().targetType().trueType();
      default:
        return *this;
    }
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

  /**
   * Produces a string representation of the definition that is useful for
   * debugging ONLY.
   *
   * This string is not guaranteed to be stable, nor is it guaranteed to include
   * complete information.
   */
  std::string toDebugString() const;
  friend std::ostream& operator<<(std::ostream&, const TypeRef&);

  static TypeRef of(Primitive);
  static TypeRef of(const StructNode&);
  static TypeRef of(const UnionNode&);
  static TypeRef of(const ExceptionNode&);
  static TypeRef of(const EnumNode&);
  static TypeRef of(const List&);
  static TypeRef of(const Set&);
  static TypeRef of(const Map&);

 private:
  Alternative type_;

  explicit TypeRef(Alternative&& type) : type_(std::move(type)) {}

  friend class detail::Resolver;
};
// `TypeRef` is a value type and should behave like one.
static_assert(std::is_copy_constructible_v<TypeRef>);
static_assert(std::is_move_constructible_v<TypeRef>);
static_assert(std::is_copy_assignable_v<TypeRef>);
static_assert(std::is_move_assignable_v<TypeRef>);

class Annotation final : folly::MoveOnly {
 public:
  const TypeRef& type() const { return *type_; }

  using Fields =
      folly::F14FastMap<std::string_view, apache::thrift::protocol::Value>;
  const Fields& fields() const { return fields_; }

 private:
  folly::not_null_unique_ptr<TypeRef> type_;
  Fields fields_;

  Annotation(TypeRef&& type, Fields&& fields);

  friend class detail::Resolver;
};

class ProgramNode final : folly::MoveOnly,
                          detail::WithResolver,
                          detail::WithName {
 public:
  using detail::WithName::name;

  std::string_view path() const { return path_; }

  using IncludesList = std::vector<folly::not_null<const ProgramNode*>>;
  IncludesList includes() const;

  using DefinitionsByName = folly::
      F14FastMap<std::string_view, folly::not_null<const DefinitionNode*>>;
  DefinitionsByName definitions() const;
  const SyntaxGraph& syntaxGraph() const;

 private:
  using DefinitionKeysByName =
      folly::F14FastMap<std::string_view, detail::DefinitionKeyRef>;

  std::string_view path_;
  std::vector<apache::thrift::type::ProgramId> includes_;
  DefinitionKeysByName definitionKeysByName_;

  ProgramNode(
      const detail::Resolver& resolver,
      std::string_view path,
      std::string_view name,
      std::vector<apache::thrift::type::ProgramId> includes,
      DefinitionKeysByName definitionKeysByName)
      : detail::WithResolver(resolver),
        detail::WithName(name),
        path_(path),
        includes_(std::move(includes)),
        definitionKeysByName_(std::move(definitionKeysByName)) {}

  friend class detail::Resolver;
};

class SyntaxGraph final {
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

 private:
  using ManagedSchema = detail::MaybeManaged<apache::thrift::type::Schema>;
  ManagedSchema rawSchema_;
  folly::not_null_unique_ptr<const detail::Resolver> resolver_;

  explicit SyntaxGraph(ManagedSchema&&);

  friend class detail::Resolver;
};
static_assert(std::is_move_constructible_v<SyntaxGraph>);
static_assert(std::is_move_assignable_v<SyntaxGraph>);

namespace detail {
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

} // namespace apache::thrift::schema

#endif // THRIFT_SCHEMA_AVAILABLE
