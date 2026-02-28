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

#include <chrono>

#include <folly/dynamic.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/util/ManagedStringView.h>
#include <thrift/lib/cpp2/util/MethodMetadata.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift {

class RpcOptions;

namespace detail {

// User exception name
inline constexpr std::string_view kHeaderUex = "uex";
inline constexpr std::string_view kHeaderProxiedUex = "puex";
// User exception message
inline constexpr std::string_view kHeaderUexw = "uexw";
inline constexpr std::string_view kHeaderProxiedUexw = "puexw";
// Server exception (code defined in ResponseChannel.h)
inline constexpr std::string_view kHeaderEx = "ex";
inline constexpr std::string_view kHeaderProxiedEx = "pex";
// Exception metadata (base64-encoded compact-serialized ErrorClassification)
inline constexpr std::string_view kHeaderExMeta = "exm";
// Any exception data (base64-encoded compact-serialized data without wrapping
// AnyStruct)
inline constexpr std::string_view kHeaderAnyex = "anyex";
inline constexpr std::string_view kHeaderProxiedAnyex = "panyex";
// Any exception type
inline constexpr std::string_view kHeaderAnyexType = "anyext";
inline constexpr std::string_view kHeaderProxiedAnyexType = "panyext";

RequestRpcMetadata makeRequestRpcMetadata(
    const RpcOptions& rpcOptions,
    RpcKind kind,
    MethodMetadata&& methodMetadata,
    std::optional<std::chrono::milliseconds> clientTimeout,
    std::variant<InteractionCreate, int64_t, std::monostate> interactionHandle,
    bool serverZstdSupported,
    ssize_t payloadSize,
    transport::THeader& header,
    std::unique_ptr<folly::IOBuf> frameworkMetadata,
    bool customCompressionEnabled);

void fillTHeaderFromResponseRpcMetadata(
    ResponseRpcMetadata& responseMetadata, transport::THeader& header);

void fillResponseRpcMetadataFromTHeader(
    transport::THeader& header, ResponseRpcMetadata& responseMetadata);

std::string serializeErrorClassification(ErrorClassification ec);
ErrorClassification deserializeErrorClassification(std::string_view str);

} // namespace detail
} // namespace apache::thrift
