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

#include <algorithm>
#include <cstdint>
#include <memory>
#include <optional>

#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/fast_thrift/frame/ErrorCode.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/MetadataProtocol.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ServerPushMetadata.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_constants.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

// Thrift protocol version range this server negotiates. Matches the legacy
// Rocket server (kRocketServerMinVersion / kRocketServerMaxVersion) so classic
// clients interoperate. Distinct from the RSocket transport version (1.0) that
// the rocket layer validates.
inline constexpr int32_t kMinNegotiableVersion = 8;
inline constexpr int32_t kMaxNegotiableVersion = 10;

// Legacy value of the protocol key some older clients prepend.
inline constexpr uint32_t kLegacyRocketProtocolKey = 1;

namespace detail {
// Clients prepend a 4-byte protocol key inside the SETUP metadata region,
// before the serialized RequestSetupMetadata. Consume it when present; older
// clients omit it, so retreat and parse from the start in that case.
inline void skipProtocolKey(folly::io::Cursor& cursor) {
  if (!cursor.canAdvance(sizeof(uint32_t))) {
    return;
  }
  const uint32_t key = cursor.readBE<uint32_t>();
  if (key != apache::thrift::RpcMetadata_constants::kRocketProtocolKey() &&
      key != kLegacyRocketProtocolKey) {
    cursor.retreat(sizeof(uint32_t));
  }
}

template <typename Reader>
apache::thrift::RequestSetupMetadata readRequestSetupMetadata(
    folly::io::Cursor cursor) {
  Reader reader;
  reader.setInput(cursor);
  apache::thrift::RequestSetupMetadata metadata;
  metadata.read(&reader);
  return metadata;
}
} // namespace detail

// Parses the client's RequestSetupMetadata, negotiates the thrift version, and
// builds the SETUP response (ServerPushMetadata{setupResponse}) to push back.
// Returns a rejection when the client's [minVersion, maxVersion] does not
// overlap [kMinNegotiableVersion, kMaxNegotiableVersion], or when the metadata
// cannot be parsed. noexcept: the rocket setup handler invokes this on a
// noexcept boundary, so all failures map to a rejection rather than escaping.
//
// zstdSupported is reported false: fast_thrift has no compression codec yet, so
// advertising support would be untruthful. Flip to true once per-request
// compression lands.
inline rocket::server::handler::SetupResponseResult makeSetupResponseResult(
    std::unique_ptr<folly::IOBuf> setupMetadata,
    rocket::server::MetadataProtocol metadataProtocol) noexcept {
  using Result = rocket::server::handler::SetupResponseResult;
  using ErrorCode = apache::thrift::fast_thrift::frame::ErrorCode;
  try {
    apache::thrift::RequestSetupMetadata request;
    if (setupMetadata) {
      folly::io::Cursor cursor(setupMetadata.get());
      detail::skipProtocolKey(cursor);
      request = metadataProtocol == rocket::server::MetadataProtocol::COMPACT
          ? detail::readRequestSetupMetadata<
                apache::thrift::CompactProtocolReader>(cursor)
          : detail::readRequestSetupMetadata<
                apache::thrift::BinaryProtocolReader>(cursor);
    }

    const int32_t clientMin = request.minVersion().value_or(0);
    const int32_t clientMax = request.maxVersion().value_or(
        request.minVersion().has_value() ? clientMin : kMinNegotiableVersion);

    if (clientMax < kMinNegotiableVersion ||
        clientMin > kMaxNegotiableVersion) {
      return Result{
          .metadataPush = nullptr, .reject = ErrorCode::INVALID_SETUP};
    }

    apache::thrift::SetupResponse setupResponse;
    setupResponse.version() = std::min(clientMax, kMaxNegotiableVersion);
    setupResponse.zstdSupported() = false;

    apache::thrift::ServerPushMetadata push;
    push.set_setupResponse(std::move(setupResponse));

    return Result{
        .metadataPush = serializeServerPushMetadata(push),
        .reject = std::nullopt};
  } catch (...) {
    return Result{.metadataPush = nullptr, .reject = ErrorCode::INVALID_SETUP};
  }
}

} // namespace apache::thrift::fast_thrift::thrift
