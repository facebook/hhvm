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

#include <cstdint>
#include <string_view>

namespace apache::thrift::fast_thrift::rocket::server {

// Per-connection wire encoding for the RpcMetadata Thrift struct, negotiated
// once at SETUP time via the metadata MIME type. Distinct from the per-RPC
// payload protocol (apache::thrift::ProtocolId) — that one describes how the
// request/response args are encoded and admits many values; this one is
// metadata-only and is exactly two-valued by the rocket-thrift wire spec.
enum class MetadataProtocol : uint8_t {
  BINARY,
  COMPACT,
};

// Wire MIME types negotiated in the SETUP frame. Match the values used by
// the standard ThriftRocketServer / RocketClient so fast and standard
// endpoints interoperate.
inline constexpr std::string_view kMetadataBinaryMimeType{
    "application/x-rocket-metadata+binary"};
inline constexpr std::string_view kMetadataCompactMimeType{
    "application/x-rocket-metadata+compact"};

// Map a wire MIME string to its enum value. Unknown / unrecognized MIMEs
// fall back to Binary (today's default for non-negotiating peers).
inline constexpr MetadataProtocol metadataProtocolFromMimeType(
    std::string_view mime) noexcept {
  return mime == kMetadataCompactMimeType ? MetadataProtocol::COMPACT
                                          : MetadataProtocol::BINARY;
}

} // namespace apache::thrift::fast_thrift::rocket::server
