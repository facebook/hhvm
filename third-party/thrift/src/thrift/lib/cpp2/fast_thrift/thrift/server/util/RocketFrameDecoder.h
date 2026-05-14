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
#include <thrift/lib/cpp2/fast_thrift/rocket/server/MetadataProtocol.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/PayloadVariants.h>

namespace apache::thrift::fast_thrift::thrift {

// Convert a parsed inbound rocket request frame into the typed thrift
// inbound payload variant. Single decode point for the server bridge —
// handlers above never see ParsedFrame.
//
// `metadataProtocol` is the SETUP-negotiated protocol used to decode the
// frame's metadata (Binary or Compact).
//
// Returns Unexpected on metadata deserialization failure or on a frame
// type that has no server-inbound mapping yet.
folly::Expected<ThriftServerInboundPayloadVariant, folly::exception_wrapper>
fromRocketFrame(
    frame::read::ParsedFrame&& frame,
    rocket::server::MetadataProtocol metadataProtocol);

} // namespace apache::thrift::fast_thrift::thrift
