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
#include <thrift/lib/cpp2/fast_thrift/common/CompactVariant.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftPayload.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftPayloadVariant.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/RocketFrameDecoder.h>

#include <cstdint>

namespace apache::thrift::fast_thrift::thrift {

// ============================================================================
// Server <-> Pipeline Interface
// ============================================================================
//
//   Pipeline  ─────ThriftServerRequestMessage─────>  Handler
//   Pipeline  <────ThriftServerResponseMessage─────  Handler

// `payload` is the typed wire-derived inbound payload produced by
// `fromRocketFrame` at the transport bridge.
#pragma pack(push, 1)
struct ThriftServerRequestMessage {
  ThriftServerInboundPayloadVariant payload;
  uint32_t streamId{0};
};
#pragma pack(pop)

/**
 * ThriftServerResponseMessage - Outbound message from handler to pipeline.
 *
 * `payload` is a typed variant of per-stream payload structs from
 * `thrift/common/ThriftPayload.h`. Today the variant carries
 * `ThriftFirstResponsePayload` (initial reply, carrying the typed
 * `ResponseRpcMetadata` which the payload's `toRocketFrame()` serializes)
 * and `ThriftErrorPayload` (failure) — the only response operations wired
 * end-to-end through the server. As stream/sink/bidi handlers come online,
 * a separate payload for subsequent stream chunks plus `ThriftCancelPayload`
 * and `ThriftRequestNPayload` will join the variant.
 *
 * The variant carries `rpcKind` internally; the transport adapter uses
 * it to set the rocket message's `streamType` for the matching pattern
 * handler. Each per-stream payload also carries its `streamId`.
 */
#pragma pack(push, 1)
struct ThriftServerResponseMessage {
  ThriftPayloadVariant<ThriftFirstResponsePayload, ThriftErrorPayload> payload;
};
#pragma pack(pop)

} // namespace apache::thrift::fast_thrift::thrift
