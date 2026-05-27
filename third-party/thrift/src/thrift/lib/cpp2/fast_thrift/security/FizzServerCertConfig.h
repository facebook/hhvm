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
#include <string>
#include <vector>

#include <fizz/protocol/CertificateVerifier.h>
#include <fizz/protocol/Factory.h>
#include <fizz/server/FizzServerContext.h>
#include <thrift/lib/cpp2/fast_thrift/security/SSLPolicy.h>

namespace apache::thrift::fast_thrift::security {

/**
 * Selects which fizz ticket cipher to instantiate when ticket-based session
 * resumption is configured.
 *   - IdentityOnly: AES128TicketIdentityOnlyCipher. Tickets store only the
 *     string returned by AsyncTransportCertificate::getIdentity() (~100 B).
 *     Lower memory + bandwidth per resumed session.
 *   - X509: AES128TicketCipher. Tickets carry the full peer X509 chain
 *     (~1-2 KB). Use when the application needs the chain on resume rather
 *     than just the identity string.
 *
 * IdentityOnly pairs with verifiers that drop X509 after verification: a
 * verifier that retains X509 state after verify() but is then handed an
 * IdentityOnly ticket on resume cannot reproduce that state, so callers
 * should configure the verifier to drop X509 when this kind is selected.
 */
enum class TicketCipherKind { IdentityOnly, X509 };

/**
 * Ticket-cipher seeds + policy. Caller is responsible for parsing whatever
 * on-disk format their service uses into these vectors of secret bytes.
 *
 * `currentSeeds.front()` is the active encryption seed. `oldSeeds` and
 * `newSeeds` accept tickets issued under previously- or about-to-be-active
 * seeds, supporting graceful rotation. Empty currentSeeds is treated as a
 * single empty active seed.
 */
struct TicketCipherSeeds {
  std::vector<std::string> oldSeeds;
  std::vector<std::string> currentSeeds;
  std::vector<std::string> newSeeds;
  std::chrono::seconds validity{std::chrono::hours{24}};
  std::chrono::seconds handshakeValidity{std::chrono::hours{1}};
  std::optional<std::string> pskContext;
  TicketCipherKind kind{TicketCipherKind::IdentityOnly};
};

/**
 * Cert/key + fizz handshake mechanics for a fast_thrift TLS-enabled server.
 *
 * Exactly one of certPath/keyPath OR certPem/keyPem must be set.
 *
 * Thrift-extension knobs negotiated during the handshake (StopTLS, params
 * negotiation, etc.) live in security::ThriftTlsConfig — kept separate so
 * the cert-rotation surface can evolve independently.
 *
 * Two extension slots let embedders layer in extra fizz behavior without
 * fast_thrift depending on the embedder's types:
 *   - customClientCertVerifier: any subclass of fizz::CertificateVerifier
 *     (e.g. one that performs revocation checks or extra identity binding
 *     on top of a base verifier).
 *   - customFactory: any subclass of fizz::Factory (e.g. one that overrides
 *     makePeerCert to attach extra metadata or makeEncryptedRead/Write
 *     RecordLayer to support an alternate close-notify protocol).
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

  // PEM bundle of trust anchors used to verify the client cert chain.
  // Required when clientAuth != None and customClientCertVerifier is unset;
  // ignored when customClientCertVerifier is set. Uses fizz's built-in
  // DefaultCertificateVerifier (OpenSSL-backed); the codepath is guarded by
  // FIZZ_CERTIFICATE_USE_OPENSSL_CERT — on a fizz build without the OpenSSL
  // backend, setting caPath throws at startup with a clear message rather
  // than abort()ing on the first connection.
  std::string caPath;

  // Optional caller-supplied client cert verifier. When set, takes
  // precedence over caPath. Lets embedders inject a CertificateVerifier
  // subclass that wraps the default verifier with extra checks (revocation,
  // identity binding, etc.) without fast_thrift depending on those types.
  std::shared_ptr<const fizz::CertificateVerifier> customClientCertVerifier;

  // Optional caller-supplied fizz factory. When set, wired via setFactory()
  // before any other context configuration. Lets embedders inject a Factory
  // subclass that customises peer-cert construction or record-layer behavior
  // (e.g. an alternate close-notify protocol) without fast_thrift depending
  // on those types.
  std::shared_ptr<fizz::Factory> customFactory;

  // ALPN protocols advertised to clients, in preference order.
  // Defaults to "rs" (Rocket) to match the rest of the fast_thrift stack.
  std::vector<std::string> alpnProtocols{"rs"};

  // Mutual TLS mode. Default None — mTLS is opt-in. Callers wanting mTLS
  // set this to Required (or Optional) and also supply caPath OR
  // customClientCertVerifier; without one of those, buildTLSParams
  // throws at startup rather than silently accepting any peer cert.
  fizz::server::ClientAuthMode clientAuth{fizz::server::ClientAuthMode::None};

  // ALPN enforcement when client offers no overlap. Default Optional is
  // stricter than fizz's natural AllowMismatch default; catches accidental
  // wire-protocol mismatches early. AllowMismatch silently accepts a peer
  // that offers no matching ALPN; Required rejects a peer that omits ALPN.
  fizz::server::AlpnMode alpnMode{fizz::server::AlpnMode::Optional};

  // When true (and FIZZ_HAVE_LIBAEGIS), prepend TLS_AEGIS_128L_SHA256 to
  // the cipher list as the highest-preference cipher.
  bool enableAegis{false};

  // Optional ticket cipher (session resumption). When unset, every new
  // handshake is a full 1-RTT; reconnect-heavy services will spend
  // significant CPU on full handshakes unless this is wired. Default kind
  // is IdentityOnly (smaller tickets); set to X509 for callers needing the
  // full chain on resume.
  std::optional<TicketCipherSeeds> ticketSeeds;

  // Total budget for the accept→handshake-complete journey (covers the
  // optional peek phase plus the fizz handshake). std::nullopt = unbounded
  // (no server-side cap; rely on client-side timeouts).
  //
  // DO NOT pass std::optional<ms>(0) to mean unbounded — use std::nullopt.
  // 0 is an immediate deadline and would expire every connection at start;
  // the HandshakeTimeout constructor CHECKs against this.
  std::optional<std::chrono::milliseconds> handshakeTimeout{
      std::chrono::seconds{60}};

  // Per-connection TLS gating. Use PERMITTED to accept both plaintext and
  // TLS on the same listening socket; the server peeks the first 9 bytes
  // and routes accordingly. DISABLED makes setSSLConfig() a no-op for the
  // accept path.
  SSLPolicy sslPolicy{SSLPolicy::REQUIRED};
};

} // namespace apache::thrift::fast_thrift::security
