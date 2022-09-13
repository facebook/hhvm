/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/client/ClientExtensions.h>
#include <fizz/client/FizzClientContext.h>
#include <fizz/protocol/Certificate.h>
#include <fizz/protocol/KeyScheduler.h>
#include <fizz/protocol/Types.h>
#include <fizz/protocol/ech/Encryption.h>
#include <fizz/record/RecordLayer.h>

namespace fizz {
namespace client {

enum class StateEnum {
  Uninitialized,
  ExpectingServerHello,
  ExpectingEncryptedExtensions,
  ExpectingCertificate,
  ExpectingCertificateVerify,
  ExpectingFinished,
  Established,
  ExpectingCloseNotify,
  Closed,
  Error,
  NUM_STATES
};

/**
 * States for client authentication:
 *  - NotRequested: server did not request client auth
 *  - Sent: server requested client auth and a matching certificate was
 *    found
 *  - RequestedNoMatch: server requested client auth but no matching
 *    certificate was found
 *  - Stored: client used PSK auth, PSK has an associated client certificate
 */
enum class ClientAuthType { NotRequested, Sent, RequestedNoMatch, Stored };

/**
 * Connection parameters for data sent as early data.
 *
 * If early data is rejected, the negotiated parameters may not be the same.
 */
struct EarlyDataParams {
  ProtocolVersion version;
  CipherSuite cipher;
  std::shared_ptr<const Cert> serverCert;
  std::shared_ptr<const Cert> clientCert;
  folly::Optional<std::string> alpn;
  Buf earlyExporterSecret;
};

enum class ECHStatus { Requested, Rejected, Accepted };

struct ECHState {
  // Status of ECH, initially Requested.
  ECHStatus status{ECHStatus::Requested};
  // Encoded encrypted (inner) client hello.
  Buf encodedECH;
  // Actual SNI sent in ECH. Could be none.
  folly::Optional<std::string> sni;
  // ECH parameters selected for use.
  ech::SupportedECHConfig supportedConfig;
  // HPKE context saved for use with HRR, if needed.
  mutable hpke::SetupResult hpkeSetup;
  // ECH random (for HRR, if needed).
  Random random;
  // GREASE PSK (sent if inner chlo has psk)
  folly::Optional<ClientPresharedKey> greasePsk;
  // ECH handshake context (initialized during HRR)
  mutable std::unique_ptr<HandshakeContext> handshakeContext;
  // In the case of rejection, the server may send a list of supported
  // configs, which we store here.
  folly::Optional<std::vector<ech::ECHConfig>> retryConfigs;
};

class State {
 public:
  /**
   * Current state of the connection.
   */
  StateEnum state() const {
    return state_;
  }

  /**
   * The FizzClientContext used on this connection.
   */
  const FizzClientContext* context() const {
    return context_.get();
  }

  /**
   * The certificate used by the server for authentication.
   */
  std::shared_ptr<const Cert> serverCert() const {
    return serverCert_;
  }

  /**
   * The certificate used by the client for authentication.
   */
  std::shared_ptr<const Cert> clientCert() const {
    return clientCert_;
  }

  /**
   * Whether or not the server requested client authentication and
   * whether a cert matched if authentication was requested.
   */
  folly::Optional<ClientAuthType> clientAuthRequested() const {
    return clientAuthRequested_;
  }

  /**
   * Signature scheme selected for client authentication
   */
  folly::Optional<SignatureScheme> clientAuthSigScheme() const {
    return clientAuthSigScheme_;
  }

  /**
   * Protocol version negotiated on this connection.
   */
  folly::Optional<ProtocolVersion> version() const {
    return version_;
  }

  /**
   * Cipher suite nogotiated on this connection.
   */
  folly::Optional<CipherSuite> cipher() const {
    return cipher_;
  }

  /**
   * The named group used if (EC)DH key exchange was used.
   */
  folly::Optional<NamedGroup> group() const {
    return group_;
  }

  /**
   * The signature scheme used if server authentication was used.
   */
  folly::Optional<SignatureScheme> sigScheme() const {
    return sigScheme_;
  }

  /**
   * Psk handshake flow used on this connection (psk not sent, psk rejected, psk
   * accepted, etc.).
   */
  folly::Optional<PskType> pskType() const {
    return pskType_;
  }

  /**
   * Psk key exchange mode used on this connection, if a psk was accepted.
   */
  folly::Optional<PskKeyExchangeMode> pskMode() const {
    return pskMode_;
  }

  /**
   * Key exchange flow used on this connection (none, normal, or hello retry).
   */
  folly::Optional<KeyExchangeType> keyExchangeType() const {
    return keyExchangeType_;
  }

  /**
   * Whether early data is used on this connection.
   */
  folly::Optional<EarlyDataType> earlyDataType() const {
    return earlyDataType_;
  }

  /**
   * Connection parameters for data sent as early data.
   */
  const folly::Optional<EarlyDataParams>& earlyDataParams() const {
    return earlyDataParams_;
  }

  /**
   * Application protocol negotiated on this connection.
   */
  const folly::Optional<std::string>& alpn() const {
    return alpn_;
  }

  /**
   * Server name that was sent in the SNI extensions.
   */
  const folly::Optional<std::string>& sni() const {
    return sni_;
  }

  /**
   * Compression algorithm used for server certificates (if any).
   */
  const folly::Optional<CertificateCompressionAlgorithm>& serverCertCompAlgo()
      const {
    return serverCertCompAlgo_;
  }

  /**
   * Certificate verifier to be used to verify server certificates on this
   * connection.
   */
  const CertificateVerifier* verifier() const {
    return verifier_.get();
  }

  /**
   * Random sent by the client.
   */
  const folly::Optional<Random>& clientRandom() const {
    return clientRandom_;
  }

  /**
   * Legacy session ID sent by the client. Will be empty unless compatibility
   * mode is in use.
   */
  const Buf& legacySessionId() const {
    return *legacySessionId_;
  }

  /**
   * Whether we sent a CCS due to compatibility mode.
   */
  bool sentCCS() const {
    return sentCCS_;
  }

  /**
   * Key scheduler used on this connection.
   *
   * The state of the key scheduler may change outside of state mutators.
   * Should not be used outside of the state machine.
   */
  KeyScheduler* keyScheduler() const {
    return keyScheduler_.get();
  }

  /**
   * Current read record layer. May be null.
   *
   * The state of the read record layer may change outside of state mutators.
   * Should not be used outside of the state machine.
   */
  ReadRecordLayer* readRecordLayer() const {
    return readRecordLayer_.get();
  }

  /**
   * Current write record layer. May be null.
   *
   * The state of the write record layer may change outside of state mutators.
   * Should not be used outside of the state machine.
   */
  const WriteRecordLayer* writeRecordLayer() const {
    return writeRecordLayer_.get();
  }

  /**
   * Record layer for writing early data. May be null.
   *
   * Should not be used outside of the state machine.
   */
  const WriteRecordLayer* earlyWriteRecordLayer() const {
    return earlyWriteRecordLayer_.get();
  }

  /**
   * Contains the client hello that was sent on the wire.
   *
   * Should not be used outside of the state machine.
   */
  const Buf& encodedClientHello() const {
    return *encodedClientHello_;
  }

  /**
   * Contains the ECH state (if ECH has been enabled).
   */
  const folly::Optional<ECHState>& echState() const {
    return echState_;
  }

  /**
   * Contains the extensions requested in the initial ClientHello. Used to
   * later verify extensions sent by the server.
   *
   * Should not be used outside of the state machine.
   */
  const folly::Optional<std::vector<ExtensionType>>& requestedExtensions()
      const {
    return requestedExtensions_;
  }

  /**
   * Client handshake secret.
   *
   * Should not be used outside of the state machine.
   */
  const Buf& clientHandshakeSecret() const {
    return *clientHandshakeSecret_;
  }

  /**
   * Server handshake secret.
   *
   * Should not be used outside of the state machine.
   */
  const Buf& serverHandshakeSecret() const {
    return *serverHandshakeSecret_;
  }

  /**
   * Resumption secret.
   *
   * Should not be used outside of the state machine.
   */
  const Buf& resumptionSecret() const {
    return *resumptionSecret_;
  }

  /**
   * Server certificate chain that has not yet been verified.
   *
   * Should not be used outside of the state machine.
   */
  const std::vector<std::shared_ptr<const PeerCert>>& unverifiedCertChain()
      const {
    return *unverifiedCertChain_;
  }

  /**
   * The certificate selected for client authentication (prior to being sent).
   *
   * Should not be used outside of the state machine.
   */
  std::shared_ptr<const SelfCert> selectedClientCert() const {
    return selectedClientCert_;
  }

  /**
   * Get the exporter master secret - needed for EKM
   */
  const folly::Optional<Buf>& exporterMasterSecret() const {
    return exporterMasterSecret_;
  }

  /**
   * CachedPsk that we are attempting to use.
   *
   * Should not be used outside of the state machine.
   */
  const folly::Optional<CachedPsk>& attemptedPsk() const {
    return attemptedPsk_;
  }

  /*
   * Get the extensions interface to add extensions to ClientHello and check the
   * extensions negotiated by server.
   */
  ClientExtensions* extensions() const {
    return extensions_.get();
  }

  /**
   * Gets the time point corresponding to when the full handshake that
   * authenticated this connection occurred (i.e. the original full handshake
   * for resumed connections).
   */
  const folly::Optional<std::chrono::system_clock::time_point>& handshakeTime()
      const {
    return handshakeTime_;
  }

  auto& state() {
    return state_;
  }

  auto& context() {
    return context_;
  }

  auto& verifier() {
    return verifier_;
  }

  auto& keyScheduler() {
    return keyScheduler_;
  }

  auto& readRecordLayer() {
    return readRecordLayer_;
  }

  auto& writeRecordLayer() {
    return writeRecordLayer_;
  }

  auto& earlyWriteRecordLayer() {
    return earlyWriteRecordLayer_;
  }

  auto& handshakeContext() const {
    return handshakeContext_;
  }

  auto& serverCert() {
    return serverCert_;
  }

  auto& clientCert() {
    return clientCert_;
  }

  auto& selectedClientCert() {
    return selectedClientCert_;
  }

  auto& clientAuthRequested() {
    return clientAuthRequested_;
  }

  auto& clientAuthSigScheme() {
    return clientAuthSigScheme_;
  }

  auto& version() {
    return version_;
  }

  auto& cipher() {
    return cipher_;
  }

  auto& group() {
    return group_;
  }

  auto& sigScheme() {
    return sigScheme_;
  }

  auto& pskType() {
    return pskType_;
  }

  auto& pskMode() {
    return pskMode_;
  }

  auto& keyExchangeType() {
    return keyExchangeType_;
  }

  auto& earlyDataType() {
    return earlyDataType_;
  }

  auto& earlyDataParams() {
    return earlyDataParams_;
  }

  auto& alpn() {
    return alpn_;
  }

  auto& sni() {
    return sni_;
  }

  auto& serverCertCompAlgo() {
    return serverCertCompAlgo_;
  }

  auto& clientRandom() {
    return clientRandom_;
  }

  auto& legacySessionId() {
    return legacySessionId_;
  }

  auto& sentCCS() {
    return sentCCS_;
  }

  auto& encodedClientHello() {
    return encodedClientHello_;
  }

  auto& echState() {
    return echState_;
  }

  auto& keyExchangers() const {
    return keyExchangers_;
  }

  auto& requestedExtensions() {
    return requestedExtensions_;
  }

  auto& clientHandshakeSecret() {
    return clientHandshakeSecret_;
  }

  auto& serverHandshakeSecret() {
    return serverHandshakeSecret_;
  }

  auto& resumptionSecret() {
    return resumptionSecret_;
  }

  auto& unverifiedCertChain() {
    return unverifiedCertChain_;
  }

  auto& attemptedPsk() {
    return attemptedPsk_;
  }

  auto& exporterMasterSecret() {
    return exporterMasterSecret_;
  }

  auto& extensions() {
    return extensions_;
  }

  auto& handshakeTime() {
    return handshakeTime_;
  }

 private:
  StateEnum state_{StateEnum::Uninitialized};

  std::shared_ptr<const FizzClientContext> context_;

  std::shared_ptr<const CertificateVerifier> verifier_;

  std::unique_ptr<KeyScheduler> keyScheduler_;

  std::unique_ptr<ReadRecordLayer> readRecordLayer_;
  std::unique_ptr<WriteRecordLayer> writeRecordLayer_;
  std::unique_ptr<EncryptedWriteRecordLayer> earlyWriteRecordLayer_;

  mutable std::unique_ptr<HandshakeContext> handshakeContext_;

  std::shared_ptr<const Cert> serverCert_;
  std::shared_ptr<const Cert> clientCert_;
  std::shared_ptr<const SelfCert> selectedClientCert_;

  folly::Optional<ClientAuthType> clientAuthRequested_;
  folly::Optional<SignatureScheme> clientAuthSigScheme_;

  folly::Optional<ProtocolVersion> version_;
  folly::Optional<CipherSuite> cipher_;
  folly::Optional<NamedGroup> group_;
  folly::Optional<SignatureScheme> sigScheme_;
  folly::Optional<PskType> pskType_;
  folly::Optional<PskKeyExchangeMode> pskMode_;
  folly::Optional<KeyExchangeType> keyExchangeType_;
  folly::Optional<EarlyDataType> earlyDataType_;
  folly::Optional<std::string> alpn_;
  folly::Optional<std::string> sni_;
  folly::Optional<CertificateCompressionAlgorithm> serverCertCompAlgo_;
  folly::Optional<std::chrono::system_clock::time_point> handshakeTime_;

  folly::Optional<EarlyDataParams> earlyDataParams_;

  folly::Optional<Random> clientRandom_;
  folly::Optional<Buf> legacySessionId_;
  bool sentCCS_{false};

  folly::Optional<Buf> encodedClientHello_;

  folly::Optional<ECHState> echState_;

  mutable folly::Optional<std::map<NamedGroup, std::unique_ptr<KeyExchange>>>
      keyExchangers_;
  folly::Optional<std::vector<ExtensionType>> requestedExtensions_;

  folly::Optional<Buf> clientHandshakeSecret_;
  folly::Optional<Buf> serverHandshakeSecret_;
  folly::Optional<Buf> resumptionSecret_;

  folly::Optional<std::vector<std::shared_ptr<const PeerCert>>>
      unverifiedCertChain_;
  folly::Optional<CachedPsk> attemptedPsk_;
  folly::Optional<Buf> exporterMasterSecret_;
  std::shared_ptr<ClientExtensions> extensions_;
};

folly::StringPiece toString(client::StateEnum);
folly::StringPiece toString(client::ClientAuthType);
folly::StringPiece toString(client::ECHStatus);

inline std::ostream& operator<<(std::ostream& os, StateEnum state) {
  os << toString(state);
  return os;
}

inline std::ostream& operator<<(std::ostream& os, ClientAuthType auth) {
  os << toString(auth);
  return os;
}
} // namespace client
} // namespace fizz
