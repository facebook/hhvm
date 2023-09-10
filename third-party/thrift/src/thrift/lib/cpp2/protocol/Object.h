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

#include <utility>

#include <folly/dynamic.h>
#include <thrift/lib/cpp/protocol/TProtocolException.h>
#include <thrift/lib/cpp/protocol/TType.h>
#include <thrift/lib/cpp2/protocol/detail/Object.h>
#include <thrift/lib/thrift/gen-cpp2/protocol_types.h>

namespace apache::thrift::protocol {

// Creates a Value struct for the given value.
//
// TT: The thrift type to use, for example
// apache::thrift::type::binary_t.
template <typename TT, typename T = type::native_type<TT>>
Value asValueStruct(T&& value) {
  Value result;
  detail::ValueHelper<TT>::set(result, std::forward<T>(value));
  return result;
}

// Schemaless deserialization of thrift serialized data
// into protocol::Object
// Protocol: protocol to use eg. apache::thrift::BinaryProtocolReader
// buf: serialized payload
// Works for binary, compact. Does not work for SimpleJson protocol as it does
// not save fieldID and field type information in serialized data. Does not work
// with json protocol because both binary & string is marked as T_STRING type in
// serailized data but both are encoded differently. Binary is base64 encoded
// and string is written as is. So during deserialization we cannot decode it
// correctly without schema. String fields are currently saved in binaryValue.
template <class Protocol>
Object parseObject(const folly::IOBuf& buf, bool string_to_binary = true) {
  Protocol prot;
  prot.setInput(&buf);
  auto result = detail::parseValue(prot, protocol::T_STRUCT, string_to_binary);
  return std::move(*result.objectValue_ref());
}

// Schemaless deserialization of thrift serialized data with mask.
// Only parses values that are masked. Unmasked fields are stored in MaskedData.
template <typename Protocol>
Object parseObjectWithoutExcludedData(
    const folly::IOBuf& buf, Mask mask, bool string_to_binary = true) {
  return detail::parseObject<Protocol, false>(
             buf, mask, noneMask(), string_to_binary)
      .included;
}

// Schemaless deserialization of thrift serialized data with mask.
// Only parses values that are masked. Unmasked fields are stored in MaskedData.
template <typename Protocol>
MaskedDecodeResult parseObject(
    const folly::IOBuf& buf, Mask mask, bool string_to_binary = true) {
  return detail::parseObject<Protocol, true>(
      buf, mask, noneMask(), string_to_binary);
}

// Schemaless deserialization of thrift serialized data with readMask and
// writeMask. Only parses values that are masked by readMask. Fields that are
// not in neither writeMask nor readMask are stored in MaskedData.
template <typename Protocol>
MaskedDecodeResult parseObject(
    const folly::IOBuf& buf,
    Mask readMask,
    Mask writeMask,
    bool string_to_binary = true) {
  return detail::parseObject<
      Protocol,
      true /* always keep excluded data with writeMask */>(
      buf, readMask, writeMask, string_to_binary);
}

// Schemaless serialization of protocol::Value into thrift serialization
// protocol Protocol: protocol to use eg. apache::thrift::BinaryProtocolWriter
// val: Value to be serialized Serialized output is same as schema based
// serialization except when struct contains an empty list, set or map
template <class Protocol>
std::unique_ptr<folly::IOBuf> serializeValue(const Value& val) {
  Protocol prot;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  prot.setOutput(&queue);
  detail::serializeValue(prot, val);
  return queue.move();
}

// Schemaless serialization of protocol::Object into thrift serialization
// protocol Protocol: protocol to use eg. apache::thrift::BinaryProtocolWriter
// obj: object to be serialized Serialized output is same as schema based
// serialization except when struct contains an empty list, set or map
template <class Protocol>
std::unique_ptr<folly::IOBuf> serializeObject(const Object& val) {
  Protocol prot;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  prot.setOutput(&queue);
  detail::serializeObject(prot, val);
  return queue.move();
}

// Serialization of protocol::Object with MaskedProtocolData.
template <class Protocol>
std::unique_ptr<folly::IOBuf> serializeObject(
    const Object& obj, MaskedProtocolData& protocolData) {
  assert(*protocolData.protocol() == detail::get_standard_protocol<Protocol>);
  Protocol prot;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  prot.setOutput(&queue);
  Value val;
  val.emplace_object(obj);
  if (protocolData.data()->full_ref()) { // entire object is not parsed
    const EncodedValue& value = detail::getByValueId(
        *protocolData.values(), protocolData.data()->full_ref().value());
    prot.writeRaw(*value.data());
  } else if (!protocolData.data()->fields_ref()) { // entire object is parsed
    detail::serializeValue(prot, val);
  } else { // use both object and masked data to serialize
    detail::serializeValue(prot, val, protocolData, *protocolData.data());
  }
  return queue.move();
}

template <class Protocol>
Value parseValue(
    const folly::IOBuf& buf,
    apache::thrift::type::BaseType baseType,
    bool string_to_binary = true) {
  Protocol prot;
  prot.setInput(&buf);
  return detail::parseValue(prot, type::toTType(baseType), string_to_binary);
}

template <class Protocol, typename Tag>
Value parseValue(const folly::IOBuf& buf, bool string_to_binary = true) {
  return parseValue<Protocol>(
      buf, type::detail::getBaseType(Tag{}), string_to_binary);
}

// Returns whether the protocol::Value/ Object is its intrinsic default.
bool isIntrinsicDefault(const Value& value);
bool isIntrinsicDefault(const Object& obj);
folly::dynamic toDynamic(const Value& value);
folly::dynamic toDynamic(const Object& obj);

using detail::toAny;

} // namespace apache::thrift::protocol
