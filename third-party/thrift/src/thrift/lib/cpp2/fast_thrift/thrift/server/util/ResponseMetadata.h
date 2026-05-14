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

#include <memory>
#include <optional>
#include <string>
#include <utility>

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/MetadataProtocol.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ResponseMetadata.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

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

// fill-in-place API: caller hands a ResponseRpcMetadata, we populate it.
// Pipeline serializes downstream (ThriftInitialResponsePayload::toRocketFrame)
// so the adapter is free of wire-format choice.

inline void fillSuccessResponseMetadata(
    apache::thrift::ResponseRpcMetadata& md) {
  apache::thrift::PayloadMetadata payloadMetadata;
  payloadMetadata.responseMetadata() =
      apache::thrift::PayloadResponseMetadata();
  md.payloadMetadata() = std::move(payloadMetadata);
}

inline void fillAppErrorResponseMetadata(
    apache::thrift::ResponseRpcMetadata& md,
    std::string exName,
    std::string errorMessage,
    apache::thrift::ErrorBlame blame = apache::thrift::ErrorBlame::SERVER) {
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
  md.payloadMetadata() = std::move(payloadMetadata);
}

inline void fillDeclaredExceptionMetadata(
    apache::thrift::ResponseRpcMetadata& md,
    std::string exName,
    std::string exWhat,
    std::optional<apache::thrift::ErrorClassification> classification =
        std::nullopt) {
  md.payloadMetadata() = buildDeclaredExceptionPayloadMetadata(
      std::move(exName), std::move(exWhat), classification);
}

// pre-serialized API: each helper allocates a fresh IOBuf using the
// caller-selected ProtocolWriter. Used by callsites that need a serialized
// metadata buffer up front (e.g., the channel-based path's request-side
// dispatch).

template <typename ProtocolWriter>
inline std::unique_ptr<folly::IOBuf> getDefaultSuccessMetadata() {
  apache::thrift::ResponseRpcMetadata metadata;
  fillSuccessResponseMetadata(metadata);
  return detail::serializeResponseMetadata<ProtocolWriter>(metadata);
}

template <typename ProtocolWriter>
inline std::unique_ptr<folly::IOBuf> makeAppErrorResponseMetadata(
    std::string exName,
    std::string errorMessage,
    apache::thrift::ErrorBlame blame = apache::thrift::ErrorBlame::SERVER) {
  apache::thrift::ResponseRpcMetadata metadata;
  fillAppErrorResponseMetadata(
      metadata, std::move(exName), std::move(errorMessage), blame);
  return detail::serializeResponseMetadata<ProtocolWriter>(metadata);
}

template <typename ProtocolWriter>
inline std::unique_ptr<folly::IOBuf> makeDeclaredExceptionMetadata(
    std::string exName,
    std::string exWhat,
    std::optional<apache::thrift::ErrorClassification> classification =
        std::nullopt) {
  apache::thrift::ResponseRpcMetadata metadata;
  fillDeclaredExceptionMetadata(
      metadata,
      std::move(exName),
      std::move(exWhat),
      std::move(classification));
  return detail::serializeResponseMetadata<ProtocolWriter>(metadata);
}

// =============================================================================
// Protocol-dispatching overloads
// =============================================================================
// Pick BinaryProtocolWriter or CompactProtocolWriter based on the
// per-connection metadata protocol negotiated at SETUP time. Call sites hold
// just a MetadataProtocol enum and call these directly.

inline std::unique_ptr<folly::IOBuf> serializeResponseMetadata(
    rocket::server::MetadataProtocol p,
    const apache::thrift::ResponseRpcMetadata& metadata) {
  return p == rocket::server::MetadataProtocol::BINARY
      ? detail::serializeResponseMetadata<apache::thrift::BinaryProtocolWriter>(
            metadata)
      : detail::serializeResponseMetadata<
            apache::thrift::CompactProtocolWriter>(metadata);
}

inline std::unique_ptr<folly::IOBuf> defaultSuccessMetadata(
    rocket::server::MetadataProtocol p) {
  return p == rocket::server::MetadataProtocol::BINARY
      ? getDefaultSuccessMetadata<apache::thrift::BinaryProtocolWriter>()
      : getDefaultSuccessMetadata<apache::thrift::CompactProtocolWriter>();
}

inline std::unique_ptr<folly::IOBuf> appErrorMetadata(
    rocket::server::MetadataProtocol p,
    std::string exName,
    std::string exWhat,
    apache::thrift::ErrorBlame blame = apache::thrift::ErrorBlame::SERVER) {
  return p == rocket::server::MetadataProtocol::BINARY
      ? makeAppErrorResponseMetadata<apache::thrift::BinaryProtocolWriter>(
            std::move(exName), std::move(exWhat), blame)
      : makeAppErrorResponseMetadata<apache::thrift::CompactProtocolWriter>(
            std::move(exName), std::move(exWhat), blame);
}

inline std::unique_ptr<folly::IOBuf> declaredExceptionMetadata(
    rocket::server::MetadataProtocol p,
    std::string exName,
    std::string exWhat,
    std::optional<apache::thrift::ErrorClassification> classification =
        std::nullopt) {
  return p == rocket::server::MetadataProtocol::BINARY
      ? makeDeclaredExceptionMetadata<apache::thrift::BinaryProtocolWriter>(
            std::move(exName), std::move(exWhat), classification)
      : makeDeclaredExceptionMetadata<apache::thrift::CompactProtocolWriter>(
            std::move(exName), std::move(exWhat), classification);
}

} // namespace apache::thrift::fast_thrift::thrift
