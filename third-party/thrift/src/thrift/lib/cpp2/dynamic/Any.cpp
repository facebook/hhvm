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

#include <thrift/lib/cpp2/dynamic/Any.h>

#include <thrift/lib/cpp2/dynamic/DynamicValue.h>
#include <thrift/lib/cpp2/dynamic/Serialization.h>
#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/cpp2/dynamic/TypeSystemTraits.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/SimpleJSONProtocol.h>

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <folly/lang/Exception.h>

namespace apache::thrift::dynamic {

DynamicValue Any::load(
    const type_system::TypeSystem& typeSystem,
    std::pmr::memory_resource* mr) const {
  if (!hasValue()) {
    folly::throw_exception<std::runtime_error>("Cannot load empty Any");
  }

  // Resolve the type::Type to a TypeRef
  auto typeRef = type_system::resolveAnyType(typeSystem, data_.type());

  // Delegate to the TypeRef overload
  return load(typeRef, mr);
}

DynamicValue Any::load(
    type_system::TypeRef typeRef, std::pmr::memory_resource* mr) const {
  if (!hasValue()) {
    folly::throw_exception<std::runtime_error>("Cannot load empty Any");
  }

  // Verify that the provided TypeRef matches the stored type
  auto expectedType = type_system::toAnyType(typeRef);
  if (data_.type() != expectedType) {
    folly::throw_exception<std::runtime_error>(
        "Type mismatch: Any contains different type than requested");
  }

  // Deserialize using the provided TypeRef
  if (data_.protocol() ==
      type::Protocol::get<type::StandardProtocol::Binary>()) {
    BinaryProtocolReader reader;
    reader.setInput(&data_.data());
    return deserializeValue(reader, typeRef, mr);
  } else if (
      data_.protocol() ==
      type::Protocol::get<type::StandardProtocol::Compact>()) {
    CompactProtocolReader reader;
    reader.setInput(&data_.data());
    return deserializeValue(reader, typeRef, mr);
  } else if (
      data_.protocol() ==
      type::Protocol::get<type::StandardProtocol::SimpleJson>()) {
    SimpleJSONProtocolReader reader;
    reader.setInput(&data_.data());
    return deserializeValue(reader, typeRef, mr);
  } else {
    folly::throw_exception<std::runtime_error>(
        "Unsupported protocol for Any deserialization");
  }
}

/* static */ Any Any::store(
    const DynamicValue& value,
    type::StandardProtocol protocol,
    std::pmr::memory_resource* mr) {
  // Convert TypeRef to type::Type
  auto anyType = type_system::toAnyType(value.type());

  // Serialize the value with the specified protocol
  folly::IOBufQueue bufQueue(folly::IOBufQueue::cacheChainLength());

  if (protocol == type::StandardProtocol::Binary) {
    BinaryProtocolWriter writer;
    writer.setOutput(&bufQueue);
    serializeValue(writer, value);
  } else if (protocol == type::StandardProtocol::Compact) {
    CompactProtocolWriter writer;
    writer.setOutput(&bufQueue);
    serializeValue(writer, value);
  } else if (protocol == type::StandardProtocol::SimpleJson) {
    SimpleJSONProtocolWriter writer;
    writer.setOutput(&bufQueue);
    serializeValue(writer, value);
  } else {
    folly::throw_exception<std::runtime_error>(
        "Unsupported protocol for Any serialization");
  }

  // Create AnyData from the serialized data
  type::AnyStruct any;
  any.data() = bufQueue.moveAsValue();
  any.protocol() = protocol;
  any.type() = std::move(anyType);

  return Any(type::AnyData{std::move(any)}, mr);
}

} // namespace apache::thrift::dynamic
