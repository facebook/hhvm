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

#include <thrift/conformance/cpp2/internal/AnyRegistry.h>

#include <any>
#include <forward_list>
#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

#include <folly/FBString.h>
#include <folly/Utility.h>
#include <folly/container/F14Map.h>
#include <thrift/conformance/cpp2/AnyRef.h>
#include <thrift/conformance/cpp2/AnySerializer.h>
#include <thrift/conformance/cpp2/AnyStructSerializer.h>
#include <thrift/conformance/cpp2/ThriftTypeInfo.h>
#include <thrift/conformance/if/gen-cpp2/any_types.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/op/StdSerializer.h>
#include <thrift/lib/cpp2/type/TypeRegistry.h>

namespace apache::thrift::conformance {

// A registry for serializers for use with apache::thrift::conformance::Any.
//
// This registry can be used to store and load
// apache::thrift::conformance::Any values using the registered serializers.
class AnyRegistry {
 public:
  // Returns the registry for structs with generated code linked in.
  static const AnyRegistry& generated() {
    return detail::getGeneratedAnyRegistry();
  }

  // Store a value in an Any using the registered serializers.
  //
  // Throws std::out_of_range if no matching serializer has been registered.
  Any store(any_ref value, const Protocol& protocol) const;
  Any store(const Any& value, const Protocol& protocol) const;
  template <StandardProtocol protocol>
  Any store(any_ref value) const {
    return store(value, getStandardProtocol<protocol>());
  }

  // Load a value from an Any using the registered serializers.
  //
  // Unless out refers to an empty std::any, the value is deserialized directly
  // into the referenced object, and the standard deserialization semantics will
  // hold. For example, set fields won't be cleared if not present in the
  // serialized value.
  //
  // Throws std::out_of_range if no matching serializer has been registered.
  // Throws std::bad_any_cast if value cannot be stored in out.
  void load(const Any& value, any_ref out) const;
  std::any load(const Any& value) const;
  template <typename T>
  T load(const Any& value) const;

  // Register a type.
  //
  // Throws std::invalid_argument if type is invalid.
  // Returns false iff the type conflicts with an existing registration.
  bool registerType(const std::type_info& typeInfo, ThriftTypeInfo type);

  // Register a serializer for a given type.
  //
  // Throws std::out_of_range error if the type has not already been
  // registered.
  // Throws std::invalid_argument if serializer contains an invalid protocol.
  // Returns false iff the serializer conflicts with an existing registration.
  bool registerSerializer(
      const std::type_info& typeInfo, const AnySerializer* serializer);
  bool registerSerializer(
      const std::type_info& typeInfo,
      std::unique_ptr<AnySerializer> serializer);

  // Returns the unique type uri for the given type, or "" if the type has not
  // been registered.
  std::string_view getTypeUri(const std::type_info& typeInfo) const noexcept;
  std::string_view getTypeUri(const Any& value) const noexcept;

  // Returns the std::type_info associated with the give value.
  //
  // Throws std::out_of_range error if the type has not been registered.
  const std::type_info& getTypeId(const Any& value) const;
  // Same as above, except returns nullptr if the type has not been registered.
  const std::type_info* tryGetTypeId(const Any& value) const noexcept;

  // Returns the serializer for the given type and protocol, or nullptr if
  // no matching serializer is found.
  const AnySerializer* getSerializer(
      const std::type_info& typeInfo, const Protocol& protocol) const noexcept;
  const AnySerializer* getSerializerByUri(
      const std::string_view uri, const Protocol& protocol) const noexcept;
  const AnySerializer* getSerializerByHash(
      type::UniversalHashAlgorithm alg,
      const folly::fbstring& typeHash,
      const Protocol& protocol) const;

  // Compile-time Type overloads.
  template <typename C = std::initializer_list<const AnySerializer*>>
  bool registerType(
      const std::type_info& typeInfo, ThriftTypeInfo type, C&& serializers);

  template <
      typename T,
      typename C = std::initializer_list<const AnySerializer*>>
  bool registerType(ThriftTypeInfo type, C&& serializers) {
    return registerType(
        typeid(T), std::move(type), std::forward<C>(serializers));
  }

  template <typename T, StandardProtocol... Ps>
  bool registerType(ThriftTypeInfo type) {
    return registerType(
        typeid(T), std::move(type), {&getAnyStandardSerializer<T, Ps>()...});
  }

  template <typename T>
  bool registerSerializer(const AnySerializer* serializer) {
    return registerSerializer(typeid(T), serializer);
  }
  template <typename T>
  bool registerSerializer(std::unique_ptr<AnySerializer> serializer) {
    return registerSerializer(typeid(T), std::move(serializer));
  }

  template <typename T>
  const AnySerializer* getSerializer(const Protocol& protocol) const noexcept {
    return getSerializer(typeid(T), protocol);
  }
  template <typename T, StandardProtocol P>
  const AnySerializer* getSerializer() const noexcept {
    return getSerializer<T>(getStandardProtocol<P>());
  }

  template <typename T>
  std::string_view getTypeUri() const noexcept {
    return getTypeUri(typeid(T));
  }

  // Generates a summary of the contents of the registry.
  std::string debugString() const;

  // Allows an invalid type name to be registered.
  [[deprecated("Do not use. Will be removed.")]] bool forceRegisterType(
      const std::type_info& typeInfo, std::string type);

 private:
  template <typename K, typename M>
  using node_map_t = folly::conditional_t<
      folly::kIsSanitizeThread,
      std::unordered_map<K, M>,
      folly::F14NodeMap<K, M>>;
  template <typename K, typename M>
  using fast_map_t = folly::conditional_t<
      folly::kIsSanitizeThread,
      std::unordered_map<K, M>,
      folly::F14FastMap<K, M>>;

  struct TypeEntry {
    TypeEntry(const std::type_info& typeInfo, ThriftTypeInfo type);

    const std::type_info& typeInfo;
    const folly::fbstring typeHash; // The type hash to use, if applicable.
    const ThriftTypeInfo type; // Referenced by uriIndex_.
    fast_map_t<Protocol, const AnySerializer*> serializers;
  };

  std::forward_list<std::unique_ptr<AnySerializer>> ownedSerializers_;
  // Referenced by uriIndex_ and idIndex_, so must be a Node map.
  node_map_t<std::type_index, TypeEntry> registry_;

  fast_map_t<std::string_view, TypeEntry*> uriIndex_;
  std::map<folly::fbstring, TypeEntry*> hashIndex_; // Must be sorted.

  TypeEntry* registerTypeImpl(
      const std::type_info& typeInfo, ThriftTypeInfo type);
  static bool registerSerializerImpl(
      const AnySerializer* serializer, TypeEntry* entry);
  bool registerSerializerImpl(
      std::unique_ptr<AnySerializer> serializer, TypeEntry* entry);

  bool genTypeHashsAndCheckForConflicts(
      std::string_view uri,
      std::vector<folly::fbstring>* typeHashs) const noexcept;
  bool genTypeHashsAndCheckForConflicts(
      const ThriftTypeInfo& typeInfo,
      std::vector<folly::fbstring>* typeHashs) const noexcept;
  void indexUri(std::string_view uri, TypeEntry* entry) noexcept;
  void indexHash(folly::fbstring&& typeHash, TypeEntry* entry) noexcept;

  // Gets the TypeEntry for the given type, or null if the type has not been
  // registered.
  const TypeEntry* getTypeEntry(
      const std::type_index& typeIndex) const noexcept;
  const TypeEntry* getTypeEntry(const std::type_info& typeInfo) const noexcept {
    return getTypeEntry(std::type_index(typeInfo));
  }
  const TypeEntry* getTypeEntryFor(const Any& value) const noexcept;
  // Look up TypeEntry by secondary index.
  const TypeEntry* getTypeEntryByUri(std::string_view uri) const noexcept;
  const TypeEntry* getTypeEntryByHash(
      const folly::fbstring& typeHash) const noexcept;

  // Same as the getTypeEntry* versions, except throws an exception that
  // indicates why the type entry could not be found.
  const TypeEntry& getAndCheckTypeEntry(const std::type_info& typeInfo) const;
  const TypeEntry& getAndCheckTypeEntryByUri(std::string_view uri) const;
  const TypeEntry& getAndCheckTypeEntryByHash(
      const folly::fbstring& typeHash) const;
  const TypeEntry& getAndCheckTypeEntryFor(const Any& value) const;
  const AnySerializer& getAndCheckSerializer(
      const TypeEntry& entry, const Protocol& protocol) const;

  const AnySerializer* getSerializer(
      const TypeEntry* entry, const Protocol& protocol) const noexcept;
};

// Implementation details.

template <typename T>
T AnyRegistry::load(const Any& value) const {
  T out;
  load(value, out);
  return out;
}

template <typename C>
bool AnyRegistry::registerType(
    const std::type_info& typeInfo, ThriftTypeInfo type, C&& serializers) {
  TypeEntry* entry = registerTypeImpl(typeInfo, std::move(type));
  if (entry == nullptr) {
    return false;
  }

  // NOTE: Only fails if serializers are redundant.
  // TODO(afuller): Make success atomic.
  bool success = true;
  for (auto&& serializer : std::forward<C>(serializers)) {
    success &= registerSerializerImpl(
        std::forward<decltype(serializer)>(serializer), entry);
  }
  return success;
}

namespace detail {

template <typename T, StandardProtocol... Ps>
void registerGeneratedStruct() {
  const ThriftTypeInfo& type = getGeneratedThriftTypeInfo<T>();
  if (!getGeneratedAnyRegistry().registerType<T, Ps...>(type)) {
    LOG(ERROR) << "Could not register: " << type.uri().value()
               << ". Registry state: "
               << getGeneratedAnyRegistry().debugString();
    folly::throw_exception<std::runtime_error>(
        "Could not register: " + type.uri().value());
  }
  using Tag = type::infer_tag<T>;
  op::registerStdSerializers<Tag, static_cast<type::StandardProtocol>(Ps)...>(
      type::detail::getGeneratedTypeRegistry());
}

} // namespace detail
} // namespace apache::thrift::conformance
