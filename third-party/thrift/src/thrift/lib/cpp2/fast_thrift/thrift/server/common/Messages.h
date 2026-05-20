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

#include <boost/intrusive_ptr.hpp>

#include <folly/ExceptionWrapper.h>
#include <thrift/lib/cpp2/fast_thrift/common/CompactVariant.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftPayloadVariant.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/PayloadVariants.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/context/ThriftConnContext.h>
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
//
// `connContext` is stamped by ConnectionContextHandler as the message
// traverses the thrift pipeline, so every inbound request carries a handle
// to its connection's context without the tail adapter having to look it up.
#pragma pack(push, 1)
struct ThriftServerRequestMessage {
  boost::intrusive_ptr<ThriftConnContext> connContext;
  ThriftServerInboundPayloadVariant payload;
  uint32_t streamId{0};
};
#pragma pack(pop)

/**
 * ThriftServerResponseMessage - Outbound message from handler to pipeline.
 *
 * `payload` carries the typed response payload; only alternatives wired
 * end-to-end today are listed in `ThriftServerOutboundPayloadVariant`,
 * new alternatives join as their handlers come online.
 */
#pragma pack(push, 1)
struct ThriftServerResponseMessage {
  ThriftServerOutboundPayloadVariant payload;
};
#pragma pack(pop)

} // namespace apache::thrift::fast_thrift::thrift
