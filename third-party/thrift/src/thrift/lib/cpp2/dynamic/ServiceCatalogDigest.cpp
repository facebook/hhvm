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

#include <thrift/lib/cpp2/dynamic/ServiceCatalogDigest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <map>
#include <span>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <vector>

#include <thrift/lib/cpp2/dynamic/Binary.h>
#include <thrift/lib/cpp2/dynamic/List.h>
#include <thrift/lib/cpp2/dynamic/Map.h>
#include <thrift/lib/cpp2/dynamic/SerializableTypeSystemBuilder.h>
#include <thrift/lib/cpp2/dynamic/Set.h>
#include <thrift/lib/cpp2/dynamic/String.h>
#include <thrift/lib/cpp2/dynamic/Struct.h>
#include <thrift/lib/cpp2/dynamic/Union.h>
#include <thrift/lib/cpp2/dynamic/detail/DigestHasher.h>
#include <thrift/lib/thrift/gen-cpp2/record_types.h>
#include <thrift/lib/thrift/gen-cpp2/service_catalog_types.h>

namespace apache::thrift::dynamic {
namespace {

template <typename Range, typename KeyFn, typename Fn>
void forEachSortedByKey(Range&& range, KeyFn&& keyFn, Fn&& fn) {
  using Elem = std::remove_reference_t<decltype(*std::begin(range))>;
  using Key = std::remove_cvref_t<std::invoke_result_t<KeyFn, const Elem&>>;
  std::map<Key, const Elem*> sorted;
  for (const auto& elem : range) {
    sorted.emplace(keyFn(elem), &elem);
  }
  for (const auto& [key, elem] : sorted) {
    fn(key, *elem);
  }
}

struct DigestContext {
  type_system::DigestMode mode = type_system::DigestMode::Full;

  bool includeAnnotationsAndDefaults() const {
    return mode == type_system::DigestMode::Full;
  }
};

void addTypeRefDefinitions(
    type_system::TypeRef type,
    type_system::SerializableTypeSystemBuilder& builder) {
  switch (type.kind()) {
    case type_system::TypeRef::Kind::STRUCT:
      builder.addDefinition(type.asStruct().uri());
      break;
    case type_system::TypeRef::Kind::UNION:
      builder.addDefinition(type.asUnion().uri());
      break;
    case type_system::TypeRef::Kind::ENUM:
      builder.addDefinition(type.asEnum().uri());
      break;
    case type_system::TypeRef::Kind::OPAQUE_ALIAS:
      builder.addDefinition(type.asOpaqueAlias().uri());
      break;
    case type_system::TypeRef::Kind::LIST:
      addTypeRefDefinitions(type.asList().elementType(), builder);
      break;
    case type_system::TypeRef::Kind::SET:
      addTypeRefDefinitions(type.asSet().elementType(), builder);
      break;
    case type_system::TypeRef::Kind::MAP:
      addTypeRefDefinitions(type.asMap().keyType(), builder);
      addTypeRefDefinitions(type.asMap().valueType(), builder);
      break;
    case type_system::TypeRef::Kind::BOOL:
    case type_system::TypeRef::Kind::BYTE:
    case type_system::TypeRef::Kind::I16:
    case type_system::TypeRef::Kind::I32:
    case type_system::TypeRef::Kind::I64:
    case type_system::TypeRef::Kind::FLOAT:
    case type_system::TypeRef::Kind::DOUBLE:
    case type_system::TypeRef::Kind::STRING:
    case type_system::TypeRef::Kind::BINARY:
    case type_system::TypeRef::Kind::ANY:
      break;
  }
}

void addAnnotationTypeDefinitions(
    std::span<const DynamicValue> annotations,
    type_system::SerializableTypeSystemBuilder& builder) {
  for (const auto& annotation : annotations) {
    const auto& uri = annotation.type().asStruct().uri();
    if (uri.starts_with("facebook.com/thrift/annotation/")) {
      continue;
    }
    builder.addDefinition(uri);
  }
}

void addFunctionTypeDefinitions(
    const ServiceDescriptor::Function& fn,
    type_system::SerializableTypeSystemBuilder& builder) {
  addAnnotationTypeDefinitions(fn.annotations, builder);
  for (const auto& param : fn.params) {
    addTypeRefDefinitions(param.type, builder);
    addAnnotationTypeDefinitions(param.annotations, builder);
  }
  if (fn.responseType) {
    addTypeRefDefinitions(*fn.responseType, builder);
  }
  for (const auto& ex : fn.exceptions) {
    addTypeRefDefinitions(ex.type, builder);
    addAnnotationTypeDefinitions(ex.annotations, builder);
  }
  if (fn.stream) {
    addTypeRefDefinitions(fn.stream->payloadType, builder);
    for (const auto& ex : fn.stream->exceptions) {
      addTypeRefDefinitions(ex.type, builder);
      addAnnotationTypeDefinitions(ex.annotations, builder);
    }
  }
  if (fn.sink) {
    addTypeRefDefinitions(fn.sink->payloadType, builder);
    if (fn.sink->finalResponseType) {
      addTypeRefDefinitions(*fn.sink->finalResponseType, builder);
    }
    for (const auto& ex : fn.sink->clientExceptions) {
      addTypeRefDefinitions(ex.type, builder);
      addAnnotationTypeDefinitions(ex.annotations, builder);
    }
    for (const auto& ex : fn.sink->serverExceptions) {
      addTypeRefDefinitions(ex.type, builder);
      addAnnotationTypeDefinitions(ex.annotations, builder);
    }
  }
}

bool containsInteractionUri(
    std::span<const ServiceDescriptor::Interaction> interactions,
    std::string_view uri) {
  for (const auto& interaction : interactions) {
    if (interaction.uri == uri) {
      return true;
    }
  }
  return false;
}

void validateCreatedInteractions(
    std::span<const ServiceDescriptor::Function> functions,
    std::span<const ServiceDescriptor::Interaction> interactions) {
  for (const auto& fn : functions) {
    if (fn.createdInteractionUri.has_value() &&
        !containsInteractionUri(interactions, *fn.createdInteractionUri)) {
      throw std::invalid_argument(
          "Function creates an interaction that is not in the catalog: " +
          fn.name);
    }
  }
}

type_system::FunctionQualifier toSerializableQualifier(FunctionQualifier q) {
  switch (q) {
    case FunctionQualifier::Unspecified:
      return type_system::FunctionQualifier::Unspecified;
    case FunctionQualifier::Idempotent:
      return type_system::FunctionQualifier::Idempotent;
    case FunctionQualifier::ReadOnly:
      return type_system::FunctionQualifier::ReadOnly;
  }
  return type_system::FunctionQualifier::Unspecified;
}

type_system::RpcKind toSerializableRpcKind(RpcKind kind) {
  switch (kind) {
    case RpcKind::Unary:
      return type_system::RpcKind::Unary;
    case RpcKind::OneWay:
      return type_system::RpcKind::OneWay;
    case RpcKind::Stream:
      return type_system::RpcKind::Stream;
    case RpcKind::Sink:
      return type_system::RpcKind::Sink;
    case RpcKind::BidirectionalStream:
      return type_system::RpcKind::BidirectionalStream;
  }
  return type_system::RpcKind::Unary;
}

std::int32_t serializableRecordFieldId(type_system::TypeRef type) {
  while (type.kind() == type_system::TypeRef::Kind::OPAQUE_ALIAS) {
    type = type.asOpaqueAlias().targetType();
  }

  switch (type.kind()) {
    case type_system::TypeRef::Kind::BOOL:
      return 1;
    case type_system::TypeRef::Kind::BYTE:
      return 2;
    case type_system::TypeRef::Kind::I16:
      return 3;
    case type_system::TypeRef::Kind::I32:
    case type_system::TypeRef::Kind::ENUM:
      return 5;
    case type_system::TypeRef::Kind::I64:
      return 6;
    case type_system::TypeRef::Kind::FLOAT:
      return 7;
    case type_system::TypeRef::Kind::DOUBLE:
      return 8;
    case type_system::TypeRef::Kind::STRING:
      return 9;
    case type_system::TypeRef::Kind::BINARY:
      return 10;
    case type_system::TypeRef::Kind::STRUCT:
    case type_system::TypeRef::Kind::UNION:
      return 11;
    case type_system::TypeRef::Kind::LIST:
      return 12;
    case type_system::TypeRef::Kind::SET:
      return 13;
    case type_system::TypeRef::Kind::MAP:
      return 14;
    case type_system::TypeRef::Kind::ANY:
      throw std::runtime_error("Any-typed annotation values are not supported");
    case type_system::TypeRef::Kind::OPAQUE_ALIAS:
      break;
  }
  throw std::runtime_error("Unsupported annotation value type");
}

type_system::TypeRef resolvedRecordType(type_system::TypeRef type) {
  while (type.kind() == type_system::TypeRef::Kind::OPAQUE_ALIAS) {
    type = type.asOpaqueAlias().targetType();
  }
  return type;
}

class Hasher : private ::apache::thrift::detail::Sha256DigestHasher<
                   ServiceCatalogDigest> {
 public:
  using Base =
      ::apache::thrift::detail::Sha256DigestHasher<ServiceCatalogDigest>;

  explicit Hasher(DigestContext ctx = {}) : Base(), ctx_(ctx) {}

  using Base::finalize;
  using Base::hash;
  using Base::hashDigest;

  void hash(const type_system::TypeId& typeId);

  void hash(const type_system::SerializableRecordUnion& record);
  void hash(const type_system::SerializableParameter& param);
  void hash(const type_system::SerializableException& ex);
  void hash(const type_system::SerializableStream& stream);
  void hash(const type_system::SerializableSink& sink);
  void hash(const type_system::SerializableBidirectionalStream& bidi);
  void hash(const type_system::SerializableStreamingResponse& streaming);
  void hash(const type_system::SerializableFunctionResponse& response);
  void hash(const type_system::SerializableFunction& fn);
  void hash(const type_system::SerializableServiceDefinition& serviceDef);
  void hash(
      const type_system::SerializableInteractionDefinition& interactionDef);
  void hash(
      const type_system::SerializableRpcInterfaceDefinition& interfaceDef);
  void hash(
      const folly::F14FastMap<
          std::string,
          type_system::SerializableRecordUnion>& annotations);
  void hash(const type_system::SerializableServiceCatalog& catalog);

  void hash(const ServiceDescriptor& descriptor, type_system::UriView uri);
  void hash(const ServiceDescriptor::Param& param);
  void hash(const ServiceDescriptor::Exception& ex);
  void hash(const ServiceDescriptor::Stream& stream);
  void hash(const ServiceDescriptor::Sink& sink);
  void hashBidirectionalStream(
      const ServiceDescriptor::Stream& stream,
      const ServiceDescriptor::Sink& sink);
  void hashResponse(const ServiceDescriptor::Function& fn);
  void hash(const ServiceDescriptor::Function& fn);
  void hashServiceDefinition(const ServiceDescriptor& descriptor);
  void hash(const ServiceDescriptor::Interaction& interaction);
  void hashAnnotations(std::span<const DynamicValue> annotations);
  void hashRecord(DynamicConstRef value);

  template <typename Range, typename HashFn>
  void hashUnorderedByDigest(const Range& range, HashFn&& hashFn) {
    std::vector<ServiceCatalogDigest> digests;
    digests.reserve(std::distance(std::begin(range), std::end(range)));

    for (const auto& elem : range) {
      Hasher h(ctx_);
      hashFn(h, elem);
      digests.push_back(h.finalize());
    }

    hash(static_cast<std::uint32_t>(digests.size()));
    std::sort(digests.begin(), digests.end());
    for (const auto& digest : digests) {
      hashDigest(digest);
    }
  }

  template <typename Range, typename KeyHashFn, typename EntryHashFn>
  void hashMapByKeyDigest(
      const Range& range, KeyHashFn&& keyHashFn, EntryHashFn&& entryHashFn) {
    using Elem = std::remove_reference_t<decltype(*std::begin(range))>;
    std::vector<std::pair<ServiceCatalogDigest, const Elem*>> sortedEntries;
    sortedEntries.reserve(std::distance(std::begin(range), std::end(range)));

    for (const auto& entry : range) {
      Hasher h(ctx_);
      keyHashFn(h, entry);
      sortedEntries.emplace_back(h.finalize(), &entry);
    }

    hash(static_cast<std::uint32_t>(sortedEntries.size()));
    std::sort(
        sortedEntries.begin(),
        sortedEntries.end(),
        [](const auto& a, const auto& b) { return a.first < b.first; });

    for (const auto& [keyDigest, entry] : sortedEntries) {
      entryHashFn(*this, *entry);
    }
  }

 private:
  void hashBinary(const Binary& binary) {
    auto cursor = binary.cursor();
    auto bytes = cursor.readFixedString(binary.computeChainDataLength());
    hash(std::string_view{bytes});
  }

  void hashDynamicMap(const Map& map) {
    std::vector<std::pair<ServiceCatalogDigest, std::size_t>> sortedEntries;
    sortedEntries.reserve(map.size());

    std::size_t ordinal = 0;
    for (const auto [key, value] : map) {
      Hasher h(ctx_);
      h.hashRecord(key);
      sortedEntries.emplace_back(h.finalize(), ordinal++);
    }

    hash(static_cast<std::uint32_t>(sortedEntries.size()));
    std::sort(
        sortedEntries.begin(),
        sortedEntries.end(),
        [](const auto& a, const auto& b) {
          return a.first == b.first ? a.second < b.second : a.first < b.first;
        });

    for (const auto& sortedEntry : sortedEntries) {
      const auto sortedOrdinal = sortedEntry.second;
      std::size_t currentOrdinal = 0;
      for (const auto [key, value] : map) {
        if (currentOrdinal++ != sortedOrdinal) {
          continue;
        }
        hashRecord(key);
        hashRecord(value);
        break;
      }
    }
  }

  type_system::TypeSystemDigest digestTypeSystem(
      const type_system::SerializableTypeSystem& typeSystem) const {
    type_system::TypeSystemHasher typeHasher;
    typeHasher.mode = ctx_.mode;
    return typeHasher(typeSystem);
  }

  type_system::TypeSystemDigest digestTypeSystem(
      const ServiceDescriptor& descriptor) const {
    const auto& typeSystem = descriptor.typeSystem();
    auto interactions = descriptor.interactions();
    validateCreatedInteractions(descriptor.functions(), interactions);
    for (const auto& interaction : interactions) {
      validateCreatedInteractions(interaction.functions, interactions);
    }

    auto builder =
        type_system::SerializableTypeSystemBuilder::withoutSourceInfo(
            typeSystem);
    for (const auto& fn : descriptor.functions()) {
      addFunctionTypeDefinitions(fn, builder);
    }
    for (const auto& interaction : interactions) {
      for (const auto& fn : interaction.functions) {
        addFunctionTypeDefinitions(fn, builder);
      }
      addAnnotationTypeDefinitions(interaction.annotations, builder);
    }
    addAnnotationTypeDefinitions(descriptor.annotations(), builder);

    auto prunedTypeSystem = std::move(builder).build();
    if (prunedTypeSystem == nullptr) {
      throw std::runtime_error(
          "ServiceCatalogHasher: failed to build type system");
    }
    return digestTypeSystem(*prunedTypeSystem);
  }

  type_system::TypeSystemDigest digestExternalTypeSystem(
      std::string_view digestBytes) const {
    type_system::TypeSystemDigest result;
    if (digestBytes.size() != result.size()) {
      throw std::invalid_argument(
          "SerializableServiceCatalog has no valid type system digest");
    }
    std::memcpy(result.data(), digestBytes.data(), result.size());
    return result;
  }

  DigestContext ctx_;
};

void Hasher::hash(const type_system::TypeId& typeId) {
  if (typeId.empty()) {
    hash(std::uint8_t{0});
    return;
  }

  hash(static_cast<std::int32_t>(typeId.kind()));

  switch (typeId.kind()) {
    case type_system::TypeId::Kind::BOOL:
    case type_system::TypeId::Kind::BYTE:
    case type_system::TypeId::Kind::I16:
    case type_system::TypeId::Kind::I32:
    case type_system::TypeId::Kind::I64:
    case type_system::TypeId::Kind::FLOAT:
    case type_system::TypeId::Kind::DOUBLE:
    case type_system::TypeId::Kind::STRING:
    case type_system::TypeId::Kind::BINARY:
    case type_system::TypeId::Kind::ANY:
      break;
    case type_system::TypeId::Kind::URI:
      hash(std::string_view{typeId.asUri()});
      break;
    case type_system::TypeId::Kind::LIST:
      hash(typeId.asList().elementType());
      break;
    case type_system::TypeId::Kind::SET:
      hash(typeId.asSet().elementType());
      break;
    case type_system::TypeId::Kind::MAP:
      hash(typeId.asMap().keyType());
      hash(typeId.asMap().valueType());
      break;
  }
}

void Hasher::hash(const type_system::SerializableRecordUnion& record) {
  hash(record.getType());
  if (auto boolVal = record.boolDatum()) {
    hash(*boolVal);
  } else if (auto int8Val = record.int8Datum()) {
    hash(*int8Val);
  } else if (auto int16Val = record.int16Datum()) {
    hash(*int16Val);
  } else if (auto int32Val = record.int32Datum()) {
    hash(*int32Val);
  } else if (auto int64Val = record.int64Datum()) {
    hash(*int64Val);
  } else if (auto float32Val = record.float32Datum()) {
    hash(*float32Val);
  } else if (auto float64Val = record.float64Datum()) {
    hash(*float64Val);
  } else if (auto textVal = record.textDatum()) {
    hash(std::string_view{*textVal});
  } else if (auto byteArrayVal = record.byteArrayDatum()) {
    hash(*byteArrayVal);
  } else if (auto fieldSetVal = record.fieldSetDatum()) {
    forEachSortedByKey(
        *fieldSetVal,
        [](const auto& entry) {
          return static_cast<std::int16_t>(entry.first);
        },
        [this](std::int16_t id, const auto& entry) {
          hash(id);
          hash(entry.second);
        });
  } else if (auto listVal = record.listDatum()) {
    for (const auto& elem : *listVal) {
      hash(elem);
    }
  } else if (auto setVal = record.setDatum()) {
    hashUnorderedByDigest(
        *setVal, [](Hasher& h, const auto& elem) { h.hash(elem); });
  } else if (auto mapVal = record.mapDatum()) {
    hashMapByKeyDigest(
        *mapVal,
        [](Hasher& h, const auto& entry) { h.hash(*entry.key()); },
        [](Hasher& h, const auto& entry) {
          h.hash(*entry.key());
          h.hash(*entry.value());
        });
  } else {
    throw std::invalid_argument(
        "ServiceCatalogHasher: unhandled SerializableRecordUnion variant");
  }
}

void Hasher::hash(
    const folly::F14FastMap<std::string, type_system::SerializableRecordUnion>&
        annotations) {
  if (!ctx_.includeAnnotationsAndDefaults()) {
    return;
  }
  hashUnorderedByDigest(annotations, [](Hasher& h, const auto& entry) {
    h.hash(std::string_view{entry.first});
    h.hash(entry.second);
  });
}

void Hasher::hashAnnotations(std::span<const DynamicValue> annotations) {
  if (!ctx_.includeAnnotationsAndDefaults()) {
    return;
  }

  std::map<std::string_view, const DynamicValue*> byUri;
  for (const auto& annotation : annotations) {
    const auto& uri = annotation.type().asStruct().uri();
    if (uri.starts_with("facebook.com/thrift/annotation/")) {
      continue;
    }
    byUri.emplace(uri, &annotation);
  }

  hashUnorderedByDigest(byUri, [](Hasher& h, const auto& entry) {
    h.hash(entry.first);
    h.hashRecord(DynamicConstRef{*entry.second});
  });
}

void Hasher::hashRecord(DynamicConstRef value) {
  auto type = resolvedRecordType(value.type());
  hash(serializableRecordFieldId(type));

  switch (type.kind()) {
    case type_system::TypeRef::Kind::BOOL:
      hash(value.asBool());
      break;
    case type_system::TypeRef::Kind::BYTE:
      hash(value.asByte());
      break;
    case type_system::TypeRef::Kind::I16:
      hash(value.asI16());
      break;
    case type_system::TypeRef::Kind::I32:
      hash(value.asI32());
      break;
    case type_system::TypeRef::Kind::I64:
      hash(value.asI64());
      break;
    case type_system::TypeRef::Kind::FLOAT:
      hash(value.asFloat());
      break;
    case type_system::TypeRef::Kind::DOUBLE:
      hash(value.asDouble());
      break;
    case type_system::TypeRef::Kind::ENUM:
      hash(value.asEnum());
      break;
    case type_system::TypeRef::Kind::STRING:
      hash(value.asString().view());
      break;
    case type_system::TypeRef::Kind::BINARY:
      hashBinary(value.asBinary());
      break;
    case type_system::TypeRef::Kind::LIST:
      for (DynamicConstRef elem : value.asList()) {
        hashRecord(elem);
      }
      break;
    case type_system::TypeRef::Kind::SET:
      hashUnorderedByDigest(value.asSet(), [](Hasher& h, DynamicConstRef elem) {
        h.hashRecord(elem);
      });
      break;
    case type_system::TypeRef::Kind::MAP:
      hashDynamicMap(value.asMap());
      break;
    case type_system::TypeRef::Kind::STRUCT: {
      const auto& structValue = value.asStruct();
      forEachSortedByKey(
          type.asStruct().fields(),
          [](const auto& field) {
            return static_cast<std::int16_t>(field.identity().id());
          },
          [this, &structValue](std::int16_t id, const auto&) {
            if (auto fieldValue = structValue.getField(FieldId{id})) {
              hash(id);
              hashRecord(*fieldValue);
            }
          });
      break;
    }
    case type_system::TypeRef::Kind::UNION: {
      const auto& unionValue = value.asUnion();
      if (!unionValue.isEmpty()) {
        const auto handle = unionValue.activeField();
        const auto id = static_cast<std::int16_t>(
            type.asUnion().fields()[handle.index()].identity().id());
        hash(id);
        hashRecord(unionValue.getField(handle));
      }
      break;
    }
    case type_system::TypeRef::Kind::ANY:
      throw std::runtime_error("Any-typed annotation values are not supported");
    case type_system::TypeRef::Kind::OPAQUE_ALIAS:
      throw std::runtime_error("Unsupported annotation value type");
  }
}

void Hasher::hash(const type_system::SerializableParameter& param) {
  hash(static_cast<std::int16_t>(param.identity()->id()));
  hash(std::string_view{param.identity()->name()});
  hash(*param.type());
  hash(*param.annotations());
}

void Hasher::hash(const ServiceDescriptor::Param& param) {
  hash(static_cast<std::int16_t>(param.id));
  hash(std::string_view{param.name});
  hash(param.type.id());
  hashAnnotations(param.annotations);
}

void Hasher::hash(const type_system::SerializableException& ex) {
  hash(static_cast<std::int16_t>(ex.identity()->id()));
  hash(std::string_view{ex.identity()->name()});
  hash(*ex.type());
  hash(*ex.annotations());
}

void Hasher::hash(const ServiceDescriptor::Exception& ex) {
  hash(static_cast<std::int16_t>(ex.id));
  hash(std::string_view{ex.name});
  hash(ex.type.id());
  hashAnnotations(ex.annotations);
}

void Hasher::hash(const type_system::SerializableStream& stream) {
  hash(*stream.payloadType());
  forEachSortedByKey(
      *stream.exceptions(),
      [](const auto& ex) {
        return static_cast<std::int16_t>(ex.identity()->id());
      },
      [this](auto, const auto& ex) { hash(ex); });
}

void Hasher::hash(const ServiceDescriptor::Stream& stream) {
  hash(stream.payloadType.id());
  forEachSortedByKey(
      stream.exceptions,
      [](const auto& ex) { return static_cast<std::int16_t>(ex.id); },
      [this](auto, const auto& ex) { hash(ex); });
}

void Hasher::hash(const type_system::SerializableSink& sink) {
  hash(*sink.payloadType());
  hash(sink.finalResponseType().has_value());
  if (sink.finalResponseType().has_value()) {
    hash(*sink.finalResponseType());
  }
  forEachSortedByKey(
      *sink.clientExceptions(),
      [](const auto& ex) {
        return static_cast<std::int16_t>(ex.identity()->id());
      },
      [this](auto, const auto& ex) { hash(ex); });
  forEachSortedByKey(
      *sink.serverExceptions(),
      [](const auto& ex) {
        return static_cast<std::int16_t>(ex.identity()->id());
      },
      [this](auto, const auto& ex) { hash(ex); });
}

void Hasher::hash(const ServiceDescriptor::Sink& sink) {
  hash(sink.payloadType.id());
  hash(sink.finalResponseType.has_value());
  if (sink.finalResponseType.has_value()) {
    hash(sink.finalResponseType->id());
  }
  forEachSortedByKey(
      sink.clientExceptions,
      [](const auto& ex) { return static_cast<std::int16_t>(ex.id); },
      [this](auto, const auto& ex) { hash(ex); });
  forEachSortedByKey(
      sink.serverExceptions,
      [](const auto& ex) { return static_cast<std::int16_t>(ex.id); },
      [this](auto, const auto& ex) { hash(ex); });
}

void Hasher::hash(const type_system::SerializableBidirectionalStream& bidi) {
  hash(*bidi.sinkPayloadType());
  hash(*bidi.streamPayloadType());
  forEachSortedByKey(
      *bidi.sinkExceptions(),
      [](const auto& ex) {
        return static_cast<std::int16_t>(ex.identity()->id());
      },
      [this](auto, const auto& ex) { hash(ex); });
  forEachSortedByKey(
      *bidi.streamExceptions(),
      [](const auto& ex) {
        return static_cast<std::int16_t>(ex.identity()->id());
      },
      [this](auto, const auto& ex) { hash(ex); });
}

void Hasher::hashBidirectionalStream(
    const ServiceDescriptor::Stream& stream,
    const ServiceDescriptor::Sink& sink) {
  hash(sink.payloadType.id());
  hash(stream.payloadType.id());
  forEachSortedByKey(
      sink.clientExceptions,
      [](const auto& ex) { return static_cast<std::int16_t>(ex.id); },
      [this](auto, const auto& ex) { hash(ex); });
  forEachSortedByKey(
      stream.exceptions,
      [](const auto& ex) { return static_cast<std::int16_t>(ex.id); },
      [this](auto, const auto& ex) { hash(ex); });
}

void Hasher::hash(const type_system::SerializableStreamingResponse& streaming) {
  hash(streaming.getType());
  if (auto stream = streaming.serverStream()) {
    hash(*stream);
  } else if (auto sink = streaming.clientSink()) {
    hash(*sink);
  } else if (auto bidi = streaming.bidirectionalStream()) {
    hash(*bidi);
  } else {
    throw std::invalid_argument(
        "ServiceCatalogHasher: unhandled streaming response variant");
  }
}

void Hasher::hash(const type_system::SerializableFunctionResponse& response) {
  hash(response.initialResponseType().has_value());
  if (response.initialResponseType().has_value()) {
    hash(*response.initialResponseType());
  }
  hash(response.streaming().has_value());
  if (response.streaming().has_value()) {
    hash(*response.streaming());
  }
  hash(response.createsInteraction().has_value());
  if (response.createsInteraction().has_value()) {
    hash(std::string_view{*response.createsInteraction()});
  }
}

void Hasher::hashResponse(const ServiceDescriptor::Function& fn) {
  hash(fn.responseType.has_value());
  if (fn.responseType.has_value()) {
    hash(fn.responseType->id());
  }
  hash(fn.stream.has_value() || fn.sink.has_value());
  if (fn.stream && fn.sink) {
    hash(type_system::SerializableStreamingResponse::Type::bidirectionalStream);
    hashBidirectionalStream(*fn.stream, *fn.sink);
  } else if (fn.stream) {
    hash(type_system::SerializableStreamingResponse::Type::serverStream);
    hash(*fn.stream);
  } else if (fn.sink) {
    hash(type_system::SerializableStreamingResponse::Type::clientSink);
    hash(*fn.sink);
  }
  hash(fn.createdInteractionUri.has_value());
  if (fn.createdInteractionUri.has_value()) {
    hash(std::string_view{*fn.createdInteractionUri});
  }
}

void Hasher::hash(const type_system::SerializableFunction& fn) {
  hash(std::string_view{*fn.name()});
  hash(*fn.qualifier());
  forEachSortedByKey(
      *fn.params(),
      [](const auto& param) {
        return static_cast<std::int16_t>(param.identity()->id());
      },
      [this](auto, const auto& param) { hash(param); });
  hash(*fn.response());
  forEachSortedByKey(
      *fn.exceptions(),
      [](const auto& ex) {
        return static_cast<std::int16_t>(ex.identity()->id());
      },
      [this](auto, const auto& ex) { hash(ex); });
  hash(*fn.rpcKind());
  hash(*fn.annotations());
}

void Hasher::hash(const ServiceDescriptor::Function& fn) {
  hash(std::string_view{fn.name});
  hash(toSerializableQualifier(fn.qualifier));
  forEachSortedByKey(
      fn.params,
      [](const auto& param) { return static_cast<std::int16_t>(param.id); },
      [this](auto, const auto& param) { hash(param); });
  hashResponse(fn);
  forEachSortedByKey(
      fn.exceptions,
      [](const auto& ex) { return static_cast<std::int16_t>(ex.id); },
      [this](auto, const auto& ex) { hash(ex); });
  hash(toSerializableRpcKind(fn.rpcKind));
  hashAnnotations(fn.annotations);
}

void Hasher::hash(
    const type_system::SerializableServiceDefinition& serviceDef) {
  forEachSortedByKey(
      *serviceDef.functions(),
      [](const auto& fn) -> std::string_view { return *fn.name(); },
      [this](auto, const auto& fn) { hash(fn); });
  hash(serviceDef.baseService().has_value());
  if (serviceDef.baseService().has_value()) {
    hash(std::string_view{*serviceDef.baseService()});
  }
  hash(*serviceDef.annotations());
}

void Hasher::hashServiceDefinition(const ServiceDescriptor& descriptor) {
  forEachSortedByKey(
      descriptor.functions(),
      [](const auto& fn) -> std::string_view { return fn.name; },
      [this](auto, const auto& fn) { hash(fn); });
  hash(false);
  hashAnnotations(descriptor.annotations());
}

void Hasher::hash(
    const type_system::SerializableInteractionDefinition& interactionDef) {
  forEachSortedByKey(
      *interactionDef.functions(),
      [](const auto& fn) -> std::string_view { return *fn.name(); },
      [this](auto, const auto& fn) { hash(fn); });
  hash(*interactionDef.annotations());
}

void Hasher::hash(const ServiceDescriptor::Interaction& interaction) {
  forEachSortedByKey(
      interaction.functions,
      [](const auto& fn) -> std::string_view { return fn.name; },
      [this](auto, const auto& fn) { hash(fn); });
  hashAnnotations(interaction.annotations);
}

void Hasher::hash(
    const type_system::SerializableRpcInterfaceDefinition& interfaceDef) {
  hash(interfaceDef.getType());
  if (auto serviceDef = interfaceDef.serviceDef()) {
    hash(*serviceDef);
  } else if (auto interactionDef = interfaceDef.interactionDef()) {
    hash(*interactionDef);
  } else {
    throw std::invalid_argument(
        "ServiceCatalogHasher: unhandled interface definition variant");
  }
}

void Hasher::hash(const type_system::SerializableServiceCatalog& catalog) {
  hash(kServiceCatalogDigestVersion);
  if (catalog.types_ref().has_value()) {
    hashDigest(digestTypeSystem(*catalog.types_ref()));
  } else {
    hashDigest(
        digestExternalTypeSystem(std::string_view{*catalog.typesDigest()}));
  }

  forEachSortedByKey(
      *catalog.interfaces(),
      [](const auto& entry) -> std::string_view { return entry.first; },
      [this](std::string_view uri, const auto& entry) {
        hash(uri);
        hash(entry.second);
      });
}

void Hasher::hash(
    const ServiceDescriptor& descriptor, type_system::UriView serviceUri) {
  struct InterfaceEntry {
    std::string_view uri;
    const ServiceDescriptor::Interaction* interaction;
  };

  hash(kServiceCatalogDigestVersion);
  hashDigest(digestTypeSystem(descriptor));

  std::vector<InterfaceEntry> interfaces;
  interfaces.reserve(1 + descriptor.interactions().size());
  interfaces.push_back(InterfaceEntry{serviceUri, nullptr});
  for (const auto& interaction : descriptor.interactions()) {
    interfaces.push_back(InterfaceEntry{interaction.uri, &interaction});
  }

  forEachSortedByKey(
      interfaces,
      [](const auto& entry) -> std::string_view { return entry.uri; },
      [this, &descriptor](std::string_view uri, const auto& entry) {
        hash(uri);
        if (entry.interaction == nullptr) {
          hash(
              type_system::SerializableRpcInterfaceDefinition::Type::
                  serviceDef);
          hashServiceDefinition(descriptor);
        } else {
          hash(
              type_system::SerializableRpcInterfaceDefinition::Type::
                  interactionDef);
          hash(*entry.interaction);
        }
      });
}

} // namespace

ServiceCatalogDigest ServiceCatalogHasher::operator()(
    const type_system::SerializableServiceCatalog& catalog) const {
  Hasher h(DigestContext{mode});
  h.hash(catalog);
  return h.finalize();
}

ServiceCatalogDigest ServiceCatalogHasher::operator()(
    const ServiceDescriptor& descriptor,
    type_system::UriView serviceUri) const {
  Hasher h(DigestContext{mode});
  h.hash(descriptor, serviceUri);
  return h.finalize();
}

} // namespace apache::thrift::dynamic
