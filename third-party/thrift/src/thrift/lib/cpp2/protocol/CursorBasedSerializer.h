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

#include <folly/io/Cursor.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/type/NativeType.h>
#include <thrift/lib/cpp2/type/ThriftType.h>

/**
 * Cursor serialization for Thrift objects.
 * Provides a way to read and write Thrift types without having to materialize
 * them in memory. This can result in CPU / memory savings depending on the
 * access pattern, particularly when interfacing with another data
 * representation.
 *
 * This API derives its efficiency from ordering requirements on reading and
 * writing the data. Fields can only be read / written in order of increasing
 * field id in the struct. (Skipping fields is permitted). Data serialized using
 * other sources requires fields to be sorted in field id order in the IDL
 * file, or the struct to be annotated with `@thrift.SerializeInFieldIdOrder`.
 *
 * The general pattern is paired calls to beginWrite()/endWrite() (or
 * beginRead/endRead) which return a sub-cursor that is then passed in to the
 * end function, though scalars and materialized types can be read/written
 * directly.
 *
 * test/CursorBasedSerializerTest.cpp has several complete examples of usage.
 */

namespace apache::thrift {

/**
 * Manages the lifetime of a Thrift object being used with cursor
 * de/serialization.
 * Must outlive the reader/writer objects it returns.
 */
template <typename T>
class CursorSerializationWrapper {
  using Tag = type::infer_tag<T>;
  static_assert(
      type::is_a_v<Tag, type::structured_c>, "T must be a thrift class");

 public:
  CursorSerializationWrapper() = default;

  explicit CursorSerializationWrapper(std::unique_ptr<folly::IOBuf> serialized)
      : serializedData_(std::move(serialized)) {}

  /**
   * Object write path (traditional Thrift serialization)
   * Serializes from a Thrift object.
   */
  /* implicit */ CursorSerializationWrapper(
      const T& t, ExternalBufferSharing sharing = COPY_EXTERNAL_BUFFER) {
    BinarySerializer::serialize(t, &queue_, sharing);
    serializedData_ = queue_.move();
  }

  /**
   * Object read path (traditional Thrift deserialization)
   * Deserializes into a (returned) Thrift object.
   */
  T deserialize() const {
    assert(serializedData_);
    T ret;
    BinarySerializer::deserialize(serializedData_.get(), ret);
    return ret;
  }

  /** Access to serialized data */
  const folly::IOBuf& serializedData() const& {
    assert(serializedData_);
    return *serializedData_;
  }
  std::unique_ptr<folly::IOBuf> serializedData() && {
    assert(serializedData_);
    return std::move(serializedData_);
  }

 private:
  std::unique_ptr<folly::IOBuf> serializedData_;
  folly::IOBufQueue queue_;
};

/**
 * Adapter (for use with `@cpp.Adapter` annotation) that permits using this
 * serialization with Thrift RPC.
 */
class CursorSerializationAdapter {
 public:
  template <typename T>
  static CursorSerializationWrapper<T> fromThrift(const T& t);
  template <typename T>
  static T toThrift(const CursorSerializationWrapper<T>& w);

  template <typename Tag, typename Protocol, typename T>
  static uint32_t encode(
      Protocol& prot_, const CursorSerializationWrapper<T>& wrapper) {
    if constexpr (std::is_same_v<Protocol, BinaryProtocolWriter>) {
      return prot_.writeRaw(wrapper.serializedData());
    } else {
      folly::throw_exception<std::runtime_error>(
          "Single pass serialization only supports binary protocol.");
    }
  }

  template <typename Tag, typename Protocol, typename T>
  static void decode(Protocol& prot_, CursorSerializationWrapper<T>& wrapper) {
    if constexpr (std::is_same_v<Protocol, BinaryProtocolReader>) {
      std::unique_ptr<folly::IOBuf> buf;
      folly::copy(prot_.getCursor())
          .cloneAtMost(buf, std::numeric_limits<size_t>::max());
      // -1 to leave the stop marker for the presult struct when used in an RPC.
      prot_.skipBytes(buf->computeChainDataLength() - 1);
      wrapper = CursorSerializationWrapper<T>{std::move(buf)};
    } else {
      folly::throw_exception<std::runtime_error>(
          "Single pass serialization only supports binary protocol.");
    }
  }

  template <bool ZC, typename Tag, typename Protocol, typename T>
  static uint32_t serializedSize(
      Protocol&, const CursorSerializationWrapper<T>& wrapper) {
    return ZC ? 0 : wrapper.serializedData().computeChainDataLength();
  }
};

// End public API

} // namespace apache::thrift
