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
#include <string_view>
#include <utility>

#include <folly/ExceptionWrapper.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp2/fast_thrift/frame/ErrorCode.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponseError.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponseMetadata.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponseSerializer.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

// Free functions that build a fully-assembled ThriftServerResponseMessage for
// each wire shape the server emits. Callers hand the result to a single
// writeResponse() entry point on the adapter — payload construction and
// pipeline write are deliberately separate concerns.

// PAYLOAD frame, application-built data + metadata.
inline ThriftServerResponseMessage makeResponseMessage(
    uint32_t streamId,
    std::unique_ptr<folly::IOBuf> data,
    std::unique_ptr<apache::thrift::ResponseRpcMetadata> metadata) {
  return ThriftServerResponseMessage{
      .payload = ThriftInitialResponsePayload{
          .data = std::move(data),
          .metadata = std::move(metadata),
          .streamId = streamId,
      }};
}

// ERROR frame with caller-supplied body + frame error code.
inline ThriftServerResponseMessage makeErrorMessage(
    uint32_t streamId,
    std::unique_ptr<folly::IOBuf> data,
    apache::thrift::fast_thrift::frame::ErrorCode errorCode) {
  return ThriftServerResponseMessage{
      .payload = ThriftErrorPayload{
          .data = std::move(data),
          .metadata = nullptr,
          .streamId = streamId,
          .errorCode = static_cast<uint32_t>(errorCode)}};
}

// Stream-0 ERROR(CONNECTION_CLOSE) — rsocket-spec graceful-drain signal.
inline ThriftServerResponseMessage makeConnectionCloseMessage() {
  return makeErrorMessage(
      /*streamId=*/0,
      /*data=*/nullptr,
      apache::thrift::fast_thrift::frame::ErrorCode::CONNECTION_CLOSE);
}

// Framework dispatch error — serializes a ResponseRpcError body and sets the
// matching rocket frame error code (parse failure, wrong RPC kind, unknown
// method, etc.).
inline ThriftServerResponseMessage makeFrameworkErrorMessage(
    uint32_t streamId,
    apache::thrift::ResponseRpcErrorCode code,
    std::string message) {
  auto err = serializeResponseRpcError(code, std::move(message));
  return makeErrorMessage(streamId, std::move(err.data), err.errorCode);
}

inline ThriftServerResponseMessage makeWrongRpcKindMessage(
    uint32_t streamId, apache::thrift::RpcKind kind) {
  return makeFrameworkErrorMessage(
      streamId,
      apache::thrift::ResponseRpcErrorCode::WRONG_RPC_KIND,
      std::string("Unsupported RPC kind: ") +
          std::to_string(static_cast<int>(kind)));
}

inline ThriftServerResponseMessage makeUnknownMethodMessage(
    uint32_t streamId, std::string_view methodName) {
  return makeFrameworkErrorMessage(
      streamId,
      apache::thrift::ResponseRpcErrorCode::UNKNOWN_METHOD,
      std::string("Unknown method: ") + std::string(methodName));
}

// PAYLOAD frame with null data + appUnknownException metadata carrying
// name/what/blame. Client surfaces this as TApplicationException.
inline ThriftServerResponseMessage makeAppErrorMessage(
    uint32_t streamId,
    std::string exName,
    std::string exWhat,
    apache::thrift::ErrorBlame blame = apache::thrift::ErrorBlame::SERVER) {
  auto md = std::make_unique<apache::thrift::ResponseRpcMetadata>();
  fillAppErrorResponseMetadata(
      *md, std::move(exName), std::move(exWhat), blame);
  return makeResponseMessage(streamId, /*data=*/nullptr, std::move(md));
}

// Undeclared exception cascade fall-through: extracts name/what from the
// wrapper.
inline ThriftServerResponseMessage makeUnknownExceptionMessage(
    uint32_t streamId,
    const folly::exception_wrapper& ew,
    apache::thrift::ErrorBlame blame = apache::thrift::ErrorBlame::SERVER) {
  return makeAppErrorMessage(
      streamId, ew.class_name().toStdString(), ew.what().toStdString(), blame);
}

// Success: serialize presult into a payload buffer, build success metadata.
template <typename Writer, typename Presult>
ThriftServerResponseMessage makeSuccessResponseMessage(
    uint32_t streamId, const Presult& presult) {
  auto data = serializeResponse<Writer>(
      [&](Writer& w) { presult.write(&w); },
      [&](Writer& w) { return presult.serializedSizeZC(&w); });
  auto md = std::make_unique<apache::thrift::ResponseRpcMetadata>();
  fillSuccessResponseMetadata(*md);
  return makeResponseMessage(streamId, std::move(data), std::move(md));
}

// Declared exception (IDL `throws`): caller has populated the matching
// presult slot via apache::thrift::detail::ap::insert_exn. Same wire family
// as the success path — distinguished only by the metadata variant.
template <typename Writer, typename Presult>
ThriftServerResponseMessage makeDeclaredExceptionMessage(
    uint32_t streamId,
    const Presult& presult,
    const folly::exception_wrapper& ew,
    std::optional<apache::thrift::ErrorClassification> classification) {
  auto data = serializeResponse<Writer>(
      [&](Writer& w) { presult.write(&w); },
      [&](Writer& w) { return presult.serializedSizeZC(&w); });
  auto md = std::make_unique<apache::thrift::ResponseRpcMetadata>();
  fillDeclaredExceptionMetadata(
      *md,
      ew.class_name().toStdString(),
      ew.what().toStdString(),
      classification);
  return makeResponseMessage(streamId, std::move(data), std::move(md));
}

} // namespace apache::thrift::fast_thrift::thrift
