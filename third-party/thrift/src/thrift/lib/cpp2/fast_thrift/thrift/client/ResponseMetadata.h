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
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/protocol/TBase64Utils.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/transport/core/RpcMetadataUtil.h>

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
