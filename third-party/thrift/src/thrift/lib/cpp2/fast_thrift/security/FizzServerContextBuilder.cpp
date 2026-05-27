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

#include <thrift/lib/cpp2/fast_thrift/security/FizzServerContextBuilder.h>

#include <fstream>
#include <sstream>
#include <stdexcept>

#include <fmt/core.h>
#include <fizz/backend/openssl/certificate/CertUtils.h>
#include <fizz/fizz-config.h>
#include <fizz/protocol/DefaultCertificateVerifier.h>
#include <fizz/record/Types.h>
#include <fizz/server/DefaultCertManager.h>
#include <fizz/server/TicketTypes.h>
#include <fizz/util/FizzUtil.h>
#include <fizz/util/Status.h>

namespace apache::thrift::fast_thrift::security {

namespace {

std::string readFile(const std::string& path) {
  std::ifstream in(path);
  if (!in) {
    throw std::runtime_error(fmt::format("Failed to open {}", path));
  }
  std::ostringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

// Resolve the client cert verifier:
//  1. customClientCertVerifier wins outright.
//  2. clientAuth == None → leave verifier unset.
//  3. !caPath.empty() → DefaultCertificateVerifier from CA file. Guarded by
//     FIZZ_CERTIFICATE_USE_OPENSSL_CERT — without the OpenSSL backend,
//     DefaultCertificateVerifier aliases to TerminatingCertificateVerifier
//     which abort()s on verify(). Throw at startup with a clear message
//     instead.
//  4. clientAuth in {Required, Optional} with no verifier source → throw at
//     startup. We refuse to spin up a server that requests client auth but
//     has no way to verify; under Optional, fizz would still accept a peer
//     cert with no verification, which silently weakens the security
//     posture the embedder asked for. Failing loud is the right call.
std::shared_ptr<const fizz::CertificateVerifier> resolveClientCertVerifier(
    const FizzServerCertConfig& certConfig) {
  if (certConfig.customClientCertVerifier) {
    return certConfig.customClientCertVerifier;
  }
  if (certConfig.clientAuth == fizz::server::ClientAuthMode::None) {
    return nullptr;
  }
  if (!certConfig.caPath.empty()) {
#if FIZZ_CERTIFICATE_USE_OPENSSL_CERT
    std::unique_ptr<fizz::DefaultCertificateVerifier> verifier;
    fizz::Error err;
    FIZZ_THROW_ON_ERROR(
        fizz::DefaultCertificateVerifier::createFromCAFiles(
            verifier,
            err,
            fizz::VerificationContext::Server,
            {certConfig.caPath}),
        err);
    return verifier;
#else
    throw std::runtime_error(
        "FizzServerCertConfig: caPath verifier requires fizz built with "
        "FIZZ_CERTIFICATE_USE_OPENSSL_CERT. Pass a customClientCertVerifier "
        "instead, or rebuild fizz with the OpenSSL backend.");
#endif
  }
  throw std::runtime_error(
      "FizzServerCertConfig: clientAuth is Required or Optional but no "
      "verifier is configured. Set caPath (PEM bundle of trust anchors) or "
      "customClientCertVerifier (a fizz::CertificateVerifier subclass), or "
      "set clientAuth=None to disable client cert verification.");
}

std::vector<fizz::NamedGroup> buildSupportedGroups() {
  std::vector<fizz::NamedGroup> groups;
#if FIZZ_HAVE_OQS && OQS_ENABLE_KEM_ml_kem_768
  groups.push_back(fizz::NamedGroup::X25519MLKEM768);
  groups.push_back(fizz::NamedGroup::X25519MLKEM512_FB);
#endif
  groups.push_back(fizz::NamedGroup::x25519);
  groups.push_back(fizz::NamedGroup::secp256r1);
  return groups;
}

std::vector<std::vector<fizz::CipherSuite>> buildSupportedCiphers(
    const fizz::server::FizzServerContext& ctx, bool enableAegis) {
  auto ciphers = ctx.getSupportedCiphers();
  if (enableAegis) {
#if FIZZ_HAVE_LIBAEGIS
    if (!ciphers.empty()) {
      ciphers.front().insert(
          ciphers.front().begin(), fizz::CipherSuite::TLS_AEGIS_128L_SHA256);
    }
#endif
  }
  return ciphers;
}

std::shared_ptr<fizz::server::TicketCipher> buildTicketCipher(
    const TicketCipherSeeds& seeds,
    std::shared_ptr<fizz::Factory> factory,
    std::shared_ptr<fizz::server::CertManager> certManager) {
  // FizzUtil::createTicketCipher's "current secret" is a single string; we
  // pass empty when currentSeeds is empty, per the contract documented on
  // TicketCipherSeeds.
  static const std::string kEmptySecret;
  const std::string& currentSecret =
      seeds.currentSeeds.empty() ? kEmptySecret : seeds.currentSeeds.front();
  folly::Optional<std::string> pskContext = seeds.pskContext.has_value()
      ? folly::Optional<std::string>{*seeds.pskContext}
      : folly::Optional<std::string>{};
  switch (seeds.kind) {
    case TicketCipherKind::IdentityOnly:
      return fizz::FizzUtil::createTicketCipher<
          fizz::server::AES128TicketIdentityOnlyCipher>(
          seeds.oldSeeds,
          currentSecret,
          seeds.newSeeds,
          seeds.validity,
          seeds.handshakeValidity,
          std::move(factory),
          std::move(certManager),
          std::move(pskContext));
    case TicketCipherKind::X509:
      return fizz::FizzUtil::createTicketCipher<
          fizz::server::AES128TicketCipher>(
          seeds.oldSeeds,
          currentSecret,
          seeds.newSeeds,
          seeds.validity,
          seeds.handshakeValidity,
          std::move(factory),
          std::move(certManager),
          std::move(pskContext));
  }
  // Unreachable; switch is exhaustive over the enum.
  return nullptr;
}

} // namespace

TLSParams buildTLSParams(
    const FizzServerCertConfig& certConfig,
    const ThriftTlsConfig& thriftConfig) {
  const bool hasPaths = !certConfig.certPath.empty();
  const bool hasBuffers = !certConfig.certPem.empty();
  if (hasPaths == hasBuffers) {
    throw std::runtime_error(
        "FizzServerCertConfig: exactly one of {certPath,keyPath} or "
        "{certPem,keyPem} must be set");
  }
  if (hasPaths && certConfig.keyPath.empty()) {
    throw std::runtime_error(
        "FizzServerCertConfig: keyPath required with certPath");
  }
  if (hasBuffers && certConfig.keyPem.empty()) {
    throw std::runtime_error(
        "FizzServerCertConfig: keyPem required with certPem");
  }

  // Resolve the verifier first so we fail loud at startup before doing any
  // cert/key loading work that might throw less informative errors.
  auto clientCertVerifier = resolveClientCertVerifier(certConfig);

  std::string certData =
      hasPaths ? readFile(certConfig.certPath) : certConfig.certPem;
  std::string keyData =
      hasPaths ? readFile(certConfig.keyPath) : certConfig.keyPem;

  std::unique_ptr<fizz::SelfCert> selfCert;
  fizz::Error err;
  if (certConfig.keyPassword.empty()) {
    FIZZ_THROW_ON_ERROR(
        fizz::openssl::CertUtils::makeSelfCert(
            selfCert, err, std::move(certData), std::move(keyData)),
        err);
  } else {
    FIZZ_THROW_ON_ERROR(
        fizz::openssl::CertUtils::makeSelfCert(
            selfCert,
            err,
            std::move(certData),
            std::move(keyData),
            certConfig.keyPassword),
        err);
  }

  auto certManager = std::make_shared<fizz::server::DefaultCertManager>();
  certManager->addCertAndSetDefault(std::move(selfCert));

  auto ctx = std::make_shared<fizz::server::FizzServerContext>();

  // Order matters for ticket cipher: setFactory must run before we read
  // getFactoryPtr() (so the cipher uses the caller's factory if supplied),
  // and setCertManager must run before we hand the cert manager to
  // createTicketCipher.
  if (certConfig.customFactory) {
    ctx->setFactory(certConfig.customFactory);
  }
  ctx->setCertManager(certManager);
  ctx->setSupportedAlpns(certConfig.alpnProtocols);
  ctx->setClientAuthMode(certConfig.clientAuth);
  ctx->setAlpnMode(certConfig.alpnMode);

  // Pin TLS 1.3 + drafts. fast_thrift's FizzHandshakeHelper rejects
  // pre-1.3 fallback by design, so leave version fallback off explicitly —
  // matches fizz's current default and future-proofs against an upstream
  // default change.
  ctx->setSupportedVersions(
      {fizz::ProtocolVersion::tls_1_3,
       fizz::ProtocolVersion::tls_1_3_28,
       fizz::ProtocolVersion::tls_1_3_26});
  ctx->setVersionFallbackEnabled(false);

  ctx->setSupportedPskModes(
      {fizz::PskKeyExchangeMode::psk_dhe_ke, fizz::PskKeyExchangeMode::psk_ke});

  ctx->setSupportedCiphers(buildSupportedCiphers(*ctx, certConfig.enableAegis));
  ctx->setSupportedGroups(buildSupportedGroups());

  if (clientCertVerifier) {
    ctx->setClientCertVerifier(std::move(clientCertVerifier));
  }

  if (certConfig.ticketSeeds) {
    auto ticketCipher = buildTicketCipher(
        *certConfig.ticketSeeds, ctx->getFactoryPtr(), certManager);
    ctx->setTicketCipher(std::move(ticketCipher));
  }

  std::shared_ptr<apache::thrift::ThriftParametersContext> thriftParams;
  if (thriftConfig.enableStopTLS) {
    thriftParams = std::make_shared<apache::thrift::ThriftParametersContext>();
    thriftParams->setUseStopTLS(true);
  }

  return TLSParams{
      std::move(ctx), std::move(thriftParams), certConfig.handshakeTimeout};
}

} // namespace apache::thrift::fast_thrift::security
