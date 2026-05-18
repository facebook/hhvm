/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/KeyDerivation.h>
#include <fizz/crypto/aead/Aead.h>
#include <fizz/util/Variant.h>
#include <folly/Optional.h>

namespace fizz {

enum class EarlySecrets {
  ExternalPskBinder,
  ResumptionPskBinder,
  ClientEarlyTraffic,
  EarlyExporter,
  ECHAcceptConfirmation,
  HRRECHAcceptConfirmation
};

enum class HandshakeSecrets {
  ClientHandshakeTraffic,
  ServerHandshakeTraffic,
  ECHAcceptConfirmation
};

enum class MasterSecrets { ExporterMaster, ResumptionMaster };

enum class AppTrafficSecrets { ClientAppTraffic, ServerAppTraffic };

#define FIZZ_KEYSCHEDULER_SECRETTYPE(F, ...) \
  F(EarlySecrets, __VA_ARGS__)               \
  F(HandshakeSecrets, __VA_ARGS__)           \
  F(MasterSecrets, __VA_ARGS__)              \
  F(AppTrafficSecrets, __VA_ARGS__)

FIZZ_DECLARE_COPYABLE_VARIANT_TYPE(SecretType, FIZZ_KEYSCHEDULER_SECRETTYPE)
#undef FIZZ_KEYSCHEDULER_SECRETTYPE

struct DerivedSecret {
  std::vector<uint8_t> secret;
  SecretType type;

  DerivedSecret() : type(EarlySecrets::ExternalPskBinder) {}

  DerivedSecret(std::vector<uint8_t> secretIn, SecretType typeIn)
      : secret(std::move(secretIn)), type(typeIn) {}

  DerivedSecret(folly::ByteRange secretIn, SecretType typeIn)
      : secret(secretIn.begin(), secretIn.end()), type(typeIn) {}

  bool operator==(const DerivedSecret& other) const {
    return secret == other.secret && type == other.type;
  }

  bool operator!=(const DerivedSecret& other) const {
    return !(*this == other);
  }
};

/**
 * Keeps track of the TLS 1.3 key derivation schedule.
 */
class KeyScheduler {
 public:
  explicit KeyScheduler(std::unique_ptr<KeyDerivation> deriver)
      : KeyScheduler(folly::none, folly::none, std::move(deriver)) {}
  virtual ~KeyScheduler() = default;

  /**
   * Derives the early secret. Must be in uninitialized state.
   */
  virtual Status deriveEarlySecret(Error& err, folly::ByteRange psk);

  /**
   * Derives the master secert. Must be in early secret state.
   */
  virtual Status deriveHandshakeSecret(Error& err);

  /**
   * Derives the master secret with a DH secret. Must be in uninitialized or
   * early secret state.
   */
  virtual Status deriveHandshakeSecret(Error& err, folly::ByteRange ecdhe);

  /**
   * Derives the master secert. Must be in handshake secret state.
   */
  virtual Status deriveMasterSecret(Error& err);

  /**
   * Derives the app traffic secrets given the handshake context. Must be in
   * master secret state. Note that this does not clear the master secret.
   */
  virtual Status deriveAppTrafficSecrets(
      Error& err,
      folly::ByteRange transcript);

  /**
   * Clears the master secret. Must be in master secret state.
   */
  virtual Status clearMasterSecret(Error& err);

  /**
   * Performs a key update on the client traffic key. Traffic secrets must be
   * derived.
   */
  virtual Status clientKeyUpdate(uint32_t& ret, Error& err);

  /**
   * Performs a key update on the server traffic key. Traffic secrets must be
   * derived.
   */
  virtual Status serverKeyUpdate(uint32_t& ret, Error& err);

  /**
   * Retreive a secret from the scheduler. Must be in the appropriate state.
   */
  virtual Status getSecret(
      DerivedSecret& ret,
      Error& err,
      EarlySecrets s,
      folly::ByteRange transcript) const;
  virtual Status getSecret(
      DerivedSecret& ret,
      Error& err,
      HandshakeSecrets s,
      folly::ByteRange transcript) const;
  virtual Status getSecret(
      DerivedSecret& ret,
      Error& err,
      MasterSecrets s,
      folly::ByteRange transcript) const;
  virtual DerivedSecret getSecret(AppTrafficSecrets s) const;

  /**
   * Derive a traffic key and iv from a traffic secret.
   */
  virtual Status getTrafficKey(
      TrafficKey& ret,
      Error& err,
      folly::ByteRange trafficSecret,
      size_t keyLength,
      size_t ivLength) const;

  /**
   * Derive a traffic key and iv from a traffic secret with the label supplied.
   */
  virtual Status getTrafficKeyWithLabel(
      TrafficKey& ret,
      Error& err,
      folly::ByteRange trafficSecret,
      folly::StringPiece keyLabel,
      folly::StringPiece ivLabel,
      size_t keyLength,
      size_t ivLength) const;

  /**
   * Derive a resumption secret with a particular ticket nonce. Does not require
   * being in master secret state.
   */
  virtual Status getResumptionSecret(
      Buf& ret,
      Error& err,
      folly::ByteRange resumptionMasterSecret,
      folly::ByteRange ticketNonce) const;

  /**
   * Clones the state of the KeyScheduler
   */
  [[nodiscard]] virtual std::unique_ptr<KeyScheduler> clone() const;

 private:
  struct EarlySecret {
    std::vector<uint8_t> secret;

    bool operator==(const EarlySecret& other) const {
      return secret == other.secret;
    }
  };
  struct HandshakeSecret {
    std::vector<uint8_t> secret;

    bool operator==(const HandshakeSecret& other) const {
      return secret == other.secret;
    }
  };
  struct MasterSecret {
    std::vector<uint8_t> secret;

    bool operator==(const MasterSecret& other) const {
      return secret == other.secret;
    }
  };
  struct AppTrafficSecret {
    std::vector<uint8_t> client;
    uint32_t clientGeneration{0};
    std::vector<uint8_t> server;
    uint32_t serverGeneration{0};

    bool operator==(const AppTrafficSecret& other) const {
      return client == other.client &&
          clientGeneration == other.clientGeneration &&
          server == other.server && serverGeneration == other.serverGeneration;
    }
  };

#define FIZZ_KEYSCHEDULER_SECRETS(F, ...) \
  F(EarlySecret, __VA_ARGS__)             \
  F(HandshakeSecret, __VA_ARGS__)         \
  F(MasterSecret, __VA_ARGS__)

  FIZZ_DECLARE_COPYABLE_VARIANT_TYPE(
      KeySchedulerSecret,
      FIZZ_KEYSCHEDULER_SECRETS)
#undef FIZZ_KEYSCHEDULER_SECRETS

  KeyScheduler(
      folly::Optional<KeySchedulerSecret> secret,
      folly::Optional<AppTrafficSecret> appTrafficSecret,
      std::unique_ptr<KeyDerivation> deriver)
      : secret_(std::move(secret)),
        appTrafficSecret_(std::move(appTrafficSecret)),
        deriver_(std::move(deriver)) {}

  folly::Optional<KeySchedulerSecret> secret_;
  folly::Optional<AppTrafficSecret> appTrafficSecret_;

  std::unique_ptr<KeyDerivation> deriver_;
};
} // namespace fizz
