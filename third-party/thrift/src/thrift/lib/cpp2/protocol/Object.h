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

#include <folly/Traits.h>
#include <folly/dynamic.h>
#include <thrift/lib/cpp/protocol/TProtocolException.h>
#include <thrift/lib/cpp/protocol/TType.h>
#include <thrift/lib/cpp2/op/Encode.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/FieldMask.h>
#include <thrift/lib/cpp2/protocol/detail/FieldMaskUtil.h>
#include <thrift/lib/cpp2/protocol/detail/Object.h>
#include <thrift/lib/thrift/gen-cpp2/protocol_types.h>

namespace apache::thrift::protocol {

// Creates a Value struct for the given value.
//
// TT: The thrift type to use, for example
// apache::thrift::type::binary_t.
using detail::asValueStruct;

// Creates a Object struct for the given structured.
template <typename T>
std::enable_if_t<
    is_thrift_struct_v<folly::remove_cvref_t<T>> ||
        is_thrift_union_v<folly::remove_cvref_t<T>>,
    Object>
asObject(T&& obj) {
  return asValueStruct<type::struct_c>(std::forward<T>(obj)).as_object();
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
  Object obj;
  detail::parseObjectInplace(prot, obj, string_to_binary);
  return obj;
}

template <class Protocol>
Object parseObject(Protocol& prot, bool string_to_binary = true) {
  Object obj;
  detail::parseObjectInplace(prot, obj, string_to_binary);
  return obj;
}

// Schemaless deserialization of thrift serialized data with mask.
// Only parses values that are masked.
//
// Note, this implementation is deprecated as it requires deprecated usage of
// mask view from `protocol::extractViewMaskFromPatch` to partially deserialize
// maps.
template <typename Protocol>
[[deprecated]] Object parseObjectWithoutExcludedData(
    const folly::IOBuf& buf, const Mask& mask, bool string_to_binary = true) {
  return detail::parseObject<Protocol, false>(
             buf, mask, noneMask(), string_to_binary)
      .included;
}

// Schemaless deserialization of thrift serialized data with mask.
// Only parses values that are masked. Unmasked fields are stored in MaskedData.
//
// Note, this implementation is deprecated as it requires deprecated usage of
// mask view from `protocol::extractViewMaskFromPatch` to partially deserialize
// maps.
template <typename Protocol>
[[deprecated]] MaskedDecodeResult parseObject(
    const folly::IOBuf& buf, const Mask& mask, bool string_to_binary = true) {
  return detail::parseObject<Protocol, true>(
      buf, mask, noneMask(), string_to_binary);
}

// Schemaless deserialization of thrift serialized data with readMask and
// writeMask. Only parses values that are masked by readMask. Fields that are
// not in neither writeMask nor readMask are stored in MaskedData. Unmasked
// fields that are not specified in writeMask are stored in MaskedData.
//
// This is designed to be used with Thrift Patch to avoid full deserialization
// when applying Thrift Patch to serialized data in a binary blob.
//
// Note, this implementation is deprecated as it requires deprecated usage of
// mask view from `protocol::extractViewMaskFromPatch` to partially deserialize
// maps.
template <typename Protocol>
[[deprecated]] MaskedDecodeResult parseObject(
    const folly::IOBuf& buf,
    const Mask& readMask,
    const Mask& writeMask,
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
using detail::serializeValue;

// Schemaless serialization of protocol::Object into thrift serialization
// protocol Protocol: protocol to use eg. apache::thrift::BinaryProtocolWriter
// obj: object to be serialized Serialized output is same as schema based
// serialization except when struct contains an empty list, set or map
template <class Protocol>
void serializeObject(const Object& val, folly::IOBufQueue& queue) {
  Protocol prot;
  prot.setOutput(&queue);
  detail::serializeObject(prot, val);
}

template <class Protocol>
std::unique_ptr<folly::IOBuf> serializeObject(const Object& val) {
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  serializeObject<Protocol>(val, queue);
  return queue.move();
}

// Serialization of protocol::Object with MaskedProtocolData.
template <class Protocol>
std::unique_ptr<folly::IOBuf> serializeObject(
    const Object& obj, const MaskedProtocolData& protocolData) {
  assert(*protocolData.protocol() == detail::get_standard_protocol<Protocol>);
  Protocol prot;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  prot.setOutput(&queue);
  if (protocolData.data()->full_ref()) { // entire object is not parsed
    const EncodedValue& value = detail::getByValueId(
        *protocolData.values(), protocolData.data()->full_ref().value());
    prot.writeRaw(*value.data());
  } else if (!protocolData.data()->fields_ref()) { // entire object is parsed
    detail::serializeObject(prot, obj);
  } else { // use both object and masked data to serialize
    detail::serializeObject(prot, obj, protocolData, *protocolData.data());
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

template <class Protocol>
Value parseValue(
    Protocol& prot, type::BaseType baseType, bool string_to_binary) {
  return detail::parseValue(prot, type::toTType(baseType), string_to_binary);
}

template <class Protocol, typename Tag>
Value parseValue(Protocol& prot, bool string_to_binary = true) {
  return parseValue<Protocol>(
      prot, type::detail::getBaseType(Tag{}), string_to_binary);
}

/// Convert protocol::Value to native thrift value.
template <class Tag>
auto fromValueStruct(const protocol::Value& v) {
  type::native_type<Tag> t;
  detail::ProtocolValueToThriftValue<Tag>{}(v, t);
  return t;
}

/// Convert protocol::Object to native thrift value.
template <class Tag>
auto fromObjectStruct(const protocol::Object& o) {
  type::native_type<Tag> t;
  detail::ProtocolValueToThriftValue<Tag>{}(o, t);
  return t;
}

// Returns whether the protocol::Value/ Object is its intrinsic default.
bool isIntrinsicDefault(const Value& value);
bool isIntrinsicDefault(const Object& obj);
folly::dynamic toDynamic(const Value& value);
folly::dynamic toDynamic(const Object& obj);

using detail::toAny;

} // namespace apache::thrift::protocol
