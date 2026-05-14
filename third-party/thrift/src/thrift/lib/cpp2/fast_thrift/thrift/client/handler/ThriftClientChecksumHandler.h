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
#include <folly/GLog.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Handler.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>
#include <thrift/lib/cpp2/transport/rocket/ChecksumGenerator.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift::client::handler {

/**
 * ThriftClientChecksumHandler — duplex handler.
 *
 * Only XXH3_64 is supported. The algorithm is selected upstream in
 * `makeRequestMetadata` from `RpcOptions::Checksum`; CRC32 and the legacy
 * SERVER_ONLY_CRC32 (bare `RequestRpcMetadata.crc32c` field) are
 * intentionally not supported by fast_thrift.
 *
 * Outbound (request): fills the checksum value on a request's metadata
 * struct before serialization. Reads `metadata->checksum()` and computes
 * the value over the request's data IOBuf:
 *
 *   - `metadata->checksum()` set with algorithm XXH3_64
 *       → compute via `rocket::ChecksumGenerator<XXH3_64>` and overwrite
 *         `checksum.checksum` + `checksum.salt`.
 *   - `metadata->checksum()` set with algorithm CRC32 → log once and skip
 *     (placeholder remains, server-side validation will fail).
 *   - Otherwise → pass through (the common case when checksumming is
 *     not requested).
 *
 * Inbound (response): validates the checksum on `ThriftInitialResponsePayload`
 * against the data IOBuf. Mirrors legacy
 * `ChecksumPayloadSerializerStrategy`:
 *
 *   - `metadata->checksum()` set with XXH3_64 → recompute and compare.
 *   - Otherwise (including when the client requested a checksum but the
 *     server didn't return one) → silent pass-through. Matches legacy.
 *   - Mismatch → replace payload with `ThriftClientResponseError` carrying
 *     `TApplicationException(CHECKSUM_MISMATCH, "Checksum mismatch")`.
 *     Channel fails just this callback; connection stays Open.
 *   - Unsupported algorithm (CRC32 or unknown) → same error path with
 *     the legacy message "Unsupported checksum algorithm".
 *
 * Note: the legacy response field `metadata->crc32c()` is NOT validated
 * on inbound — legacy doesn't either. See
 * `transport/rocket/payload/ChecksumPayloadSerializerStrategy.h:171-251`.
 *
 * Pipeline placement — request: AFTER `ThriftClientChannel` (which
 * builds the metadata), BEFORE `ThriftClientTransportAdapter` (which
 * serializes via `ThriftRequestResponsePayload::toRocketFrame()`).
 * Response: AFTER `ThriftClientTransportAdapter` (which produces the
 * typed `ThriftClientInboundPayloadVariant`), BEFORE `ThriftClientChannel`.
 */
class ThriftClientChecksumHandler {
 public:
  ThriftClientChecksumHandler() = default;

  // === HandlerLifecycle ===

  template <typename Context>
  void handlerAdded(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {}

  // === OutboundHandler ===

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onPipelineInactive(Context& /*ctx*/) noexcept {}

  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onWrite(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto& request = msg.get<ThriftRequestMessage>();

    if (FOLLY_LIKELY(request.payload.is<ThriftRequestResponsePayload>())) {
      auto& payload = request.payload.get<ThriftRequestResponsePayload>();
      fillChecksum(*payload.metadata, *payload.data);
    }

    return ctx.fireWrite(std::move(msg));
  }

  // === InboundHandler ===

  template <typename Context>
  void onPipelineActive(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onReadReady(Context& /*ctx*/) noexcept {}

  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onRead(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto& response = msg.get<ThriftResponseMessage>();

    // Pass through the transport-failure variant unchanged.
    if (FOLLY_UNLIKELY(
            !response.payload.is<ThriftClientInboundPayloadVariant>())) {
      return ctx.fireRead(std::move(msg));
    }

    auto& inbound = response.payload.get<ThriftClientInboundPayloadVariant>();
    if (!inbound.is<ThriftInitialResponsePayload>()) {
      // Subsequent stream chunks, errors, cancels, etc. — no checksum
      // contract on those payload alternatives today.
      return ctx.fireRead(std::move(msg));
    }

    auto& payload = inbound.get<ThriftInitialResponsePayload>();
    if (auto error = validateChecksum(*payload.metadata, payload.data.get())) {
      // Mismatch: convert to per-request transport error so the channel
      // fails just this callback. Connection stays Open.
      response.payload = ThriftClientResponseError{.ew = std::move(error)};
      return ctx.fireRead(std::move(msg));
    }

    return ctx.fireRead(std::move(msg));
  }

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

 private:
  static void fillChecksum(
      apache::thrift::RequestRpcMetadata& metadata,
      folly::IOBuf& data) noexcept {
    auto checksum = metadata.checksum();
    if (!checksum.has_value()) {
      return;
    }
    computeStructChecksum(*checksum, data);
    metadata.checksum() = *checksum;
  }

  static void computeStructChecksum(
      apache::thrift::Checksum& checksum, folly::IOBuf& data) noexcept {
    switch (checksum.algorithm().value()) {
      case apache::thrift::ChecksumAlgorithm::XXH3_64: {
        auto response = apache::thrift::rocket::ChecksumGenerator<
                            apache::thrift::rocket::XXH3_64>{}
                            .calculateChecksumFromIOBuf(data);
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

  // Returns an empty exception_wrapper when validation passes (or is
  // skipped because no checksum was sent). Returns a CHECKSUM_MISMATCH
  // TApplicationException on mismatch or unsupported algorithm.
  static folly::exception_wrapper validateChecksum(
      const apache::thrift::ResponseRpcMetadata& metadata,
      folly::IOBuf* data) noexcept {
    const auto& checksumRef = metadata.checksum();
    if (!checksumRef.has_value()) {
      // Server didn't echo a checksum. Silent pass-through, matching
      // legacy behavior.
      return {};
    }
    return validateStructChecksum(*checksumRef, data);
  }

  static folly::exception_wrapper validateStructChecksum(
      const apache::thrift::Checksum& checksum, folly::IOBuf* data) noexcept {
    switch (checksum.algorithm().value()) {
      case apache::thrift::ChecksumAlgorithm::NONE:
        return {};
      case apache::thrift::ChecksumAlgorithm::XXH3_64:
        return compareChecksum<apache::thrift::rocket::XXH3_64>(checksum, data);
      case apache::thrift::ChecksumAlgorithm::CRC32:
        FB_LOG_ONCE(ERROR)
            << "ChecksumAlgorithm::CRC32 is not supported by fast_thrift; "
               "use XXH3_64. Failing response.";
        return folly::make_exception_wrapper<
            apache::thrift::TApplicationException>(
            apache::thrift::TApplicationException::CHECKSUM_MISMATCH,
            "Unsupported checksum algorithm");
      default:
        FB_LOG_ONCE(ERROR) << "Unexpected ChecksumAlgorithm value: "
                           << static_cast<int>(checksum.algorithm().value())
                           << ". Failing response.";
        return folly::make_exception_wrapper<
            apache::thrift::TApplicationException>(
            apache::thrift::TApplicationException::CHECKSUM_MISMATCH,
            "Unsupported checksum algorithm");
    }
  }

  template <typename Algo>
  static folly::exception_wrapper compareChecksum(
      const apache::thrift::Checksum& checksum, folly::IOBuf* data) noexcept {
    if (data == nullptr) {
      return folly::make_exception_wrapper<
          apache::thrift::TApplicationException>(
          apache::thrift::TApplicationException::CHECKSUM_MISMATCH,
          "Checksum mismatch");
    }
    if (!apache::thrift::rocket::ChecksumGenerator<Algo>{}
             .validateChecksumFromIOBuf(
                 checksum.checksum().value(), checksum.salt().value(), *data)) {
      return folly::make_exception_wrapper<
          apache::thrift::TApplicationException>(
          apache::thrift::TApplicationException::CHECKSUM_MISMATCH,
          "Checksum mismatch");
    }
    return {};
  }
};

static_assert(
    apache::thrift::fast_thrift::channel_pipeline::OutboundHandler<
        ThriftClientChecksumHandler,
        apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl>,
    "ThriftClientChecksumHandler must satisfy OutboundHandler concept");

static_assert(
    apache::thrift::fast_thrift::channel_pipeline::InboundHandler<
        ThriftClientChecksumHandler,
        apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl>,
    "ThriftClientChecksumHandler must satisfy InboundHandler concept");

} // namespace apache::thrift::fast_thrift::thrift::client::handler
