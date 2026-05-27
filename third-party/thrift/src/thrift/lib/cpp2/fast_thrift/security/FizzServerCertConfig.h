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
#include <optional>
#include <string>
#include <vector>

#include <fizz/server/FizzServerContext.h>
#include <thrift/lib/cpp2/fast_thrift/security/SSLPolicy.h>

namespace apache::thrift::fast_thrift::security {

/**
 * Cert/key + fizz handshake mechanics for a fast_thrift TLS-enabled server.
 * Mirrors the cert-related fields of wangle::SSLContextConfig:
 *   - cert/key may be supplied as PEM file paths OR as in-memory PEM strings
 *   - clientAuth defaults to Required (matches wangle default)
 *
 * Exactly one of certPath/keyPath OR certPem/keyPem must be set.
 *
 * Thrift-extension knobs negotiated during the handshake (StopTLS, params
 * negotiation, etc.) live in security::ThriftTlsConfig — kept separate so
 * the cert-rotation surface can evolve independently.
 */
struct FizzServerCertConfig {
  // PEM file path mode.
  std::string certPath;
  std::string keyPath;

  // In-memory PEM mode. Used when certPath is empty.
  std::string certPem;
  std::string keyPem;

  // Optional password for an encrypted private key.
  std::string keyPassword;

  // ALPN protocols advertised to clients, in preference order.
  // Defaults to "rs" (Rocket) to match the rest of the fast_thrift stack.
  std::vector<std::string> alpnProtocols{"rs"};

  // Mutual TLS mode. Required = client must present a valid cert
  // (matches wangle::SSLContextConfig default).
  fizz::server::ClientAuthMode clientAuth{
      fizz::server::ClientAuthMode::Required};

  // Total budget for the accept→handshake-complete journey (covers the
  // optional peek phase plus the fizz handshake). std::nullopt = unbounded
  // (no server-side cap; rely on client-side timeouts).
  //
  // DO NOT pass std::optional<ms>(0) to mean unbounded — use std::nullopt.
  // 0 is an immediate deadline and would expire every connection at start;
  // the HandshakeTimeout constructor CHECKs against this.
  std::optional<std::chrono::milliseconds> handshakeTimeout{
      std::chrono::seconds{30}};

  // Per-connection TLS gating. Default REQUIRED matches the classic
  // ThriftServer default. Use PERMITTED to accept both plaintext and TLS
  // on the same listening socket; the server peeks the first 9 bytes and
  // routes accordingly. DISABLED makes setSSLConfig() a no-op for the
  // accept path.
  SSLPolicy sslPolicy{SSLPolicy::REQUIRED};
};

} // namespace apache::thrift::fast_thrift::security
