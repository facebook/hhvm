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
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Handler.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftPayload.h>
#include <thrift/lib/cpp2/transport/rocket/ChecksumGenerator.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift::client::handler {

/**
 * ThriftClientChecksumHandler — outbound-only handler that fills the
 * checksum value on a request's metadata struct before serialization.
 *
 * Only XXH3_64 is supported. The algorithm is selected upstream in
 * `makeRequestMetadata` from `RpcOptions::Checksum`; CRC32 and the legacy
 * SERVER_ONLY_CRC32 (bare `RequestRpcMetadata.crc32c` field) are
 * intentionally not supported by fast_thrift.
 *
 *   - `metadata->checksum()` set with algorithm XXH3_64
 *       → compute via `rocket::ChecksumGenerator<XXH3_64>` and overwrite
 *         `checksum.checksum` + `checksum.salt`.
 *   - Otherwise → pass through (the common case when checksumming is
 *     not requested).
 *
 * Must run AFTER `ThriftClientChannel` (which builds the metadata) and
 * BEFORE `ThriftClientTransportAdapter` (which serializes the metadata
 * via `ThriftRequestResponsePayload::toRocketFrame()`).
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
};

static_assert(
    apache::thrift::fast_thrift::channel_pipeline::OutboundHandler<
        ThriftClientChecksumHandler,
        apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl>,
    "ThriftClientChecksumHandler must satisfy OutboundHandler concept");

} // namespace apache::thrift::fast_thrift::thrift::client::handler
