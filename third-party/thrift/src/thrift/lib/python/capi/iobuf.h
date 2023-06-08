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

#include <Python.h>

#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>

namespace apache {
namespace thrift {
namespace python {
namespace capi {
namespace detail {

// Serialization into python for structs and unions
template <typename S>
std::enable_if_t<
    apache::thrift::is_thrift_class_v<S>,
    std::unique_ptr<folly::IOBuf>>
serialize_to_iobuf(S&& s) {
  folly::IOBufQueue queue;
  apache::thrift::BinaryProtocolWriter protocol;
  protocol.setOutput(&queue);

  s.write(&protocol);
  return queue.move();
}

// Deserialization from python to cpp for structs and unions
template <typename S>
std::enable_if_t<apache::thrift::is_thrift_class_v<S>, S> deserialize_iobuf(
    std::unique_ptr<folly::IOBuf>&& buf) {
  apache::thrift::BinaryProtocolReader protReader;
  protReader.setInput(buf.get());
  S f;
  f.read(&protReader);
  return f;
}

} // namespace detail
} // namespace capi
} // namespace python
} // namespace thrift

} // namespace apache
