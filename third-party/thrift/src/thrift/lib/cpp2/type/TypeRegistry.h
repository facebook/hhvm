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

#include <forward_list>
#include <initializer_list>
#include <memory>
#include <typeindex>

#include <folly/Utility.h>
#include <folly/container/F14Map.h>
#include <thrift/lib/cpp2/op/Serializer.h>
#include <thrift/lib/cpp2/type/Any.h>
#include <thrift/lib/cpp2/type/AnyValue.h>
#include <thrift/lib/cpp2/type/NativeType.h>
#include <thrift/lib/cpp2/type/Protocol.h>
#include <thrift/lib/cpp2/type/Runtime.h>
#include <thrift/lib/cpp2/type/detail/TypeRegistry.h>

namespace apache::thrift::type {

// A registry of type information and functionality.
//
// All registered serializers must either be owned by the TypeRegistry, or be
// externally guarneteed to out live all TypeRegistry instances they are
// registered with.
class TypeRegistry {
 public:
  // Returns the registry for types with generated code linked in.
  static const TypeRegistry& generated() {
    return detail::getGeneratedTypeRegistry();
  }

  // Store a value in an AnyData using the registered serializers.
  //
  // Throws std::out_of_range if no matching serializer has been registered.
  AnyData store(ConstRef value, const Protocol& protocol) const;
  template <StandardProtocol P>
  AnyData store(ConstRef value) const {
    return store(value, Protocol::get<P>());
  }
  template <
      typename T,
      std::enable_if_t<!std::is_constructible_v<ConstRef, T>, bool> = true>
  AnyData store(T&& value, const Protocol& protocol) const {
    return storeImpl(AnyConstRef(value), protocol);
  }
  template <
      StandardProtocol P,
      typename T,
      std::enable_if_t<!std::is_constructible_v<ConstRef, T>, bool> = true>
  AnyData store(T&& value) const {
    return store(std::forward<T>(value), Protocol::get<P>());
  }

  template <typename Tag>
  AnyData store(const native_type<Tag>& value, const Protocol& protocol) const {
    return store(Ref::to<Tag>(value), protocol);
  }
  template <typename Tag, StandardProtocol P>
  AnyData store(const native_type<Tag>& value) const {
    return store(Ref::to<Tag>(value), Protocol::get<P>());
  }

  // Load a value from an AnyData using the registered serializers.
  //
  // Unless out refers to an empty std::any, the value is deserialized directly
  // into the referenced object, and the standard deserialization semantics will
  // hold. For example, set fields won't be cleared if not present in the
  // serialized value.
  //
  // Throws std::out_of_range if no matching serializer has been registered.
  // Throws std::bad_any_cast if value cannot be stored in out.
  void load(const AnyData& data, Ref out) const;
  void load(const AnyData& data, AnyRef out) const;
  AnyValue load(const AnyData& data) const;

  // Registers the given serializer for the given type
  //
  // All registered serializers must either be owned by the TypeRegistry or out
  // live all TypeRegistry that they have been registered with.
  //
  // Returns false if the type/protocol pair conflicts with an existing entry,
  // and cannot be registered.
  bool registerSerializer(const op::Serializer& serializer, const Type& type);
  bool registerSerializer(
      std::unique_ptr<op::Serializer> serializer, const Type& type) {
    ownedSerializers_.emplace_front(std::move(serializer));
    return registerSerializer(*ownedSerializers_.front(), type);
  }

  // Like registerSerializer above, except registers the same serializer for
  // multiple types.
  //
  // If any entry cannot be registered, false is returned, with all
  // successufully registered entries remaining registered.
  // TODO(afuller): Consider making success/failure all or nothing (aka atomic).
  template <
      typename C = std::initializer_list<Type>,
      typename = typename C::value_type>
  bool registerSerializer(const op::Serializer& serializer, const C&& types);
  template <
      typename C = std::initializer_list<Type>,
      typename = typename C::value_type>
  bool registerSerializer(
      std::unique_ptr<op::Serializer> serializer, const C&& types);

  // Check if a given type is registered.
  //
  // Returns true iff the type is registered.
  bool isRegistered(const Type& type) const;

 private:
  struct TypeEntry {
    folly::F14FastMap<Protocol, const op::Serializer*> protocols;
    const op::Serializer& getSerializer(const Protocol& protocol) const;
  };

  folly::F14FastMap<Type, TypeEntry> types_;
  std::forward_list<std::unique_ptr<op::Serializer>> ownedSerializers_;

  const TypeEntry& getEntry(const Type& type) const;

  template <typename T>
  AnyData storeImpl(T&& value, const Protocol& protocol) const;
};

// Implemenation details

template <typename C, typename>
bool TypeRegistry::registerSerializer(
    const op::Serializer& serializer, const C&& types) {
  bool success = true;
  for (const auto& type : types) {
    success &= registerSerializer(serializer, type);
  }
  return success;
}
template <typename C, typename>
bool TypeRegistry::registerSerializer(
    std::unique_ptr<op::Serializer> serializer, const C&& types) {
  if (types.empty()) {
    return true;
  }
  ownedSerializers_.emplace_front(std::move(serializer));
  return registerSerializer(*ownedSerializers_.front(), types);
}

template <typename T>
AnyData TypeRegistry::storeImpl(T&& value, const Protocol& protocol) const {
  if (value.type() == Type::get<type::void_t>()) {
    return {};
  }

  // Encode the value.
  const auto& serializer = getEntry(value.type()).getSerializer(protocol);
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());

  // Allocate 16KB at a time; leave some room for the IOBuf overhead
  // TODO(afuller): This is the size we use by default, for structs. Consider
  // adjusting it, based on what is being encoded.
  constexpr size_t kDesiredGrowth = (1 << 14) - 64;
  serializer.encode(value, folly::io::QueueAppender(&queue, kDesiredGrowth));

  // Return the resulting Any.
  SemiAny builder;
  builder.data() = queue.moveAsValue();
  builder.protocol() = protocol;
  builder.type() = value.type();
  return AnyData{std::move(builder)};
}

} // namespace apache::thrift::type
