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
#include <thrift/lib/cpp2/schema/detail/Resolver.h>
#include <thrift/lib/cpp2/schema/detail/SchemaBackedResolver.h>

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

const DefinitionNode& lookUpDefinition(
    const SyntaxGraph& syntaxGraph,
    const apache::thrift::type::DefinitionKey& definitionKey) {
  if (const DefinitionNode* def =
          syntaxGraph.resolver_->definitionOf(definitionKey)) {
    return *def;
  }
  folly::throw_exception<std::out_of_range>(
      fmt::format("Definition not found for key '{}'", definitionKey));
}

WithName::WithName(std::string_view name) : name_(name) {
  FOLLY_SAFE_DCHECK(
      name_.data()[name_.size()] == '\0',
      "name must be backed by a null-terminated string!");
}

} // namespace detail

TypeRef FieldNode::type() const {
  return *type_;
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
  return *type_;
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

ProgramNode::DefinitionsByName ProgramNode::definitionsByName() const {
  DefinitionsByName result;
  for (folly::not_null<const DefinitionNode*> definition : definitions_) {
    result.emplace(definition->name(), definition);
  }
  return result;
}

/* static */ SyntaxGraph SyntaxGraph::fromSchema(
    folly::not_null<const type::Schema*> schema) {
  return SyntaxGraph{detail::createResolverfromSchemaRef(*schema)};
}

/* static */ SyntaxGraph SyntaxGraph::fromSchema(type::Schema&& schema) {
  return SyntaxGraph{detail::createResolverfromSchema(std::move(schema))};
}

SyntaxGraph::SyntaxGraph(std::unique_ptr<detail::Resolver> resolver)
    : resolver_(std::move(resolver)) {}

SyntaxGraph::SyntaxGraph(SyntaxGraph&&) noexcept = default;
SyntaxGraph& SyntaxGraph::operator=(SyntaxGraph&&) noexcept = default;
SyntaxGraph::~SyntaxGraph() noexcept = default;

ProgramNode::IncludesList SyntaxGraph::programs() const {
  return resolver_->programs();
}

namespace detail {

WithAnnotations::WithAnnotations(std::vector<Annotation>&& annotations)
    : annotations_(std::move(annotations)) {}

folly::span<const Annotation> WithAnnotations::annotations() const {
  return annotations_;
}

const DefinitionNode& lazyResolve(
    const Resolver& resolver, const type::DefinitionKey& definitionKey) {
  if (const auto* definition = resolver.definitionOf(definitionKey)) {
    return *definition;
  }
  folly::throw_exception<InvalidSyntaxGraphError>(
      fmt::format("Definition key {} not found", definitionKey));
}

} // namespace detail

} // namespace apache::thrift::schema

#endif // THRIFT_SCHEMA_AVAILABLE
