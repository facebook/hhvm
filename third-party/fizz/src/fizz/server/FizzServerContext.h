/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/Certificate.h>
#include <fizz/protocol/Factory.h>
#include <fizz/protocol/clock/SystemClock.h>
#include <fizz/protocol/ech/Decrypter.h>
#include <fizz/record/Types.h>
#include <fizz/server/CertManager.h>
#include <fizz/server/CookieCipher.h>
#include <fizz/server/Negotiator.h>
#include <fizz/server/ReplayCache.h>
#include <fizz/server/TicketCipher.h>

namespace fizz {
namespace server {

/**
 * The amount of client clock skew to tolerate when accepting early data. Early
 * data will only be accepted with clock skew between before and after.
 *
 * For example:
 * ClockSkewTolerance{
 *   std::chrono::milliseconds(-2000),
 *   std::chrono::milliseconds(1000)
 * };
 * Would accept client clocks between 2 seconds slow, and 1 second fast.
 */
struct ClockSkewTolerance {
  std::chrono::milliseconds before;
  std::chrono::milliseconds after;
};

/**
 * Different modes of operation for client authentication. The names are self-
 * explanatory.
 */
enum class ClientAuthMode { None, Optional, Required };

// ALPN enforcement types
enum class AlpnMode { AllowMismatch, Optional, Required };

class FizzServerContext {
 public:
  FizzServerContext();

  explicit FizzServerContext(std::shared_ptr<Factory> factory)
      : factory_(std::move(factory)) {}

  virtual ~FizzServerContext() = default;

  /**
   * Set the supported protocol versions, in preference order.
   */
  void setSupportedVersions(std::vector<ProtocolVersion> versions) {
    supportedVersions_ = std::move(versions);
  }
  const auto& getSupportedVersions() const {
    return supportedVersions_;
  }

  /**
   * Set the supported ciphers, in preference order.
   */
  void setSupportedCiphers(std::vector<std::vector<CipherSuite>> ciphers) {
    supportedCiphers_ = std::move(ciphers);
  }
  const auto& getSupportedCiphers() const {
    return supportedCiphers_;
  }

  /**
   * Set the supported signature schemes, in preference order.
   */
  void setSupportedSigSchemes(std::vector<SignatureScheme> schemes) {
    supportedSigSchemes_ = std::move(schemes);
  }
  const auto& getSupportedSigSchemes() const {
    return supportedSigSchemes_;
  }

  /**
   * Set the supported named groups, in preference order.
   */
  void setSupportedGroups(std::vector<NamedGroup> groups) {
    supportedGroups_ = std::move(groups);
  }
  const auto& getSupportedGroups() const {
    return supportedGroups_;
  }

  /**
   * Set the supported psk modes, in preference order.
   */
  void setSupportedPskModes(std::vector<PskKeyExchangeMode> modes) {
    supportedPskModes_ = std::move(modes);
  }
  const auto& getSupportedPskModes() const {
    return supportedPskModes_;
  }

  /**
   * Set whether to request client authentication.
   */
  void setClientAuthMode(ClientAuthMode authmode) {
    clientAuthMode_ = authmode;
  }

  ClientAuthMode getClientAuthMode() const {
    return clientAuthMode_;
  }

  /**
   * Set whether to attempt fallback to another implementation if no supported
   * version match is found. If enabled connect callbacks should implement
   * fizzHandshakeAttemptFallback.
   */
  void setVersionFallbackEnabled(bool enabled) {
    versionFallbackEnabled_ = enabled;
  }
  bool getVersionFallbackEnabled() const {
    return versionFallbackEnabled_;
  }

  /**
   * Sets the supported ALPN supported protocols, in preference order.
   */
  void setSupportedAlpns(std::vector<std::string> protocols) {
    supportedAlpns_ = std::move(protocols);
  }

  const std::vector<std::string>& getSupportedAlpns() const {
    return supportedAlpns_;
  }

  /**
   * Negotaitate a ALPN protocol given a client's offer. zeroRttAlpn will be set
   * to the protocol used for early data if sent by the client.
   */
  folly::Optional<std::string> negotiateAlpn(
      const std::vector<std::string>& clientProtocols,
      const folly::Optional<std::string>& zeroRttAlpn) const {
    // If we support the zero rtt protocol we select it.
    if (zeroRttAlpn &&
        std::find(
            supportedAlpns_.begin(), supportedAlpns_.end(), *zeroRttAlpn) !=
            supportedAlpns_.end()) {
      return zeroRttAlpn;
    }
    return negotiate(supportedAlpns_, clientProtocols);
  }

  /**
   * Sets the ticket cipher to use. Resumption will be disabled if not set.
   */
  void setTicketCipher(std::shared_ptr<TicketCipher> ticketCipher) {
    ticketCipher_ = std::move(ticketCipher);
  }
  const TicketCipher* getTicketCipher() const {
    return ticketCipher_.get();
  }

  /**
   * Sets the cookie cipher to use. Stateless client retries will be rejected
   * if not set.
   */
  void setCookieCipher(std::shared_ptr<CookieCipher> cookieCipher) {
    cookieCipher_ = std::move(cookieCipher);
  }
  const CookieCipher* getCookieCipher() const {
    return cookieCipher_.get();
  }

  /**
   * Sets the CertManager to use.
   */
  void setCertManager(std::shared_ptr<CertManager> manager) {
    certManager_ = std::move(manager);
  }

  /**
   * Sets the certificate verifier to use for client authentication
   */
  void setClientCertVerifier(
      std::shared_ptr<const CertificateVerifier> verifier) {
    clientCertVerifier_ = std::move(verifier);
  }

  const std::shared_ptr<const CertificateVerifier>& getClientCertVerifier()
      const {
    return clientCertVerifier_;
  }

  /**
   * Chooses a certificate based on given sni and peer signature schemes +
   * extensions.
   */
  folly::Optional<std::pair<std::shared_ptr<SelfCert>, SignatureScheme>>
  getCert(
      const folly::Optional<std::string>& sni,
      const std::vector<SignatureScheme>& peerSigSchemes,
      const std::vector<Extension>& peerExtensions) const {
    auto result = certManager_->getCert(
        sni, supportedSigSchemes_, peerSigSchemes, peerExtensions);
    if (result) {
      return std::make_pair(result->cert, result->scheme);
    } else {
      return folly::none;
    }
  }

  /**
   * Return a certificate that matches identity. Will return nullptr if a
   * matching certificate is not found.
   */
  std::shared_ptr<SelfCert> getCert(const std::string& identity) const {
    return certManager_->getCert(identity);
  }

  /**
   * Sets the early data settings.
   */
  void setEarlyDataSettings(
      bool acceptEarlyData,
      ClockSkewTolerance clockSkewTolerance,
      const std::shared_ptr<ReplayCache>& replayCache) {
    acceptEarlyData_ = acceptEarlyData;
    clockSkewTolerance_ = clockSkewTolerance;
    replayCache_ = replayCache;
  }

  bool getAcceptEarlyData(ProtocolVersion version) const {
    if (earlyDataFbOnly_ &&
        (version != ProtocolVersion::tls_1_3_23_fb &&
         version != ProtocolVersion::tls_1_3_26_fb)) {
      return false;
    }
    return acceptEarlyData_;
  }
  ClockSkewTolerance getClockSkewTolerance() const {
    return clockSkewTolerance_;
  }
  ReplayCache* getReplayCache() const {
    return replayCache_.get();
  }

  void setEarlyDataFbOnly(bool fbOnly) {
    earlyDataFbOnly_ = fbOnly;
  }

  /**
   * Sets the max_early_data_size to advertise when sending early data
   * compatible tickets. This limit is currently not enforced when accepting
   * early data.
   */
  void setMaxEarlyDataSize(uint32_t maxEarlyDataSize) {
    maxEarlyDataSize_ = maxEarlyDataSize;
  }
  uint32_t getMaxEarlyDataSize() const {
    return maxEarlyDataSize_;
  }

  /**
   * Set the factory to use. Should generally only be changed for testing.
   */
  void setFactory(std::shared_ptr<Factory> factory) {
    factory_ = std::move(factory);
  }
  const Factory* getFactory() const {
    return factory_.get();
  }
  std::shared_ptr<Factory> getFactoryPtr() const {
    return factory_;
  }

  /**
   * Fizz will automatically send NewSessionTicket before reporting handshake
   * success if this is true.
   * Application can use the writeNewSessionTicket API alternatively if this is
   * set to false.
   * Default is true.
   */
  void setSendNewSessionTicket(bool sendNewSessionTicket) {
    sendNewSessionTicket_ = sendNewSessionTicket;
  }
  bool getSendNewSessionTicket() const {
    return sendNewSessionTicket_;
  }

  /**
   * Set supported cert compression algorithms. Note: It is expected that any
   * certificate used has been initialized with compressors corresponding to the
   * algorithms set here.
   */
  void setSupportedCompressionAlgorithms(
      std::vector<CertificateCompressionAlgorithm> algos) {
    supportedCompressionAlgos_ = algos;
  }
  const auto& getSupportedCompressionAlgorithms() const {
    return supportedCompressionAlgos_;
  }

  /**
   * Whether to omit the early record layer when sending early data. This will
   * also omit the EndOfEarlyData message.
   * Default is false, and using this requires a custom record layer.
   */
  void setOmitEarlyRecordLayer(bool enabled) {
    omitEarlyRecordLayer_ = enabled;
  }
  bool getOmitEarlyRecordLayer() const {
    return omitEarlyRecordLayer_;
  }

  void setClock(std::shared_ptr<Clock> clock) {
    clock_ = clock;
  }
  const Clock& getClock() const {
    return *clock_;
  }

  /**
   * Set ALPN enforcement mode.
   * When set to Optional or Required while ALPN fails,
   * no_application_protocol alert is sent.
   */
  void setAlpnMode(AlpnMode mode) {
    alpnMode_ = mode;
  }

  AlpnMode getAlpnMode() const {
    return alpnMode_;
  }

  /**
   * Return an ECH decrypter that is able to decrypt an encrypted client hello.
   */
  void setECHDecrypter(std::shared_ptr<ech::Decrypter> decrypter) {
    decrypter_ = decrypter;
  }

  std::shared_ptr<ech::Decrypter> getECHDecrypter() const {
    return decrypter_;
  }

 private:
  std::shared_ptr<Factory> factory_;

  std::shared_ptr<TicketCipher> ticketCipher_;
  std::shared_ptr<CookieCipher> cookieCipher_;

  std::shared_ptr<CertManager> certManager_;
  std::shared_ptr<const CertificateVerifier> clientCertVerifier_;

  std::vector<ProtocolVersion> supportedVersions_ = {ProtocolVersion::tls_1_3};
  std::vector<std::vector<CipherSuite>> supportedCiphers_ = {
      {
          CipherSuite::TLS_AES_128_GCM_SHA256,
#if FOLLY_OPENSSL_HAS_CHACHA
          CipherSuite::TLS_CHACHA20_POLY1305_SHA256,
#endif // FOLLY_OPENSSL_HAS_CHACHA
      },
      {CipherSuite::TLS_AES_256_GCM_SHA384},
  };
  std::vector<SignatureScheme> supportedSigSchemes_ = {
      SignatureScheme::ecdsa_secp256r1_sha256,
      SignatureScheme::ecdsa_secp384r1_sha384,
      SignatureScheme::ecdsa_secp521r1_sha512,
      SignatureScheme::rsa_pss_sha256};
  std::vector<NamedGroup> supportedGroups_ = {
      NamedGroup::x25519,
      NamedGroup::secp256r1};
  std::vector<PskKeyExchangeMode> supportedPskModes_ = {
      PskKeyExchangeMode::psk_dhe_ke,
      PskKeyExchangeMode::psk_ke};
  std::vector<std::string> supportedAlpns_;

  bool versionFallbackEnabled_{false};
  ClientAuthMode clientAuthMode_{ClientAuthMode::None};

  bool acceptEarlyData_{false};
  uint32_t maxEarlyDataSize_{std::numeric_limits<uint32_t>::max()};
  ClockSkewTolerance clockSkewTolerance_;
  std::shared_ptr<ReplayCache> replayCache_;
  std::shared_ptr<Clock> clock_ = std::make_shared<SystemClock>();

  std::vector<CertificateCompressionAlgorithm> supportedCompressionAlgos_;

  bool earlyDataFbOnly_{false};

  bool sendNewSessionTicket_{true};

  bool omitEarlyRecordLayer_{false};

  AlpnMode alpnMode_{AlpnMode::AllowMismatch};

  std::shared_ptr<ech::Decrypter> decrypter_;
};
} // namespace server
} // namespace fizz
