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

#include <thrift/conformance/cpp2/AnyRegistry.h>

#include <glog/logging.h>

#include <folly/CppAttributes.h>
#include <folly/Demangle.h>
#include <folly/Singleton.h>
#include <folly/String.h>
#include <folly/io/Cursor.h>
#include <thrift/conformance/cpp2/Any.h>
#include <thrift/lib/cpp2/type/UniversalName.h>

namespace apache::thrift::conformance {
using type::containsUniversalHash;
using type::findByUniversalHash;
using type::getUniversalHash;
using type::getUniversalHashPrefix;
using type::hash_size_t;
using type::maybeGetUniversalHashPrefix;
namespace detail {

AnyRegistry& getGeneratedAnyRegistry() {
  struct GeneratedTag {};
  return folly::detail::createGlobal<AnyRegistry, GeneratedTag>();
}

} // namespace detail

namespace {

folly::fbstring maybeGetTypeHash(
    const ThriftTypeInfo& type,
    hash_size_t defaultTypeHashBytes = type::kDefaultTypeHashBytes) {
  if (type.typeHashBytes().has_value()) {
    // Use the custom size.
    defaultTypeHashBytes = type.typeHashBytes().value_unchecked();
  }
  return maybeGetUniversalHashPrefix(
      type::UniversalHashAlgorithm::Sha2_256,
      type.uri().value(),
      defaultTypeHashBytes);
}

} // namespace

AnyRegistry::TypeEntry::TypeEntry(
    const std::type_info& typeInfo, ThriftTypeInfo type)
    : typeInfo(typeInfo),
      typeHash(maybeGetTypeHash(type)),
      type(std::move(type)) {}

bool AnyRegistry::registerType(
    const std::type_info& typeInfo, ThriftTypeInfo type) {
  return registerTypeImpl(typeInfo, std::move(type)) != nullptr;
}

bool AnyRegistry::registerSerializer(
    const std::type_info& type, const AnySerializer* serializer) {
  return registerSerializerImpl(
      serializer, &registry_.at(std::type_index(type)));
}

bool AnyRegistry::registerSerializer(
    const std::type_info& type, std::unique_ptr<AnySerializer> serializer) {
  return registerSerializerImpl(
      std::move(serializer), &registry_.at(std::type_index(type)));
}

std::string_view AnyRegistry::getTypeUri(
    const std::type_info& type) const noexcept {
  const auto* entry = getTypeEntry(type);
  if (entry == nullptr) {
    return {};
  }
  return entry->type.uri().value();
}

std::string_view AnyRegistry::getTypeUri(const Any& value) const noexcept {
  const auto* entry = getTypeEntryFor(value);
  if (entry == nullptr) {
    return {};
  }
  return entry->type.uri().value();
}

const std::type_info& AnyRegistry::getTypeId(const Any& value) const {
  return getAndCheckTypeEntryFor(value).typeInfo;
}

// Same as above, except returns nullptr if the type has not been registered.
const std::type_info* AnyRegistry::tryGetTypeId(
    const Any& value) const noexcept {
  const auto* entry = getTypeEntryFor(value);
  if (entry == nullptr) {
    return nullptr;
  }
  return &entry->typeInfo;
}

const AnySerializer* AnyRegistry::getSerializer(
    const std::type_info& type, const Protocol& protocol) const noexcept {
  return getSerializer(getTypeEntry(type), protocol);
}

const AnySerializer* AnyRegistry::getSerializerByUri(
    const std::string_view uri, const Protocol& protocol) const noexcept {
  return getSerializer(getTypeEntryByUri(uri), protocol);
}

const AnySerializer* AnyRegistry::getSerializerByHash(
    type::UniversalHashAlgorithm alg,
    const folly::fbstring& typeHash,
    const Protocol& protocol) const {
  if (alg != type::UniversalHashAlgorithm::Sha2_256) {
    folly::throw_exception<std::runtime_error>(
        "Unsupported hash algorithm: " + std::to_string(static_cast<int>(alg)));
  }
  return getSerializer(getTypeEntryByHash(typeHash), protocol);
}

Any AnyRegistry::store(any_ref value, const Protocol& protocol) const {
  if (value.type() == typeid(Any)) {
    // Use the Any specific overload.
    return store(any_cast<const Any&>(value), protocol);
  }

  const auto& entry = getAndCheckTypeEntry(value.type());
  const auto& serializer = getAndCheckSerializer(entry, protocol);

  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  // Allocate 16KB at a time; leave some room for the IOBuf overhead
  constexpr size_t kDesiredGrowth = (1 << 14) - 64;
  serializer.encode(value, folly::io::QueueAppender(&queue, kDesiredGrowth));

  Any result;
  if (entry.typeHash.empty()) {
    result.type() = entry.type.uri().value();
  } else {
    result.typeHashPrefixSha2_256() = entry.typeHash;
  }
  setProtocol(protocol, result);
  result.data() = queue.moveAsValue();
  return result;
}

Any AnyRegistry::store(const Any& value, const Protocol& protocol) const {
  if (hasProtocol(value, protocol)) {
    return value;
  }
  return store(load(value), protocol);
}

void AnyRegistry::load(const Any& value, any_ref out) const {
  const auto& entry = getAndCheckTypeEntryFor(value);
  const auto& serializer = getAndCheckSerializer(entry, getProtocol(value));
  folly::io::Cursor cursor(&*value.data());
  serializer.decode(entry.typeInfo, cursor, out);
}

std::any AnyRegistry::load(const Any& value) const {
  std::any out;
  load(value, out);
  return out;
}

std::string AnyRegistry::debugString() const {
  std::string result = "AnyRegistry[\n";
  // Using the sorted map, hashIndex_, to produce stable results.
  for (const auto& indx : hashIndex_) {
    const TypeEntry& entry = *indx.second;
    result += "  ";
    result += entry.type.uri().value();
    result += " (";
    result += folly::hexlify(indx.first);
    result += ")";
    if (!entry.serializers.empty()) {
      result += ":\n";
      // Convert to a set, so output is deterministic.
      std::set<Protocol> protocols;
      for (const auto& ser : entry.serializers) {
        protocols.emplace(ser.first);
      }
      for (const auto& protocol : protocols) {
        result += "    ";
        result += protocol.name();
        result += ",\n";
      }
    } else {
      result += ",\n";
    }
  }
  result += "]";
  return result;
}

bool AnyRegistry::forceRegisterType(
    const std::type_info& typeInfo, std::string type) {
  if (getTypeEntryByUri(type) != nullptr) {
    return false;
  }

  ThriftTypeInfo info;
  info.uri() = std::move(type);
  info.typeHashBytes() = 0;

  auto result = registry_.emplace(
      std::type_index(typeInfo), TypeEntry(typeInfo, std::move(info)));
  if (!result.second) {
    return false;
  }

  TypeEntry* entry = &result.first->second;
  indexUri(*entry->type.uri(), entry);
  return true;
}

auto AnyRegistry::registerTypeImpl(
    const std::type_info& typeInfo, ThriftTypeInfo type) -> TypeEntry* {
  validateThriftTypeInfo(type);
  std::vector<folly::fbstring> typeHashs;
  typeHashs.reserve(type.altUris()->size() + 1);
  if (!genTypeHashsAndCheckForConflicts(type, &typeHashs)) {
    return nullptr;
  }

  auto result = registry_.emplace(
      std::type_index(typeInfo), TypeEntry(typeInfo, std::move(type)));
  if (!result.second) {
    return nullptr;
  }

  TypeEntry* entry = &result.first->second;

  // Add to secondary indexes.
  indexUri(*entry->type.uri(), entry);
  for (const auto& alias : *entry->type.altUris()) {
    indexUri(alias, entry);
  }

  for (auto& hash : typeHashs) {
    indexHash(std::move(hash), entry);
  }
  return &result.first->second;
}

bool AnyRegistry::registerSerializerImpl(
    const AnySerializer* serializer, TypeEntry* entry) {
  if (serializer == nullptr) {
    return false;
  }
  validateProtocol(serializer->getProtocol());
  return entry->serializers.emplace(serializer->getProtocol(), serializer)
      .second;
}

bool AnyRegistry::registerSerializerImpl(
    std::unique_ptr<AnySerializer> serializer, TypeEntry* entry) {
  if (!registerSerializerImpl(serializer.get(), entry)) {
    return false;
  }
  ownedSerializers_.emplace_front(std::move(serializer));
  return true;
}

bool AnyRegistry::genTypeHashsAndCheckForConflicts(
    std::string_view uri,
    std::vector<folly::fbstring>* typeHashs) const noexcept {
  if (uri.empty() || uriIndex_.contains(uri)) {
    return false; // Already exists.
  }

  auto typeHash = getUniversalHash(type::UniversalHashAlgorithm::Sha2_256, uri);
  // Find shortest valid type hash prefix.
  folly::fbstring minTypeHash(
      getUniversalHashPrefix(typeHash, kMinTypeHashBytes));
  // Check if the minimum type hash would be ambiguous.
  if (containsUniversalHash(hashIndex_, minTypeHash)) {
    return false; // Ambigous with another typeHash.
  }
  typeHashs->emplace_back(std::move(typeHash));
  return true;
}

bool AnyRegistry::genTypeHashsAndCheckForConflicts(
    const ThriftTypeInfo& type,
    std::vector<folly::fbstring>* typeHashs) const noexcept {
  // Ensure uri and all aliases are availabile.
  if (!genTypeHashsAndCheckForConflicts(*type.uri(), typeHashs)) {
    return false;
  }
  for (const auto& alias : *type.altUris()) {
    if (!genTypeHashsAndCheckForConflicts(alias, typeHashs)) {
      return false;
    }
  }
  return true;
}

void AnyRegistry::indexUri(std::string_view uri, TypeEntry* entry) noexcept {
  auto res = uriIndex_.emplace(uri, entry);
  DCHECK(res.second);
}

void AnyRegistry::indexHash(
    folly::fbstring&& typeHash, TypeEntry* entry) noexcept {
  auto res = hashIndex_.emplace(std::move(typeHash), entry);
  DCHECK(res.second);
}

auto AnyRegistry::getTypeEntry(const std::type_index& typeIndex) const noexcept
    -> const TypeEntry* {
  auto itr = registry_.find(typeIndex);
  if (itr == registry_.end()) {
    return nullptr;
  }
  return &itr->second;
}

auto AnyRegistry::getTypeEntryByHash(
    const folly::fbstring& typeHash) const noexcept -> const TypeEntry* {
  if (typeHash.size() < kMinTypeHashBytes) {
    return nullptr;
  }
  auto itr = findByUniversalHash(hashIndex_, typeHash);
  if (itr == hashIndex_.end()) {
    // No match.
    return nullptr;
  }
  return itr->second;
}

auto AnyRegistry::getTypeEntryByUri(std::string_view uri) const noexcept
    -> const TypeEntry* {
  auto itr = uriIndex_.find(uri);
  if (itr == uriIndex_.end()) {
    return nullptr;
  }
  return itr->second;
}

auto AnyRegistry::getTypeEntryFor(const Any& value) const noexcept
    -> const TypeEntry* {
  if (value.type().has_value() && !value.type()->empty()) {
    return getTypeEntryByUri(value.type().value_unchecked());
  }
  if (value.typeHashPrefixSha2_256().has_value()) {
    return getTypeEntryByHash(value.typeHashPrefixSha2_256().value_unchecked());
  }
  return nullptr;
}

auto AnyRegistry::getAndCheckTypeEntryFor(const Any& value) const
    -> const TypeEntry& {
  if (value.type().has_value() && !value.type().value_unchecked().empty()) {
    return getAndCheckTypeEntryByUri(value.type().value_unchecked());
  }
  if (value.typeHashPrefixSha2_256().has_value()) {
    return getAndCheckTypeEntryByHash(
        value.typeHashPrefixSha2_256().value_unchecked());
  }
  throw std::invalid_argument("any must have a type");
}

const AnySerializer* AnyRegistry::getSerializer(
    const TypeEntry* entry, const Protocol& protocol) const noexcept {
  if (entry == nullptr) {
    return nullptr;
  }

  auto itr = entry->serializers.find(protocol);
  if (itr == entry->serializers.end()) {
    return nullptr;
  }
  return itr->second;
}

auto AnyRegistry::getAndCheckTypeEntry(const std::type_info& typeInfo) const
    -> const TypeEntry& {
  const TypeEntry* result = getTypeEntry(typeInfo);
  if (result == nullptr) {
    throw std::out_of_range(
        fmt::format("Type not registered: {}", folly::demangle(typeInfo)));
  }
  return *result;
}

auto AnyRegistry::getAndCheckTypeEntryByUri(std::string_view uri) const
    -> const TypeEntry& {
  const TypeEntry* result = getTypeEntryByUri(uri);
  if (result == nullptr) {
    throw std::out_of_range(fmt::format("Type uri not registered: {}", uri));
  }
  return *result;
}

auto AnyRegistry::getAndCheckTypeEntryByHash(
    const folly::fbstring& typeHash) const -> const TypeEntry& {
  const TypeEntry* result = getTypeEntryByHash(typeHash);
  if (result == nullptr) {
    throw std::out_of_range(
        fmt::format("Type hash not registered: {}", folly::hexlify(typeHash)));
  }
  return *result;
}

const AnySerializer& AnyRegistry::getAndCheckSerializer(
    const TypeEntry& entry, const Protocol& protocol) const {
  auto itr = entry.serializers.find(protocol);
  if (itr == entry.serializers.end()) {
    folly::throw_exception<std::out_of_range>(fmt::format(
        "Serializer not found: {}#{}",
        entry.type.uri().value(),
        protocol.name()));
  }
  return *itr->second;
}

} // namespace apache::thrift::conformance
