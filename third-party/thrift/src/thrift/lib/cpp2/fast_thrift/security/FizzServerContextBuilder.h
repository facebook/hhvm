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
#include <memory>
#include <optional>

#include <fizz/server/FizzServerContext.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersContext.h>

#include <thrift/lib/cpp2/fast_thrift/security/FizzServerCertConfig.h>
#include <thrift/lib/cpp2/fast_thrift/security/ThriftTlsConfig.h>

namespace apache::thrift::fast_thrift::security {

/**
 * Result of building the server-side TLS context from a cert/handshake
 * config and the thrift-extension config.
 *
 * `fizzContext` is always set. `thriftParams` is set iff the config opts
 * into a Thrift-specific TLS extension (e.g. StopTLS V1). When set, the
 * caller is responsible for constructing a fresh
 * `ThriftParametersServerExtension(thriftParams)` per connection and
 * passing it to the `AsyncFizzServer` ctor.
 */
struct BuiltFizzServerContext {
  std::shared_ptr<const fizz::server::FizzServerContext> fizzContext;
  std::shared_ptr<apache::thrift::ThriftParametersContext> thriftParams;
};

/**
 * Build an immutable FizzServerContext (and matching Thrift parameters
 * context, if needed) from a cert/handshake config and the thrift-extension
 * config.
 *
 * The returned objects are safe to share across all connections served by
 * a single server instance. Cert rotation is out of scope for v1 — callers
 * that need rotation should rebuild the context and replace it.
 *
 * Throws std::runtime_error on malformed PEM, missing file, or invalid
 * configuration (e.g. neither path nor buffer set).
 */
BuiltFizzServerContext buildFizzServerContext(
    const FizzServerCertConfig& certConfig,
    const ThriftTlsConfig& thriftConfig);

/**
 * Server-side TLS parameters: the fizz context, optional Thrift TLS
 * extension params, and the per-connection handshake timeout. Plumbed
 * into ConnectionManager / ConnectionHandler via a folly::observer so
 * cert + extension state can be hot-reloaded; the handshake timeout is
 * captured per connection by snapshotting the observer at accept time.
 *
 * Same lifetime contract as BuiltFizzServerContext: fizzContext is
 * always set; thriftParams is set iff a Thrift-specific TLS extension
 * is opted into. handshakeTimeout::nullopt = no server-side cap.
 *
 * SSLPolicy is intentionally NOT included here — the acceptance pipeline
 * shape is fixed at construction time and cannot change at runtime.
 */
struct TLSParams {
  std::shared_ptr<const fizz::server::FizzServerContext> fizzContext;
  std::shared_ptr<apache::thrift::ThriftParametersContext> thriftParams;
  std::optional<std::chrono::milliseconds> handshakeTimeout;
};

/**
 * Build immutable TLS parameters (fizz context, matching Thrift parameters
 * context if needed, and the handshake timeout) from a cert/handshake
 * config and the thrift-extension config.
 *
 * Throws on the same conditions as buildFizzServerContext.
 */
TLSParams buildTLSParams(
    const FizzServerCertConfig& certConfig,
    const ThriftTlsConfig& thriftConfig);

} // namespace apache::thrift::fast_thrift::security
