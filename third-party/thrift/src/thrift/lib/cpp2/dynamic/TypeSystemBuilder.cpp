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
#include <thrift/lib/cpp2/dynamic/detail/TypeSystemImpl.h>
#include <thrift/lib/cpp2/dynamic/detail/TypeSystemUtils.h>

#include <folly/Overload.h>
#include <folly/container/F14Set.h>
#include <folly/container/MapUtil.h>
#include <folly/container/span.h>

#include <functional>
#include <utility>
#include <variant>

namespace apache::thrift::type_system {

using detail::DefinitionEntry;
using detail::TSDefinition;
using detail::TypeSystemImpl;

namespace {

// A helper to collect all URIs that are transitively referenced by a set of
// root URIs.
class TypeSystemURICollector {
 public:
  explicit TypeSystemURICollector(const TypeSystem& source) : source_(source) {}

  void addRoot(UriView uri) {
    auto def = source_.getUserDefinedType(uri);
    if (!def.has_value()) {
      throw InvalidTypeError(
          fmt::format(
              "Type with URI '{}' is not defined in this TypeSystem.", uri));
    }
    forEachTransitiveDependency(source_, *def, [&](DefinitionRef ref) {
      return visitedUris_.insert(std::string(ref.uri())).second;
    });
  }

  const folly::F14FastSet<Uri>& uris() const { return visitedUris_; }

 private:
  const TypeSystem& source_;
  folly::F14FastSet<Uri> visitedUris_;
};

} // namespace

std::unique_ptr<TypeSystem> TypeSystemBuilder::build() && {
  return std::move(*this).buildDerivedFrom(nullptr);
}

std::unique_ptr<TypeSystem> TypeSystemBuilder::buildDerivedFrom(
    std::shared_ptr<const TypeSystem> base) && {
  // Detect conflicts with base TypeSystem
  if (base) {
    for (const auto& [uri, entry] : definitions_) {
      // Check URI conflicts
      if (base->getUserDefinedType(uri).has_value()) {
        throw InvalidTypeError(
            fmt::format(
                "Type '{}' conflicts with existing definition in base TypeSystem",
                uri));
      }
      // Check source identifier conflicts
      if (entry.sourceInfo.has_value()) {
        SourceIdentifierView sourceId{
            *entry.sourceInfo->locator(), *entry.sourceInfo->name()};
        if (base->getUserDefinedTypeBySourceIdentifier(sourceId).has_value()) {
          throw InvalidTypeError(
              fmt::format(
                  "Source identifier '{}' at location '{}' conflicts with "
                  "existing definition in base TypeSystem",
                  sourceId.name,
                  sourceId.location));
        }
      }
    }
  }

  auto typeSystem = std::make_unique<TypeSystemImpl>(std::move(base));
  typeSystem->insertDefinitions(std::move(definitions_));
  return typeSystem;
}

/* static */ std::unique_ptr<TypeSystem> TypeSystemBuilder::buildPrunedFrom(
    const TypeSystem& source,
    std::span<const UriView> rootUris,
    PruneOptions options) {
  TypeSystemURICollector uriCollector(source);
  for (const auto& uri : rootUris) {
    uriCollector.addRoot(uri);
  }

  const auto& uris = uriCollector.uris();

  // Create a TypeSystem with stub nodes for each required URI.
  // This is needed to address circular dependencies.
  auto typeSystem = std::make_unique<TypeSystemImpl>(nullptr);

  for (const auto& uri : uris) {
    auto sourceDef = source.getUserDefinedType(uri);
    auto stubDef = sourceDef->visit(
        [&](const StructNode& node) -> TSDefinition {
          return StructNode{uri, {}, false, {}, std::string(node.debugName())};
        },
        [&](const UnionNode& node) -> TSDefinition {
          return UnionNode{uri, {}, false, {}, std::string(node.debugName())};
        },
        [&](const EnumNode& node) -> TSDefinition {
          return EnumNode{uri, {}, {}, std::string(node.debugName())};
        },
        [&](const OpaqueAliasNode& node) -> TSDefinition {
          return OpaqueAliasNode{
              uri, TypeRef{TypeRef::Bool{}}, {}, std::string(node.debugName())};
        });
    typeSystem->definitions().emplace(uri, std::move(stubDef));
  }

  // Remap a source TypeRef to point at the new TypeSystem's nodes.
  const auto remapType = [&](const TypeRef& sourceRef) -> TypeRef {
    return typeSystem->typeOf(sourceRef.id());
  };

  // Copy annotations, remapping annotation structs.
  const auto copyAnnotations =
      [&](const AnnotationsMap& sourceAnnots) -> AnnotationsMap {
    AnnotationsMap result;
    result.reserve(sourceAnnots.size());
    for (const auto& [uri, record] : sourceAnnots) {
      result.emplace(uri, record);
    }
    return result;
  };

  // Copy fields from source, remapping TypeRefs.
  const auto copyFields = [&](std::span<const FieldDefinition> sourceFields)
      -> std::vector<FieldDefinition> {
    std::vector<FieldDefinition> result;
    result.reserve(sourceFields.size());
    for (const auto& field : sourceFields) {
      result.emplace_back(
          field.identity(),
          field.presence(),
          remapType(field.type()),
          field.customDefault()
              ? std::optional<SerializableRecord>{*field.customDefault()}
              : std::nullopt,
          copyAnnotations(field.annotations()));
    }
    return result;
  };

  // Fill in nodes from source definitions
  for (const auto& uri : uris) {
    auto sourceDef = source.getUserDefinedType(uri);
    TSDefinition& stubDef = typeSystem->definitions().find(uri)->second;

    sourceDef->visit(
        [&](const StructNode& node) {
          auto& structNode = std::get<StructNode>(stubDef);
          structNode = StructNode(
              uri,
              copyFields(node.fields()),
              node.isSealed(),
              copyAnnotations(node.annotations()),
              std::string(node.debugName()));
          if (options.includeSourceInfo) {
            auto sourceInfo =
                source.getSourceIdentiferForUserDefinedType(*sourceDef);
            if (sourceInfo.has_value()) {
              typeSystem->tryAddToSourceIndex(
                  SourceIdentifier{
                      std::string(sourceInfo->location),
                      std::string(sourceInfo->name)},
                  DefinitionRef(&structNode));
            }
          }
        },
        [&](const UnionNode& node) {
          auto& unionNode = std::get<UnionNode>(stubDef);
          unionNode = UnionNode(
              uri,
              copyFields(node.fields()),
              node.isSealed(),
              copyAnnotations(node.annotations()),
              std::string(node.debugName()));
          if (options.includeSourceInfo) {
            auto sourceInfo =
                source.getSourceIdentiferForUserDefinedType(*sourceDef);
            if (sourceInfo.has_value()) {
              typeSystem->tryAddToSourceIndex(
                  SourceIdentifier{
                      std::string(sourceInfo->location),
                      std::string(sourceInfo->name)},
                  DefinitionRef(&unionNode));
            }
          }
        },
        [&](const EnumNode& node) {
          std::vector<EnumNode::Value> values;
          values.reserve(node.values().size());
          for (const auto& v : node.values()) {
            values.push_back(
                EnumNode::Value{
                    v.name, v.i32, copyAnnotations(v.annotations())});
          }
          auto& enumNode = std::get<EnumNode>(stubDef);
          enumNode = EnumNode(
              uri,
              std::move(values),
              copyAnnotations(node.annotations()),
              std::string(node.debugName()));
          if (options.includeSourceInfo) {
            auto sourceInfo =
                source.getSourceIdentiferForUserDefinedType(*sourceDef);
            if (sourceInfo.has_value()) {
              typeSystem->tryAddToSourceIndex(
                  SourceIdentifier{
                      std::string(sourceInfo->location),
                      std::string(sourceInfo->name)},
                  DefinitionRef(&enumNode));
            }
          }
        },
        [&](const OpaqueAliasNode& node) {
          auto& opaqueAliasNode = std::get<OpaqueAliasNode>(stubDef);
          opaqueAliasNode = OpaqueAliasNode(
              uri,
              remapType(node.targetType()),
              copyAnnotations(node.annotations()),
              std::string(node.debugName()));
          if (options.includeSourceInfo) {
            auto sourceInfo =
                source.getSourceIdentiferForUserDefinedType(*sourceDef);
            if (sourceInfo.has_value()) {
              typeSystem->tryAddToSourceIndex(
                  SourceIdentifier{
                      std::string(sourceInfo->location),
                      std::string(sourceInfo->name)},
                  DefinitionRef(&opaqueAliasNode));
            }
          }
        });
  }

  return typeSystem;
}

/* static */ std::unique_ptr<TypeSystem> TypeSystemBuilder::buildPrunedFrom(
    const TypeSystem& source,
    std::span<const DefinitionRef> rootDefs,
    PruneOptions options) {
  std::vector<UriView> uris;
  uris.reserve(rootDefs.size());
  for (const auto& ref : rootDefs) {
    uris.push_back(ref.uri());
  }
  return buildPrunedFrom(source, uris, options);
}

void TypeSystemBuilder::addType(
    Uri uri,
    const SerializableStructDefinition& structDef,
    std::optional<SerializableThriftSourceInfo> sourceInfo) {
  detail::validateIdentitiesAreUnique(uri, *structDef.fields());

  detail::DefinitionEntry entry;
  entry.definition.set_structDef(std::move(structDef));
  entry.sourceInfo = std::move(sourceInfo);
  tryEmplace(std::move(uri), std::move(entry));
}

void TypeSystemBuilder::addType(
    Uri uri,
    const SerializableUnionDefinition& unionDef,
    std::optional<SerializableThriftSourceInfo> sourceInfo) {
  detail::validateIdentitiesAreUnique(uri, *unionDef.fields());
  detail::validateFieldsAreOptional(uri, unionDef);

  detail::DefinitionEntry entry;
  entry.definition.set_unionDef(std::move(unionDef));
  entry.sourceInfo = std::move(sourceInfo);
  tryEmplace(std::move(uri), std::move(entry));
}

void TypeSystemBuilder::addType(
    Uri uri,
    const SerializableEnumDefinition& enumDef,
    std::optional<SerializableThriftSourceInfo> sourceInfo) {
  detail::validateEnumMappingsAreUnique(uri, enumDef);

  detail::DefinitionEntry entry;
  entry.definition.set_enumDef(std::move(enumDef));
  entry.sourceInfo = std::move(sourceInfo);
  tryEmplace(std::move(uri), std::move(entry));
}

void TypeSystemBuilder::addType(
    Uri uri,
    const SerializableOpaqueAliasDefinition& opaqueAliasDef,
    std::optional<SerializableThriftSourceInfo> sourceInfo) {
  detail::validateOpaqueAliasIsNotUserDefined(uri, opaqueAliasDef);

  detail::DefinitionEntry entry;
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
      throw InvalidTypeError(
          fmt::format(
              "Invalid SerializableTypeDefinition::Type: {}",
              apache::thrift::util::enumNameSafe(def.getType())));
  }
}

void TypeSystemBuilder::addTypes(SerializableTypeSystem typeSystemDef) {
  for (auto& [uri, entry] : *typeSystemDef.types()) {
    addType(uri, entry);
  }
}

std::unique_ptr<TypeSystem> fromSerializable(
    SerializableTypeSystem typeSystemDef) {
  TypeSystemBuilder builder;
  builder.addTypes(std::move(typeSystemDef));
  return std::move(builder).build();
}

void TypeSystemBuilder::tryEmplace(Uri uri, detail::DefinitionEntry&& def) {
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
