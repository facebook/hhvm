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

#include <thrift/lib/cpp2/schema/detail/SchemaBackedResolver.h>

#include <thrift/lib/cpp2/schema/SyntaxGraph.h>
#include <thrift/lib/cpp2/schema/detail/Merge.h>

#ifdef THRIFT_SCHEMA_AVAILABLE

namespace apache::thrift::syntax_graph::detail {
namespace type = apache::thrift::type;
namespace protocol = apache::thrift::protocol;
using apache::thrift::util::enumNameSafe;

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

class SchemaIndex {
 public:
  explicit SchemaIndex(Resolver& resolver) : resolver_(resolver) {}

  // Annotations are referred to in schema.thrift using DefinitionKey, not
  // TypeStruct. Therefore, we use this map to keep their resolved TypeStruct
  // instances alive.
  //
  // Using F14NodeMap for reference stability.
  using AnnotationTypeByDefinitionKey = folly::F14NodeMap<
      DefinitionKeyRef,
      type::Type,
      DefinitionKeyHash,
      DefinitionKeyEqual>;
  AnnotationTypeByDefinitionKey annotationTypeByDefinitionKey_;

  // Every top-level definition has exactly one associated graph node. These are
  // stored and kept alive in this map.
  using DefinitionsByKey = folly::F14NodeMap<
      DefinitionKeyRef,
      DefinitionNode,
      DefinitionKeyHash,
      DefinitionKeyEqual>;
  DefinitionsByKey definitionsByKey_;

  // Every top-level definition is defined in a .thrift file. This map allows
  // back-references from these definition graph nodes to have efficient lookup
  // of their containing file.
  using ProgramsById = folly::F14NodeMap<type::ProgramId, ProgramNode>;
  ProgramsById programsById_;

  // A mapping of value IDs to a runtime representation of their value. This
  // matches how the data is stored in schema.thrift and de-duplicated.
  using ValuesById = folly::F14NodeMap<type::ValueId, protocol::Value>;
  ValuesById valuesById_;

  // A mapping of a definition to its containing Thrift file. Chaining this with
  // the ProgramsById map produces a mapping from DefinitionKey → Program graph
  // node.
  using ProgramIdsByDefinitionKey = folly::F14FastMap<
      DefinitionKeyRef,
      type::ProgramId,
      DefinitionKeyHash,
      DefinitionKeyEqual>;

  // An index of URI to definition key, to allow layering type system over
  // syntax graph.
  using DefinitionKeysByUri =
      folly::F14FastMap<std::string_view, DefinitionKeyRef>;
  DefinitionKeysByUri definitionKeysByUri_;

  // A set of unresolved definition keys collected while updating indexes.
  // This can be used to detect missing definitions in selective resolver.
  folly::F14FastSet<DefinitionKeyRef, DefinitionKeyHash, DefinitionKeyEqual>
      unresolvedDefinitionRefs_;

  void updateProgramsById(
      ProgramsById&, const type::Schema&, const DefinitionsByKey&);
  void updateValuesById(ValuesById&, const type::Schema& schema);

  static ProgramIdsByDefinitionKey createProgramIdsByDefinitionKey(
      const type::Schema&);
  void updateDefinitionsByKey(
      DefinitionsByKey&, const type::Schema&, const ProgramIdsByDefinitionKey&);

  void updateDefinitionKeysByUri(DefinitionKeysByUri&, const DefinitionsByKey&);

  DefinitionNode createDefinition(
      const ProgramIdsByDefinitionKey&,
      const type::DefinitionKey&,
      const type::DefinitionAttrs&,
      DefinitionNode::Alternative&&,
      const type::Schema&);
  StructNode createStruct(
      const type::DefinitionKey&, const type::Struct&, const type::Schema&);
  UnionNode createUnion(
      const type::DefinitionKey&, const type::Union&, const type::Schema&);
  ExceptionNode createException(
      const type::DefinitionKey&, const type::Exception&, const type::Schema&);
  FieldNode createField(
      const type::DefinitionKey& parentDefinitionKey,
      const type::Field&,
      const type::Schema&);

  EnumNode createEnum(
      const type::DefinitionKey&, const type::Enum&, const type::Schema&);
  TypedefNode createTypedef(const type::DefinitionKey&, const type::Typedef&);
  ConstantNode createConstant(const type::DefinitionKey&, const type::Const&);

  ServiceNode createService(
      const type::DefinitionKey&, const type::Service&, const type::Schema&);
  InteractionNode createInteraction(
      const type::DefinitionKey&,
      const type::Interaction&,
      const type::Schema&);
  FunctionNode createFunction(
      const type::DefinitionKey&, const type::Function&, const type::Schema&);

  std::vector<Annotation> createAnnotations(
      const std::map<type::DefinitionKey, type::Annotation>& annotations,
      const type::Schema&);
  // Tries to get the TypeStruct representing a structured annotation type.
  // Due to circular dependency concerns in the Thrift compiler, the
  // standard annotation library does not bundle its runtime schema
  // information. For now, we pretend that they do not exist, in which case,
  // this function returns nullptr.
  const type::Type* tryGetAnnotationType(
      const type::DefinitionKey&, const type::Schema&);

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
  static const type::DefinitionKey& definitionKeyOf(const type::TypeUri&);
  /**
   * Creates a `TypeRef` object, which is really a non-owning and unresolved
   * reference to a Thrift type.
   *
   * This function does not resolve composite types like typedefs, list etc.
   * (i.e. types that refer to other types). Non-composite types, such as struct
   * definitions are lazily resolved. This means that it's safe to call
   * typeOf(...) even if the pointed-to type has not been seen yet.
   */
  TypeRef typeOf(const type::TypeStruct&);
  TypeRef typeOf(const type::Type& type) { return typeOf(type.toThrift()); }

  template <typename T>
  detail::Lazy<T> createLazyUnresolved(const type::DefinitionKey& key) {
    unresolvedDefinitionRefs_.insert(key);
    return typename detail::Lazy<T>::Unresolved(resolver_, key);
  }

  Resolver& resolver_;

 public:
  const ProgramNode& programOf(const type::ProgramId&) const;
  const protocol::Value& valueOf(const type::ValueId&) const;
  const DefinitionNode* definitionOf(const type::DefinitionKey&) const;
  const DefinitionNode* definitionForUri(std::string_view uri) const;
  ProgramNode::IncludesList programs() const;

  void updateIndices(const type::Schema& schema, bool resolve = false) {
    updateDefinitionsByKey(
        definitionsByKey_, schema, createProgramIdsByDefinitionKey(schema));
    updateProgramsById(programsById_, schema, definitionsByKey_);
    updateValuesById(valuesById_, schema);
    updateDefinitionKeysByUri(definitionKeysByUri_, definitionsByKey_);

    auto unresolved = std::move(unresolvedDefinitionRefs_);
    if (!resolve) {
      return;
    }
    for (const auto& keyRef : unresolved) {
      if (!definitionsByKey_.contains(keyRef)) {
        folly::throw_exception<InvalidSyntaxGraphError>(
            fmt::format("Definition {} cannot be resolved.", keyRef.get()));
      }
    }
  }
};

class FullyResolvedSchemaRefBackedResolver : public SchemaBackedResolver {
 public:
  explicit FullyResolvedSchemaRefBackedResolver(const type::Schema& schema)
      : schema_(schema) {
    index_->updateIndices(schema_, true);
  }

  const ProgramNode& programOf(const type::ProgramId& id) const override {
    return index_->programOf(id);
  }
  const protocol::Value& valueOf(const type::ValueId& id) const override {
    return index_->valueOf(id);
  }
  const DefinitionNode* definitionOf(
      const type::DefinitionKey& key) const override {
    return index_->definitionOf(key);
  }
  ProgramNode::IncludesList programs() const override {
    return index_->programs();
  }

 private:
  const type::Schema& schema_;
};

class FullyResolvedSchemaBackedResolver : public SchemaBackedResolver {
 public:
  explicit FullyResolvedSchemaBackedResolver(type::Schema&& schema)
      : schema_(std::move(schema)) {
    index_->updateIndices(schema_, true);
  }

  const ProgramNode& programOf(const type::ProgramId& id) const override {
    return index_->programOf(id);
  }
  const protocol::Value& valueOf(const type::ValueId& id) const override {
    return index_->valueOf(id);
  }
  const DefinitionNode* definitionOf(
      const type::DefinitionKey& key) const override {
    return index_->definitionOf(key);
  }
  ProgramNode::IncludesList programs() const override {
    return index_->programs();
  }

 private:
  const type::Schema schema_;
};

FieldNode::PresenceQualifier presenceOf(const type::FieldQualifier& qualifier) {
  switch (qualifier) {
    case type::FieldQualifier::Default:
    case type::FieldQualifier::Terse:
    case type::FieldQualifier::Fill:
      return FieldNode::PresenceQualifier::UNQUALIFIED;
    case type::FieldQualifier::Optional:
      return FieldNode::PresenceQualifier::OPTIONAL_;
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

template <typename... F>
decltype(auto) visitDefinition(
    const type::Definition& definition, F&&... visitors) {
  auto overloaded = folly::overload(std::forward<F>(visitors)...);
  switch (definition.getType()) {
    case type::Definition::Type::structDef:
      return overloaded(*definition.structDef());
    case type::Definition::Type::unionDef:
      return overloaded(*definition.unionDef());
    case type::Definition::Type::exceptionDef:
      return overloaded(*definition.exceptionDef());
    case type::Definition::Type::enumDef:
      return overloaded(*definition.enumDef());
    case type::Definition::Type::typedefDef:
      return overloaded(*definition.typedefDef());
    case type::Definition::Type::constDef:
      return overloaded(*definition.constDef());
    case type::Definition::Type::serviceDef:
      return overloaded(*definition.serviceDef());
    case type::Definition::Type::interactionDef:
      return overloaded(*definition.interactionDef());
    case type::Definition::Type::__EMPTY__:
    default:
      folly::throw_exception<InvalidSyntaxGraphError>(fmt::format(
          "Unknown Definition::Type '{}'", enumNameSafe(definition.getType())));
  }
}

const ProgramNode& SchemaIndex::programOf(const type::ProgramId& id) const {
  return folly::get_or_throw<InvalidSyntaxGraphError>(
      programsById_, id, "Unknown ProgramId: ");
}

const type::DefinitionKey& SchemaIndex::definitionKeyOf(
    const type::TypeUri& typeUri) {
  using T = type::TypeUri::Type;
  switch (typeUri.getType()) {
    case T::definitionKey:
      return *typeUri.definitionKey();
    case T::uri:
    case T::typeHashPrefixSha2_256:
    case T::scopedName:
    case T::__EMPTY__:
    default:
      folly::throw_exception<InvalidSyntaxGraphError>(fmt::format(
          "Unsupported TypeUri::Type '{}'", enumNameSafe(typeUri.getType())));
  }
}

TypeRef SchemaIndex::typeOf(const type::TypeStruct& type) {
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
        return createLazyUnresolved<EnumNode>(
            definitionKeyOf(*type.name()->enumType()));
      case T::typedefType:
        return createLazyUnresolved<TypedefNode>(
            definitionKeyOf(*type.name()->typedefType()));
      case T::structType:
        return createLazyUnresolved<StructNode>(
            definitionKeyOf(*type.name()->structType()));
      case T::unionType:
        return createLazyUnresolved<UnionNode>(
            definitionKeyOf(*type.name()->unionType()));
      case T::exceptionType:
        return createLazyUnresolved<ExceptionNode>(
            definitionKeyOf(*type.name()->exceptionType()));
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

const protocol::Value& SchemaIndex::valueOf(const type::ValueId& id) const {
  return folly::get_or_throw<InvalidSyntaxGraphError>(
      valuesById_, id, "Unknown ValueId: ");
}

const DefinitionNode* SchemaIndex::definitionOf(
    const type::DefinitionKey& definitionKey) const {
  return folly::get_ptr(definitionsByKey_, definitionKey);
}

const DefinitionNode* SchemaIndex::definitionForUri(
    std::string_view uri) const {
  auto definitionKey = folly::get_ptr(definitionKeysByUri_, uri);
  if (!definitionKey) {
    return nullptr;
  }
  return definitionOf(*definitionKey);
}

ProgramNode::IncludesList SchemaIndex::programs() const {
  ProgramNode::IncludesList programs;
  for (const auto& [_, program] : programsById_) {
    programs.emplace_back(&program);
  }
  return programs;
}

void SchemaIndex::updateProgramsById(
    SchemaIndex::ProgramsById& result,
    const type::Schema& schema,
    const DefinitionsByKey& definitionsByKey) {
  for (const type::Program& program : *schema.programs()) {
    if (result.contains(*program.id())) {
      continue;
    }
    ProgramNode::Definitions definitions;
    for (const type::DefinitionKey& definitionKey : *program.definitionKeys()) {
      if (const DefinitionNode* definition =
              folly::get_ptr(definitionsByKey, definitionKey)) {
        definitions.emplace_back(definition);
      }
    }
    result.emplace(
        *program.id(),
        ProgramNode(
            resolver_,
            *program.path(),
            *program.name(),
            *program.includes(),
            std::move(definitions),
            *program.namespaces()));
  }
}

void SchemaIndex::updateValuesById(
    SchemaIndex::ValuesById& result, const type::Schema& schema) {
  result.insert(schema.valuesMap()->begin(), schema.valuesMap()->end());
}

/* static */ SchemaIndex::ProgramIdsByDefinitionKey
SchemaIndex::createProgramIdsByDefinitionKey(const type::Schema& schema) {
  ProgramIdsByDefinitionKey result;
  for (const type::Program& program : *schema.programs()) {
    for (const type::DefinitionKey& definitionKey : *program.definitionKeys()) {
      result.emplace(definitionKey, *program.id());
    }
  }
  return result;
}

void SchemaIndex::updateDefinitionsByKey(
    SchemaIndex::DefinitionsByKey& result,
    const type::Schema& schema,
    const SchemaIndex::ProgramIdsByDefinitionKey& programIdsByDefinitionKey) {
  for (const auto& entry : *schema.definitionsMap()) {
    if (result.contains(entry.first)) {
      continue;
    }
    const type::DefinitionKey& definitionKey = entry.first;
    const type::Definition& definition = entry.second;
    const auto& definitionAttrs = visitDefinition(
        definition, [](auto&& def) -> const type::DefinitionAttrs& {
          return *def.attrs();
        });
    auto alternative = visitDefinition(
        definition,
        [&](const type::Struct& structDef) -> DefinitionNode::Alternative {
          return createStruct(definitionKey, structDef, schema);
        },
        [&](const type::Union& unionDef) -> DefinitionNode::Alternative {
          return createUnion(definitionKey, unionDef, schema);
        },
        [&](const type::Exception& exceptionDef)
            -> DefinitionNode::Alternative {
          return createException(definitionKey, exceptionDef, schema);
        },
        [&](const type::Enum& enumDef) -> DefinitionNode::Alternative {
          return createEnum(definitionKey, enumDef, schema);
        },
        [&](const type::Typedef& typedefDef) -> DefinitionNode::Alternative {
          return createTypedef(definitionKey, typedefDef);
        },
        [&](const type::Const& constDef) -> DefinitionNode::Alternative {
          return createConstant(definitionKey, constDef);
        },
        [&](const type::Service& serviceDef) -> DefinitionNode::Alternative {
          return createService(definitionKey, serviceDef, schema);
        },
        [&](const type::Interaction& interactionDef)
            -> DefinitionNode::Alternative {
          return createInteraction(definitionKey, interactionDef, schema);
        });
    result.emplace(
        definitionKey,
        createDefinition(
            programIdsByDefinitionKey,
            definitionKey,
            definitionAttrs,
            std::move(alternative),
            schema));
  }
}

void SchemaIndex::updateDefinitionKeysByUri(
    SchemaIndex::DefinitionKeysByUri& result,
    const SchemaIndex::DefinitionsByKey& definitionsByKey) {
  for (const auto& [definitionKey, definition] : definitionsByKey) {
    auto uri = definition.visit([](const auto& def) {
      if constexpr (std::is_base_of_v<
                        detail::WithUri,
                        std::decay_t<decltype(def)>>) {
        return def.uri();
      } else {
        return std::string_view{};
      }
    });

    if (uri.empty() || result.contains(uri)) {
      continue;
    }

    result.emplace(uri, definitionKey);
  }
}

DefinitionNode SchemaIndex::createDefinition(
    const SchemaIndex::ProgramIdsByDefinitionKey& programIdsByDefinitionKey,
    const type::DefinitionKey& definitionKey,
    const type::DefinitionAttrs& attrs,
    DefinitionNode::Alternative&& alternative,
    const type::Schema& schema) {
  return DefinitionNode(
      resolver_,
      folly::get_or_throw<InvalidSyntaxGraphError>(
          programIdsByDefinitionKey,
          definitionKey,
          "Unknown ProgramId for DefinitionKey: "),
      createAnnotations(*attrs.annotationsByKey(), schema),
      *attrs.name(),
      std::move(alternative));
}

StructNode SchemaIndex::createStruct(
    const type::DefinitionKey& definitionKey,
    const type::Struct& structDef,
    const type::Schema& schema) {
  std::vector<FieldNode> fields;
  for (const type::Field& field : *structDef.fields()) {
    fields.emplace_back(createField(definitionKey, field, schema));
  }
  return StructNode(
      resolver_, definitionKey, *structDef.uri(), std::move(fields));
}

UnionNode SchemaIndex::createUnion(
    const type::DefinitionKey& definitionKey,
    const type::Union& unionDef,
    const type::Schema& schema) {
  std::vector<FieldNode> fields;
  for (const type::Field& field : *unionDef.fields()) {
    fields.emplace_back(createField(definitionKey, field, schema));
  }
  return UnionNode(
      resolver_, definitionKey, *unionDef.uri(), std::move(fields));
}

ExceptionNode SchemaIndex::createException(
    const type::DefinitionKey& definitionKey,
    const type::Exception& exceptionDef,
    const type::Schema& schema) {
  std::vector<FieldNode> fields;
  for (const type::Field& field : *exceptionDef.fields()) {
    fields.emplace_back(createField(definitionKey, field, schema));
  }
  return ExceptionNode(
      resolver_, definitionKey, *exceptionDef.uri(), std::move(fields));
}

FieldNode SchemaIndex::createField(
    const type::DefinitionKey& parentDefinitionKey,
    const type::Field& field,
    const type::Schema& schema) {
  return FieldNode(
      resolver_,
      parentDefinitionKey,
      createAnnotations(*field.annotationsByKey(), schema),
      *field.id(),
      presenceOf(*field.qualifier()),
      *field.name(),
      folly::copy_to_unique_ptr(typeOf(*field.type())),
      valueIdOf(*field.customDefault()));
}

EnumNode SchemaIndex::createEnum(
    const type::DefinitionKey& definitionKey,
    const type::Enum& enumDef,
    const type::Schema& schema) {
  std::vector<EnumNode::Value> values;
  for (const type::EnumValue& enumValue : *enumDef.values()) {
    values.emplace_back(
        *enumValue.name(),
        *enumValue.value(),
        createAnnotations(*enumValue.annotationsByKey(), schema));
  }
  return EnumNode(resolver_, definitionKey, *enumDef.uri(), std::move(values));
}

TypedefNode SchemaIndex::createTypedef(
    const type::DefinitionKey& definitionKey, const type::Typedef& typedefDef) {
  return TypedefNode(resolver_, definitionKey, typeOf(*typedefDef.type()));
}

ConstantNode SchemaIndex::createConstant(
    const type::DefinitionKey& definitionKey, const type::Const& constDef) {
  return ConstantNode(
      resolver_, definitionKey, typeOf(*constDef.type()), *constDef.value());
}

ServiceNode SchemaIndex::createService(
    const type::DefinitionKey& definitionKey,
    const type::Service& service,
    const type::Schema& schema) {
  std::vector<FunctionNode> functions;
  for (const type::Function& function : *service.functions()) {
    functions.emplace_back(createFunction(definitionKey, function, schema));
  }
  auto baseServiceKey = [&]() -> std::optional<detail::DefinitionKeyRef> {
    const auto& baseServiceUri = *service.baseService()->uri();
    if (isEmptyTypeUri(baseServiceUri)) {
      return std::nullopt;
    }
    return definitionKeyOf(baseServiceUri);
  }();
  return ServiceNode(
      resolver_,
      definitionKey,
      *service.uri(),
      std::move(functions),
      std::move(baseServiceKey));
}

InteractionNode SchemaIndex::createInteraction(
    const type::DefinitionKey& definitionKey,
    const type::Interaction& interaction,
    const type::Schema& schema) {
  std::vector<FunctionNode> functions;
  for (const type::Function& function : *interaction.functions()) {
    functions.emplace_back(createFunction(definitionKey, function, schema));
  }
  return InteractionNode(
      resolver_, definitionKey, *interaction.uri(), std::move(functions));
}

FunctionNode SchemaIndex::createFunction(
    const type::DefinitionKey& interfaceDefinitionKey,
    const type::Function& function,
    const type::Schema& schema) {
  const bool isVoid = *function.returnType() == type::Type();
  std::unique_ptr<TypeRef> initialResponse = isVoid
      ? nullptr
      : folly::copy_to_unique_ptr(typeOf(*function.returnType()));

  auto interaction = [&]() -> std::optional<detail::Lazy<InteractionNode>> {
    if (const auto& interactionTypeUri = *function.interactionType()->uri();
        !isEmptyTypeUri(interactionTypeUri)) {
      return createLazyUnresolved<InteractionNode>(
          definitionKeyOf(interactionTypeUri));
    }
    return std::nullopt;
  }();

  const auto collectExceptions =
      [this](folly::span<const type::Field> exceptions) {
        std::vector<FunctionNode::Exception> result;
        for (const type::Field& ex : exceptions) {
          result.emplace_back(
              resolver_,
              *ex.id(),
              *ex.name(),
              folly::copy_to_unique_ptr(typeOf(*ex.type())));
        }
        return result;
      };

  auto sinkOrStream = [&]() -> FunctionNode::Response::SinkOrStream {
    if (const auto& streamRef = function.streamOrSink()->streamType();
        streamRef.has_value()) {
      return FunctionNode::Stream(
          typeOf(*streamRef->payload()),
          collectExceptions(*streamRef->exceptions()));
    } else if (const auto& sinkRef = function.streamOrSink()->sinkType();
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
        resolver_,
        *param.id(),
        *param.name(),
        folly::copy_to_unique_ptr(typeOf(*param.type())));
  }

  return FunctionNode(
      resolver_,
      interfaceDefinitionKey,
      createAnnotations(*function.annotationsByKey(), schema),
      FunctionNode::Response(
          std::move(initialResponse),
          std::move(interaction),
          std::move(sinkOrStream)),
      *function.name(),
      std::move(params),
      collectExceptions(*function.exceptions()));
}

std::vector<Annotation> SchemaIndex::createAnnotations(
    const std::map<type::DefinitionKey, type::Annotation>& annotations,
    const type::Schema& schema) {
  std::vector<Annotation> result;
  for (const auto& [definitionKey, annotation] : annotations) {
    const type::Type* rawType = tryGetAnnotationType(definitionKey, schema);
    if (rawType == nullptr) {
      // Most likely, a standard annotation library struct, which are not
      // bundled due to circular dependency issues.
      continue;
    }
    result.emplace_back(typeOf(*rawType), *annotation.fields());
  }
  return result;
}

namespace {

type::TypeUri createTypeUri(const type::DefinitionKey& definitionKey) {
  type::TypeUri typeUri;
  typeUri.definitionKey() = definitionKey;
  return typeUri;
}

type::Type createUnparamedType(type::TypeName&& typeName) {
  type::TypeStruct typeStruct;
  typeStruct.name() = std::move(typeName);
  return type::Type(std::move(typeStruct));
}
} // namespace

const type::Type* SchemaIndex::tryGetAnnotationType(
    const type::DefinitionKey& definitionKey, const type::Schema& schema) {
  if (auto found = annotationTypeByDefinitionKey_.find(definitionKey);
      found != annotationTypeByDefinitionKey_.end()) {
    return std::addressof(found->second);
  }

  // We need to access the raw struct because the DefinitionNode for the
  // annotation might not exist yet.
  const type::Definition* definition =
      folly::get_ptr(*schema.definitionsMap(), definitionKey);
  if (definition == nullptr) {
    return nullptr;
  }

  auto type = std::invoke([&]() -> type::Type {
    using T = type::Definition::Type;
    switch (definition->getType()) {
      case T::typedefDef: {
        type::TypeName typeName;
        typeName.typedefType() = createTypeUri(definitionKey);
        return createUnparamedType(std::move(typeName));
      }
      case T::structDef: {
        type::TypeName typeName;
        typeName.structType() = createTypeUri(definitionKey);
        return createUnparamedType(std::move(typeName));
      }
      case T::enumDef:
        FOLLY_SAFE_FATAL("Structured annotation cannot be an enum type");
        return {};
      // The cases below should never happen ideally. However, the compiler has
      // a habit of spitting out invalid schema and the strictness below makes
      // it much easier to debug failures in cases where the schema information
      // is incorrect.
      case T::unionDef: {
        type::TypeName typeName;
        typeName.unionType() = createTypeUri(definitionKey);
        return createUnparamedType(std::move(typeName));
      }
      case T::exceptionDef: {
        type::TypeName typeName;
        typeName.exceptionType() = createTypeUri(definitionKey);
        return createUnparamedType(std::move(typeName));
      }
      default:
        FOLLY_SAFE_FATAL(
            "Structured annotation does not refer to a type definition");
        return {};
    }
  });

  auto [element, inserted] =
      annotationTypeByDefinitionKey_.emplace(definitionKey, std::move(type));
  FOLLY_SAFE_CHECK(inserted);
  return std::addressof(element->second);
}

folly::not_null_unique_ptr<Resolver> createResolverfromSchema(
    type::Schema&& schema) {
  return std::make_unique<FullyResolvedSchemaBackedResolver>(std::move(schema));
}
folly::not_null_unique_ptr<Resolver> createResolverfromSchemaRef(
    const type::Schema& schema) {
  return std::make_unique<FullyResolvedSchemaRefBackedResolver>(schema);
}

SchemaBackedResolver::SchemaBackedResolver()
    : index_(std::make_unique<SchemaIndex>(*this)) {}
SchemaBackedResolver::~SchemaBackedResolver() = default;

const ProgramNode& IncrementalResolver::programOf(
    const type::ProgramId& id) const {
  return index_->programOf(id);
}
const protocol::Value& IncrementalResolver::valueOf(
    const type::ValueId& id) const {
  return index_->valueOf(id);
}
const DefinitionNode* IncrementalResolver::definitionOf(
    const type::DefinitionKey& key) const {
  return index_->definitionOf(key);
}
ProgramNode::IncludesList IncrementalResolver::programs() const {
  return index_->programs();
}

void IncrementalResolver::readSchema(
    folly::Synchronized<type::Schema>::LockedPtr& schema,
    folly::span<const std::string_view> bundle) const {
  auto src = schema::detail::mergeSchemas(bundle);
  auto& dst = *schema;

  // Merge new schema data in
  // TODO: avoid deserializing shared deps
  std::copy_if(
      std::make_move_iterator(src.programs()->begin()),
      std::make_move_iterator(src.programs()->end()),
      std::back_inserter(*dst.programs()),
      [this](const auto& program) {
        return !index_->programsById_.contains(*program.id());
      });
  dst.valuesMap()->insert(
      std::make_move_iterator(src.valuesMap()->begin()),
      std::make_move_iterator(src.valuesMap()->end()));
  dst.definitionsMap()->insert(
      std::make_move_iterator(src.definitionsMap()->begin()),
      std::make_move_iterator(src.definitionsMap()->end()));

  index_->updateIndices(dst);
}

const DefinitionNode& IncrementalResolver::getDefinitionNode(
    const type::DefinitionKey& key,
    type::ProgramId programId,
    std::string_view name,
    ::folly::Range<const ::std::string_view*> (*bundle)()) const {
  {
    auto schemaReadGuard = schema_.rlock();
    if (auto* def = index_->definitionOf(key)) {
      return *def;
    }
  }

  if (!bundle) {
    folly::throw_exception<std::out_of_range>(
        fmt::format("Definition `{}` does not have bundled schema.", name));
  }

  auto schemaWriteGuard = schema_.wlock();
  if (auto* def = index_->definitionOf(key)) {
    return *def;
  }

  readSchema(schemaWriteGuard, bundle());

  if (auto* def = index_->definitionOf(key)) {
    return *def;
  }

  if (index_->programsById_.contains(programId)) {
    folly::throw_exception<InvalidSyntaxGraphError>(fmt::format(
        "Definition `{}` not found in its program's schema.", name));
  }
  folly::throw_exception<std::out_of_range>(
      fmt::format("Definition `{}` does not have bundled schema.", name));
}

const DefinitionNode* SchemaBackedResolver::getDefinitionNodeByUri(
    std::string_view uri) const {
  return index_->definitionForUri(uri);
}

const DefinitionNode* IncrementalResolver::getDefinitionNodeByUri(
    std::string_view uri) const {
  auto schemaReadGuard = schema_.rlock();
  return index_->definitionForUri(uri);
}

const DefinitionNode* IncrementalResolver::getDefinitionNodeByUri(
    const std::string_view uri,
    type::ProgramId programId,
    folly::span<const std::string_view> bundle) const {
  {
    auto schemaReadGuard = schema_.rlock();
    if (auto* def = index_->definitionForUri(uri)) {
      return def;
    }
  }

  if (bundle.empty()) {
    return nullptr;
  }

  auto schemaWriteGuard = schema_.wlock();
  if (auto* def = index_->definitionForUri(uri)) {
    return def;
  }

  readSchema(schemaWriteGuard, bundle);

  if (auto* def = index_->definitionForUri(uri)) {
    return def;
  }

  if (index_->programsById_.contains(programId)) {
    folly::throw_exception<InvalidSyntaxGraphError>(
        fmt::format("Definition `{}` not found in its program's schema.", uri));
  }
  return nullptr;
}

} // namespace apache::thrift::syntax_graph::detail
#endif
