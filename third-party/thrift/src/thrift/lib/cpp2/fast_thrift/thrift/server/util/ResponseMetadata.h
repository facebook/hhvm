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

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

#include <memory>

namespace apache::thrift::fast_thrift::thrift {

inline constexpr size_t kResponseMetadataHeadroomBytes = 16;

namespace detail {

/**
 * Serialize ResponseRpcMetadata into IOBuf using Binary protocol.
 * Pre-computes serialized size for right-sized buffer allocation
 * and reserves headroom for downstream frame header serialization.
 */
inline std::unique_ptr<folly::IOBuf> serializeResponseMetadata(
    const apache::thrift::ResponseRpcMetadata& metadata) {
  apache::thrift::BinaryProtocolWriter writer;
  auto serializedSize = metadata.serializedSizeZC(&writer);
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  auto buf =
      folly::IOBuf::create(kResponseMetadataHeadroomBytes + serializedSize);
  buf->advance(kResponseMetadataHeadroomBytes);
  queue.append(std::move(buf));
  writer.setOutput(&queue);
  metadata.write(&writer);
  return queue.move();
}

} // namespace detail

/**
 * Returns a clone of a cached pre-serialized default success metadata IOBuf.
 * Avoids re-serializing the same empty ResponseRpcMetadata for every
 * successful response.
 */
inline std::unique_ptr<folly::IOBuf> getDefaultSuccessMetadata() {
  static const auto cached = []() {
    apache::thrift::ResponseRpcMetadata metadata;
    apache::thrift::PayloadMetadata payloadMetadata;
    payloadMetadata.responseMetadata() =
        apache::thrift::PayloadResponseMetadata();
    metadata.payloadMetadata() = std::move(payloadMetadata);
    return detail::serializeResponseMetadata(metadata);
  }();
  return cached->clone();
}

/**
 * Build serialized error metadata for an exception response.
 */
inline std::unique_ptr<folly::IOBuf> makeErrorResponseMetadata(
    std::string errorMessage) {
  apache::thrift::ResponseRpcMetadata responseMetadata;
  apache::thrift::PayloadMetadata payloadMetadata;
  apache::thrift::PayloadExceptionMetadataBase exBase;
  apache::thrift::PayloadExceptionMetadata exMeta;
  exMeta.appUnknownException() =
      apache::thrift::PayloadAppUnknownExceptionMetdata();
  exBase.metadata() = std::move(exMeta);
  exBase.what_utf8() = std::move(errorMessage);
  payloadMetadata.exceptionMetadata() = std::move(exBase);
  responseMetadata.payloadMetadata() = std::move(payloadMetadata);
  return detail::serializeResponseMetadata(responseMetadata);
}

} // namespace apache::thrift::fast_thrift::thrift
