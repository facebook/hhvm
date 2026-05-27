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
 * Server-side TLS parameters: the fizz context, optional thrift TLS
 * extension params, and the per-connection handshake timeout. Produced by
 * buildTLSParams() and threaded into ConnectionManager / ConnectionHandler
 * to drive every accepted connection's TLS handshake.
 *
 * `fizzContext` is always set. `thriftParams` is set iff the config opts
 * into a Thrift-specific TLS extension (e.g. StopTLS V1). When set, the
 * caller is responsible for constructing a fresh
 * `ThriftParametersServerExtension(thriftParams)` per connection and
 * passing it to the `AsyncFizzServer` ctor.
 *
 * SSLPolicy is intentionally NOT included here — the acceptance pipeline
 * shape is fixed at construction time and cannot change at runtime.
 */
struct TLSParams {
  std::shared_ptr<const fizz::server::FizzServerContext> fizzContext;
  std::shared_ptr<apache::thrift::ThriftParametersContext> thriftParams;
  // nullopt = no server-side timeout cap. See FizzServerCertConfig
  // for the contract; mirrors that field.
  std::optional<std::chrono::milliseconds> handshakeTimeout;
};

/**
 * Build immutable TLS parameters (fizz context, matching Thrift parameters
 * context if needed, and the handshake timeout) from a cert/handshake
 * config and the thrift-extension config.
 *
 * The returned objects are safe to share across all connections served by
 * a single server instance. Hot-reload is supported by calling
 * buildTLSParams() again with a fresh FizzServerCertConfig and handing the
 * result to ConnectionManager::setTLSParams; in-flight handshakes keep the
 * old params alive via their captured shared_ptr.
 *
 * Throws std::runtime_error on malformed PEM, missing file, or invalid
 * configuration (e.g. neither path nor buffer set).
 */
TLSParams buildTLSParams(
    const FizzServerCertConfig& certConfig,
    const ThriftTlsConfig& thriftConfig);

} // namespace apache::thrift::fast_thrift::security
