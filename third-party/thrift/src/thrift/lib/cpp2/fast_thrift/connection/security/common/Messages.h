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

#include <memory>

#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncTransport.h>
#include <thrift/lib/cpp2/fast_thrift/security/FizzServerContextBuilder.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersServerExtension.h>

namespace apache::thrift::fast_thrift::connection::security {

// The TLS pipeline is request/response shaped: a TLSRequestMessage is driven
// out along the work (write) path, and a TLSResponseMessage returns along the
// read path once security is resolved.

// Outbound (write/work path). One per accepted socket, submitted by the tail
// adapter. Each stage upgrades `transport` in-place (AsyncSocket →
// AsyncFizzServer after handshake → plaintext AsyncTransport after StopTLS)
// and forwards the same type.
//
// `tlsParams` is stamped by TLSConfigHandler, snapshotted from a
// folly::Observer so stages see the current params for this accept without
// each holding the Observer themselves; it is read by the classifier and the
// handshake.
//
// `extension` is null until the fizz handshake completes, then read by
// StopTLSV1Handler to decide whether to downgrade.
struct TLSRequestMessage {
  folly::AsyncTransport::UniquePtr transport;
  folly::SocketAddress clientAddr;
  std::shared_ptr<const apache::thrift::fast_thrift::security::TLSParams>
      tlsParams;
  std::shared_ptr<apache::thrift::ThriftParametersServerExtension> extension;
};

// Inbound (read/return path). TLSFinalizer collapses a resolved
// TLSRequestMessage down to this at the head; the stages pass it through
// untouched to the tail adapter, which hands the resolved transport off.
// Carries only what handoff needs — no negotiation state.
struct TLSResponseMessage {
  folly::AsyncTransport::UniquePtr transport;
  folly::SocketAddress clientAddr;
};

} // namespace apache::thrift::fast_thrift::connection::security
