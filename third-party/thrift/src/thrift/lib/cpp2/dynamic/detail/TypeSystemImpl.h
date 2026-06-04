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

#include <thrift/lib/cpp2/dynamic/SerializableRecord.h>
#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/thrift/gen-cpp2/type_system_types.h>

#include <folly/Overload.h>
#include <folly/Utility.h>
#include <folly/container/F14Map.h>
#include <folly/container/F14Set.h>
#include <folly/container/MapUtil.h>
#include <folly/container/span.h>
#include <folly/lang/Assume.h>

#include <fmt/core.h>

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

// WARNING: This code is highly experimental.
// DO NOT USE for any production code.
//
// This header contains the shared, flat, owning implementation of the
// TypeSystem interface (TypeSystemImpl) along with the machinery to populate
// it from serializable definitions. It is shared between TypeSystemBuilder
// (which produces standalone/derived TypeSystems) and AccumulatingTypeSystem
// (which appends to a single instance over time).
namespace apache::thrift::type_system::detail {

using TSDefinition =
    std::variant<StructNode, UnionNode, EnumNode, OpaqueAliasNode>;

/**
 * A serializable definition paired with its optional source info, as collected
 * prior to materialization into runtime nodes.
 */
struct DefinitionEntry {
  SerializableTypeDefinition definition;
  std::optional<SerializableThriftSourceInfo> sourceInfo;
};

/**
 * For structured types, both field ids AND names must be unique.
 */
inline void validateIdentitiesAreUnique(
    UriView uri, folly::span<const SerializableFieldDefinition> fields) {
  folly::F14FastSet<FieldId> seenIds;
  folly::F14FastSet<FieldName> seenNames;

  for (const auto& field : fields) {
    if (seenIds.contains(field.identity()->id())) {
      throw InvalidTypeError(
          fmt::format(
              "Duplicate field id '{}' in structured type '{}'",
              folly::to_underlying(field.identity()->id()),
              uri));
    }
    seenIds.insert(field.identity()->id());

    if (seenNames.contains(field.identity()->name())) {
      throw InvalidTypeError(
          fmt::format(
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
inline void validateFieldsAreOptional(
    UriView uri, const SerializableUnionDefinition& unionDef) {
  for (const SerializableFieldDefinition& field : *unionDef.fields()) {
    if (field.presence() != PresenceQualifier::OPTIONAL_) {
      throw InvalidTypeError(
          fmt::format(
              "field '{}' must be optional in union '{}'",
              field.identity()->name(),
              uri));
    }
  }
}

/**
 * For enums, both enum names AND values must be unique.
 */
inline void validateEnumMappingsAreUnique(
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
      throw InvalidTypeError(
          fmt::format(
              "Duplicate value '{}' in enum '{}'", *entry.datum(), uri));
    }
    seenValues.insert(*entry.datum());
  }
}

/**
 * Opaque aliases are not allowed to have a target type which is a user-defined
 * type.
 */
inline void validateOpaqueAliasIsNotUserDefined(
    UriView uri, const SerializableOpaqueAliasDefinition& opaqueAliasDef) {
  if (opaqueAliasDef.targetType()->kind() == TypeId::Kind::URI) {
    throw InvalidTypeError(
        fmt::format(
            "Opaque alias '{}' cannot have target type of a user-defined type '{}'",
            uri,
            *opaqueAliasDef.targetType()));
  }
}

/**
 * Runs the appropriate structural validation for a single serializable
 * definition (field/enum uniqueness, union optionality, opaque alias target).
 */
inline void validateDefinition(
    UriView uri, const SerializableTypeDefinition& def) {
  switch (def.getType()) {
    case SerializableTypeDefinition::Type::structDef:
      validateIdentitiesAreUnique(uri, *def.structDef()->fields());
      break;
    case SerializableTypeDefinition::Type::unionDef:
      validateIdentitiesAreUnique(uri, *def.unionDef()->fields());
      validateFieldsAreOptional(uri, *def.unionDef());
      break;
    case SerializableTypeDefinition::Type::enumDef:
      validateEnumMappingsAreUnique(uri, *def.enumDef());
      break;
    case SerializableTypeDefinition::Type::opaqueAliasDef:
      validateOpaqueAliasIsNotUserDefined(uri, *def.opaqueAliasDef());
      break;
    case SerializableTypeDefinition::Type::__EMPTY__:
      break;
  }
}

/**
 * The flat, owning implementation of the TypeSystem interface.
 *
 * Concrete definition nodes are stored in a folly::F14NodeMap, which is
 * reference-stable across insertions. This means that DefinitionRef and TypeRef
 * objects (which hold raw pointers to nodes) remain valid even as new types are
 * appended via insertDefinitions(). Nodes are never relocated, only added.
 *
 * Optionally delegates to a base TypeSystem (the "overlay" relationship used by
 * TypeSystemBuilder::buildDerivedFrom).
 */
class TypeSystemImpl final : public TypeSystem {
 public:
  explicit TypeSystemImpl(std::shared_ptr<const TypeSystem> base = nullptr)
      : baseTypeSystem_(std::move(base)) {}

  // Sets (or replaces) the base TypeSystem. Intended to be called before any
  // definitions are inserted; the base participates in reference resolution and
  // lookup fallback.
  void setBase(std::shared_ptr<const TypeSystem> base) {
    baseTypeSystem_ = std::move(base);
  }

  // Returns the base TypeSystem, or nullptr if none is set.
  const TypeSystem* base() const { return baseTypeSystem_.get(); }

  std::optional<DefinitionRef> getUserDefinedType(UriView uri) const final {
    if (auto def = definitions_.find(uri); def != definitions_.end()) {
      return folly::variant_match(
          def->second, [](auto& d) { return DefinitionRef(&d); });
    }
    if (baseTypeSystem_) {
      return baseTypeSystem_->getUserDefinedType(uri);
    }
    return std::nullopt;
  }

  std::optional<folly::F14FastSet<Uri>> getKnownUris() const final {
    folly::F14FastSet<Uri> result;
    result.reserve(definitions_.size());
    for (const auto& [uri, _] : definitions_) {
      result.insert(uri);
    }
    if (baseTypeSystem_) {
      if (auto baseUris = baseTypeSystem_->getKnownUris()) {
        result.insert(baseUris->begin(), baseUris->end());
      } else {
        return std::nullopt; // Base is not enumerable
      }
    }
    return result;
  }

  void tryAddToSourceIndex(
      SourceIdentifier sourceIdentifier, DefinitionRef def) {
    {
      auto [_, inserted] =
          sourceIndexedDefinitions[sourceIdentifier.location].emplace(
              sourceIdentifier.name, def);
      if (!inserted) {
        throw InvalidTypeError(
            fmt::format(
                "Duplicate source identifier name '{}' at location '{}'",
                sourceIdentifier.name,
                sourceIdentifier.location));
      }
    }

    {
      auto [_, inserted] =
          definitionToSourceIdentifier.emplace(def, sourceIdentifier);
      if (!inserted) {
        throw InvalidTypeError(
            fmt::format(
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
      if (auto result = folly::get_optional<std::optional>(
              *atLocation, sourceIdentifier.name)) {
        return result;
      }
    }
    if (baseTypeSystem_) {
      return baseTypeSystem_->getUserDefinedTypeBySourceIdentifier(
          sourceIdentifier);
    }
    return std::nullopt;
  }

  std::optional<SourceIdentifierView> getSourceIdentiferForUserDefinedType(
      DefinitionRef ref) const final {
    const SourceIdentifier* sourceIdentifier =
        folly::get_ptr(definitionToSourceIdentifier, ref);
    if (sourceIdentifier) {
      return *sourceIdentifier;
    }
    if (baseTypeSystem_) {
      return baseTypeSystem_->getSourceIdentiferForUserDefinedType(ref);
    }
    return std::nullopt;
  }

  folly::F14FastMap<std::string, DefinitionRef> getUserDefinedTypesAtLocation(
      std::string_view location) const final {
    auto* localResult = folly::get_ptr(sourceIndexedDefinitions, location);
    if (!localResult) {
      // No local definitions at this location, delegate entirely to base
      if (baseTypeSystem_) {
        return baseTypeSystem_->getUserDefinedTypesAtLocation(location);
      }
      return {};
    }
    auto result = *localResult;
    if (baseTypeSystem_) {
      auto baseResult =
          baseTypeSystem_->getUserDefinedTypesAtLocation(location);
      result.insert(
          std::make_move_iterator(baseResult.begin()),
          std::make_move_iterator(baseResult.end()));
    }
    return result;
  }

  // Resolves a TypeId to a TypeRef against this instance's definitions (and its
  // base, if any). Recursive for container types.
  TypeRef typeOf(const TypeId& typeId) {
    return typeId.visit(
        [&](const Uri& uri) -> TypeRef {
          if (auto def = definitions_.find(uri); def != definitions_.end()) {
            return folly::variant_match(
                def->second, [](auto& d) { return TypeRef(d); });
          }
          if (baseTypeSystem_) {
            if (auto def = baseTypeSystem_->getUserDefinedType(uri)) {
              return TypeRef::fromDefinition(*def);
            }
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

  /**
   * Materializes a batch of serializable definitions into runtime nodes and
   * inserts them into this instance.
   *
   * Consumes `defs` (taken by rvalue reference): definition contents and
   * source-info strings are moved out of its entries, which are left in a valid
   * but unspecified state on return.
   *
   * Uses a two-phase stub-then-fill approach so that definitions within the
   * batch may reference each other (including cyclically). New definitions may
   * also reference types that already exist in this instance (from a previous
   * insertDefinitions() call) or in the base TypeSystem.
   *
   * Existing nodes are never modified or relocated, so any DefinitionRef /
   * TypeRef previously obtained from this instance remains valid.
   *
   * Preconditions:
   *   - Every URI in `defs` is new — i.e. not already present in this instance.
   *     Callers are responsible for deduplicating against existing definitions
   *     (and the base) before calling. Re-inserting an existing URI is a
   *     logic error and is ignored at the stub stage (the existing node wins),
   *     which would then be incorrectly overwritten at the fill stage.
   *   - Each definition has already passed structural validation
   *     (see validateDefinition).
   *
   * Throws:
   *   - InvalidTypeError if a referenced TypeId cannot be resolved, or if a
   *     source identifier collides with an existing one.
   */
  void insertDefinitions(folly::F14FastMap<Uri, DefinitionEntry>&& defs) {
    // Phase 1: insert uninitialized stub nodes for every URI in the batch so
    // that (possibly cyclic) references can be resolved in phase 2.
    for (auto& [uri, entry] : defs) {
      // A stub of the correct alternative only; its fields (including the debug
      // name) are overwritten in phase 2, so no strings are copied into it
      // here.
      auto uninitDef = std::invoke([&]() -> TSDefinition {
        switch (entry.definition.getType()) {
          case SerializableTypeDefinition::Type::structDef:
            return StructNode{uri, {}, {}, {}, {}};
          case SerializableTypeDefinition::Type::unionDef:
            return UnionNode{uri, {}, {}, {}, {}};
          case SerializableTypeDefinition::Type::enumDef:
            return EnumNode{uri, {}, {}, {}};
          case SerializableTypeDefinition::Type::opaqueAliasDef:
            return OpaqueAliasNode{uri, TypeRef{TypeRef::Bool{}}, {}, {}};
          case SerializableTypeDefinition::Type::__EMPTY__:
            break;
        }
        folly::assume_unreachable();
      });
      definitions_.emplace(uri, std::move(uninitDef));
    }

    const auto makeAnnots = [&](folly::F14FastMap<Uri, SerializableRecordUnion>
                                    annotations) {
      AnnotationsMap ret;
      ret.reserve(annotations.size());
      annotations.eraseInto(
          annotations.begin(), annotations.end(), [&](auto&& k, auto&& v) {
            // Validate uri exists in the type system and resolve to struct
            // type.
            auto typeref = typeOf(k);
            if (!typeref.isStruct()) {
              throw InvalidTypeError(
                  fmt::format(
                      "Definition for uri '{}' for annotation is not struct.",
                      k));
            }
            // Validate if the value is a struct.
            if (!v.fieldSetDatum_ref().has_value()) {
              throw InvalidTypeError(
                  fmt::format(
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
            typeOf(*field.type()),
            field.customDefaultPartialRecord().has_value()
                ? std::optional{SerializableRecord::fromThrift(
                      std::move(*field.customDefaultPartialRecord()))}
                : std::nullopt,
            makeAnnots(std::move(*field.annotations())));
      }
      return result;
    };

    // Phase 2: fill in the stub nodes by move-assigning the fully-resolved node
    // into the existing (stable) slot.
    for (auto& [uri, entry] : defs) {
      SerializableTypeDefinition& def = entry.definition;
      std::optional<SerializableThriftSourceInfo>& sourceInfo =
          entry.sourceInfo;
      // We created uninitialized stubs above so we can assume they exist.
      TSDefinition& uninitDef = definitions_.find(uri)->second;
      std::string defName = sourceInfo.has_value()
          ? std::string(*sourceInfo->name())
          : std::string{};

      switch (def.getType()) {
        case SerializableTypeDefinition::Type::structDef: {
          SerializableStructDefinition& structDef = *def.structDef();
          StructNode& structNode = std::get<StructNode>(uninitDef);
          structNode = StructNode(
              uri,
              makeFields(std::move(*structDef.fields())),
              *structDef.isSealed(),
              makeAnnots(std::move(*structDef.annotations())),
              std::move(defName));
          if (sourceInfo.has_value()) {
            tryAddToSourceIndex(
                SourceIdentifier{
                    std::move(*sourceInfo->locator()),
                    std::move(*sourceInfo->name())},
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
              makeAnnots(std::move(*unionDef.annotations())),
              std::move(defName));
          if (sourceInfo.has_value()) {
            tryAddToSourceIndex(
                SourceIdentifier{
                    std::move(*sourceInfo->locator()),
                    std::move(*sourceInfo->name())},
                DefinitionRef(&unionNode));
          }
        } break;
        case SerializableTypeDefinition::Type::enumDef: {
          SerializableEnumDefinition& enumDef = *def.enumDef();
          std::vector<EnumNode::Value> values;
          values.reserve(enumDef.values()->size());
          for (SerializableEnumValueDefinition& mapping : *enumDef.values()) {
            values.emplace_back(
                EnumNode::Value{
                    std::move(*mapping.name()),
                    *mapping.datum(),
                    makeAnnots(std::move(*mapping.annotations()))});
          }
          EnumNode& enumNode = std::get<EnumNode>(uninitDef);
          enumNode = EnumNode(
              uri,
              std::move(values),
              makeAnnots(std::move(*enumDef.annotations())),
              std::move(defName));
          if (sourceInfo.has_value()) {
            tryAddToSourceIndex(
                SourceIdentifier{
                    std::move(*sourceInfo->locator()),
                    std::move(*sourceInfo->name())},
                DefinitionRef(&enumNode));
          }
        } break;
        case SerializableTypeDefinition::Type::opaqueAliasDef: {
          SerializableOpaqueAliasDefinition& opaqueAliasDef =
              *def.opaqueAliasDef();
          OpaqueAliasNode& opaqueAliasNode =
              std::get<OpaqueAliasNode>(uninitDef);
          opaqueAliasNode = OpaqueAliasNode(
              uri,
              typeOf(*opaqueAliasDef.targetType()),
              makeAnnots(std::move(*opaqueAliasDef.annotations())),
              std::move(defName));
          if (sourceInfo.has_value()) {
            tryAddToSourceIndex(
                SourceIdentifier{
                    std::move(*sourceInfo->locator()),
                    std::move(*sourceInfo->name())},
                DefinitionRef(&opaqueAliasNode));
          }
        } break;
        case SerializableTypeDefinition::Type::__EMPTY__:
          break;
      }
    }
  }

  using DefinitionsMap = folly::
      F14NodeMap<Uri, TSDefinition, UriHeterogeneousHash, std::equal_to<>>;

  // Mutable access to the underlying definition map. Intended for builders that
  // populate nodes in place via two-phase (stub-then-fill) construction: the
  // map is reference-stable, so references obtained while stubbing stay valid
  // as the remaining definitions are inserted.
  DefinitionsMap& definitions() { return definitions_; }

 private:
  using Location = std::string;
  using DefinitionName = std::string;
  // Map of definition name (within one location) to definition
  using NameToDefinitionsMap = TypeSystem::NameToDefinitionsMap;
  // Map of location to all definitions at that location
  using LocationToDefinitionsMap =
      folly::F14FastMap<Location, NameToDefinitionsMap>;

  std::shared_ptr<const TypeSystem> baseTypeSystem_;
  DefinitionsMap definitions_;
  LocationToDefinitionsMap sourceIndexedDefinitions;
  folly::F14FastMap<DefinitionRef, SourceIdentifier>
      definitionToSourceIdentifier;
};

} // namespace apache::thrift::type_system::detail
