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

#include <folly/ExceptionWrapper.h>
#include <folly/Expected.h>
#include <folly/lang/Exception.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/protocol/TBase64Utils.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/transport/core/RpcMetadataUtil.h>
#include <thrift/lib/cpp2/type/Any.h>
#include <thrift/lib/cpp2/type/Protocol.h>
#include <thrift/lib/cpp2/type/Runtime.h>
#include <thrift/lib/cpp2/type/TypeRegistry.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * Deserialize ResponseRpcMetadata from a ParsedFrame's metadata section
 * using Binary protocol. Returns an error on deserialization failure.
 */
inline folly::exception_wrapper deserializeResponseMetadata(
    const apache::thrift::fast_thrift::frame::read::ParsedFrame& frame,
    apache::thrift::ResponseRpcMetadata& metadata) noexcept {
  try {
    if (frame.hasMetadata() && frame.metadataSize() > 0) {
      apache::thrift::BinaryProtocolReader reader;
      reader.setInput(frame.metadataCursor());
      metadata.read(&reader);
    }
  } catch (...) {
    return folly::make_exception_wrapper<apache::thrift::TApplicationException>(
        "Failed to deserialize response metadata");
  }
  return folly::exception_wrapper{};
}

/**
 * Process exception metadata and populate otherMetadata headers.
 * Returns an error for undeclared exceptions, empty for declared.
 */
inline folly::exception_wrapper processExceptionMetadata(
    const apache::thrift::PayloadExceptionMetadataBase& exMeta,
    folly::F14NodeMap<std::string, std::string>& otherMetadata) {
  auto exceptionNameRef = exMeta.name_utf8();
  auto exceptionWhatRef = exMeta.what_utf8();

  auto exceptionMetadataRef = exMeta.metadata();
  if (!exceptionMetadataRef) {
    return folly::make_exception_wrapper<apache::thrift::TApplicationException>(
        "Missing payload exception metadata");
  }

  if (exceptionNameRef) {
    otherMetadata[std::string(apache::thrift::detail::kHeaderUex)] =
        *exceptionNameRef;
  }
  if (exceptionWhatRef) {
    otherMetadata[std::string(apache::thrift::detail::kHeaderUexw)] =
        *exceptionWhatRef;
  }

  if (exceptionMetadataRef->getType() ==
      apache::thrift::PayloadExceptionMetadata::Type::declaredException) {
    if (auto dExClass =
            exceptionMetadataRef->declaredException()->errorClassification()) {
      auto serialized =
          apache::thrift::CompactSerializer::serialize<std::string>(*dExClass);
      otherMetadata[std::string(apache::thrift::detail::kHeaderExMeta)] =
          apache::thrift::protocol::base64Encode(
              folly::StringPiece(serialized));
    }
    return folly::exception_wrapper{};
  }

  return folly::make_exception_wrapper<apache::thrift::TApplicationException>(
      exceptionWhatRef.value_or(""));
}

/**
 * Action the caller should take with the response data IOBuf, derived from
 * the typed payload metadata. Used by the FastClient adapter path; legacy
 * ChannelBase path uses processPayloadMetadata() which mutates otherMetadata
 * for THeader projection.
 */
enum class PayloadAction : uint8_t {
  // Data IOBuf is a presult struct; caller should run normal recv_wrapped<>
  // (handles success and declared exceptions encoded as presult fields).
  PassThrough,
  // Data IOBuf is a SemiAnyStruct; caller should extract via thrift Any
  // registry to recover the typed exception (extractAnyException).
  ExtractAnyException,
};

/**
 * Inspect ResponseRpcMetadata's payloadMetadata field and decide what the
 * caller should do with the data IOBuf. Pure function — does not mutate.
 *
 * Returns:
 *   PassThrough           — normal response or declared exception
 *   ExtractAnyException   — anyException; caller deserializes SemiAnyStruct
 *   error exception_wrapper — undeclared / appUnknownException / malformed
 */
inline folly::Expected<PayloadAction, folly::exception_wrapper>
classifyPayloadAction(const apache::thrift::ResponseRpcMetadata& metadata) {
  const auto& payloadMetadataRef = metadata.payloadMetadata();
  if (!payloadMetadataRef) {
    return PayloadAction::PassThrough;
  }

  switch (payloadMetadataRef->getType()) {
    case apache::thrift::PayloadMetadata::Type::__EMPTY__:
    case apache::thrift::PayloadMetadata::Type::responseMetadata:
      return PayloadAction::PassThrough;

    case apache::thrift::PayloadMetadata::Type::exceptionMetadata: {
      const auto& exMeta = payloadMetadataRef->get_exceptionMetadata();
      const auto& exMetaInner = exMeta.metadata();
      if (!exMetaInner) {
        return folly::makeUnexpected(
            folly::make_exception_wrapper<
                apache::thrift::TApplicationException>(
                "Missing payload exception metadata"));
      }
      switch (exMetaInner->getType()) {
        case apache::thrift::PayloadExceptionMetadata::Type::declaredException:
          return PayloadAction::PassThrough;
        case apache::thrift::PayloadExceptionMetadata::Type::anyException:
          return PayloadAction::ExtractAnyException;
        case apache::thrift::PayloadExceptionMetadata::Type::__EMPTY__:
        case apache::thrift::PayloadExceptionMetadata::Type::
            DEPRECATED_proxyException:
        case apache::thrift::PayloadExceptionMetadata::Type::
            appUnknownException:
        default:
          return folly::makeUnexpected(
              folly::make_exception_wrapper<
                  apache::thrift::TApplicationException>(
                  exMeta.what_utf8().value_or("")));
      }
    }

    default:
      return folly::makeUnexpected(
          folly::make_exception_wrapper<apache::thrift::TApplicationException>(
              "Invalid payload metadata type"));
  }
}

/**
 * Decode a SemiAnyStruct-encoded thrift Any exception out of the response
 * data IOBuf and return it as a typed exception_wrapper.
 *
 * For Rocket protocol v10+, the data IOBuf carries a SemiAnyStruct that
 * contains the type URI plus the inner serialized exception. This function
 * deserializes the SemiAnyStruct (with the negotiated payload protocol),
 * looks the type up in the global TypeRegistry, and returns the typed
 * exception_wrapper.
 *
 * Falls back to TApplicationException with a descriptive message if the
 * SemiAnyStruct fails to deserialize or the type is not registered.
 */
inline folly::exception_wrapper extractAnyException(
    std::unique_ptr<folly::IOBuf> data, uint16_t protocolId) {
  apache::thrift::type::SemiAnyStruct anyException;
  try {
    if (protocolId == apache::thrift::protocol::T_COMPACT_PROTOCOL) {
      apache::thrift::CompactSerializer::deserialize(data.get(), anyException);
    } else {
      apache::thrift::BinarySerializer::deserialize(data.get(), anyException);
    }
  } catch (...) {
    return folly::make_exception_wrapper<apache::thrift::TApplicationException>(
        "anyException deserialization failure: " +
        folly::exceptionStr(folly::current_exception()).toStdString());
  }

  // Per RpcMetadata.thrift: when SemiAnyStruct.protocol is unset, it defaults
  // to the protocol used to serialize the SemiAnyStruct.
  if (*anyException.protocol() == apache::thrift::type::kNoProtocol) {
    anyException.protocol() =
        protocolId == apache::thrift::protocol::T_COMPACT_PROTOCOL
        ? apache::thrift::type::Protocol::get<
              apache::thrift::type::StandardProtocol::Compact>()
        : apache::thrift::type::Protocol::get<
              apache::thrift::type::StandardProtocol::Binary>();
  }

  try {
    apache::thrift::type::AnyData anyData(std::move(anyException));
    if (auto ew = apache::thrift::type::TypeRegistry::generated()
                      .load(anyData)
                      .asExceptionWrapper()) {
      return ew;
    }
  } catch (...) {
    return folly::make_exception_wrapper<apache::thrift::TApplicationException>(
        "anyException load failure: " +
        folly::exceptionStr(folly::current_exception()).toStdString());
  }
  return folly::make_exception_wrapper<apache::thrift::TApplicationException>(
      "anyException type not registered in TypeRegistry");
}

/**
 * Process payload metadata from ResponseRpcMetadata.
 *
 * Handles declared exceptions (populates otherMetadata headers) and
 * undeclared exceptions (returns error). Returns empty on success.
 */
inline folly::exception_wrapper processPayloadMetadata(
    apache::thrift::ResponseRpcMetadata& metadata) {
  auto payloadMetadataRef = metadata.payloadMetadata();
  if (!payloadMetadataRef) {
    return folly::exception_wrapper{};
  }

  auto& otherMetadata = metadata.otherMetadata().ensure();

  switch (payloadMetadataRef->getType()) {
    case apache::thrift::PayloadMetadata::Type::__EMPTY__:
    case apache::thrift::PayloadMetadata::Type::responseMetadata:
      return folly::exception_wrapper{};

    case apache::thrift::PayloadMetadata::Type::exceptionMetadata: {
      auto& exMeta = payloadMetadataRef->get_exceptionMetadata();
      return processExceptionMetadata(exMeta, otherMetadata);
    }

    default:
      return folly::make_exception_wrapper<
          apache::thrift::TApplicationException>(
          "Invalid payload metadata type");
  }
}

} // namespace apache::thrift::fast_thrift::thrift
