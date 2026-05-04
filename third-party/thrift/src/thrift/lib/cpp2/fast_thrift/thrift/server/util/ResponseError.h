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
#include <string_view>
#include <unordered_map>

#include <folly/ExceptionWrapper.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/GeneratedCodeHelper.h>
#include <thrift/lib/cpp2/fast_thrift/frame/ErrorCode.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

struct SerializedResponseError {
  std::unique_ptr<folly::IOBuf> data;
  apache::thrift::fast_thrift::frame::ErrorCode errorCode;
};

/**
 * Maps an exCode string (from sendErrorWrapped) to ResponseRpcErrorCode.
 * Returns std::nullopt for app-level errors that should be sent as
 * PAYLOAD frames with appUnknownException metadata.
 *
 * Mirrors the errorCodeMap in RocketThriftRequests.cpp
 * processFirstResponseHelper().
 */
inline std::optional<apache::thrift::ResponseRpcErrorCode> mapExCodeToErrorCode(
    std::string_view exCode) {
  // Use the numeric string values that match the extern constants
  // in ResponseChannel.h/cpp.
  static const auto& errorCodeMap =
      *new std::unordered_map<std::string_view, ResponseRpcErrorCode>(
          {{"0", ResponseRpcErrorCode::UNKNOWN},
           {"1", ResponseRpcErrorCode::OVERLOAD},
           {"22", ResponseRpcErrorCode::APP_OVERLOAD},
           {"2", ResponseRpcErrorCode::TASK_EXPIRED},
           {"5", ResponseRpcErrorCode::QUEUE_OVERLOADED},
           {"14", ResponseRpcErrorCode::INJECTED_FAILURE},
           {"15", ResponseRpcErrorCode::QUEUE_TIMEOUT},
           {"17", ResponseRpcErrorCode::RESPONSE_TOO_BIG},
           {"25", ResponseRpcErrorCode::UNKNOWN_METHOD},
           {"21", ResponseRpcErrorCode::WRONG_RPC_KIND},
           {"26", ResponseRpcErrorCode::UNKNOWN_INTERACTION_ID},
           {"27", ResponseRpcErrorCode::INTERACTION_CONSTRUCTOR_ERROR},
           {"28", ResponseRpcErrorCode::REQUEST_PARSING_FAILURE},
           {"30", ResponseRpcErrorCode::CHECKSUM_MISMATCH},
           {"31", ResponseRpcErrorCode::UNIMPLEMENTED_METHOD},
           {"32", ResponseRpcErrorCode::TENANT_QUOTA_EXCEEDED},
           {"34", ResponseRpcErrorCode::INTERACTION_LOADSHEDDED},
           {"35", ResponseRpcErrorCode::INTERACTION_LOADSHEDDED_QUEUE_TIMEOUT},
           {"36", ResponseRpcErrorCode::INTERACTION_LOADSHEDDED_OVERLOAD},
           {"37", ResponseRpcErrorCode::INTERACTION_LOADSHEDDED_APP_OVERLOAD}});

  auto it = errorCodeMap.find(exCode);
  if (it != errorCodeMap.end()) {
    return it->second;
  }
  return std::nullopt;
}

/**
 * Maps ResponseRpcErrorCode to ResponseRpcErrorCategory.
 * Mirrors the logic in RocketThriftRequests.cpp makeResponseRpcError().
 */
inline apache::thrift::ResponseRpcErrorCategory mapErrorCodeToCategory(
    apache::thrift::ResponseRpcErrorCode code) {
  switch (code) {
    case apache::thrift::ResponseRpcErrorCode::REQUEST_PARSING_FAILURE:
    case apache::thrift::ResponseRpcErrorCode::WRONG_RPC_KIND:
    case apache::thrift::ResponseRpcErrorCode::UNKNOWN_METHOD:
    case apache::thrift::ResponseRpcErrorCode::CHECKSUM_MISMATCH:
    case apache::thrift::ResponseRpcErrorCode::UNKNOWN_INTERACTION_ID:
    case apache::thrift::ResponseRpcErrorCode::UNIMPLEMENTED_METHOD:
      return apache::thrift::ResponseRpcErrorCategory::INVALID_REQUEST;
    case apache::thrift::ResponseRpcErrorCode::OVERLOAD:
    case apache::thrift::ResponseRpcErrorCode::QUEUE_OVERLOADED:
    case apache::thrift::ResponseRpcErrorCode::QUEUE_TIMEOUT:
    case apache::thrift::ResponseRpcErrorCode::APP_OVERLOAD:
    case apache::thrift::ResponseRpcErrorCode::TENANT_QUOTA_EXCEEDED:
    case apache::thrift::ResponseRpcErrorCode::INTERACTION_LOADSHEDDED:
    case apache::thrift::ResponseRpcErrorCode::INTERACTION_LOADSHEDDED_OVERLOAD:
    case apache::thrift::ResponseRpcErrorCode::
        INTERACTION_LOADSHEDDED_APP_OVERLOAD:
    case apache::thrift::ResponseRpcErrorCode::
        INTERACTION_LOADSHEDDED_QUEUE_TIMEOUT:
      return apache::thrift::ResponseRpcErrorCategory::LOADSHEDDING;
    case apache::thrift::ResponseRpcErrorCode::SHUTDOWN:
      return apache::thrift::ResponseRpcErrorCategory::SHUTDOWN;
    case apache::thrift::ResponseRpcErrorCode::UNKNOWN:
    case apache::thrift::ResponseRpcErrorCode::TASK_EXPIRED:
    case apache::thrift::ResponseRpcErrorCode::INJECTED_FAILURE:
    case apache::thrift::ResponseRpcErrorCode::RESPONSE_TOO_BIG:
    case apache::thrift::ResponseRpcErrorCode::INTERRUPTION:
    case apache::thrift::ResponseRpcErrorCode::INTERACTION_CONSTRUCTOR_ERROR:
      return apache::thrift::ResponseRpcErrorCategory::INTERNAL_ERROR;
  }
  return apache::thrift::ResponseRpcErrorCategory::INTERNAL_ERROR;
}

/**
 * Maps ResponseRpcErrorCategory to the RSocket ERROR frame error code.
 * Mirrors the logic in RocketThriftRequests.cpp makeRocketException().
 */
inline apache::thrift::fast_thrift::frame::ErrorCode mapCategoryToErrorCode(
    apache::thrift::ResponseRpcErrorCategory category) {
  switch (category) {
    case apache::thrift::ResponseRpcErrorCategory::INVALID_REQUEST:
      return apache::thrift::fast_thrift::frame::ErrorCode::INVALID;
    case apache::thrift::ResponseRpcErrorCategory::LOADSHEDDING:
    case apache::thrift::ResponseRpcErrorCategory::SHUTDOWN:
      return apache::thrift::fast_thrift::frame::ErrorCode::REJECTED;
    case apache::thrift::ResponseRpcErrorCategory::INTERNAL_ERROR:
      return apache::thrift::fast_thrift::frame::ErrorCode::CANCELED;
  }
  return apache::thrift::fast_thrift::frame::ErrorCode::CANCELED;
}

/**
 * Builds and serializes a ResponseRpcError for ERROR frame transmission.
 * The data is CompactProtocol-serialized, matching the standard Rocket
 * server's ERROR frame format so the client's decodeErrorFrame() works.
 *
 * Returns the serialized data and the matching frame error code that the
 * caller hands to ThriftServerAppAdapter::writeError.
 */
inline SerializedResponseError serializeResponseRpcError(
    apache::thrift::ResponseRpcErrorCode code, std::string message) {
  auto category = mapErrorCodeToCategory(code);

  apache::thrift::ResponseRpcError error;
  error.name_utf8() = apache::thrift::TEnumTraits<
      apache::thrift::ResponseRpcErrorCode>::findName(code);
  error.what_utf8() = std::move(message);
  error.code() = code;
  error.category() = category;

  apache::thrift::CompactProtocolWriter writer;
  auto size = error.serializedSizeZC(&writer);
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  writer.setOutput(&queue, size);
  error.write(&writer);

  return SerializedResponseError{
      .data = queue.move(),
      .errorCode = mapCategoryToErrorCode(category),
  };
}

/**
 * Refines the error code for QUEUE_OVERLOADED: if the exception is
 * TApplicationException::LOADSHEDDING, returns SHUTDOWN instead.
 * Legacy Rocket does this check in processFirstResponseHelper.
 */
inline apache::thrift::ResponseRpcErrorCode refineErrorCode(
    apache::thrift::ResponseRpcErrorCode code,
    const folly::exception_wrapper& ew) {
  if (code == apache::thrift::ResponseRpcErrorCode::QUEUE_OVERLOADED) {
    if (auto* tae = ew.get_exception<TApplicationException>()) {
      if (tae->getType() == TApplicationException::LOADSHEDDING) {
        return apache::thrift::ResponseRpcErrorCode::SHUTDOWN;
      }
    }
  }
  return code;
}

/**
 * Builds an ErrorClassification for a declared exception type Ex by reading
 * the compile-time annotations baked onto the generated exception type
 * (kind/blame/safety from `@thrift.ExceptionMetadata`) and applying any
 * runtime override the throwing site may have attached.
 *
 * Returns std::nullopt if the exception type carries no classification info,
 * so callers can pass through to ResponseMetadata helpers without setting a
 * classification field.
 */
template <typename Ex>
inline std::optional<apache::thrift::ErrorClassification>
getDeclaredExceptionClassification(const folly::exception_wrapper& ew) {
  apache::thrift::ErrorClassification classification;

  constexpr auto exceptionKind = apache::thrift::detail::st::
      struct_private_access::__fbthrift_cpp2_gen_exception_kind<Ex>();
  classification.kind() =
      apache::thrift::util::detail::fromExceptionKind(exceptionKind);
  constexpr auto exceptionBlame = apache::thrift::detail::st::private_access::
      __fbthrift_cpp2_gen_exception_blame<Ex>();
  classification.blame() =
      apache::thrift::util::detail::fromExceptionBlame(exceptionBlame);
  constexpr auto exceptionSafety = apache::thrift::detail::st::private_access::
      __fbthrift_cpp2_gen_exception_safety<Ex>();
  classification.safety() =
      apache::thrift::util::detail::fromExceptionSafety(exceptionSafety);

  ew.with_exception(
      [&classification](
          const apache::thrift::ExceptionMetadataOverrideBase& ex) {
        if (ex.errorKind() != apache::thrift::ExceptionKind::UNSPECIFIED) {
          classification.kind() =
              apache::thrift::util::detail::fromExceptionKind(ex.errorKind());
        }
        if (ex.errorBlame() != apache::thrift::ExceptionBlame::UNSPECIFIED) {
          classification.blame() =
              apache::thrift::util::detail::fromExceptionBlame(ex.errorBlame());
        }
        if (ex.errorSafety() != apache::thrift::ExceptionSafety::UNSPECIFIED) {
          classification.safety() =
              apache::thrift::util::detail::fromExceptionSafety(
                  ex.errorSafety());
        }
      });

  return classification;
}

} // namespace apache::thrift::fast_thrift::thrift
