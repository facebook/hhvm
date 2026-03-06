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

#include <thrift/lib/cpp2/dynamic/TypeSystemDigest.h>

#include <algorithm>
#include <cstring>
#include <map>

#include <folly/io/IOBuf.h>
#include <folly/lang/Assume.h>
#include <folly/lang/Bits.h>
#include <folly/ssl/OpenSSLHash.h>
#include <thrift/lib/thrift/gen-cpp2/record_types.h>
#include <thrift/lib/thrift/gen-cpp2/type_id_types.h>

namespace apache::thrift::type_system {

namespace {

// Helper to write integers in little-endian format (portable across platforms)
template <typename T>
  requires(std::is_integral_v<T> && !std::is_same_v<T, bool>)
constexpr T toLittleEndian(T value) {
  return folly::Endian::little(value);
}

// Iterates over a range in sorted order by key extracted via keyFn.
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

class Hasher {
 public:
  Hasher() { digest_.hash_init(EVP_sha256()); }

  // Hash bool as a single byte (0 or 1)
  void hash(bool v) {
    std::uint8_t byte = v ? 1 : 0;
    digest_.hash_update(folly::ByteRange(&byte, 1));
  }

  // Hash integral types using little-endian encoding for cross-platform
  // portability
  template <typename T>
    requires(std::is_integral_v<T> && !std::is_same_v<T, bool>)
  void hash(T v) {
    auto le = toLittleEndian(v);
    digest_.hash_update(
        folly::ByteRange(
            reinterpret_cast<const std::uint8_t*>(&le), sizeof(le)));
  }

  // Hash enum types by casting to their underlying i32 value
  // Note: this assumes all enums are thrift-related & should be mapped
  // to i32 values.
  template <typename T>
    requires std::is_enum_v<T>
  void hash(T v) {
    hash(static_cast<std::int32_t>(v));
  }

  // Hash floating-point types using IEEE 754 representation in little-endian.
  //
  // This hashes the raw bit pattern, so distinct NaN representations would
  // produce different digests. This is safe because SerializableRecord rejects
  // NaN and negative zero at construction time (see ensureValidFloatOrThrow).
  void hash(float v) {
    static_assert(sizeof(float) == 4, "float must be 4 bytes");
    std::uint32_t bits;
    std::memcpy(&bits, &v, sizeof(bits));
    hash(bits);
  }

  void hash(double v) {
    static_assert(sizeof(double) == 8, "double must be 8 bytes");
    std::uint64_t bits;
    std::memcpy(&bits, &v, sizeof(bits));
    hash(bits);
  }

  // Hash PrimitiveDatum wrapper by unwrapping and hashing the underlying value
  template <typename T>
  void hash(detail::PrimitiveDatum<T> v) {
    hash(static_cast<T>(v));
  }

  // Strings are hashed as size-prefixed envelopes
  void hash(std::string_view s) {
    hash(static_cast<std::uint32_t>(s.size()));
    digest_.hash_update(folly::ByteRange(s));
  }

  // Byte arrays are hashed as size-prefixed envelopes
  void hash(folly::ByteRange b) {
    hash(static_cast<std::uint32_t>(b.size()));
    digest_.hash_update(b);
  }

  // Byte arrays are hashed as size-prefixed envelopes
  void hash(const folly::IOBuf& buf) {
    hash(static_cast<std::uint32_t>(buf.computeChainDataLength()));
    digest_.hash_update(buf);
  }

  void hash(const TypeId& typeId);
  void hash(const TypeIdUnion& typeId);
  void hash(const SerializableRecordUnion& record);
  void hash(const SerializableFieldDefinition& field);
  void hash(const SerializableTypeDefinition& def);
  void hash(const SerializableStructDefinition& def);
  void hash(const SerializableUnionDefinition& def);
  void hash(const SerializableEnumDefinition& def);
  void hash(const SerializableOpaqueAliasDefinition& def);

  void hash(
      const folly::F14FastMap<std::string, SerializableRecordUnion>&
          annotations);

  // Runtime type hashing - produces same digest as serializable equivalents
  void hash(const SerializableRecord& record);
  void hash(const FieldDefinition& field);
  void hash(const StructNode& node);
  void hash(const UnionNode& node);
  void hash(const EnumNode& node);
  void hash(const OpaqueAliasNode& node);
  void hash(const AnnotationsMap& annotations);
  void hash(const TypeRef& typeRef);

  // Hash elements in an order-independent way by sorting by element digest.
  // Each element is hashed to produce a digest, then elements are sorted
  // lexicographically by digest and hashed in that order.
  template <typename Range, typename HashFn>
  void hashUnorderedByDigest(const Range& range, HashFn&& hashFn) {
    std::vector<TypeSystemDigest> digests;
    digests.reserve(std::distance(std::begin(range), std::end(range)));

    for (const auto& elem : range) {
      Hasher h;
      hashFn(h, elem);
      digests.push_back(h.finalize());
    }

    hash(static_cast<std::uint32_t>(digests.size()));
    std::sort(digests.begin(), digests.end());

    for (const auto& d : digests) {
      digest_.hash_update(
          folly::ByteRange(
              reinterpret_cast<const std::uint8_t*>(d.data()), d.size()));
    }
  }

  // Hash map entries in order-independent way by sorting by key digest.
  // KeyHashFn should hash only the key, EntryHashFn should hash key+value.
  template <typename Range, typename KeyHashFn, typename EntryHashFn>
  void hashMapByKeyDigest(
      const Range& range, KeyHashFn&& keyHashFn, EntryHashFn&& entryHashFn) {
    using Elem = std::remove_reference_t<decltype(*std::begin(range))>;
    std::vector<std::pair<TypeSystemDigest, const Elem*>> sortedEntries;
    sortedEntries.reserve(std::distance(std::begin(range), std::end(range)));

    for (const auto& entry : range) {
      Hasher h;
      keyHashFn(h, entry);
      sortedEntries.emplace_back(h.finalize(), &entry);
    }

    hash(static_cast<std::uint32_t>(sortedEntries.size()));

    std::sort(
        sortedEntries.begin(),
        sortedEntries.end(),
        [](const auto& a, const auto& b) { return a.first < b.first; });

    for (const auto& [keyDigest, entryPtr] : sortedEntries) {
      entryHashFn(*this, *entryPtr);
    }
  }

  // Convenience overload where key hash and entry hash use the same function
  template <typename Range, typename EntryHashFn>
  void hashMapByKeyDigest(const Range& range, EntryHashFn&& entryHashFn) {
    hashMapByKeyDigest(
        range,
        std::forward<EntryHashFn>(entryHashFn),
        std::forward<EntryHashFn>(entryHashFn));
  }

  TypeSystemDigest finalize() {
    TypeSystemDigest result;
    digest_.hash_final(
        folly::MutableByteRange(
            reinterpret_cast<std::uint8_t*>(result.data()), result.size()));
    return result;
  }

 private:
  folly::ssl::OpenSSLHash::Digest digest_;
};

void Hasher::hash(const TypeId& typeId) {
  if (typeId.empty()) {
    hash(std::uint8_t{0});
    return;
  }

  hash(static_cast<std::int32_t>(typeId.kind()));

  switch (typeId.kind()) {
    case TypeId::Kind::BOOL:
    case TypeId::Kind::BYTE:
    case TypeId::Kind::I16:
    case TypeId::Kind::I32:
    case TypeId::Kind::I64:
    case TypeId::Kind::FLOAT:
    case TypeId::Kind::DOUBLE:
    case TypeId::Kind::STRING:
    case TypeId::Kind::BINARY:
    case TypeId::Kind::ANY: {
      break;
    }
    case TypeId::Kind::URI:
      hash(std::string_view{typeId.asUri()});
      break;
    case TypeId::Kind::LIST:
      hash(typeId.asList().elementType());
      break;
    case TypeId::Kind::SET:
      hash(typeId.asSet().elementType());
      break;
    case TypeId::Kind::MAP:
      hash(typeId.asMap().keyType());
      hash(typeId.asMap().valueType());
      break;
  }
}

void Hasher::hash(const TypeIdUnion& typeId) {
  if (typeId.getType() == TypeIdUnion::Type::__EMPTY__) {
    hash(uint8_t{0});
    return;
  }

  hash(static_cast<std::int32_t>(typeId.getType()));

  if (auto uri = typeId.userDefinedType()) {
    hash(std::string_view{*uri});
  } else if (auto list = typeId.listType()) {
    hash(list->elementType());
  } else if (auto set = typeId.setType()) {
    hash(set->elementType());
  } else if (auto map = typeId.mapType()) {
    hash(map->keyType());
    hash(map->valueType());
  }
  // Primitives (boolType, byteType, etc.) have no additional data to hash —
  // only the discriminant (already hashed above) distinguishes them.
  // If a new non-primitive variant is added to TypeIdUnion, it must be handled
  // explicitly above.
}

// Maps SerializableRecord::Kind to thrift field ID for SerializableRecordUnion
// This ensures hash(SerializableRecord) == hash(SerializableRecordUnion)
// Note: Thrift field IDs have gaps (field 4 is missing in record.thrift)
constexpr std::int32_t kindToThriftFieldId(SerializableRecord::Kind kind) {
  switch (kind) {
    case SerializableRecord::Kind::BOOL:
      return 1;
    case SerializableRecord::Kind::INT8:
      return 2;
    case SerializableRecord::Kind::INT16:
      return 3;
    case SerializableRecord::Kind::INT32:
      return 5; // Field 4 is missing in record.thrift
    case SerializableRecord::Kind::INT64:
      return 6;
    case SerializableRecord::Kind::FLOAT32:
      return 7;
    case SerializableRecord::Kind::FLOAT64:
      return 8;
    case SerializableRecord::Kind::TEXT:
      return 9;
    case SerializableRecord::Kind::BYTE_ARRAY:
      return 10;
    case SerializableRecord::Kind::FIELD_SET:
      return 11;
    case SerializableRecord::Kind::LIST:
      return 12;
    case SerializableRecord::Kind::SET:
      return 13;
    case SerializableRecord::Kind::MAP:
      return 14;
  }
  folly::assume_unreachable();
}

// --- Runtime TypeSystem nodes --- //

void Hasher::hash(const TypeRef& typeRef) {
  typeRef.visit(
      [this](const StructNode& node) { hash(node); },
      [this](const UnionNode& node) { hash(node); },
      [this](const EnumNode& node) { hash(node); },
      [this](const OpaqueAliasNode& node) { hash(node); },
      [](const auto&) {
        folly::throw_exception<std::invalid_argument>(
            "TypeSystemHasher: Only structured types can be hashed.");
      });
}

void Hasher::hash(const StructNode& node) {
  hash(static_cast<std::int32_t>(SerializableTypeDefinition::Type::structDef));
  forEachSortedByKey(
      node.fields(),
      [](const auto& f) { return f.identity().id(); },
      [this](auto, const auto& f) { hash(f); });

  hash(node.isSealed());
  hash(node.annotations());
}

void Hasher::hash(const UnionNode& node) {
  hash(static_cast<std::int32_t>(SerializableTypeDefinition::Type::unionDef));
  forEachSortedByKey(
      node.fields(),
      [](const auto& f) { return f.identity().id(); },
      [this](auto, const auto& f) { hash(f); });

  hash(node.isSealed());
  hash(node.annotations());
}

void Hasher::hash(const EnumNode& node) {
  hash(static_cast<std::int32_t>(SerializableTypeDefinition::Type::enumDef));
  forEachSortedByKey(
      node.values(),
      [](const auto& v) { return v.i32; },
      [this](auto, const auto& v) {
        hash(std::string_view{v.name});
        hash(v.i32);
        hash(v.annotations());
      });

  hash(node.annotations());
}

void Hasher::hash(const OpaqueAliasNode& node) {
  hash(
      static_cast<std::int32_t>(
          SerializableTypeDefinition::Type::opaqueAliasDef));
  hash(node.targetType().id());
  hash(node.annotations());
}

void Hasher::hash(const AnnotationsMap& annotations) {
  hashUnorderedByDigest(annotations, [](Hasher& h, const auto& entry) {
    h.hash(std::string_view{entry.first});
    h.hash(entry.second);
  });
}

void Hasher::hash(const FieldDefinition& field) {
  hash(static_cast<std::int16_t>(field.identity().id()));
  hash(std::string_view{field.identity().name()});
  hash(static_cast<std::int32_t>(field.presence()));
  hash(field.type().id());

  if (auto def = field.customDefault()) {
    hash(*def);
  }
  hash(field.annotations());
}

// --- Serializable TypeSystem nodes --- //

void Hasher::hash(const SerializableTypeDefinition& def) {
  hash(static_cast<std::int32_t>(def.getType()));

  if (auto s = def.structDef()) {
    hash(*s);
  } else if (auto u = def.unionDef()) {
    hash(*u);
  } else if (auto e = def.enumDef()) {
    hash(*e);
  } else if (auto o = def.opaqueAliasDef()) {
    hash(*o);
  } else {
    folly::throw_exception<std::invalid_argument>(
        "TypeSystemHasher: unhandled SerializableTypeDefinition variant");
  }
}

void Hasher::hash(const SerializableStructDefinition& def) {
  forEachSortedByKey(
      *def.fields(),
      [](const auto& f) { return f.identity()->id(); },
      [this](auto, const auto& f) { hash(f); });

  hash(*def.isSealed());
  hash(*def.annotations());
}

void Hasher::hash(const SerializableUnionDefinition& def) {
  forEachSortedByKey(
      *def.fields(),
      [](const auto& f) { return f.identity()->id(); },
      [this](auto, const auto& f) { hash(f); });

  hash(*def.isSealed());
  hash(*def.annotations());
}

void Hasher::hash(const SerializableEnumDefinition& def) {
  forEachSortedByKey(
      *def.values(),
      [](const auto& v) { return *v.datum(); },
      [this](auto, const auto& v) {
        hash(std::string_view{*v.name()});
        hash(*v.datum());
        hash(*v.annotations());
      });

  hash(*def.annotations());
}

void Hasher::hash(const SerializableOpaqueAliasDefinition& def) {
  hash(*def.targetType());
  hash(*def.annotations());
}

void Hasher::hash(const SerializableFieldDefinition& field) {
  hash(static_cast<std::int16_t>(field.identity()->id()));
  hash(std::string_view{field.identity()->name()});
  hash(static_cast<std::int32_t>(*field.presence()));
  hash(*field.type());

  if (auto def = field.customDefaultPartialRecord()) {
    hash(*def);
  }
  hash(*field.annotations());
}

void Hasher::hash(
    const folly::F14FastMap<std::string, SerializableRecordUnion>&
        annotations) {
  hashUnorderedByDigest(annotations, [](Hasher& h, const auto& entry) {
    h.hash(std::string_view{entry.first});
    h.hash(entry.second);
  });
}

void Hasher::hash(const SerializableRecordUnion& record) {
  hash(record.getType());
  if (auto v = record.boolDatum()) {
    hash(*v);
  } else if (auto v = record.int8Datum()) {
    hash(*v);
  } else if (auto v = record.int16Datum()) {
    hash(*v);
  } else if (auto v = record.int32Datum()) {
    hash(*v);
  } else if (auto v = record.int64Datum()) {
    hash(*v);
  } else if (auto v = record.float32Datum()) {
    hash(*v);
  } else if (auto v = record.float64Datum()) {
    hash(*v);
  } else if (auto v = record.textDatum()) {
    hash(std::string_view{*v});
  } else if (auto v = record.byteArrayDatum()) {
    hash(*v);
  } else if (auto v = record.fieldSetDatum()) {
    forEachSortedByKey(
        *v,
        [](const auto& entry) { return static_cast<int16_t>(entry.first); },
        [this](int16_t id, const auto& entry) {
          hash(id);
          hash(entry.second);
        });
  } else if (auto v = record.listDatum()) {
    for (const auto& elem : *v) {
      hash(elem);
    }
  } else if (auto v = record.setDatum()) {
    hashUnorderedByDigest(
        *v, [](Hasher& h, const auto& elem) { h.hash(elem); });
  } else if (auto v = record.mapDatum()) {
    // Sort by key digest, then hash key+value pairs in order
    hashMapByKeyDigest(
        *v,
        [](Hasher& h, const auto& entry) { h.hash(*entry.key()); },
        [](Hasher& h, const auto& entry) {
          h.hash(*entry.key());
          h.hash(*entry.value());
        });
  } else {
    folly::throw_exception<std::invalid_argument>(
        "TypeSystemHasher: unhandled SerializableRecordUnion variant");
  }
}

void Hasher::hash(const SerializableRecord& record) {
  hash(kindToThriftFieldId(record.kind()));

  record.visit(
      [this]<typename T>(detail::PrimitiveDatum<T> v) { hash(v); },
      [this](const SerializableRecord::Text& v) { hash(std::string_view{v}); },
      [this](const SerializableRecord::ByteArray& v) { hash(*v); },
      [this](const SerializableRecord::FieldSet& v) {
        forEachSortedByKey(
            v,
            [](const auto& entry) { return static_cast<int16_t>(entry.first); },
            [this](std::int16_t id, const auto& entry) {
              hash(id);
              hash(entry.second);
            });
      },
      [this](const SerializableRecord::List& v) {
        for (const auto& elem : v) {
          hash(elem);
        }
      },
      [this](const SerializableRecord::Set& v) {
        hashUnorderedByDigest(
            v, [](Hasher& h, const SerializableRecord& elem) { h.hash(elem); });
      },
      [this](const SerializableRecord::Map& v) {
        hashMapByKeyDigest(
            v,
            [](Hasher& h, const auto& entry) { h.hash(entry.first); },
            [](Hasher& h, const auto& entry) {
              h.hash(entry.first);
              h.hash(entry.second);
            });
      });
}

} // namespace

TypeSystemDigest TypeSystemHasher::operator()(
    const SerializableTypeSystem& typeSystem) const {
  Hasher h;

  h.hash(kTypeSystemDigestVersion);

  forEachSortedByKey(
      *typeSystem.types(),
      [](const auto& entry) -> std::string_view { return entry.first; },
      [&h](std::string_view uri, const auto& entry) {
        // sourceInfo is intentionally NOT hashed
        h.hash(uri);
        h.hash(*entry.second.definition());
      });

  return h.finalize();
}

TypeSystemDigest TypeSystemHasher::operator()(
    const TypeSystem& typeSystem) const {
  Hasher h;

  h.hash(kTypeSystemDigestVersion);

  auto uris = typeSystem.getKnownUris();
  if (!uris) {
    folly::throw_exception<std::runtime_error>(
        "TypeSystem does not support getKnownUris");
  }

  forEachSortedByKey(
      *uris,
      [](const auto& uri) -> std::string_view { return uri; },
      [&h, &typeSystem](std::string_view uri, const auto&) {
        // sourceInfo is intentionally NOT hashed
        h.hash(uri);
        auto defRef = typeSystem.getUserDefinedTypeOrThrow(uri);
        h.hash(TypeRef::fromDefinition(defRef));
      });

  return h.finalize();
}

TypeSystemDigest TypeSystemHasher::operator()(const TypeRef& typeRef) const {
  Hasher h;
  h.hash(typeRef);
  return h.finalize();
}

TypeSystemDigest TypeSystemHasher::operator()(
    const SerializableTypeDefinition& def) const {
  Hasher h;
  h.hash(def);
  return h.finalize();
}

TypeSystemDigest TypeSystemHasher::operator()(
    const SerializableStructDefinition& def) const {
  Hasher h;
  h.hash(
      static_cast<std::int32_t>(SerializableTypeDefinition::Type::structDef));
  h.hash(def);
  return h.finalize();
}

TypeSystemDigest TypeSystemHasher::operator()(
    const SerializableUnionDefinition& def) const {
  Hasher h;
  h.hash(static_cast<std::int32_t>(SerializableTypeDefinition::Type::unionDef));
  h.hash(def);
  return h.finalize();
}

TypeSystemDigest TypeSystemHasher::operator()(
    const SerializableEnumDefinition& def) const {
  Hasher h;
  h.hash(static_cast<std::int32_t>(SerializableTypeDefinition::Type::enumDef));
  h.hash(def);
  return h.finalize();
}

TypeSystemDigest TypeSystemHasher::operator()(
    const SerializableOpaqueAliasDefinition& def) const {
  Hasher h;
  h.hash(
      static_cast<std::int32_t>(
          SerializableTypeDefinition::Type::opaqueAliasDef));
  h.hash(def);
  return h.finalize();
}

TypeSystemDigest TypeSystemHasher::operator()(const TypeId& typeId) const {
  Hasher h;
  h.hash(typeId);
  return h.finalize();
}

TypeSystemDigest TypeSystemHasher::operator()(const TypeIdUnion& typeId) const {
  Hasher h;
  h.hash(typeId);
  return h.finalize();
}

} // namespace apache::thrift::type_system
