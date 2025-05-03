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

#include <stdexcept>

#include <folly/Singleton.h>
#include <folly/lang/Exception.h>
#include <thrift/lib/cpp2/type/TypeRegistry.h>

namespace apache::thrift::type {
namespace detail {

FOLLY_EXPORT TypeRegistry& getGeneratedTypeRegistry() {
  struct GeneratedTag {};
  return folly::detail::createGlobal<TypeRegistry, GeneratedTag>();
}

} // namespace detail

AnyData TypeRegistry::store(ConstRef value, const Protocol& protocol) const {
  return storeImpl(value, protocol);
}

void TypeRegistry::load(const AnyData& data, Ref out) const {
  if (data.type() == Type::get<type::void_t>()) {
    if (out.type() != Type::get<type::void_t>()) {
      folly::throw_exception<std::bad_any_cast>();
    }
    return;
  }
  folly::io::Cursor cursor{&data.data()};
  getEntry(data.type()).getSerializer(data.protocol()).decode(cursor, out);
}

void TypeRegistry::load(const AnyData& data, AnyRef out) const {
  if (data.type() == Type::get<type::void_t>()) {
    if (out.type() != Type::get<type::void_t>()) {
      folly::throw_exception<std::bad_any_cast>();
    }
    return;
  }
  folly::io::Cursor cursor{&data.data()};
  getEntry(data.type()).getSerializer(data.protocol()).decode(cursor, out);
}

AnyValue TypeRegistry::load(const AnyData& data) const {
  if (data.type() == Type::get<type::void_t>()) {
    return {};
  }
  folly::io::Cursor cursor{&data.data()};
  return getEntry(data.type())
      .getSerializer(data.protocol())
      .decode(data.type(), cursor);
}

bool TypeRegistry::registerSerializer(
    const op::Serializer& serializer, const Type& type) {
  if (!type.isFull()) {
    folly::throw_exception<std::runtime_error>(
        "Can only register types with fill uris.");
  }
  return types_[type]
      .protocols.emplace(serializer.getProtocol(), &serializer)
      .second;
}

bool TypeRegistry::isRegistered(const Type& type) const {
  if (!type.isFull()) { // TODO: Implement look up by type hash.
    folly::throw_exception<std::out_of_range>("Not Implemented");
  }

  // Lookup by exact type.
  return types_.find(type) != types_.end();
}

auto TypeRegistry::getEntry(const Type& type) const -> const TypeEntry& {
  if (!type.isFull()) { // TODO(afuller): Implement look by type hash.
    folly::throw_exception<std::out_of_range>("Not Implemented");
  }

  // Lookup by exact type.
  auto itr = types_.find(type);
  if (itr != types_.end()) {
    return itr->second;
  }

  // TODO(afuller): Improve error message.
  folly::throw_exception<std::out_of_range>("Type not registered.");
}

const op::Serializer& TypeRegistry::TypeEntry::getSerializer(
    const Protocol& protocol) const {
  auto itr = protocols.find(protocol);
  if (itr == protocols.end()) {
    folly::throw_exception<std::out_of_range>(
        fmt::format("Protocol not registered {}.", protocol.name()));
  }
  return *itr->second;
}

} // namespace apache::thrift::type
