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

#include <thrift/lib/cpp2/dynamic/SerializableTypeSystemBuilder.h>

#include <thrift/lib/cpp2/dynamic/detail/TypeSystemUtils.h>

namespace apache::thrift::type_system {

std::unique_ptr<SerializableTypeSystem>
SerializableTypeSystemBuilder::build() && {
  return std::make_unique<SerializableTypeSystem>(
      std::move(serializableTypeSystem_));
}

std::vector<SerializableFieldDefinition>
SerializableTypeSystemBuilder::toSerializableField(
    std::span<const FieldDefinition> fields) {
  std::vector<SerializableFieldDefinition> result;
  result.reserve(fields.size());
  for (const auto& field : fields) {
    auto& fieldDef = result.emplace_back();
    fieldDef.identity() = field.identity();
    fieldDef.presence() = field.presence();
    fieldDef.type() = field.type().id();
    if (const auto* customDefault = field.customDefault()) {
      fieldDef.customDefaultPartialRecord() =
          SerializableRecord::toThrift(*customDefault);
    }
    fieldDef.annotations() = detail::toRawAnnotations(field.annotations());
  }
  return result;
}

void SerializableTypeSystemBuilder::addDefinition(UriView uri) {
  // TODO(dokwon): Support heterogeneous look up for type system.
  if (serializableTypeSystem_.types()->contains(std::string{uri})) {
    return;
  }

  auto type = typeSystem_->getUserDefinedType(uri);
  if (!type.has_value()) {
    throw InvalidTypeError(
        fmt::format(
            "Type with URI '{}' is not defined in this TypeSystem.", uri));
  }
  forEachTransitiveDependency(*typeSystem_, *type, [&](DefinitionRef ref) {
    return serializeDefinition(ref);
  });
}

bool SerializableTypeSystemBuilder::serializeDefinition(DefinitionRef ref) {
  auto ret = serializableTypeSystem_.types()->try_emplace(ref.uri());
  if (!ret.second) {
    return false;
  }
  auto& entry = ret.first->second;
  entry.definition() = ref.visit(
      [&](const StructNode& node) {
        SerializableTypeDefinition result;
        auto& structDef = result.structDef().ensure();
        structDef.fields() = toSerializableField(node.fields());
        structDef.isSealed() = node.isSealed();
        structDef.annotations() = detail::toRawAnnotations(node.annotations());
        return result;
      },
      [&](const UnionNode& node) {
        SerializableTypeDefinition result;
        auto& unionDef = result.unionDef().ensure();
        unionDef.fields() = toSerializableField(node.fields());
        unionDef.isSealed() = node.isSealed();
        unionDef.annotations() = detail::toRawAnnotations(node.annotations());
        return result;
      },
      [&](const EnumNode& node) {
        SerializableTypeDefinition result;
        auto& enumDef = result.enumDef().ensure();
        enumDef.values()->reserve(node.values().size());
        for (const auto& v : node.values()) {
          auto& enumValue = enumDef.values()->emplace_back();
          enumValue.name() = v.name;
          enumValue.datum() = v.i32;
          enumValue.annotations() = detail::toRawAnnotations(v.annotations());
        }
        enumDef.annotations() = detail::toRawAnnotations(node.annotations());
        return result;
      },
      [&](const OpaqueAliasNode& node) {
        SerializableTypeDefinition result;
        auto& opaqueAliasDef = result.opaqueAliasDef().ensure();
        opaqueAliasDef.targetType() = node.targetType().id();
        opaqueAliasDef.annotations() =
            detail::toRawAnnotations(node.annotations());
        return result;
      });

  if (withSourceInfo_) {
    auto sourceInfoView =
        typeSystem_->getSourceIdentiferForUserDefinedType(ref);
    if (sourceInfoView.has_value()) {
      auto& s = entry.sourceInfo().ensure();
      s.locator() = sourceInfoView->location;
      s.name() = sourceInfoView->name;
    };
  }
  return true;
}

/* static */ std::unique_ptr<SerializableTypeSystem>
SerializableTypeSystemBuilder::buildPrunedFrom(
    const TypeSystem& source,
    std::span<const UriView> rootUris,
    PruneOptions options) {
  auto builder = options.includeSourceInfo ? withSourceInfo(source)
                                           : withoutSourceInfo(source);
  for (const auto& uri : rootUris) {
    builder.addDefinition(uri);
  }
  return std::move(builder).build();
}

/* static */ std::unique_ptr<SerializableTypeSystem>
SerializableTypeSystemBuilder::buildPrunedFrom(
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

} // namespace apache::thrift::type_system
