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
#include <thrift/lib/cpp2/transport/rocket/framing/ErrorCode.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

#include <fmt/core.h>

namespace apache::thrift::fast_thrift::thrift {

namespace detail {

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
    case apache::thrift::ResponseRpcErrorCode::INTERACTION_LOADSHEDDED:
      return {ExType::LOADSHEDDING, kInteractionLoadsheddedErrorCode};
    case apache::thrift::ResponseRpcErrorCode::INTERACTION_LOADSHEDDED_OVERLOAD:
      return {ExType::LOADSHEDDING, kInteractionLoadsheddedOverloadErrorCode};
    case apache::thrift::ResponseRpcErrorCode::
        INTERACTION_LOADSHEDDED_APP_OVERLOAD:
      return {
          ExType::LOADSHEDDING, kInteractionLoadsheddedAppOverloadErrorCode};
    case apache::thrift::ResponseRpcErrorCode::
        INTERACTION_LOADSHEDDED_QUEUE_TIMEOUT:
      return {
          ExType::LOADSHEDDING, kInteractionLoadsheddedQueueTimeoutErrorCode};
    default:
      return {ExType::UNKNOWN, kUnknownErrorCode};
  }
}

} // namespace detail

/**
 * Decoded error frame result with full metadata for ThriftClientChannel.
 */
struct DecodedErrorResponse {
  apache::thrift::TApplicationException::TApplicationExceptionType exType{
      apache::thrift::TApplicationException::UNKNOWN};
  std::optional<std::string> exCode;
  std::optional<int64_t> load;
  std::string what;
};

/**
 * Decodes an ERROR frame into structured data with metadata.
 *
 * For error codes CANCELED/INVALID/REJECTED: parses the payload as
 * ResponseRpcError and returns a DecodedErrorResponse with exType, exCode,
 * what, and load metadata.
 *
 * For other error codes (APPLICATION_ERROR, CONNECTION_ERROR, etc.):
 * returns an exception_wrapper containing a generic TApplicationException.
 *
 * This mirrors the logic in RocketClientChannelBase::decodeResponseError().
 */
inline folly::Expected<DecodedErrorResponse, folly::exception_wrapper>
decodeErrorFrameAsResponse(
    const apache::thrift::fast_thrift::frame::read::ParsedFrame& frame) {
  DCHECK(frame.type() == apache::thrift::fast_thrift::frame::FrameType::ERROR);

  apache::thrift::fast_thrift::frame::read::ErrorView errorView(frame);
  auto errorCode =
      static_cast<apache::thrift::rocket::ErrorCode>(errorView.errorCode());

  if (errorCode == apache::thrift::rocket::ErrorCode::CANCELED ||
      errorCode == apache::thrift::rocket::ErrorCode::INVALID ||
      errorCode == apache::thrift::rocket::ErrorCode::REJECTED) {
    apache::thrift::ResponseRpcError responseError;
    try {
      if (frame.dataSize() > 0) {
        apache::thrift::CompactProtocolReader reader;
        auto cursor = frame.dataCursor();
        reader.setInput(cursor);
        responseError.read(&reader);
      }
    } catch (...) {
      return folly::makeUnexpected(
          folly::make_exception_wrapper<apache::thrift::TApplicationException>(
              apache::thrift::TApplicationException::UNKNOWN,
              fmt::format(
                  "Error parsing error frame: {}",
                  folly::exceptionStr(folly::current_exception())
                      .toStdString())));
    }

    auto [exType, exCode] =
        detail::mapErrorCodeToException(responseError.code().value_or(
            apache::thrift::ResponseRpcErrorCode::UNKNOWN));

    DecodedErrorResponse result;
    result.exType = exType;
    result.exCode = std::move(exCode);
    result.what = responseError.what_utf8().value_or("");
    if (auto loadRef = responseError.load()) {
      result.load = *loadRef;
    }
    return result;
  }

  return folly::makeUnexpected(
      folly::make_exception_wrapper<apache::thrift::TApplicationException>(
          apache::thrift::TApplicationException::UNKNOWN,
          fmt::format(
              "Unexpected error frame type: {}",
              static_cast<uint32_t>(errorCode))));
}

/**
 * Decodes an ERROR frame into an exception wrapper, discarding exCode and
 * load metadata. Use decodeErrorFrameAsResponse() if those are needed.
 */
inline folly::exception_wrapper decodeErrorFrame(
    const apache::thrift::fast_thrift::frame::read::ParsedFrame& frame) {
  auto result = decodeErrorFrameAsResponse(frame);
  if (result.hasValue()) {
    return folly::make_exception_wrapper<apache::thrift::TApplicationException>(
        result->exType, std::move(result->what));
  }
  return std::move(result.error());
}

} // namespace apache::thrift::fast_thrift::thrift
