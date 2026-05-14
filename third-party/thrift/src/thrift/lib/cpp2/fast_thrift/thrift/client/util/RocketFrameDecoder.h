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

#include <folly/Expected.h>
#include <folly/lang/Exception.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftPayload.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftPayloadVariant.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * Inbound payload variant produced by `fromRocketFrame` — typed
 * representation of a wire-derived response frame on the client (and,
 * eventually, server-inbound stream control frames).
 *
 * Callers downstream of the rocket→thrift boundary operate on this
 * variant and never see `ParsedFrame`. Swapping rocket for another
 * transport means writing a different `from*Frame` returning the same
 * variant; the thrift pipeline above is unchanged.
 */
using ThriftClientInboundPayloadVariant = ThriftPayloadVariant<
    ThriftFirstResponsePayload,
    ThriftErrorPayload,
    ThriftCancelPayload,
    ThriftRequestNPayload,
    ThriftMetadataPushPayload>;

/**
 * Convert a parsed rocket frame into the typed thrift inbound payload
 * variant. Decouples the thrift pipeline from rocket — handlers above
 * the bridge consume the variant, never `ParsedFrame`.
 *
 * Dispatch is by frame type; per-pattern semantics (RR vs Stream first
 * vs subsequent) come from `kind`:
 *   - PAYLOAD on REQUEST_RESPONSE → `ThriftFirstResponsePayload`
 *     (deserializes `ResponseRpcMetadata` from the metadata IOBuf).
 *   - ERROR  → `ThriftErrorPayload` (errorCode + remaining payload).
 *   - CANCEL → `ThriftCancelPayload` (header-only).
 *   - REQUEST_N → `ThriftRequestNPayload`.
 *   - METADATA_PUSH → `ThriftMetadataPushPayload` (connection-level,
 *     metadata-only). `kind` is ignored for this frame type.
 *
 * Stream / Sink / Bidi PAYLOADs are not yet routed — they require
 * first-vs-subsequent disambiguation that lives in stream state. Calling
 * `fromRocketFrame` with PAYLOAD on those kinds returns an error.
 *
 * Returns folly::Unexpected on:
 *   - Metadata deserialization failure (corrupt wire bytes).
 *   - Unsupported (frame type, RpcKind) combination.
 */
folly::Expected<ThriftClientInboundPayloadVariant, folly::exception_wrapper>
fromRocketFrame(
    apache::thrift::fast_thrift::frame::read::ParsedFrame&& frame,
    apache::thrift::RpcKind kind);

} // namespace apache::thrift::fast_thrift::thrift
