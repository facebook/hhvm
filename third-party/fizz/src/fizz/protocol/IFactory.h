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

#include <fizz/protocol/Types.h>
#include <fizz/record/Types.h>
#include <folly/io/async/AsyncTransportCertificate.h>

namespace fizz {

class Aead;
class EncryptedReadRecordLayer;
class EncryptedWriteRecordLayer;
class HandshakeContext;
class KeyDerivation;
class KeyExchange;
class KeyScheduler;
class PeerCert;
class PlaintextReadRecordLayer;
class PlaintextWriteRecordLayer;

/**
 * This class instantiates various objects to facilitate testing.
 */
class IFactory {
 public:
  enum class KeyExchangeMode { Server, Client };

  virtual ~IFactory() = default;

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

  [[nodiscard]] virtual std::shared_ptr<folly::AsyncTransportCertificate>
  makeIdentityOnlyCert(std::string ident) const = 0;

  [[nodiscard]] virtual std::string getHkdfPrefix() const = 0;
};
} // namespace fizz
