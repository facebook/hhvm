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

#include <folly/Function.h>
#include <folly/Try.h>
#include <folly/io/Cursor.h>
#include <folly/io/async/AsyncTransport.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::rocket {

/**
 * Defines strategy contract for serializing and deserializing payloads.
 */
template <typename Child>
class PayloadSerializerStrategy {
 public:
  template <class T>
  FOLLY_ALWAYS_INLINE folly::Try<T> unpackAsCompressed(
      Payload&& payload, bool decodeMetadataUsingBinary) {
    return child_.unpackAsCompressed(
        std::move(payload), decodeMetadataUsingBinary);
  }

  template <class T>
  FOLLY_ALWAYS_INLINE folly::Try<T> unpack(
      Payload&& payload, bool decodeMetadataUsingBinary) {
    return child_.unpack(std::move(payload), decodeMetadataUsingBinary);
  }

  template <typename T>
  FOLLY_ALWAYS_INLINE std::unique_ptr<folly::IOBuf> packCompact(const T& data) {
    return child_.packCompact(data);
  }

  template <typename Metadata>
  FOLLY_ALWAYS_INLINE rocket::Payload packWithFds(
      Metadata* metadata,
      std::unique_ptr<folly::IOBuf>&& payload,
      folly::SocketFds fds,
      bool encodeMetadataUsingBinary,
      folly::AsyncTransport* transport) {
    return child_.packWithFds(
        metadata,
        std::move(payload),
        std::move(fds),
        encodeMetadataUsingBinary,
        transport);
  }

  template <class PayloadType>
  FOLLY_ALWAYS_INLINE Payload pack(
      PayloadType&& payload,
      bool encodeMetadataUsingBinary,
      folly::AsyncTransport* transport) {
    return child_.pack(
        std::forward<PayloadType>(payload),
        encodeMetadataUsingBinary,
        transport);
  }

  template <typename T>
  FOLLY_ALWAYS_INLINE size_t
  unpackCompact(T& output, const folly::IOBuf* buffer) {
    return child_.unpackCompact(output, buffer);
  }

  template <typename T>
  FOLLY_ALWAYS_INLINE size_t
  unpackCompact(T& output, folly::io::Cursor& cursor) {
    return child_.unpackCompact(output, cursor);
  }

  template <typename T>
  FOLLY_ALWAYS_INLINE size_t
  unpackBinary(T& output, const folly::IOBuf* buffer) {
    return child_.unpackBinary(output, buffer);
  }

  template <typename T>
  FOLLY_ALWAYS_INLINE size_t
  unpackBinary(T& output, folly::io::Cursor& cursor) {
    return child_.unpackBinary(output, cursor);
  }

  FOLLY_ALWAYS_INLINE std::unique_ptr<folly::IOBuf> compressBuffer(
      std::unique_ptr<folly::IOBuf>&& buffer,
      CompressionAlgorithm compressionAlgorithm) {
    return child_.compressBuffer(std::move(buffer), compressionAlgorithm);
  }

  FOLLY_ALWAYS_INLINE std::unique_ptr<folly::IOBuf> uncompressBuffer(
      std::unique_ptr<folly::IOBuf>&& buffer,
      CompressionAlgorithm compressionAlgorithm) {
    return child_.uncompressBuffer(std::move(buffer), compressionAlgorithm);
  }

  virtual ~PayloadSerializerStrategy() = default;

 protected:
  explicit PayloadSerializerStrategy(Child& child) : child_(child) {}

 private:
  Child& child_;
};

} // namespace apache::thrift::rocket
