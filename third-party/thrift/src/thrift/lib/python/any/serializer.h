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

#include <Python.h>

#include <memory>

#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/SimpleJSONProtocol.h>
#include <thrift/lib/cpp2/protocol/TableBasedSerializerImpl.h>
#include <thrift/lib/python/types.h>

namespace apache::thrift::python {

template <typename Writer>
std::unique_ptr<folly::IOBuf> serialize_type(
    const detail::TypeInfo& typeInfo, const PyObject* object) {
  auto queue = folly::IOBufQueue{folly::IOBufQueue::cacheChainLength()};
  Writer writer(SHARE_EXTERNAL_BUFFER);
  writer.setOutput(&queue);
  auto value = typeInfo.get(&object, typeInfo);
  if (value.hasValue()) {
    detail::writeThriftValue(&writer, typeInfo, value.value());
  }
  return queue.move();
}

template <typename Reader>
PyObject* deserialize_type(
    const detail::TypeInfo& typeInfo, const folly::IOBuf* buf) {
  Reader reader(SHARE_EXTERNAL_BUFFER);
  reader.setInput(buf);
  detail::ProtocolReaderStructReadState<Reader> readState;
  PyObject* obj = nullptr;
  detail::readThriftValue(&reader, typeInfo, readState, &obj);
  return obj;
}

std::unique_ptr<folly::IOBuf> serialize_type(
    const detail::TypeInfo& typeInfo,
    const PyObject* object,
    protocol::PROTOCOL_TYPES protocol) {
  switch (protocol) {
    case protocol::PROTOCOL_TYPES::T_COMPACT_PROTOCOL:
      return serialize_type<CompactProtocolWriter>(typeInfo, object);
    case protocol::PROTOCOL_TYPES::T_BINARY_PROTOCOL:
      return serialize_type<BinaryProtocolWriter>(typeInfo, object);
    case protocol::PROTOCOL_TYPES::T_SIMPLE_JSON_PROTOCOL:
      return serialize_type<SimpleJSONProtocolWriter>(typeInfo, object);
    default:
      throw TProtocolException(
          TProtocolException::NOT_IMPLEMENTED, "protocol not supported yet");
  }
}

PyObject* deserialize_type(
    const detail::TypeInfo& typeInfo,
    const folly::IOBuf* buf,
    protocol::PROTOCOL_TYPES protocol) {
  switch (protocol) {
    case protocol::PROTOCOL_TYPES::T_COMPACT_PROTOCOL:
      return deserialize_type<CompactProtocolReader>(typeInfo, buf);
    case protocol::PROTOCOL_TYPES::T_BINARY_PROTOCOL:
      return deserialize_type<BinaryProtocolReader>(typeInfo, buf);
    case protocol::PROTOCOL_TYPES::T_SIMPLE_JSON_PROTOCOL:
      return deserialize_type<SimpleJSONProtocolReader>(typeInfo, buf);
    default:
      throw TProtocolException(
          TProtocolException::NOT_IMPLEMENTED, "protocol not supported yet");
  }
}

} // namespace apache::thrift::python
