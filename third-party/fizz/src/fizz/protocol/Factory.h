/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <string>

#include <fizz/crypto/KeyDerivation.h>
#include <fizz/crypto/RandomGenerator.h>
#include <fizz/crypto/aead/Aead.h>
#include <fizz/crypto/exchange/KeyExchange.h>
#include <fizz/protocol/Certificate.h>
#include <fizz/protocol/HandshakeContext.h>
#include <fizz/protocol/KeyScheduler.h>
#include <fizz/protocol/Types.h>
#include <fizz/record/EncryptedRecordLayer.h>
#include <fizz/record/PlaintextRecordLayer.h>
#include <fizz/record/Types.h>

namespace fizz {

class PeerCert;

/**
 * This class instantiates various objects to facilitate testing.
 */
class Factory {
 public:
  enum class KeyExchangeMode { Server, Client };

  virtual ~Factory() = default;

  virtual std::unique_ptr<PlaintextReadRecordLayer>
  makePlaintextReadRecordLayer() const = 0;

  virtual std::unique_ptr<PlaintextWriteRecordLayer>
  makePlaintextWriteRecordLayer() const = 0;

  virtual std::unique_ptr<EncryptedReadRecordLayer>
  makeEncryptedReadRecordLayer(EncryptionLevel encryptionLevel) const = 0;

  virtual std::unique_ptr<EncryptedWriteRecordLayer>
  makeEncryptedWriteRecordLayer(EncryptionLevel encryptionLevel) const = 0;

  virtual std::unique_ptr<KeyScheduler> makeKeyScheduler(
      CipherSuite cipher) const = 0;

  virtual std::unique_ptr<KeyDerivation> makeKeyDeriver(
      CipherSuite cipher) const = 0;

  virtual std::unique_ptr<HandshakeContext> makeHandshakeContext(
      CipherSuite cipher) const = 0;

  virtual std::unique_ptr<KeyExchange> makeKeyExchange(
      NamedGroup group,
      KeyExchangeMode mode) const = 0;

  [[nodiscard]] virtual std::unique_ptr<Aead> makeAead(
      CipherSuite cipher) const = 0;

  [[nodiscard]] virtual Random makeRandom() const = 0;

  [[nodiscard]] virtual uint32_t makeTicketAgeAdd() const = 0;

  [[nodiscard]] virtual std::unique_ptr<folly::IOBuf> makeRandomBytes(
      size_t count) const = 0;

  virtual std::shared_ptr<PeerCert> makePeerCert(
      CertificateEntry certEntry,
      bool /*leaf*/) const = 0;

  [[nodiscard]] virtual std::shared_ptr<Cert> makeIdentityOnlyCert(
      std::string ident) const = 0;

  [[nodiscard]] virtual std::string getHkdfPrefix() const = 0;
};
} // namespace fizz
