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

#include <thrift/lib/cpp/protocol/TProtocolException.h>
#include <thrift/lib/cpp/protocol/TType.h>
#include <thrift/lib/cpp2/protocol/detail/Object.h>

namespace apache::thrift::protocol {

// Creates a Value struct for the given value.
//
// TT: The thrift type to use, for example
// apache::thrift::type::binary_t.
template <typename TT, typename T>
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
  return *result.objectValue_ref();
}

// Schemaless serialization of protocol::Object
// into thrift serialization protocol
// Protocol: protocol to use eg. apache::thrift::BinaryProtocolWriter
// obj: object to be serialized
// Serialized output is same as schema based serialization except when struct
// contains an empty list, set or map
template <class Protocol>
std::unique_ptr<folly::IOBuf> serializeObject(const Object& obj) {
  Protocol prot;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  prot.setOutput(&queue);
  Value val;
  val.objectValue_ref() = obj;
  detail::serializeValue(prot, val);
  return queue.move();
}

// Returns whether the protocol::Value/ Object is its intrinsic default.
bool isIntrinsicDefault(const Value& value);
bool isIntrinsicDefault(const Object& obj);

} // namespace apache::thrift::protocol
