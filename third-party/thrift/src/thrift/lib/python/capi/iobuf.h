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

#include <Python.h>

#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>

namespace apache {
namespace thrift {
namespace python {
namespace capi {
namespace detail {
namespace {

template <typename S>
using read_method_t =
    decltype(std::declval<S&>().read(std::declval<BinaryProtocolReader*>()));
template <typename S>
constexpr bool has_read_method_v = folly::is_detected_v<read_method_t, S>;

template <typename S>
using write_method_t =
    decltype(std::declval<S&>().write(std::declval<BinaryProtocolReader*>()));
template <typename S>
constexpr bool has_write_method_v = folly::is_detected_v<write_method_t, S>;

template <typename W>
using to_thrift_wrap_method_t = decltype(std::declval<W&>().toThrift());
template <typename W>
constexpr bool is_wrap_v = folly::is_detected_v<to_thrift_wrap_method_t, W>;

} // namespace

// Serialization into python for structs and unions
template <typename S>
std::enable_if_t<has_write_method_v<S>, std::unique_ptr<folly::IOBuf>>
serialize_to_iobuf(S&& s) {
  folly::IOBufQueue queue;
  apache::thrift::BinaryProtocolWriter protocol;
  protocol.setOutput(&queue);

  s.write(&protocol);
  return queue.move();
}

// Deserialization from python to cpp for structs and unions
template <typename S>
std::enable_if_t<has_read_method_v<S>, S> deserialize_iobuf(
    std::unique_ptr<folly::IOBuf>&& buf) {
  apache::thrift::BinaryProtocolReader protReader;
  protReader.setInput(buf.get());
  S f;
  f.read(&protReader);
  return f;
}

// Serialize a wrapped thrift type (e.g., Patch)
template <typename W>
std::enable_if_t<is_wrap_v<W>, std::unique_ptr<folly::IOBuf>>
serialize_to_iobuf(W&& s) {
  return serialize_to_iobuf(std::forward<W>(s).toThrift());
}

// Deserialize a wrapped thrift type (e.g., Patch)
template <typename W>
std::enable_if_t<is_wrap_v<W>, W> deserialize_iobuf(
    std::unique_ptr<folly::IOBuf>&& buf) {
  return W{deserialize_iobuf<typename W::underlying_type>(
      std::forward<std::unique_ptr<folly::IOBuf>>(buf))};
}

} // namespace detail
} // namespace capi
} // namespace python
} // namespace thrift

} // namespace apache
