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

#include <folly/Overload.h>
#include <folly/container/F14Set.h>
#include <folly/container/MapUtil.h>
#include <folly/container/span.h>
#include <folly/lang/Assume.h>
#include <folly/lang/SafeAssert.h>

#include <functional>
#include <utility>
#include <variant>

namespace apache::thrift::type_system {

namespace {

using TSDefinition =
    std::variant<StructNode, UnionNode, EnumNode, OpaqueAliasNode>;

class TypeSystemImpl final : public TypeSystem {
 private:
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

  folly::F14FastSet<Uri> getKnownUris() const final {
    folly::F14FastSet<Uri> result;
    result.reserve(definitions.size());
    for (const auto& [uri, _] : definitions) {
      result.insert(uri);
    }
    return result;
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
          return TypeRef(TypeRef::List(typeOf(list.elementType())));
        },
        [&](const TypeId::Set& set) -> TypeRef {
          return TypeRef(TypeRef::Set(typeOf(set.elementType())));
        },
        [&](const TypeId::Map& map) -> TypeRef {
          return TypeRef(
              TypeRef::Map(typeOf(map.keyType()), typeOf(map.valueType())));
        },
        [](const auto& primitive) -> TypeRef { return TypeRef(primitive); });
  }
};

} // namespace

std::unique_ptr<TypeSystem> TypeSystemBuilder::build() && {
  auto typeSystem = std::make_unique<TypeSystemImpl>();

  // Fill in definitions with uninitialized stubs
  for (auto& entry : definitions_) {
    const Uri& uri = entry.first;
    SerializableTypeDefinition& def = entry.second;
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
      -> std::vector<FieldNode> {
    std::vector<FieldNode> result;
    result.reserve(fields.size());
    for (auto& field : fields) {
      result.emplace_back(
          std::move(*field.identity()),
          *field.presence(),
          typeSystem->typeOf(*field.type()),
          field.customDefaultValue().has_value()
              ? std::optional{SerializableRecord::fromThrift(
                    std::move(*field.customDefaultValue()))}
              : std::nullopt,
          makeAnnots(std::move(*field.annotations())));
    }
    return result;
  };

  for (auto& entry : definitions_) {
    const Uri& uri = entry.first;
    SerializableTypeDefinition& def = entry.second;
    // We created uninitialized stubs above so we can assume they exist
    TSDefinition& uninitDef = typeSystem->definitions.find(uri)->second;

    switch (def.getType()) {
      case SerializableTypeDefinition::Type::structDef: {
        SerializableStructDefinition& structDef = *def.structDef_ref();
        std::get<StructNode>(uninitDef) = StructNode(
            uri,
            makeFields(std::move(*structDef.fields())),
            *structDef.isSealed(),
            makeAnnots(std::move(*structDef.annotations())));
      } break;
      case SerializableTypeDefinition::Type::unionDef: {
        SerializableUnionDefinition& unionDef = *def.unionDef_ref();
        std::get<UnionNode>(uninitDef) = UnionNode(
            uri,
            makeFields(std::move(*unionDef.fields())),
            *unionDef.isSealed(),
            makeAnnots(std::move(*unionDef.annotations())));
      } break;
      case SerializableTypeDefinition::Type::enumDef: {
        SerializableEnumDefinition& enumDef = *def.enumDef_ref();
        std::vector<EnumNode::Value> values;
        values.reserve(enumDef.values()->size());
        for (SerializableEnumValueDefinition& mapping : *enumDef.values()) {
          values.emplace_back(EnumNode::Value{
              std::move(*mapping.name()),
              *mapping.datum(),
              makeAnnots(std::move(*mapping.annotations()))});
        }
        std::get<EnumNode>(uninitDef) = EnumNode(
            uri,
            std::move(values),
            makeAnnots(std::move(*enumDef.annotations())));
      } break;
      case SerializableTypeDefinition::Type::opaqueAliasDef: {
        SerializableOpaqueAliasDefinition& opaqueAliasDef =
            *def.opaqueAliasDef_ref();
        std::get<OpaqueAliasNode>(uninitDef) = OpaqueAliasNode(
            uri,
            typeSystem->typeOf(*opaqueAliasDef.targetType()),
            makeAnnots(std::move(*opaqueAliasDef.annotations())));
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
    Uri uri, SerializableStructDefinition structDef) {
  validateIdentitiesAreUnique(uri, *structDef.fields());

  SerializableTypeDefinition def;
  def.set_structDef(std::move(structDef));
  tryEmplace(std::move(uri), std::move(def));
}

void TypeSystemBuilder::addType(Uri uri, SerializableUnionDefinition unionDef) {
  validateIdentitiesAreUnique(uri, *unionDef.fields());
  validateFieldsAreOptional(uri, unionDef);

  SerializableTypeDefinition def;
  def.set_unionDef(std::move(unionDef));
  tryEmplace(std::move(uri), std::move(def));
}

void TypeSystemBuilder::addType(Uri uri, SerializableEnumDefinition enumDef) {
  validateEnumMappingsAreUnique(uri, enumDef);

  SerializableTypeDefinition def;
  def.set_enumDef(std::move(enumDef));
  tryEmplace(std::move(uri), std::move(def));
}

void TypeSystemBuilder::addType(
    Uri uri, SerializableOpaqueAliasDefinition opaqueAliasDef) {
  validateOpaqueAliasIsNotUserDefined(uri, opaqueAliasDef);

  SerializableTypeDefinition def;
  def.set_opaqueAliasDef(std::move(opaqueAliasDef));
  tryEmplace(std::move(uri), std::move(def));
}

void TypeSystemBuilder::addTypes(SerializableTypeSystem typeSystemDef) {
  for (auto& [uri, entry] : *typeSystemDef.types()) {
    const SerializableTypeDefinition& def = *entry.definition();
    switch (def.getType()) {
      case SerializableTypeDefinition::Type::structDef:
        addType(uri, std::move(*def.structDef_ref()));
        break;
      case SerializableTypeDefinition::Type::unionDef:
        addType(uri, std::move(*def.unionDef_ref()));
        break;
      case SerializableTypeDefinition::Type::enumDef:
        addType(uri, std::move(*def.enumDef_ref()));
        break;
      case SerializableTypeDefinition::Type::opaqueAliasDef:
        addType(uri, std::move(*def.opaqueAliasDef_ref()));
        break;
      default:
        folly::assume_unreachable();
        break;
    }
  }
}

void TypeSystemBuilder::tryEmplace(Uri uri, SerializableTypeDefinition&& def) {
  auto [_, inserted] = definitions_.emplace(uri, std::move(def));
  if (!inserted) {
    throw InvalidTypeError(
        fmt::format("Duplicate definition for Uri '{}'", uri));
  }
}

} // namespace apache::thrift::type_system
