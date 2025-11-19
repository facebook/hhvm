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

namespace apache::thrift::type_system {

std::unique_ptr<SerializableTypeSystem>
SerializableTypeSystemBuilder::build() && {
  return std::make_unique<SerializableTypeSystem>(
      std::move(serializableTypeSystem_));
}

void SerializableTypeSystemBuilder::addType(TypeRef ref) {
  ref.visit(
      [&](const StructNode& structDef) { addDefinition(structDef.uri()); },
      [&](const UnionNode& unionDef) { addDefinition(unionDef.uri()); },
      [&](const EnumNode& enumDef) { addDefinition(enumDef.uri()); },
      [&](const OpaqueAliasNode& opaqueAliasDef) {
        addType(opaqueAliasDef.targetType());
      },
      [&](const TypeRef::List& list) { addType(list.elementType()); },
      [&](const TypeRef::Set& set) { addType(set.elementType()); },
      [&](const TypeRef::Map& map) {
        addType(map.keyType());
        addType(map.valueType());
      },
      [](const auto&) {});
}

std::vector<SerializableFieldDefinition>
SerializableTypeSystemBuilder::toSerializableField(
    folly::span<const FieldDefinition> fields) {
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

    // Add dependent definitions.
    addType(field.type());
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
  addDefinition(*type);
}

void SerializableTypeSystemBuilder::addDefinition(DefinitionRef ref) {
  auto ret = serializableTypeSystem_.types()->try_emplace(ref.uri());
  if (!ret.second) {
    return;
  }
  auto& entry = ret.first->second;
  entry.definition() = ref.visit(
      [&](const StructNode& node) {
        SerializableTypeDefinition result;
        auto& structDef = result.structDef().ensure();
        structDef.fields() = toSerializableField(node.fields());
        structDef.isSealed() = node.isSealed();
        structDef.annotations() = detail::toRawAnnotations(node.annotations());
        addAnnotations(*structDef.annotations());
        return result;
      },
      [&](const UnionNode& node) {
        SerializableTypeDefinition result;
        auto& unionDef = result.unionDef().ensure();
        unionDef.fields() = toSerializableField(node.fields());
        unionDef.isSealed() = node.isSealed();
        unionDef.annotations() = detail::toRawAnnotations(node.annotations());
        addAnnotations(*unionDef.annotations());
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
          addAnnotations(*enumValue.annotations());
        }
        enumDef.annotations() = detail::toRawAnnotations(node.annotations());
        addAnnotations(*enumDef.annotations());
        return result;
      },
      [&](const OpaqueAliasNode& node) {
        SerializableTypeDefinition result;
        auto& opaqueAliasDef = result.opaqueAliasDef().ensure();
        opaqueAliasDef.targetType() = node.targetType().id();
        opaqueAliasDef.annotations() =
            detail::toRawAnnotations(node.annotations());
        addAnnotations(*opaqueAliasDef.annotations());
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
}

void SerializableTypeSystemBuilder::addAnnotations(
    const detail::RawAnnotations& annotations) {
  for (const auto& [uri, _] : annotations) {
    // Standard annotations are not currently bundled due to circular dependency
    // concerns. Skip it for now.
    if (uri.starts_with("facebook.com/thrift/annotation/")) {
      continue;
    }
    addDefinition(uri);
  }
}

} // namespace apache::thrift::type_system
