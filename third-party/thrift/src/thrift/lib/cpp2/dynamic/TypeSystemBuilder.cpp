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

#include <thrift/lib/cpp2/dynamic/TypeSystemBuilder.h>

#include <thrift/lib/cpp/util/EnumUtils.h>
#include <thrift/lib/cpp2/dynamic/detail/TypeSystem.h>

#include <folly/Overload.h>
#include <folly/container/F14Set.h>
#include <folly/container/MapUtil.h>
#include <folly/container/span.h>

#include <functional>
#include <utility>
#include <variant>

namespace apache::thrift::type_system {

namespace {

using TSDefinition =
    std::variant<StructNode, UnionNode, EnumNode, OpaqueAliasNode>;

class TypeSystemImpl final : public SourceIndexedTypeSystem {
 public:
  using DefinitionsMap = folly::F14NodeMap<
      Uri,
      TSDefinition,
      detail::UriHeterogeneousHash,
      std::equal_to<>>;
  DefinitionsMap definitions;

  std::optional<DefinitionRef> getUserDefinedType(UriView uri) const final {
    if (auto def = definitions.find(uri); def != definitions.end()) {
      return folly::variant_match(
          def->second, [](auto& d) { return DefinitionRef(&d); });
    }
    return std::nullopt;
  }

  std::optional<folly::F14FastSet<Uri>> getKnownUris() const final {
    folly::F14FastSet<Uri> result;
    result.reserve(definitions.size());
    for (const auto& [uri, _] : definitions) {
      result.insert(uri);
    }
    return result;
  }

  using Location = std::string;
  using DefinitionName = std::string;
  // Map of definition name (within one location) to definition
  using NameToDefinitionsMap = SourceIndexedTypeSystem::NameToDefinitionsMap;
  // Map of location to all definitions at that location
  using LocationToDefinitionsMap =
      folly::F14FastMap<Location, NameToDefinitionsMap>;

  LocationToDefinitionsMap sourceIndexedDefinitions;
  folly::F14FastMap<DefinitionRef, SourceIdentifier>
      definitionToSourceIdentifier;

  void tryAddToSourceIndex(
      SourceIdentifier sourceIdentifier, DefinitionRef def) {
    {
      auto [_, inserted] =
          sourceIndexedDefinitions[sourceIdentifier.location].emplace(
              sourceIdentifier.name, def);
      if (!inserted) {
        throw InvalidTypeError(fmt::format(
            "Duplicate source identifier name '{}' at location '{}'",
            sourceIdentifier.name,
            sourceIdentifier.location));
      }
    }

    {
      auto [_, inserted] =
          definitionToSourceIdentifier.emplace(def, sourceIdentifier);
      if (!inserted) {
        throw InvalidTypeError(fmt::format(
            "Duplicate definition for source identifier name '{}' at location '{}'",
            sourceIdentifier.name,
            sourceIdentifier.location));
      }
    }
  }

  std::optional<DefinitionRef> getUserDefinedTypeBySourceIdentifier(
      SourceIdentifierView sourceIdentifier) const final {
    if (const NameToDefinitionsMap* atLocation = folly::get_ptr(
            sourceIndexedDefinitions, sourceIdentifier.location)) {
      return folly::get_optional<std::optional>(
          *atLocation, sourceIdentifier.name);
    }
    return std::nullopt;
  }

  std::optional<SourceIdentifierView> getSourceIdentiferForUserDefinedType(
      DefinitionRef ref) const final {
    const SourceIdentifier* sourceIdentifier =
        folly::get_ptr(definitionToSourceIdentifier, ref);
    return sourceIdentifier
        ? std::optional<SourceIdentifierView>{*sourceIdentifier}
        : std::nullopt;
  }

  folly::F14FastMap<std::string, DefinitionRef> getUserDefinedTypesAtLocation(
      std::string_view location) const final {
    return folly::get_default(sourceIndexedDefinitions, location);
  }

  // NOTE: This function should ONLY be called within TypeSystemBuilder::build.
  // It is defined here instead of as a local lambda because the implementation
  // is recursive and such lambdas as a pain until C++23's "deducing this".
  TypeRef typeOf(const TypeId& typeId) {
    return typeId.visit(
        [&](const Uri& uri) -> TypeRef {
          if (auto def = definitions.find(uri); def != definitions.end()) {
            return folly::variant_match(
                def->second, [](auto& d) { return TypeRef(d); });
          }
          throw InvalidTypeError(
              fmt::format("Definition for uri '{}' was not found", uri));
        },
        [&](const TypeId::List& list) -> TypeRef {
          return TypeSystem::ListOf(typeOf(list.elementType()));
        },
        [&](const TypeId::Set& set) -> TypeRef {
          return TypeSystem::SetOf(typeOf(set.elementType()));
        },
        [&](const TypeId::Map& map) -> TypeRef {
          return TypeSystem::MapOf(
              typeOf(map.keyType()), typeOf(map.valueType()));
        },
        [](const auto& primitive) -> TypeRef { return TypeRef(primitive); });
  }
};

} // namespace

std::unique_ptr<TypeSystem> TypeSystemBuilder::build() && {
  auto typeSystem = std::make_unique<TypeSystemImpl>();

  // Fill in definitions with uninitialized stubs
  for (auto& [uri, entry] : definitions_) {
    SerializableTypeDefinition& def = entry.definition;
    auto uninitDef = std::invoke([&]() -> TSDefinition {
      switch (def.getType()) {
        case SerializableTypeDefinition::Type::structDef:
          return StructNode{{}, {}, {}, {}};
        case SerializableTypeDefinition::Type::unionDef:
          return UnionNode{{}, {}, {}, {}};
        case SerializableTypeDefinition::Type::enumDef:
          return EnumNode{{}, {}, {}};
        case SerializableTypeDefinition::Type::opaqueAliasDef:
          return OpaqueAliasNode{{}, TypeRef{TypeRef::Bool{}}, {}};
        default:
          break;
      }
      folly::assume_unreachable();
    });
    typeSystem->definitions.emplace(uri, std::move(uninitDef));
  }

  const auto makeAnnots =
      [&](folly::F14FastMap<Uri, SerializableRecordUnion> annotations) {
        AnnotationsMap ret;
        ret.reserve(annotations.size());
        annotations.eraseInto(
            annotations.begin(), annotations.end(), [&](auto&& k, auto&& v) {
              // Validate uri exists in the type system and resolve to struct
              // type.
              auto typeref = typeSystem->typeOf(k);
              if (!typeref.isStruct()) {
                throw InvalidTypeError(fmt::format(
                    "Definition for uri '{}' for annotation is not struct.",
                    k));
              }
              // Validate if the value is a struct.
              if (!v.fieldSetDatum_ref().has_value()) {
                throw InvalidTypeError(fmt::format(
                    "Value for uri '{}' for annotation is not a struct.", k));
              }
              ret.emplace(
                  std::move(k), SerializableRecord::fromThrift(std::move(v)));
            });
        return ret;
      };

  const auto makeFields = [&](std::vector<SerializableFieldDefinition> fields)
      -> std::vector<FieldDefinition> {
    std::vector<FieldDefinition> result;
    result.reserve(fields.size());
    for (auto& field : fields) {
      result.emplace_back(
          std::move(*field.identity()),
          *field.presence(),
          typeSystem->typeOf(*field.type()),
          field.customDefaultPartialRecord().has_value()
              ? std::optional{SerializableRecord::fromThrift(
                    std::move(*field.customDefaultPartialRecord()))}
              : std::nullopt,
          makeAnnots(std::move(*field.annotations())));
    }
    return result;
  };

  for (auto& [uri, entry] : definitions_) {
    SerializableTypeDefinition& def = entry.definition;
    std::optional<SerializableThriftSourceInfo>& sourceInfo = entry.sourceInfo;
    // We created uninitialized stubs above so we can assume they exist
    TSDefinition& uninitDef = typeSystem->definitions.find(uri)->second;

    switch (def.getType()) {
      case SerializableTypeDefinition::Type::structDef: {
        SerializableStructDefinition& structDef = *def.structDef();
        StructNode& structNode = std::get<StructNode>(uninitDef);
        structNode = StructNode(
            uri,
            makeFields(std::move(*structDef.fields())),
            *structDef.isSealed(),
            makeAnnots(std::move(*structDef.annotations())));
        if (sourceInfo.has_value()) {
          typeSystem->tryAddToSourceIndex(
              SourceIdentifier{*sourceInfo->locator(), *sourceInfo->name()},
              DefinitionRef(&structNode));
        }
      } break;
      case SerializableTypeDefinition::Type::unionDef: {
        SerializableUnionDefinition& unionDef = *def.unionDef();
        UnionNode& unionNode = std::get<UnionNode>(uninitDef);
        unionNode = UnionNode(
            uri,
            makeFields(std::move(*unionDef.fields())),
            *unionDef.isSealed(),
            makeAnnots(std::move(*unionDef.annotations())));
        if (sourceInfo.has_value()) {
          typeSystem->tryAddToSourceIndex(
              SourceIdentifier{*sourceInfo->locator(), *sourceInfo->name()},
              DefinitionRef(&unionNode));
        }
      } break;
      case SerializableTypeDefinition::Type::enumDef: {
        SerializableEnumDefinition& enumDef = *def.enumDef();
        std::vector<EnumNode::Value> values;
        values.reserve(enumDef.values()->size());
        for (SerializableEnumValueDefinition& mapping : *enumDef.values()) {
          values.emplace_back(EnumNode::Value{
              std::move(*mapping.name()),
              *mapping.datum(),
              makeAnnots(std::move(*mapping.annotations()))});
        }
        EnumNode& enumNode = std::get<EnumNode>(uninitDef);
        enumNode = EnumNode(
            uri,
            std::move(values),
            makeAnnots(std::move(*enumDef.annotations())));
        if (sourceInfo.has_value()) {
          typeSystem->tryAddToSourceIndex(
              SourceIdentifier{*sourceInfo->locator(), *sourceInfo->name()},
              DefinitionRef(&enumNode));
        }
      } break;
      case SerializableTypeDefinition::Type::opaqueAliasDef: {
        SerializableOpaqueAliasDefinition& opaqueAliasDef =
            *def.opaqueAliasDef();
        OpaqueAliasNode& opaqueAliasNode = std::get<OpaqueAliasNode>(uninitDef);
        opaqueAliasNode = OpaqueAliasNode(
            uri,
            typeSystem->typeOf(*opaqueAliasDef.targetType()),
            makeAnnots(std::move(*opaqueAliasDef.annotations())));
        if (sourceInfo.has_value()) {
          typeSystem->tryAddToSourceIndex(
              SourceIdentifier{*sourceInfo->locator(), *sourceInfo->name()},
              DefinitionRef(&opaqueAliasNode));
        }
      } break;
      default:
        break;
    }
  }

  return typeSystem;
}

namespace {

/**
 * For structured types, both field ids AND names must be unique.
 */
void validateIdentitiesAreUnique(
    UriView uri, folly::span<const SerializableFieldDefinition> fields) {
  folly::F14FastSet<FieldId> seenIds;
  folly::F14FastSet<FieldName> seenNames;

  for (const auto& field : fields) {
    if (seenIds.contains(field.identity()->id())) {
      throw InvalidTypeError(fmt::format(
          "Duplicate field id '{}' in structured type '{}'",
          field.identity()->id(),
          uri));
    }
    seenIds.insert(field.identity()->id());

    if (seenNames.contains(field.identity()->name())) {
      throw InvalidTypeError(fmt::format(
          "Duplicate field name '{}' in structured type '{}'",
          field.identity()->name(),
          uri));
    }
    seenNames.insert(field.identity()->name());
  }
}

/**
 * For unions, all fields must be optional.
 */
void validateFieldsAreOptional(
    UriView uri, const SerializableUnionDefinition& unionDef) {
  for (const SerializableFieldDefinition& field : *unionDef.fields()) {
    if (field.presence() != PresenceQualifier::OPTIONAL_) {
      throw InvalidTypeError(fmt::format(
          "field '{}' must be optional in union '{}'",
          field.identity()->name(),
          uri));
    }
  }
}

/**
 * For enums, both enum names AND values must be unique.
 */
void validateEnumMappingsAreUnique(
    UriView uri, const SerializableEnumDefinition& enumDef) {
  folly::F14FastSet<std::string_view> seenNames;
  folly::F14FastSet<std::int32_t> seenValues;

  for (const SerializableEnumValueDefinition& entry : *enumDef.values()) {
    if (seenNames.contains(*entry.name())) {
      throw InvalidTypeError(
          fmt::format("Duplicate name '{}' in enum '{}'", *entry.name(), uri));
    }
    seenNames.insert(*entry.name());

    if (seenValues.contains(*entry.datum())) {
      throw InvalidTypeError(fmt::format(
          "Duplicate value '{}' in enum '{}'", *entry.datum(), uri));
    }
    seenValues.insert(*entry.datum());
  }
}

/**
 * Opaque aliases are not allowed to have a target type which is a user-defined
 * type.
 */
void validateOpaqueAliasIsNotUserDefined(
    UriView uri, const SerializableOpaqueAliasDefinition& opaqueAliasDef) {
  if (opaqueAliasDef.targetType()->kind() == TypeId::Kind::URI) {
    throw InvalidTypeError(fmt::format(
        "Opaque alias '{}' cannot have target type of a user-defined type '{}'",
        uri,
        *opaqueAliasDef.targetType()));
  }
}

} // namespace

void TypeSystemBuilder::addType(
    Uri uri,
    const SerializableStructDefinition& structDef,
    std::optional<SerializableThriftSourceInfo> sourceInfo) {
  validateIdentitiesAreUnique(uri, *structDef.fields());

  DefinitionEntry entry;
  entry.definition.set_structDef(std::move(structDef));
  entry.sourceInfo = std::move(sourceInfo);
  tryEmplace(std::move(uri), std::move(entry));
}

void TypeSystemBuilder::addType(
    Uri uri,
    const SerializableUnionDefinition& unionDef,
    std::optional<SerializableThriftSourceInfo> sourceInfo) {
  validateIdentitiesAreUnique(uri, *unionDef.fields());
  validateFieldsAreOptional(uri, unionDef);

  DefinitionEntry entry;
  entry.definition.set_unionDef(std::move(unionDef));
  entry.sourceInfo = std::move(sourceInfo);
  tryEmplace(std::move(uri), std::move(entry));
}

void TypeSystemBuilder::addType(
    Uri uri,
    const SerializableEnumDefinition& enumDef,
    std::optional<SerializableThriftSourceInfo> sourceInfo) {
  validateEnumMappingsAreUnique(uri, enumDef);

  DefinitionEntry entry;
  entry.definition.set_enumDef(std::move(enumDef));
  entry.sourceInfo = std::move(sourceInfo);
  tryEmplace(std::move(uri), std::move(entry));
}

void TypeSystemBuilder::addType(
    Uri uri,
    const SerializableOpaqueAliasDefinition& opaqueAliasDef,
    std::optional<SerializableThriftSourceInfo> sourceInfo) {
  validateOpaqueAliasIsNotUserDefined(uri, opaqueAliasDef);

  DefinitionEntry entry;
  entry.definition.set_opaqueAliasDef(std::move(opaqueAliasDef));
  entry.sourceInfo = std::move(sourceInfo);
  tryEmplace(std::move(uri), std::move(entry));
}

void TypeSystemBuilder::addType(
    Uri uri, const SerializableTypeDefinitionEntry& typeDefinitionEntry) {
  const SerializableTypeDefinition& def = *typeDefinitionEntry.definition();
  std::optional<SerializableThriftSourceInfo> sourceInfo =
      typeDefinitionEntry.sourceInfo().to_optional();
  switch (def.getType()) {
    case SerializableTypeDefinition::Type::structDef:
      addType(uri, *def.structDef(), std::move(sourceInfo));
      break;
    case SerializableTypeDefinition::Type::unionDef:
      addType(uri, *def.unionDef(), std::move(sourceInfo));
      break;
    case SerializableTypeDefinition::Type::enumDef:
      addType(uri, *def.enumDef(), std::move(sourceInfo));
      break;
    case SerializableTypeDefinition::Type::opaqueAliasDef:
      addType(uri, *def.opaqueAliasDef(), std::move(sourceInfo));
      break;
    default:
      throw InvalidTypeError(fmt::format(
          "Invalid SerializableTypeDefinition::Type: {}",
          apache::thrift::util::enumNameSafe(def.getType())));
  }
}

void TypeSystemBuilder::addTypes(SerializableTypeSystem typeSystemDef) {
  for (auto& [uri, entry] : *typeSystemDef.types()) {
    addType(uri, entry);
  }
}

void TypeSystemBuilder::tryEmplace(Uri uri, DefinitionEntry&& def) {
  auto [_, inserted] = definitions_.try_emplace(std::move(uri), std::move(def));
  if (!inserted) {
    throw InvalidTypeError(
        fmt::format("Duplicate definition for Uri '{}'", uri));
  }
}

/* static */ FieldIdentity TypeSystemBuilder::DefinitionHelper::Identity(
    std::int16_t id, std::string_view name) {
  return FieldIdentity{FieldId{id}, std::string(name)};
}

/* static */ SerializableFieldDefinition
TypeSystemBuilder::DefinitionHelper::Field(
    FieldIdentity identity,
    PresenceQualifier presence,
    TypeId type,
    std::optional<SerializableRecord> customDefault,
    const AnnotationsMap& annotations) {
  SerializableFieldDefinition def;
  def.identity() = std::move(identity);
  def.presence() = presence;
  def.type() = type;
  if (customDefault.has_value()) {
    def.customDefaultPartialRecord() =
        SerializableRecord::toThrift(*customDefault);
  }
  def.annotations() = detail::toRawAnnotations(annotations);
  return def;
}

/* static */ SerializableStructDefinition
TypeSystemBuilder::DefinitionHelper::Struct(
    std::vector<SerializableFieldDefinition> fields,
    bool isSealed,
    const AnnotationsMap& annotations) {
  SerializableStructDefinition def;
  def.fields() = fields;
  def.isSealed() = isSealed;
  def.annotations() = detail::toRawAnnotations(annotations);
  return def;
}

/* static */ SerializableUnionDefinition
TypeSystemBuilder::DefinitionHelper::Union(
    std::vector<SerializableFieldDefinition> fields,
    bool isSealed,
    const AnnotationsMap& annotations) {
  SerializableUnionDefinition def;
  def.fields() = fields;
  def.isSealed() = isSealed;
  def.annotations() = detail::toRawAnnotations(annotations);
  return def;
}

/* static */ SerializableEnumDefinition
TypeSystemBuilder::DefinitionHelper::Enum(
    const std::vector<EnumValue>& values, const AnnotationsMap& annotations) {
  SerializableEnumDefinition enumDef;
  for (auto& [name, value, enumValueAnnotations] : values) {
    SerializableEnumValueDefinition v;
    v.name() = name;
    v.datum() = value;
    v.annotations() = detail::toRawAnnotations(enumValueAnnotations);
    enumDef.values()->emplace_back(std::move(v));
  }
  enumDef.annotations() = detail::toRawAnnotations(annotations);
  return enumDef;
}

/* static */ SerializableOpaqueAliasDefinition
TypeSystemBuilder::DefinitionHelper::OpaqueAlias(
    TypeId targetType, const AnnotationsMap& annotations) {
  SerializableOpaqueAliasDefinition def;
  def.targetType() = targetType;
  def.annotations() = detail::toRawAnnotations(annotations);
  return def;
}

/* static */ SerializableThriftSourceInfo
TypeSystemBuilder::DefinitionHelper::SourceInfo(
    std::string_view location, std::string_view name) {
  SerializableThriftSourceInfo entry;
  entry.locator() = std::string(location);
  entry.name() = std::string(name);
  return entry;
}

} // namespace apache::thrift::type_system
