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

#include <thrift/lib/python/Serializer.h>

#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/JSONProtocol.h>
#include <thrift/lib/cpp2/protocol/SimpleJSONProtocol.h>
#include <thrift/lib/python/types.h>

namespace apache::thrift::python {

using apache::thrift::protocol::PROTOCOL_TYPES;

std::unique_ptr<folly::IOBuf> serialize(
    const DynamicStructInfo& dynamicStructInfo,
    const PyObject* object,
    PROTOCOL_TYPES protocol) {
  switch (protocol) {
    case PROTOCOL_TYPES::T_COMPACT_PROTOCOL:
      return serializeWithWriter<CompactProtocolWriter>(
          dynamicStructInfo, object);
    case PROTOCOL_TYPES::T_BINARY_PROTOCOL:
      return serializeWithWriter<BinaryProtocolWriter>(
          dynamicStructInfo, object);
    case PROTOCOL_TYPES::T_SIMPLE_JSON_PROTOCOL:
      return serializeWithWriter<SimpleJSONProtocolWriter>(
          dynamicStructInfo, object);
    // Deprecated, remove as soon as thrift-python migration complete
    case PROTOCOL_TYPES::T_JSON_PROTOCOL:
      return serializeWithWriter<JSONProtocolWriter>(dynamicStructInfo, object);
    default:
      throw TProtocolException(
          TProtocolException::NOT_IMPLEMENTED, "protocol not supported yet");
  }
}

std::unique_ptr<folly::IOBuf> mutable_serialize(
    const DynamicStructInfo& dynamicStructInfo,
    const void* object,
    PROTOCOL_TYPES protocol) {
  return serialize(dynamicStructInfo, getListObjectItemBase(object), protocol);
}

size_t deserialize(
    const DynamicStructInfo& dynamicStructInfo,
    const folly::IOBuf* buf,
    PyObject* object,
    PROTOCOL_TYPES protocol) {
  switch (protocol) {
    case PROTOCOL_TYPES::T_COMPACT_PROTOCOL:
      return deserializeWithReader<CompactProtocolReader>(
          dynamicStructInfo, buf, object);
    case PROTOCOL_TYPES::T_BINARY_PROTOCOL:
      return deserializeWithReader<BinaryProtocolReader>(
          dynamicStructInfo, buf, object);
    case PROTOCOL_TYPES::T_SIMPLE_JSON_PROTOCOL:
      return deserializeWithReader<SimpleJSONProtocolReader>(
          dynamicStructInfo, buf, object);
    case PROTOCOL_TYPES::T_JSON_PROTOCOL:
      return deserializeWithReader<JSONProtocolReader>(
          dynamicStructInfo, buf, object);
    default:
      throw TProtocolException(
          TProtocolException::NOT_IMPLEMENTED, "protocol not supported yet");
  }
}

size_t mutable_deserialize(
    const DynamicStructInfo& dynamicStructInfo,
    const folly::IOBuf* buf,
    void* object,
    PROTOCOL_TYPES protocol) {
  return deserialize(
      dynamicStructInfo, buf, getListObjectItemBase(object), protocol);
}

} // namespace apache::thrift::python
