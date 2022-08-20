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
#include <thrift/lib/python/types.h>

namespace apache {
namespace thrift {
namespace python {

template <typename Writer>
std::unique_ptr<folly::IOBuf> serialize(
    const DynamicStructInfo& dynamicStructInfo, const PyObject* object) {
  auto queue = folly::IOBufQueue{folly::IOBufQueue::cacheChainLength()};
  Writer writer(SHARE_EXTERNAL_BUFFER);
  writer.setOutput(&queue);
  detail::write(&writer, dynamicStructInfo.getStructInfo(), object);
  return queue.move();
}

template <typename Reader>
size_t deserialize(
    const DynamicStructInfo& dynamicStructInfo,
    const folly::IOBuf* buf,
    PyObject* object) {
  Reader reader(SHARE_EXTERNAL_BUFFER);
  reader.setInput(buf);
  detail::read(&reader, dynamicStructInfo.getStructInfo(), object);
  return reader.getCursor().getCurrentPosition();
}

using apache::thrift::protocol::PROTOCOL_TYPES;

std::unique_ptr<folly::IOBuf> serialize(
    const DynamicStructInfo& dynamicStructInfo,
    const PyObject* object,
    PROTOCOL_TYPES protocol) {
  switch (protocol) {
    case PROTOCOL_TYPES::T_COMPACT_PROTOCOL:
      return serialize<CompactProtocolWriter>(dynamicStructInfo, object);
    case PROTOCOL_TYPES::T_BINARY_PROTOCOL:
      return serialize<BinaryProtocolWriter>(dynamicStructInfo, object);
    case PROTOCOL_TYPES::T_SIMPLE_JSON_PROTOCOL:
      return serialize<SimpleJSONProtocolWriter>(dynamicStructInfo, object);
    default:
      throw TProtocolException(
          TProtocolException::NOT_IMPLEMENTED, "protocol not supported yet");
  }
}

size_t deserialize(
    const DynamicStructInfo& dynamicStructInfo,
    const folly::IOBuf* buf,
    PyObject* object,
    PROTOCOL_TYPES protocol) {
  switch (protocol) {
    case PROTOCOL_TYPES::T_COMPACT_PROTOCOL:
      return deserialize<CompactProtocolReader>(dynamicStructInfo, buf, object);
    case PROTOCOL_TYPES::T_BINARY_PROTOCOL:
      return deserialize<BinaryProtocolReader>(dynamicStructInfo, buf, object);
    case PROTOCOL_TYPES::T_SIMPLE_JSON_PROTOCOL:
      return deserialize<SimpleJSONProtocolReader>(
          dynamicStructInfo, buf, object);
    default:
      throw TProtocolException(
          TProtocolException::NOT_IMPLEMENTED, "protocol not supported yet");
  }
}

} // namespace python
} // namespace thrift
} // namespace apache
