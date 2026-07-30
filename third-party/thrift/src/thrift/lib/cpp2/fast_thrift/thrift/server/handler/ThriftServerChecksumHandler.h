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
#include <string>
#include <utility>

#include <folly/ExceptionWrapper.h>
#include <folly/GLog.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Handler.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/context/ThriftRequestContext.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponsePayloads.h>
#include <thrift/lib/cpp2/transport/rocket/ChecksumGenerator.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * ThriftServerChecksumHandler — duplex handler, mirror of the client-side
 * ThriftClientChecksumHandler. Only XXH3_64 is supported.
 *
 * Inbound (request): validates the checksum on the request against the request
 * data IOBuf.
 *
 *   - `metadata->checksum()` set with XXH3_64 → recompute and compare. On
 *     success, record the algorithm on the per-request ThriftRequestContext so
 *     the response echoes it (see the outbound stamp in FastHandlerCallback).
 *   - Mismatch → emit a CHECKSUM_MISMATCH error response for this stream and
 *     drop the request. The request never reaches the tail, so it is not
 *     dispatched. The connection stays open.
 *   - Unsupported algorithm (CRC32 or unknown) → same error path.
 *   - No checksum → pass-through (the common case).
 *
 * Outbound (response): reads the algorithm the request recorded on the
 * response's `requestContext` (forwarded onto the message by
 * FastHandlerCallback). When non-NONE, stamps `metadata->checksum()` with that
 * algorithm and computes the value over the response data IOBuf. Responses with
 * no requestContext (framework errors) or a non-initial payload pass through.
 *
 * Pipeline order: MUST sit downstream of ThriftServerRequestContextHandler
 * (which allocates the per-request ThriftRequestContext).
 */
template <typename Context>
class ThriftServerChecksumHandler {
 public:
  // HandlerLifecycle
  void handlerAdded(Context& /*ctx*/) noexcept {}
  void handlerRemoved(Context& /*ctx*/) noexcept {}

  // InboundHandler
  channel_pipeline::Result onRead(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto& request = msg.get<ThriftServerRequestMessage>();

    if (FOLLY_UNLIKELY(
            !request.payload.template is<ThriftRequestResponsePayload>())) {
      return ctx.fireRead(std::move(msg));
    }

    auto& rr = request.payload.template get<ThriftRequestResponsePayload>();
    if (rr.metadata == nullptr || !rr.metadata->checksum().has_value()) {
      return ctx.fireRead(std::move(msg));
    }

    const auto& checksum = *rr.metadata->checksum();
    if (auto error = validateChecksum(checksum, rr.data.get())) {
      // Mismatch/unsupported: fail just this request. fireWrite propagates the
      // error response toward head from here; it never re-enters our own
      // onWrite and never reaches the tail, so the request is dropped with
      // nothing dispatched.
      return ctx.fireWrite(
          channel_pipeline::erase_and_box(makeFrameworkErrorMessage(
              request.streamId,
              apache::thrift::ResponseRpcErrorCode::CHECKSUM_MISMATCH,
              std::move(*error))));
    }

    if (request.requestContext != nullptr) {
      request.requestContext->setChecksumAlgorithm(
          checksum.algorithm().value());
    }
    return ctx.fireRead(std::move(msg));
  }

  void onReadReady(Context& /*ctx*/) noexcept {}

  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

  void onPipelineActive(Context& /*ctx*/) noexcept {}

  // OutboundHandler
  channel_pipeline::Result onWrite(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto& response = msg.get<ThriftServerResponseMessage>();
    if (response.requestContext != nullptr &&
        response.payload.template is<ThriftInitialResponsePayload>()) {
      fillChecksum(
          response.payload.template get<ThriftInitialResponsePayload>(),
          response.requestContext->getChecksumAlgorithm());
    }
    return ctx.fireWrite(std::move(msg));
  }

  void onWriteReady(Context& /*ctx*/) noexcept {}

  void onPipelineInactive(Context& /*ctx*/) noexcept {}

 private:
  // Returns std::nullopt when validation passes (or is skipped). Returns an
  // error message for the CHECKSUM_MISMATCH response on mismatch or unsupported
  // algorithm.
  static std::optional<std::string> validateChecksum(
      const apache::thrift::Checksum& checksum, folly::IOBuf* data) noexcept {
    switch (checksum.algorithm().value()) {
      case apache::thrift::ChecksumAlgorithm::NONE:
        return std::nullopt;
      case apache::thrift::ChecksumAlgorithm::XXH3_64:
        if (data == nullptr ||
            !apache::thrift::rocket::ChecksumGenerator<
                 apache::thrift::rocket::XXH3_64>{}
                 .validateChecksumFromIOBuf(
                     checksum.checksum().value(),
                     checksum.salt().value(),
                     *data)) {
          return "Checksum mismatch";
        }
        return std::nullopt;
      case apache::thrift::ChecksumAlgorithm::CRC32:
        FB_LOG_ONCE(ERROR)
            << "ChecksumAlgorithm::CRC32 is not supported by fast_thrift; "
               "use XXH3_64. Failing request.";
        return "Unsupported checksum algorithm";
      default:
        FB_LOG_ONCE(ERROR) << "Unexpected ChecksumAlgorithm value: "
                           << static_cast<int>(checksum.algorithm().value())
                           << ". Failing request.";
        return "Unsupported checksum algorithm";
    }
  }

  // Stamps the algorithm onto the response metadata and computes the value over
  // the response data. No-op when no checksum was requested. The only algorithm
  // ever recorded on the request context is XXH3_64 (CRC32/unknown requests are
  // rejected inbound), so the value is always computed here.
  static void fillChecksum(
      ThriftInitialResponsePayload& payload,
      apache::thrift::ChecksumAlgorithm algorithm) noexcept {
    if (algorithm == apache::thrift::ChecksumAlgorithm::NONE ||
        payload.metadata == nullptr || payload.data == nullptr) {
      return;
    }
    apache::thrift::Checksum checksum;
    checksum.algorithm() = algorithm;
    payload.metadata->checksum() = checksum;
    computeChecksum(*payload.metadata->checksum(), payload.data.get());
  }

  static void computeChecksum(
      apache::thrift::Checksum& checksum, folly::IOBuf* data) noexcept {
    if (data == nullptr) {
      return;
    }
    switch (checksum.algorithm().value()) {
      case apache::thrift::ChecksumAlgorithm::XXH3_64: {
        auto response = apache::thrift::rocket::ChecksumGenerator<
                            apache::thrift::rocket::XXH3_64>{}
                            .calculateChecksumFromIOBuf(*data);
        checksum.checksum() = response.checksum;
        checksum.salt() = response.salt;
        break;
      }
      case apache::thrift::ChecksumAlgorithm::CRC32:
        FB_LOG_ONCE(ERROR)
            << "ChecksumAlgorithm::CRC32 is not supported by fast_thrift; "
               "use XXH3_64. Skipping checksum.";
        break;
      case apache::thrift::ChecksumAlgorithm::NONE:
        break;
      default:
        FB_LOG_ONCE(ERROR) << "Unexpected ChecksumAlgorithm value: "
                           << static_cast<int>(checksum.algorithm().value())
                           << ". Skipping checksum.";
        break;
    }
  }
};

static_assert(
    channel_pipeline::OutboundHandler<
        ThriftServerChecksumHandler<channel_pipeline::detail::ContextImpl>,
        channel_pipeline::detail::ContextImpl>,
    "ThriftServerChecksumHandler must satisfy OutboundHandler concept");

static_assert(
    channel_pipeline::InboundHandler<
        ThriftServerChecksumHandler<channel_pipeline::detail::ContextImpl>,
        channel_pipeline::detail::ContextImpl>,
    "ThriftServerChecksumHandler must satisfy InboundHandler concept");

} // namespace apache::thrift::fast_thrift::thrift
