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

template <typename W>
using to_thrift_wrap_method_t = decltype(std::declval<W&>().toThrift());
template <typename W>
constexpr bool is_wrap_v = folly::is_detected_v<to_thrift_wrap_method_t, W>;

} // namespace

/**
 * Serialize C++ thrift struct `S` to IOBuf for python for structs and unions.
 * If `S` is a wrapped C++ struct, convert to thrift before serializing.
 */
template <typename S>
std::unique_ptr<folly::IOBuf> serialize_to_iobuf(S&& s) {
  if constexpr (apache::thrift::is_thrift_class_v<S>) {
    folly::IOBufQueue queue;
    apache::thrift::BinaryProtocolWriter protocol;
    protocol.setOutput(&queue);

    s.write(&protocol);
    return queue.move();
  } else if constexpr (is_wrap_v<S>) {
    return serialize_to_iobuf(std::forward<S>(s).toThrift());
  } else {
    static_assert(
        folly::always_false<S>,
        "Serialize should take thrift class or wrapped thrift class");
  }
}

/**
 * Deserialize C++ thrift struct `S` from IOBuf for structs and unions.
 * If `S` is a wrapped C++ struct, construct wrapper after deserializing.
 */
template <typename S>
S deserialize_iobuf(std::unique_ptr<folly::IOBuf>&& buf) {
  if constexpr (apache::thrift::is_thrift_class_v<S>) {
    apache::thrift::BinaryProtocolReader protReader;
    protReader.setInput(buf.get());
    S f;
    f.read(&protReader);
    return f;
  } else if constexpr (is_wrap_v<S>) {
    return S{deserialize_iobuf<typename S::underlying_type>(
        std::forward<std::unique_ptr<folly::IOBuf>>(buf))};
  } else {
    static_assert(
        folly::always_false<S>,
        "Deserialize should take thrift class or wrapped thrift class type");
  }
}

/**
 * Serialize an adapted thrift type T, which is return type of type adapter
 * A::fromThrift
 *
 * The `if` branch is an edge case for serializing a field adapter struct T.
 * This is needed to handle a field adapter denoted by a structured annotation
 * struct S annotated with cpp.Adapter and cpp.Transitive annotations.
 */
template <typename Adapter, typename T>
std::unique_ptr<folly::IOBuf> serialize_adapted_to_iobuf(T&& s) {
  if constexpr (apache::thrift::is_thrift_class_v<T>) {
    return serialize_to_iobuf(std::forward<T>(s));
  } else {
    return serialize_to_iobuf(Adapter::toThrift(std::forward<T>(s)));
  }
}

/**
 * Deserialize an adapted thrift type T, which is return type of type adapter
 * A::fromThrift

 * The `if` branch is an edge case for deserializing a field adapter struct T.
 * This is needed to handle a field adapter denoted by a structured annotation
 * struct T annotated with cpp.Adapter and cpp.Transitive annotations.
 */
template <typename T, typename Adapter>
T deserialize_iobuf_to_adapted(std::unique_ptr<folly::IOBuf>&& buf) {
  if constexpr (apache::thrift::is_thrift_class_v<T>) {
    return deserialize_iobuf<T>(std::move(buf));
  } else {
    using S =
        std::remove_reference_t<decltype(Adapter::toThrift(std::declval<T>()))>;
    return Adapter::fromThrift(deserialize_iobuf<S>(std::move(buf)));
  }
}

} // namespace detail
} // namespace capi
} // namespace python
} // namespace thrift
} // namespace apache
