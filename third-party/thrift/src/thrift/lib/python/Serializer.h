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

#include <thrift/lib/python/types.h>

namespace apache::thrift::python {

template <typename TProtocolWriter>
std::unique_ptr<folly::IOBuf> serializeWithWriter(
    const DynamicStructInfo& dynamicStructInfo, const PyObject* object) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  TProtocolWriter writer(SHARE_EXTERNAL_BUFFER);
  writer.setOutput(&queue);
  detail::write(&writer, dynamicStructInfo.getStructInfo(), object);
  return queue.move();
}

template <typename TProtocolReader>
size_t deserializeWithReader(
    const DynamicStructInfo& dynamicStructInfo,
    const folly::IOBuf* buf,
    PyObject* object) {
  TProtocolReader reader(SHARE_EXTERNAL_BUFFER);
  reader.setInput(buf);
  detail::read(&reader, dynamicStructInfo.getStructInfo(), object);
  return reader.getCursor().getCurrentPosition();
}

using apache::thrift::protocol::PROTOCOL_TYPES;

std::unique_ptr<folly::IOBuf> serialize(
    const DynamicStructInfo& dynamicStructInfo,
    const PyObject* object,
    PROTOCOL_TYPES protocol);

/**
 * It receives an `object` which should be a valid Python list object, and it
 * calls `serialize()` function above with a pointer to the beginning of the
 * 'item' array in the PyListObject, where memory is allocated for the members.
 * (see `getListObjectItemBase()`)
 */
std::unique_ptr<folly::IOBuf> mutable_serialize(
    const DynamicStructInfo& dynamicStructInfo,
    const void* object,
    PROTOCOL_TYPES protocol);

size_t deserialize(
    const DynamicStructInfo& dynamicStructInfo,
    const folly::IOBuf* buf,
    PyObject* object,
    PROTOCOL_TYPES protocol);

/**
 * It receives an `object` which should be a valid Python list object, and it
 * calls `deserialize()` function above with a pointer to the beginning of the
 * 'item' array in the PyListObject, where memory is allocated for the members.
 * (see `getListObjectItemBase()`)
 */
size_t mutable_deserialize(
    const DynamicStructInfo& dynamicStructInfo,
    const folly::IOBuf* buf,
    void* object,
    PROTOCOL_TYPES protocol);

} // namespace apache::thrift::python
