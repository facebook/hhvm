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
#include <thrift/lib/cpp2/transport/rocket/payload/PayloadSerializerStrategy.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::rocket {

struct CustomCompressionPayloadSerializerStrategyOptions {
  // Options can be added as needed
};

/**
 * A payload serializer strategy that applies custom compression.
 * It delegates to another strategy for the actual serialization and
 * deserialization.
 */
template <typename DelegateStrategy>
class CustomCompressionPayloadSerializerStrategy final
    : PayloadSerializerStrategy<
          CustomCompressionPayloadSerializerStrategy<DelegateStrategy>> {
 public:
  CustomCompressionPayloadSerializerStrategy(
      CustomCompressionPayloadSerializerStrategyOptions /* options */ = {})
      : PayloadSerializerStrategy<
            CustomCompressionPayloadSerializerStrategy<DelegateStrategy>>(
            *this),
        delegate_(DelegateStrategy()) {}

  template <class T>
  FOLLY_ERASE folly::Try<T> unpackAsCompressed(
      Payload&& payload, bool decodeMetadataUsingBinary) {
    return delegate_.template unpackAsCompressed<T>(
        std::move(payload), decodeMetadataUsingBinary);
  }

  template <class T>
  FOLLY_ERASE folly::Try<T> unpack(
      Payload&& payload, bool decodeMetadataUsingBinary) {
    auto unpackedPayload = delegate_.template unpack<T>(
        std::move(payload), decodeMetadataUsingBinary);

    if (unpackedPayload.hasValue()) {
      if (auto compress = unpackedPayload->metadata.compression()) {
        if (*compress == CompressionAlgorithm::CUSTOM) {
          // TODO: handle custom compression here
        }
      }
    }

    return unpackedPayload;
  }

  template <typename T>
  FOLLY_ERASE std::unique_ptr<folly::IOBuf> packCompact(const T& data) {
    return delegate_.packCompact(data);
  }

  template <typename Metadata>
  FOLLY_ERASE rocket::Payload packWithFds(
      Metadata* metadata,
      std::unique_ptr<folly::IOBuf>&& payload,
      folly::SocketFds fds,
      bool encodeMetadataUsingBinary,
      folly::AsyncTransport* transport) {
    if (auto compress = metadata->compression()) {
      if (*compress == CompressionAlgorithm::CUSTOM) {
        // TODO: handle custom compression here
      }
    }
    return delegate_.packWithFds(
        metadata,
        std::move(payload),
        std::move(fds),
        encodeMetadataUsingBinary,
        transport);
  }

  template <class PayloadType>
  FOLLY_ERASE Payload pack(
      PayloadType&& payload,
      bool encodeMetadataUsingBinary,
      folly::AsyncTransport* transport) {
    auto metadata = std::forward<PayloadType>(payload).metadata;
    return packWithFds(
        &metadata,
        std::forward<PayloadType>(payload).payload,
        std::forward<PayloadType>(payload).fds,
        encodeMetadataUsingBinary,
        transport);
  }

  template <typename T>
  FOLLY_ERASE size_t unpackCompact(T& output, const folly::IOBuf* buffer) {
    return delegate_.unpackCompact(output, buffer);
  }

  template <typename T>
  FOLLY_ERASE size_t unpackCompact(T& output, const folly::io::Cursor& cursor) {
    return delegate_.unpackCompact(output, cursor);
  }

  template <typename T>
  FOLLY_ERASE size_t unpackBinary(T& output, const folly::IOBuf* buffer) {
    return delegate_.unpackBinary(output, buffer);
  }

  template <typename T>
  FOLLY_ERASE size_t unpackBinary(T& output, const folly::io::Cursor& cursor) {
    return delegate_.unpackBinary(output, cursor);
  }

  FOLLY_ERASE
  std::unique_ptr<folly::IOBuf> compressBuffer(
      std::unique_ptr<folly::IOBuf>&& buffer,
      CompressionAlgorithm compressionAlgorithm) {
    if (compressionAlgorithm != CompressionAlgorithm::CUSTOM) {
      return delegate_.compressBuffer(std::move(buffer), compressionAlgorithm);
    }

    // todo: implement custom compression logic here
    return std::move(buffer);
  }

  FOLLY_ERASE
  std::unique_ptr<folly::IOBuf> uncompressBuffer(
      std::unique_ptr<folly::IOBuf>&& buffer,
      CompressionAlgorithm compressionAlgorithm) {
    if (compressionAlgorithm != CompressionAlgorithm::CUSTOM) {
      return delegate_.uncompressBuffer(
          std::move(buffer), compressionAlgorithm);
    }

    // todo: implement custom compression logic here
    return std::move(buffer);
  }

 private:
  DelegateStrategy delegate_;
};

} // namespace apache::thrift::rocket
