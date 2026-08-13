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

#include <thrift/lib/cpp2/fast_thrift/rocket/server/MetadataProtocol.h>

namespace apache::thrift::fast_thrift::rocket::server {

/**
 * RSocket-level parameters read off a validated SETUP frame.
 */
struct SetupParameters {
  uint16_t majorVersion{0};
  uint16_t minorVersion{0};
  uint32_t keepaliveTime{0};
  uint32_t maxLifetime{0};
  bool hasLease{false};
  // Wire encoding for per-connection RpcMetadata, negotiated via the SETUP
  // frame's metadata MIME type. Defaults to Binary (matches today's behavior).
  MetadataProtocol metadataProtocol{MetadataProtocol::BINARY};
};

} // namespace apache::thrift::fast_thrift::rocket::server
