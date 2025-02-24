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

#include <thrift/lib/cpp2/schema/SyntaxGraph.h>

#include <thrift/lib/cpp/util/EnumUtils.h>

#include <folly/MapUtil.h>
#include <folly/container/Array.h>
#include <folly/lang/SafeAssert.h>

#include <fmt/core.h>

#include <ostream>
#include <stdexcept>

#ifdef THRIFT_SCHEMA_AVAILABLE

namespace type = apache::thrift::type;
namespace protocol = apache::thrift::protocol;
using apache::thrift::util::enumNameSafe;

namespace apache::thrift::schema {

namespace detail {

namespace {

/**
 * Transparent hashing for DefinitionKey and DefinitionKeyRef. This enables
 * heterogenous access in F14Map / F14Set.
 */
class DefinitionKeyHash : public std::hash<type::DefinitionKey> {
 private:
  using Delegate = std::hash<type::DefinitionKey>;

 public:
  using is_transparent = void;

  using Delegate::operator();
  std::size_t operator()(const DefinitionKeyRef& other) const {
    return Delegate::operator()(other.get());
  }
};

/**
 * Transparent equality comparison for DefinitionKey and DefinitionKeyRef. This
 * enables heterogenous access in F14Map / F14Set.
 */
class DefinitionKeyEqual : public std::equal_to<type::DefinitionKey> {
 private:
  using Delegate = std::equal_to<type::DefinitionKey>;

 public:
  using is_transparent = void;

  using Delegate::operator();
  bool operator()(
      const DefinitionKeyRef& lhs, const DefinitionKeyRef& rhs) const {
    return Delegate::operator()(lhs.get(), rhs.get());
  }
  bool operator()(
      const type::DefinitionKey& lhs, const DefinitionKeyRef& rhs) const {
    return Delegate::operator()(lhs, rhs.get());
  }
  bool operator()(
      const DefinitionKeyRef& lhs, const type::DefinitionKey& rhs) const {
    return Delegate::operator()(lhs.get(), rhs);
  }
};

} // namespace

class Resolver final {
 private:
  friend class ::apache::thrift::schema::SyntaxGraph;
  friend const DefinitionNode& lazyResolve(
      const Resolver&, const type::DefinitionKey&);

  // Lifetime managed by SyntaxGraph
  folly::not_null<const type::Schema*> rawSchema_;
  folly::not_null<SyntaxGraph*> syntaxGraph_;

  // Every top-level definition has exactly one associated graph node. These are
  // stored and kept alive in this map.
  using DefinitionsByKey = folly::F14FastMap<
      DefinitionKeyRef,
      DefinitionNode,
      DefinitionKeyHash,
      DefinitionKeyEqual>;
  DefinitionsByKey definitionsByKey_;

  // Every top-level definition is defined in a .thrift file. This map allows
  // back-references from these definition graph nodes to have efficient lookup
  // of their containing file.
  using ProgramsById = folly::F14FastMap<type::ProgramId, ProgramNode>;
  ProgramsById programsById_;

  // A mapping of value IDs to a runtime representation of their value. This
  // matches how the data is stored in schema.thrift and de-duplicated.
  using ValuesById = folly::F14FastMap<type::ValueId, protocol::Value>;
  ValuesById valuesById_;

  // A mapping of a definition to its containing Thrift file. Chaining this with
  // the ProgramsById map produces a mapping from DefinitionKey → Program graph
  // node.
  using ProgramIdsByDefinitionKey = folly::F14FastMap<
      DefinitionKeyRef,
      type::ProgramId,
      DefinitionKeyHash,
      DefinitionKeyEqual>;

  ProgramsById createProgramsById(const type::Schema&, const DefinitionsByKey&);
  ValuesById createValuesById(const type::Schema& schema);

  static ProgramIdsByDefinitionKey createProgramIdsByDefinitionKey(
      const type::Schema&);
  DefinitionsByKey createDefinitionsByKey(
      const type::Schema&, const ProgramIdsByDefinitionKey&);

  DefinitionNode createDefinition(
      const ProgramIdsByDefinitionKey&,
      const type::DefinitionKey&,
      const type::DefinitionAttrs&,
      DefinitionNode::Alternative&&);
  StructNode createStruct(const type::DefinitionKey&, const type::Struct&);
  UnionNode createUnion(const type::DefinitionKey&, const type::Union&);
  ExceptionNode createException(
      const type::DefinitionKey&, const type::Exception&);
  FieldNode createField(
      const type::DefinitionKey& parentDefinitionKey, const type::Field&);

  EnumNode createEnum(const type::DefinitionKey&, const type::Enum&);
  TypedefNode createTypedef(const type::DefinitionKey&, const type::Typedef&);
  ConstantNode createConstant(const type::DefinitionKey&, const type::Const&);

  ServiceNode createService(const type::DefinitionKey&, const type::Service&);
  InteractionNode createInteraction(
      const type::DefinitionKey&, const type::Interaction&);
  FunctionNode createFunction(
      const type::DefinitionKey&, const type::Function&);

  std::vector<Annotation> createAnnotations(
      const std::map<type::DefinitionKey, type::Annotation>& annotations);

 public:
  explicit Resolver(const type::Schema& schema, SyntaxGraph& syntaxGraph)
      : rawSchema_(&schema),
        syntaxGraph_(&syntaxGraph),
        definitionsByKey_(createDefinitionsByKey(
            schema, createProgramIdsByDefinitionKey(schema))),
        programsById_(createProgramsById(schema, definitionsByKey_)),
        valuesById_(createValuesById(schema)) {}

  const SyntaxGraph& syntaxGraph() const { return *syntaxGraph_; }
  /**
   * Programs are identified by a separate namespace of IDs than definitions.
   */
  const ProgramNode& programOf(const type::ProgramId&) const;
  /**
   * All graph nodes representing definitions are identified by their
   * `DefinitionKey` rather than URI. However, schema.thrift can sometimes refer
   * to definitions using URIs. In those cases, we may need to "normalize" the
   * reference.
   *
   * Note that `TypeUri` does not mean the reference is a URI. The naming is
   * confusing but the `TypeUri` is a union that can possibly contain a URI, or
   * a `DefinitionKey`.
   */
  const type::DefinitionKey& definitionKeyOf(const type::TypeUri&) const;
  /**
   * Creates a `TypeRef` object, which is really a non-owning and unresolved
   * reference to a Thrift type.
   *
   * This function does not resolve composite types like typedefs, list etc.
   * (i.e. types that refer to other types). Non-composite types, such as struct
   * definitions are lazily resolved. This means that it's safe to call
   * typeOf(...) even if the pointed-to type has not been seen yet.
   */
  TypeRef typeOf(const type::TypeStruct&) const;
  TypeRef typeOf(const type::Type& type) const {
    return typeOf(type.toThrift());
  }
  /**
   * schema.thrift de-duplicates Thrift values in the IDL via interning. This
   * function performs a lookup to access an interned value by its ID.
   */
  const protocol::Value& valueOf(const type::ValueId&) const;
};

} // namespace detail

TypeRef FieldNode::type() const {
  return resolver().typeOf(*type_);
}

const protocol::Value* FOLLY_NULLABLE FieldNode::customDefault() const {
  if (customDefaultId_.has_value()) {
    return &resolver().valueOf(*customDefaultId_);
  }
  return nullptr;
}

const StructuredNode& FieldNode::parent() const {
  return detail::lazyResolve(resolver(), parent_).asStructured();
}

std::string StructNode::toDebugString() const {
  return fmt::format(
      "Struct(uri='{}', {})", uri(), definition().toDebugString());
}
std::ostream& operator<<(std::ostream& out, const StructNode& s) {
  return out << s.toDebugString();
}

std::string UnionNode::toDebugString() const {
  return fmt::format(
      "Union(uri='{}', {})", uri(), definition().toDebugString());
}
std::ostream& operator<<(std::ostream& out, const UnionNode& u) {
  return out << u.toDebugString();
}

std::string ExceptionNode::toDebugString() const {
  return fmt::format(
      "Exception(uri='{}', {})", uri(), definition().toDebugString());
}
std::ostream& operator<<(std::ostream& out, const ExceptionNode& e) {
  return out << e.toDebugString();
}

std::string EnumNode::toDebugString() const {
  return fmt::format("Enum(uri='{}', {})", uri(), definition().toDebugString());
}
std::ostream& operator<<(std::ostream& out, const EnumNode& e) {
  return out << e.toDebugString();
}

TypedefNode::TypedefNode(
    const detail::Resolver& resolver,
    const type::DefinitionKey& definitionKey,
    TypeRef&& targetType)
    : detail::WithDefinition(resolver, definitionKey),
      targetType_(folly::copy_to_unique_ptr(std::move(targetType))) {}

std::string TypedefNode::toDebugString() const {
  return fmt::format(
      "Typedef(to={}, {})",
      targetType().toDebugString(),
      definition().toDebugString());
}
std::ostream& operator<<(std::ostream& out, const TypedefNode& t) {
  return out << t.toDebugString();
}

ConstantNode::ConstantNode(
    const detail::Resolver& resolver,
    const type::DefinitionKey& definitionKey,
    TypeRef&& type,
    type::ValueId valueId)
    : detail::WithDefinition(resolver, definitionKey),
      type_(folly::copy_to_unique_ptr(std::move(type))),
      valueId_(valueId) {}

const apache::thrift::protocol::Value& ConstantNode::value() const {
  return resolver().valueOf(valueId_);
}

List::List(TypeRef&& elementType)
    : elementType_(folly::copy_to_unique_ptr(std::move(elementType))) {}
List::List(const List& other)
    : elementType_(folly::copy_to_unique_ptr(other.elementType())) {}
List& List::operator=(const List& other) {
  elementType_ = folly::copy_to_unique_ptr(other.elementType());
  return *this;
}

bool operator==(const List& lhs, const List& rhs) {
  return lhs.elementType() == rhs.elementType();
}

std::string List::toDebugString() const {
  return fmt::format("List(of={})", elementType().toDebugString());
}
std::ostream& operator<<(std::ostream& out, const List& l) {
  return out << l.toDebugString();
}

/* static */ List List::of(TypeRef elementType) {
  return List(std::move(elementType));
}

Set::Set(TypeRef&& elementType)
    : elementType_(folly::copy_to_unique_ptr(std::move(elementType))) {}
Set::Set(const Set& other)
    : elementType_(folly::copy_to_unique_ptr(other.elementType())) {}
Set& Set::operator=(const Set& other) {
  elementType_ = folly::copy_to_unique_ptr(other.elementType());
  return *this;
}

bool operator==(const Set& lhs, const Set& rhs) {
  return lhs.elementType() == rhs.elementType();
}

std::string Set::toDebugString() const {
  return fmt::format("Set(of={})", elementType().toDebugString());
}
std::ostream& operator<<(std::ostream& out, const Set& s) {
  return out << s.toDebugString();
}

/* static */ Set Set::of(TypeRef elementType) {
  return Set(std::move(elementType));
}

Map::Map(TypeRef&& keyType, TypeRef&& valueType)
    : keyType_(folly::copy_to_unique_ptr(std::move(keyType))),
      valueType_(folly::copy_to_unique_ptr(std::move(valueType))) {}
Map::Map(const Map& other)
    : keyType_(folly::copy_to_unique_ptr(other.keyType())),
      valueType_(folly::copy_to_unique_ptr(other.valueType())) {}
Map& Map::operator=(const Map& other) {
  keyType_ = folly::copy_to_unique_ptr(other.keyType());
  valueType_ = folly::copy_to_unique_ptr(other.valueType());
  return *this;
}

bool operator==(const Map& lhs, const Map& rhs) {
  return std::tie(lhs.keyType(), lhs.valueType()) ==
      std::tie(rhs.keyType(), rhs.valueType());
}

std::string Map::toDebugString() const {
  return fmt::format(
      "Map(of={}, to={})",
      keyType().toDebugString(),
      valueType().toDebugString());
}
std::ostream& operator<<(std::ostream& out, const Map& m) {
  return out << m.toDebugString();
}

/* static */ Map Map::of(TypeRef keyType, TypeRef valueType) {
  return Map(std::move(keyType), std::move(valueType));
}

std::string_view toString(Primitive p) {
  switch (p) {
    case Primitive::BOOL:
      return "BOOL";
    case Primitive::BYTE:
      return "BYTE";
    case Primitive::I16:
      return "I16";
    case Primitive::I32:
      return "I32";
    case Primitive::I64:
      return "I64";
    case Primitive::FLOAT:
      return "FLOAT";
    case Primitive::DOUBLE:
      return "DOUBLE";
    case Primitive::STRING:
      return "STRING";
    case Primitive::BINARY:
      return "BINARY";
    default:
      folly::throw_exception<std::logic_error>(
          fmt::format("Unknown Primitive value '{}'", p));
  };
}

FunctionStream::FunctionStream(
    TypeRef&& payloadType, std::vector<TypeRef>&& exceptions)
    : payloadType_(folly::copy_to_unique_ptr(std::move(payloadType))),
      exceptions_(std::move(exceptions)) {}

folly::span<const TypeRef> FunctionStream::exceptions() const {
  return exceptions_;
}

FunctionSink::FunctionSink(
    TypeRef&& payloadType,
    TypeRef&& finalResponseType,
    std::vector<TypeRef>&& clientExceptions,
    std::vector<TypeRef>&& serverExceptions)
    : payloadType_(folly::copy_to_unique_ptr(std::move(payloadType))),
      finalResponseType_(
          folly::copy_to_unique_ptr(std::move(finalResponseType))),
      clientExceptions_(std::move(clientExceptions)),
      serverExceptions_(std::move(serverExceptions)) {}

folly::span<const TypeRef> FunctionSink::clientExceptions() const {
  return clientExceptions_;
}

folly::span<const TypeRef> FunctionSink::serverExceptions() const {
  return serverExceptions_;
}

TypeRef FunctionParam::type() const {
  return resolver().typeOf(*type_);
}

FunctionNode::FunctionNode(
    const detail::Resolver& resolver,
    const apache::thrift::type::DefinitionKey& parent,
    std::vector<Annotation>&& annotations,
    Response&& response,
    std::string_view name,
    std::vector<Param>&& params,
    std::vector<TypeRef>&& exceptions)
    : detail::WithResolver(resolver),
      detail::WithName(name),
      detail::WithAnnotations(std::move(annotations)),
      parent_(parent),
      response_(std::move(response)),
      params_(std::move(params)),
      exceptions_(std::move(exceptions)) {}

const RpcInterfaceNode& FunctionNode::parent() const {
  return detail::lazyResolve(resolver(), parent_).asRpcInterface();
}

folly::span<const TypeRef> FunctionNode::exceptions() const {
  return exceptions_;
}

const ServiceNode* FOLLY_NULLABLE ServiceNode::baseService() const {
  return baseServiceKey_.has_value()
      ? &detail::lazyResolve(resolver(), *baseServiceKey_).asService()
      : nullptr;
}

DefinitionNode::DefinitionNode(
    const detail::Resolver& resolver,
    apache::thrift::type::ProgramId programId,
    std::vector<Annotation>&& annotations,
    std::string_view name,
    Alternative&& definition)
    : detail::WithResolver(resolver),
      detail::WithName(name),
      detail::WithAnnotations(std::move(annotations)),
      programId_(programId),
      definition_(std::move(definition)) {}

const ProgramNode& DefinitionNode::program() const {
  return resolver().programOf(programId_);
}

std::string DefinitionNode::toDebugString() const {
  std::string_view kindString = visit(
      [](const StructNode&) { return "Struct"; },
      [](const UnionNode&) { return "Union"; },
      [](const ExceptionNode&) { return "Exception"; },
      [](const EnumNode&) { return "Enum"; },
      [](const TypedefNode&) { return "Typedef"; },
      [](const ConstantNode&) { return "Constant"; },
      [](const ServiceNode&) { return "Service"; },
      [](const InteractionNode&) { return "Interaction"; });
  return fmt::format(
      "Definition(kind={}, name='{}', program='{}.thrift')",
      kindString,
      name(),
      program().name());
}
std::ostream& operator<<(std::ostream& out, const DefinitionNode& definition) {
  return out << definition.toDebugString();
}

/* static */ TypeRef TypeRef::of(Primitive p) {
  return TypeRef(p);
}
/* static */ TypeRef TypeRef::of(const StructNode& s) {
  return TypeRef(detail::Lazy<StructNode>::Resolved(s));
}
/* static */ TypeRef TypeRef::of(const UnionNode& u) {
  return TypeRef(detail::Lazy<UnionNode>::Resolved(u));
}
/* static */ TypeRef TypeRef::of(const ExceptionNode& e) {
  return TypeRef(detail::Lazy<ExceptionNode>::Resolved(e));
}
/* static */ TypeRef TypeRef::of(const EnumNode& e) {
  return TypeRef(detail::Lazy<EnumNode>::Resolved(e));
}
/* static */ TypeRef TypeRef::of(const List& list) {
  return TypeRef(list);
}
/* static */ TypeRef TypeRef::of(const Set& set) {
  return TypeRef(set);
}
/* static */ TypeRef TypeRef::of(const Map& map) {
  return TypeRef(map);
}

bool operator==(const TypeRef& lhs, const TypeRef& rhs) {
  if (lhs.kind() != rhs.kind()) {
    return false;
  }
  return lhs.visit(
      [&](Primitive p) -> bool { return rhs.asPrimitive() == p; },
      [&](const StructNode& s) -> bool { return &rhs.asStruct() == &s; },
      [&](const UnionNode& u) -> bool { return &rhs.asUnion() == &u; },
      [&](const ExceptionNode& e) -> bool { return &rhs.asException() == &e; },
      [&](const EnumNode& e) -> bool { return &rhs.asEnum() == &e; },
      [&](const TypedefNode& t) -> bool { return &rhs.asTypedef() == &t; },
      [&](const List& l) -> bool { return rhs.asList() == l; },
      [&](const Set& s) -> bool { return rhs.asSet() == s; },
      [&](const Map& m) -> bool { return rhs.asMap() == m; });
}

bool operator==(const TypeRef& lhs, const DefinitionNode& rhs) {
  return lhs.visit(
      [&](const StructNode& s) -> bool {
        return rhs.isStruct() && &rhs.asStruct() == &s;
      },
      [&](const UnionNode& u) -> bool {
        return rhs.isUnion() && &rhs.asUnion() == &u;
      },
      [&](const ExceptionNode& e) -> bool {
        return rhs.isException() && &rhs.asException() == &e;
      },
      [&](const EnumNode& e) -> bool {
        return rhs.isEnum() && &rhs.asEnum() == &e;
      },
      [&](const TypedefNode& t) -> bool {
        return rhs.isTypedef() && &rhs.asTypedef() == &t;
      },
      [&](auto&&) -> bool {
        // All other forms are non-type definitions.
        return false;
      });
}

std::string TypeRef::toDebugString() const {
  return visit(
      [&](Primitive p) -> std::string { return std::string(toString(p)); },
      [&](const auto& t) -> std::string { return t.toDebugString(); });
}
std::ostream& operator<<(std::ostream& out, const TypeRef& type) {
  return out << type.toDebugString();
}

Annotation::Annotation(TypeRef&& type, Fields&& fields)
    : type_(folly::copy_to_unique_ptr(std::move(type))),
      fields_(std::move(fields)) {}

ProgramNode::IncludesList ProgramNode::includes() const {
  IncludesList includes;
  for (const type::ProgramId& include : includes_) {
    includes.emplace_back(&resolver().programOf(include));
  }
  return includes;
}

ProgramNode::DefinitionsByName ProgramNode::definitions() const {
  DefinitionsByName result;
  for (const auto& [name, definitionKey] : definitionKeysByName_) {
    result.emplace(name, &detail::lazyResolve(resolver(), definitionKey));
  }
  return result;
}

const SyntaxGraph& ProgramNode::syntaxGraph() const {
  return resolver().syntaxGraph();
}

/* static */ SyntaxGraph SyntaxGraph::fromSchema(
    folly::not_null<const type::Schema*> schema) {
  return SyntaxGraph{ManagedSchema(*schema)};
}

/* static */ SyntaxGraph SyntaxGraph::fromSchema(type::Schema&& schema) {
  return SyntaxGraph{ManagedSchema(std::move(schema))};
}

SyntaxGraph::SyntaxGraph(ManagedSchema&& schema)
    : rawSchema_(std::move(schema)),
      resolver_(
          folly::make_not_null_unique<detail::Resolver>(*rawSchema_, *this)) {}

SyntaxGraph::SyntaxGraph(SyntaxGraph&&) noexcept = default;
SyntaxGraph& SyntaxGraph::operator=(SyntaxGraph&&) noexcept = default;
SyntaxGraph::~SyntaxGraph() noexcept = default;

ProgramNode::IncludesList SyntaxGraph::programs() const {
  ProgramNode::IncludesList programs;
  for (const auto& [_, program] : resolver_->programsById_) {
    programs.emplace_back(&program);
  }
  return programs;
}

namespace detail {

WithAnnotations::WithAnnotations(std::vector<Annotation>&& annotations)
    : annotations_(std::move(annotations)) {}

folly::span<const Annotation> WithAnnotations::annotations() const {
  return annotations_;
}

namespace {

template <typename... F>
decltype(auto) visitDefinition(
    const type::Definition& definition, F&&... visitors) {
  auto overloaded = folly::overload(std::forward<F>(visitors)...);
  switch (definition.getType()) {
    case type::Definition::Type::structDef:
      return overloaded(*definition.structDef_ref());
    case type::Definition::Type::unionDef:
      return overloaded(*definition.unionDef_ref());
    case type::Definition::Type::exceptionDef:
      return overloaded(*definition.exceptionDef_ref());
    case type::Definition::Type::enumDef:
      return overloaded(*definition.enumDef_ref());
    case type::Definition::Type::typedefDef:
      return overloaded(*definition.typedefDef_ref());
    case type::Definition::Type::constDef:
      return overloaded(*definition.constDef_ref());
    case type::Definition::Type::serviceDef:
      return overloaded(*definition.serviceDef_ref());
    case type::Definition::Type::interactionDef:
      return overloaded(*definition.interactionDef_ref());
    case type::Definition::Type::__EMPTY__:
    default:
      folly::throw_exception<InvalidSyntaxGraphError>(fmt::format(
          "Unknown Definition::Type '{}'", enumNameSafe(definition.getType())));
  }
}

FieldNode::PresenceQualifier presenceOf(const type::FieldQualifier& qualifier) {
  switch (qualifier) {
    case type::FieldQualifier::Default:
    case type::FieldQualifier::Terse:
    case type::FieldQualifier::Fill:
      return FieldNode::PresenceQualifier::UNQUALIFIED;
    case type::FieldQualifier::Optional:
      return FieldNode::PresenceQualifier::OPTIONAL;
    default:
      folly::throw_exception<InvalidSyntaxGraphError>(
          fmt::format("Unknown FieldQualifier '{}'", enumNameSafe(qualifier)));
  }
}

std::optional<type::ValueId> valueIdOf(const type::ValueId& valueIdField) {
  // schema.thrift leaves behind a value of 0 for fields without custom
  // defaults.
  return valueIdField == type::ValueId{0} ? std::nullopt
                                          : std::optional{valueIdField};
}

bool isEmptyTypeUri(const type::TypeUri& typeUri) {
  return typeUri.getType() == type::TypeUri::Type::__EMPTY__;
}

} // namespace

const DefinitionNode& lazyResolve(
    const Resolver& resolver, const type::DefinitionKey& definitionKey) {
  return folly::get_or_throw<InvalidSyntaxGraphError>(
      resolver.definitionsByKey_, definitionKey);
}

const ProgramNode& Resolver::programOf(const type::ProgramId& id) const {
  return folly::get_or_throw<InvalidSyntaxGraphError>(
      programsById_, id, "Unknown ProgramId: ");
}

const type::DefinitionKey& Resolver::definitionKeyOf(
    const type::TypeUri& typeUri) const {
  using T = type::TypeUri::Type;
  switch (typeUri.getType()) {
    case T::definitionKey:
      return *typeUri.definitionKey_ref();
    case T::uri:
    case T::typeHashPrefixSha2_256:
    case T::scopedName:
    case T::__EMPTY__:
    default:
      folly::throw_exception<InvalidSyntaxGraphError>(fmt::format(
          "Unsupported TypeUri::Type '{}'", enumNameSafe(typeUri.getType())));
  }
}

TypeRef Resolver::typeOf(const type::TypeStruct& type) const {
  return TypeRef([&]() -> TypeRef::Alternative {
    using T = type::TypeName::Type;
    T t = type.name()->getType();
    switch (t) {
      case T::boolType:
        return Primitive::BOOL;
      case T::byteType:
        return Primitive::BYTE;
      case T::i16Type:
        return Primitive::I16;
      case T::i32Type:
        return Primitive::I32;
      case T::i64Type:
        return Primitive::I64;
      case T::floatType:
        return Primitive::FLOAT;
      case T::doubleType:
        return Primitive::DOUBLE;
      case T::stringType:
        return Primitive::STRING;
      case T::binaryType:
        return Primitive::BINARY;
      case T::enumType:
        return detail::Lazy<EnumNode>::Unresolved(
            *this, definitionKeyOf(*type.name()->enumType_ref()));
      case T::typedefType:
        return detail::Lazy<TypedefNode>::Unresolved(
            *this, definitionKeyOf(*type.name()->typedefType_ref()));
      case T::structType:
        return detail::Lazy<StructNode>::Unresolved(
            *this, definitionKeyOf(*type.name()->structType_ref()));
      case T::unionType:
        return detail::Lazy<UnionNode>::Unresolved(
            *this, definitionKeyOf(*type.name()->unionType_ref()));
      case T::exceptionType:
        return detail::Lazy<ExceptionNode>::Unresolved(
            *this, definitionKeyOf(*type.name()->exceptionType_ref()));
      case T::listType: {
        const auto& params = *type.params();
        if (params.size() != 1) {
          folly::throw_exception<InvalidSyntaxGraphError>(fmt::format(
              "Invalid number of type params for list: {}", params.size()));
        }
        return List(typeOf(params.front()));
      }
      case T::setType: {
        const auto& params = *type.params();
        if (params.size() != 1) {
          folly::throw_exception<InvalidSyntaxGraphError>(fmt::format(
              "Invalid number of type params for set: {}", params.size()));
        }
        return Set(typeOf(params.front()));
      }
      case T::mapType: {
        const auto& params = *type.params();
        if (params.size() != 2) {
          folly::throw_exception<InvalidSyntaxGraphError>(fmt::format(
              "Invalid number of type params for map: {}", params.size()));
        }
        return Map(typeOf(params[0]), typeOf(params[1]));
      }
      default:
        folly::throw_exception<InvalidSyntaxGraphError>(
            fmt::format("Unknown TypeName '{}'", enumNameSafe(t)));
    }
  }());
}

const protocol::Value& Resolver::valueOf(const type::ValueId& id) const {
  return folly::get_or_throw<InvalidSyntaxGraphError>(
      valuesById_, id, "Unknown ValueId: ");
}

Resolver::ProgramsById Resolver::createProgramsById(
    const type::Schema& schema, const DefinitionsByKey& definitionsByKey) {
  ProgramsById result;
  for (const type::Program& program : *schema.programs()) {
    ProgramNode::DefinitionKeysByName definitionKeysByName;
    for (const type::DefinitionKey& definitionKey : *program.definitionKeys()) {
      if (const DefinitionNode* definition =
              folly::get_ptr(definitionsByKey, definitionKey)) {
        definitionKeysByName.emplace(definition->name(), definitionKey);
      }
    }
    result.emplace(
        *program.id(),
        ProgramNode(
            *this,
            *program.path(),
            *program.name(),
            *program.includes(),
            std::move(definitionKeysByName)));
  }
  return result;
}

Resolver::ValuesById Resolver::createValuesById(const type::Schema& schema) {
  ValuesById result;
  for (const auto& [valueId, value] : *schema.valuesMap()) {
    result.emplace(valueId, value);
  }
  return result;
}

/* static */ Resolver::ProgramIdsByDefinitionKey
Resolver::createProgramIdsByDefinitionKey(const type::Schema& schema) {
  ProgramIdsByDefinitionKey result;
  for (const type::Program& program : *schema.programs()) {
    for (const type::DefinitionKey& definitionKey : *program.definitionKeys()) {
      result.emplace(definitionKey, *program.id());
    }
  }
  return result;
}

Resolver::DefinitionsByKey Resolver::createDefinitionsByKey(
    const type::Schema& schema,
    const Resolver::ProgramIdsByDefinitionKey& programIdsByDefinitionKey) {
  DefinitionsByKey result;
  for (const auto& entry : *schema.definitionsMap()) {
    const type::DefinitionKey& definitionKey = entry.first;
    const type::Definition& definition = entry.second;
    const auto& definitionAttrs = visitDefinition(
        definition, [](auto&& def) -> const type::DefinitionAttrs& {
          return *def.attrs();
        });
    auto alternative = visitDefinition(
        definition,
        [&](const type::Struct& structDef) -> DefinitionNode::Alternative {
          return createStruct(definitionKey, structDef);
        },
        [&](const type::Union& unionDef) -> DefinitionNode::Alternative {
          return createUnion(definitionKey, unionDef);
        },
        [&](const type::Exception& exceptionDef)
            -> DefinitionNode::Alternative {
          return createException(definitionKey, exceptionDef);
        },
        [&](const type::Enum& enumDef) -> DefinitionNode::Alternative {
          return createEnum(definitionKey, enumDef);
        },
        [&](const type::Typedef& typedefDef) -> DefinitionNode::Alternative {
          return createTypedef(definitionKey, typedefDef);
        },
        [&](const type::Const& constDef) -> DefinitionNode::Alternative {
          return createConstant(definitionKey, constDef);
        },
        [&](const type::Service& serviceDef) -> DefinitionNode::Alternative {
          return createService(definitionKey, serviceDef);
        },
        [&](const type::Interaction& interactionDef)
            -> DefinitionNode::Alternative {
          return createInteraction(definitionKey, interactionDef);
        });
    result.emplace(
        definitionKey,
        createDefinition(
            programIdsByDefinitionKey,
            definitionKey,
            definitionAttrs,
            std::move(alternative)));
  }
  return result;
}

DefinitionNode Resolver::createDefinition(
    const Resolver::ProgramIdsByDefinitionKey& programIdsByDefinitionKey,
    const type::DefinitionKey& definitionKey,
    const type::DefinitionAttrs& attrs,
    DefinitionNode::Alternative&& alternative) {
  return DefinitionNode(
      *this,
      folly::get_or_throw<InvalidSyntaxGraphError>(
          programIdsByDefinitionKey,
          definitionKey,
          "Unknown ProgramId for DefinitionKey: "),
      createAnnotations(*attrs.annotationsByKey()),
      *attrs.name(),
      std::move(alternative));
}

StructNode Resolver::createStruct(
    const type::DefinitionKey& definitionKey, const type::Struct& structDef) {
  std::vector<FieldNode> fields;
  for (const type::Field& field : *structDef.fields()) {
    fields.emplace_back(createField(definitionKey, field));
  }
  return StructNode(*this, definitionKey, *structDef.uri(), std::move(fields));
}

UnionNode Resolver::createUnion(
    const type::DefinitionKey& definitionKey, const type::Union& unionDef) {
  std::vector<FieldNode> fields;
  for (const type::Field& field : *unionDef.fields()) {
    fields.emplace_back(createField(definitionKey, field));
  }
  return UnionNode(*this, definitionKey, *unionDef.uri(), std::move(fields));
}

ExceptionNode Resolver::createException(
    const type::DefinitionKey& definitionKey,
    const type::Exception& exceptionDef) {
  std::vector<FieldNode> fields;
  for (const type::Field& field : *exceptionDef.fields()) {
    fields.emplace_back(createField(definitionKey, field));
  }
  return ExceptionNode(
      *this, definitionKey, *exceptionDef.uri(), std::move(fields));
}

FieldNode Resolver::createField(
    const type::DefinitionKey& parentDefinitionKey, const type::Field& field) {
  return FieldNode(
      *this,
      parentDefinitionKey,
      *field.id(),
      presenceOf(*field.qualifier()),
      *field.name(),
      *field.type(),
      valueIdOf(*field.customDefault()));
}

EnumNode Resolver::createEnum(
    const type::DefinitionKey& definitionKey, const type::Enum& enumDef) {
  std::vector<EnumNode::Value> values;
  for (const type::EnumValue& enumValue : *enumDef.values()) {
    values.emplace_back(EnumNode::Value(*enumValue.name(), *enumValue.value()));
  }
  return EnumNode(*this, definitionKey, *enumDef.uri(), std::move(values));
}

TypedefNode Resolver::createTypedef(
    const type::DefinitionKey& definitionKey, const type::Typedef& typedefDef) {
  return TypedefNode(*this, definitionKey, typeOf(*typedefDef.type()));
}

ConstantNode Resolver::createConstant(
    const type::DefinitionKey& definitionKey, const type::Const& constDef) {
  return ConstantNode(
      *this, definitionKey, typeOf(*constDef.type()), *constDef.value());
}

ServiceNode Resolver::createService(
    const type::DefinitionKey& definitionKey, const type::Service& service) {
  std::vector<FunctionNode> functions;
  for (const type::Function& function : *service.functions()) {
    functions.emplace_back(createFunction(definitionKey, function));
  }
  auto baseServiceKey = [&]() -> std::optional<detail::DefinitionKeyRef> {
    const auto& baseServiceUri = *service.baseService()->uri();
    if (isEmptyTypeUri(baseServiceUri)) {
      return std::nullopt;
    }
    return definitionKeyOf(baseServiceUri);
  }();
  return ServiceNode(
      *this,
      definitionKey,
      *service.uri(),
      std::move(functions),
      std::move(baseServiceKey));
}

InteractionNode Resolver::createInteraction(
    const type::DefinitionKey& definitionKey,
    const type::Interaction& interaction) {
  std::vector<FunctionNode> functions;
  for (const type::Function& function : *interaction.functions()) {
    functions.emplace_back(createFunction(definitionKey, function));
  }
  return InteractionNode(
      *this, definitionKey, *interaction.uri(), std::move(functions));
}

FunctionNode Resolver::createFunction(
    const type::DefinitionKey& interfaceDefinitionKey,
    const type::Function& function) {
  const bool isVoid = *function.returnType() == type::Type();
  std::unique_ptr<TypeRef> initialResponse = isVoid
      ? nullptr
      : folly::copy_to_unique_ptr(typeOf(*function.returnType()));

  auto interaction = [&]() -> std::optional<detail::Lazy<InteractionNode>> {
    if (const auto& interactionTypeUri = *function.interactionType()->uri();
        !isEmptyTypeUri(interactionTypeUri)) {
      return detail::Lazy<InteractionNode>::Unresolved(
          *this, definitionKeyOf(interactionTypeUri));
    }
    return std::nullopt;
  }();

  const auto collectExceptions =
      [this](folly::span<const type::Field> exceptions) {
        std::vector<TypeRef> result;
        for (const type::Field& ex : exceptions) {
          result.emplace_back(typeOf(*ex.type()));
        }
        return result;
      };

  auto sinkOrStream = [&]() -> FunctionNode::Response::SinkOrStream {
    if (const auto& streamRef = function.streamOrSink()->streamType_ref();
        streamRef.has_value()) {
      return FunctionNode::Stream(
          typeOf(*streamRef->payload()),
          collectExceptions(*streamRef->exceptions()));
    } else if (const auto& sinkRef = function.streamOrSink()->sinkType_ref();
               sinkRef.has_value()) {
      return FunctionNode::Sink(
          typeOf(*sinkRef->payload()),
          typeOf(*sinkRef->finalResponse()),
          collectExceptions(*sinkRef->clientExceptions()),
          collectExceptions(*sinkRef->serverExceptions()));
    } else {
      return {};
    }
  }();

  std::vector<FunctionNode::Param> params;
  for (const type::Field& param : *function.paramlist()->fields()) {
    params.emplace_back(
        FunctionNode::Param(*this, *param.id(), *param.name(), *param.type()));
  }

  return FunctionNode(
      *this,
      interfaceDefinitionKey,
      createAnnotations(*function.annotationsByKey()),
      FunctionNode::Response(
          std::move(initialResponse),
          std::move(interaction),
          std::move(sinkOrStream)),
      *function.name(),
      std::move(params),
      collectExceptions(*function.exceptions()));
}

std::vector<Annotation> Resolver::createAnnotations(
    const std::map<type::DefinitionKey, type::Annotation>& annotations) {
  std::vector<Annotation> result;
  for (const auto& [definitionKey, annotation] : annotations) {
    // We need to access the raw struct because the DefinitionNode for the
    // annotation might not exist yet.
    const type::Definition* definition =
        folly::get_ptr(*rawSchema_->definitionsMap(), definitionKey);
    if (definition == nullptr) {
      // Due to circular dependency concerns in the Thrift compiler, the
      // standard annotation library does not bundle its runtime schema
      // information. For now, we pretend that they do not exist.
      continue;
    }
    Annotation::Fields fields;
    for (const auto& [fieldName, value] : *annotation.fields()) {
      fields.emplace(fieldName, value);
    }

    FOLLY_SAFE_CHECK(
        definition->getType() == type::Definition::Type::structDef,
        "Annotations should always be structs (not unions, nor exceptions)");
    auto type =
        TypeRef(detail::Lazy<StructNode>::Unresolved(*this, definitionKey));
    result.emplace_back(Annotation(std::move(type), std::move(fields)));
  }
  return result;
}

} // namespace detail

} // namespace apache::thrift::schema

#endif // THRIFT_SCHEMA_AVAILABLE
