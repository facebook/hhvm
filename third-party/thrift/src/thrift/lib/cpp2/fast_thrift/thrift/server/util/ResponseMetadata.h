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

#include <optional>

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
 * Legacy version without ErrorBlame — prefer makeAppErrorResponseMetadata().
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

/**
 * Build serialized app error metadata with ErrorBlame.
 * Used by sendErrorWrapped for app-level errors (PAYLOAD frame).
 */
inline std::unique_ptr<folly::IOBuf> makeAppErrorResponseMetadata(
    std::string exName,
    std::string errorMessage,
    apache::thrift::ErrorBlame blame = apache::thrift::ErrorBlame::SERVER) {
  apache::thrift::ResponseRpcMetadata responseMetadata;
  apache::thrift::PayloadMetadata payloadMetadata;
  apache::thrift::PayloadExceptionMetadataBase exBase;
  apache::thrift::PayloadExceptionMetadata exMeta;

  apache::thrift::PayloadAppUnknownExceptionMetdata appUnknown;
  apache::thrift::ErrorClassification errorClassification;
  errorClassification.blame() = blame;
  appUnknown.errorClassification() = errorClassification;
  exMeta.appUnknownException() = appUnknown;

  if (!exName.empty()) {
    exBase.name_utf8() = std::move(exName);
  }
  exBase.what_utf8() = std::move(errorMessage);
  exBase.metadata() = std::move(exMeta);
  payloadMetadata.exceptionMetadata() = std::move(exBase);
  responseMetadata.payloadMetadata() = std::move(payloadMetadata);
  return detail::serializeResponseMetadata(responseMetadata);
}

/**
 * Build the inner PayloadMetadata for a declared exception (IDL `throws`).
 */
inline apache::thrift::PayloadMetadata buildDeclaredExceptionPayloadMetadata(
    std::string exName,
    std::string exWhat,
    std::optional<apache::thrift::ErrorClassification> classification) {
  apache::thrift::PayloadMetadata payloadMetadata;
  apache::thrift::PayloadExceptionMetadataBase exBase;
  apache::thrift::PayloadExceptionMetadata exMeta;

  apache::thrift::PayloadDeclaredExceptionMetadata declaredEx;
  if (classification) {
    declaredEx.errorClassification() = *classification;
  }
  exMeta.declaredException() = declaredEx;

  if (!exName.empty()) {
    exBase.name_utf8() = std::move(exName);
  }
  if (!exWhat.empty()) {
    exBase.what_utf8() = std::move(exWhat);
  }
  exBase.metadata() = std::move(exMeta);
  payloadMetadata.exceptionMetadata() = std::move(exBase);
  return payloadMetadata;
}

/**
 * Build serialized declared exception metadata with optional
 * ErrorClassification. Used by sendReply when isException is set.
 */
inline std::unique_ptr<folly::IOBuf> makeDeclaredExceptionMetadata(
    std::string exName,
    std::string exWhat,
    std::optional<apache::thrift::ErrorClassification> classification =
        std::nullopt) {
  apache::thrift::ResponseRpcMetadata responseMetadata;
  responseMetadata.payloadMetadata() = buildDeclaredExceptionPayloadMetadata(
      std::move(exName), std::move(exWhat), classification);
  return detail::serializeResponseMetadata(responseMetadata);
}

} // namespace apache::thrift::fast_thrift::thrift
