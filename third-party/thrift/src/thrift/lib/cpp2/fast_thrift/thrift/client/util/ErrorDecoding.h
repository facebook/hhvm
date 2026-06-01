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
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameViews.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

#include <fmt/core.h>

namespace apache::thrift::fast_thrift::thrift {

namespace detail {

// Error codes from RSocket protocol
constexpr uint32_t kErrorCodeCanceled = 0x00000203;
constexpr uint32_t kErrorCodeInvalid = 0x00000204;
constexpr uint32_t kErrorCodeRejected = 0x00000202;

/**
 * Maps ResponseRpcErrorCode to TApplicationException type.
 *
 */
inline std::pair<
    apache::thrift::TApplicationException::TApplicationExceptionType,
    std::optional<std::string>>
mapErrorCodeToException(apache::thrift::ResponseRpcErrorCode code) {
  using ExType =
      apache::thrift::TApplicationException::TApplicationExceptionType;

  switch (code) {
    case apache::thrift::ResponseRpcErrorCode::OVERLOAD:
      return {ExType::LOADSHEDDING, kOverloadedErrorCode};
    case apache::thrift::ResponseRpcErrorCode::TASK_EXPIRED:
      return {ExType::TIMEOUT, kTaskExpiredErrorCode};
    case apache::thrift::ResponseRpcErrorCode::QUEUE_OVERLOADED:
    case apache::thrift::ResponseRpcErrorCode::SHUTDOWN:
      return {ExType::LOADSHEDDING, kQueueOverloadedErrorCode};
    case apache::thrift::ResponseRpcErrorCode::INJECTED_FAILURE:
      return {ExType::INJECTED_FAILURE, kInjectedFailureErrorCode};
    case apache::thrift::ResponseRpcErrorCode::REQUEST_PARSING_FAILURE:
      return {ExType::UNSUPPORTED_CLIENT_TYPE, kRequestParsingErrorCode};
    case apache::thrift::ResponseRpcErrorCode::QUEUE_TIMEOUT:
      return {ExType::TIMEOUT, kServerQueueTimeoutErrorCode};
    case apache::thrift::ResponseRpcErrorCode::RESPONSE_TOO_BIG:
      return {ExType::INTERNAL_ERROR, kResponseTooBigErrorCode};
    case apache::thrift::ResponseRpcErrorCode::WRONG_RPC_KIND:
      return {
          ExType::UNKNOWN_METHOD, kRequestTypeDoesntMatchServiceFunctionType};
    case apache::thrift::ResponseRpcErrorCode::UNKNOWN_METHOD:
      return {ExType::UNKNOWN_METHOD, kMethodUnknownErrorCode};
    case apache::thrift::ResponseRpcErrorCode::CHECKSUM_MISMATCH:
      return {ExType::CHECKSUM_MISMATCH, kUnknownErrorCode};
    case apache::thrift::ResponseRpcErrorCode::INTERRUPTION:
      return {ExType::INTERRUPTION, std::nullopt};
    case apache::thrift::ResponseRpcErrorCode::APP_OVERLOAD:
      return {ExType::LOADSHEDDING, kAppOverloadedErrorCode};
    case apache::thrift::ResponseRpcErrorCode::UNKNOWN_INTERACTION_ID:
      return {ExType::UNKNOWN, kInteractionIdUnknownErrorCode};
    case apache::thrift::ResponseRpcErrorCode::INTERACTION_CONSTRUCTOR_ERROR:
      return {ExType::UNKNOWN, kInteractionConstructorErrorErrorCode};
    case apache::thrift::ResponseRpcErrorCode::UNIMPLEMENTED_METHOD:
      return {ExType::UNKNOWN_METHOD, kUnimplementedMethodErrorCode};
    case apache::thrift::ResponseRpcErrorCode::TENANT_QUOTA_EXCEEDED:
      return {ExType::LOADSHEDDING, kTenantQuotaExceededErrorCode};
    default:
      return {ExType::UNKNOWN, kUnknownErrorCode};
  }
}

} // namespace detail

/**
 * Decodes an ERROR frame into an exception wrapper.
 *
 * For error codes CANCELED/INVALID/REJECTED: parses the payload as
 * ResponseRpcError and converts to TApplicationException with the
 * appropriate type.
 *
 * For other error codes (APPLICATION_ERROR, CONNECTION_ERROR, etc.):
 * creates a generic TApplicationException.
 *
 * This mirrors the logic in RocketClientChannelBase::decodeResponseError().
 *
 * @param frame The parsed ERROR frame
 * @return An exception wrapper containing the appropriate exception
 */
inline folly::exception_wrapper decodeErrorFrame(
    const apache::thrift::fast_thrift::frame::read::ParsedFrame& frame) {
  DCHECK(frame.type() == apache::thrift::fast_thrift::frame::FrameType::ERROR);

  apache::thrift::fast_thrift::frame::read::ErrorView errorView(frame);
  uint32_t errorCode = errorView.errorCode();

  // For CANCELED, INVALID, REJECTED: parse payload as ResponseRpcError
  if (errorCode == detail::kErrorCodeCanceled ||
      errorCode == detail::kErrorCodeInvalid ||
      errorCode == detail::kErrorCodeRejected) {
    // Try to parse the payload as ResponseRpcError
    apache::thrift::ResponseRpcError responseError;
    try {
      if (frame.dataSize() > 0) {
        apache::thrift::CompactProtocolReader reader;
        auto cursor = frame.dataCursor();
        reader.setInput(cursor);
        responseError.read(&reader);
      }
    } catch (...) {
      return folly::make_exception_wrapper<
          apache::thrift::TApplicationException>(
          apache::thrift::TApplicationException::UNKNOWN,
          fmt::format(
              "Error parsing error frame: {}",
              folly::exceptionStr(folly::current_exception()).toStdString()));
    }

    auto [exType, exCode] =
        detail::mapErrorCodeToException(responseError.code().value_or(
            apache::thrift::ResponseRpcErrorCode::UNKNOWN));

    return folly::make_exception_wrapper<apache::thrift::TApplicationException>(
        exType, responseError.what_utf8().value_or(""));
  }

  // For other error codes: create generic TApplicationException
  return folly::make_exception_wrapper<apache::thrift::TApplicationException>(
      apache::thrift::TApplicationException::UNKNOWN,
      fmt::format("Unexpected error frame type: {}", errorCode));
}

} // namespace apache::thrift::fast_thrift::thrift
