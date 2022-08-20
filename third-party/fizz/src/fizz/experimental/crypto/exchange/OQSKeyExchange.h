/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/exchange/KeyExchange.h>
#include <fizz/protocol/Factory.h>
#include <folly/Memory.h>
#include <oqs/kem.h>

namespace fizz {
class OQSKeyExchange : public KeyExchange {
 public:
  static std::unique_ptr<OQSKeyExchange> createOQSKeyExchange(
      Factory::KeyExchangeMode mode,
      const std::string& algorithm);

  ~OQSKeyExchange() override = default;

 protected:
  inline static void freeKem(OQS_KEM* kem) {
    OQS_KEM_free(kem);
  }
  using kemDeleter_ = folly::static_function_deleter<OQS_KEM, &freeKem>;

  inline static void securelyFreeKey(folly::IOBuf* key) {
    if (key != nullptr && key->writableData() != nullptr) {
      key->unshare();
      key->coalesce();
      OQS_MEM_cleanse(key->writableData(), key->length());
    }
    delete key;
  }
  using keyDeleter_ =
      folly::static_function_deleter<folly::IOBuf, &securelyFreeKey>;

  std::unique_ptr<OQS_KEM, kemDeleter_> kem_;

  /*
   * algorithm: one of the macro defined in oqs/kem.h
   */
  explicit OQSKeyExchange(const std::string& algorithm);
  void generateKeyPair() override = 0;
  /**
   * The Fizz API is not compatible with KEM APIs as in KEM the server and
   * client call different routine (encap() vs. decap()), but in ECDH both call
   * the same routine (getKeyShare() and generateSharedSecret()). To adapt KEM
   * into existing fizz APIs, we have to determine the whether the caller is
   * server or client.
   */
  std::unique_ptr<folly::IOBuf> getKeyShare() const override = 0;
  std::unique_ptr<folly::IOBuf> generateSharedSecret(
      folly::ByteRange keyShare) const override = 0;
  std::unique_ptr<KeyExchange> clone() const override = 0;
  std::size_t getExpectedKeyShareSize() const override = 0;
  virtual bool isInitiated() const = 0;
  virtual void checkChained() const = 0;
};

class OQSClientKeyExchange : public OQSKeyExchange {
 private:
  std::unique_ptr<folly::IOBuf, keyDeleter_> publicKey_;
  std::unique_ptr<folly::IOBuf, keyDeleter_> secretKey_;

 public:
  explicit OQSClientKeyExchange(const std::string& algorithm);
  ~OQSClientKeyExchange() override = default;
  void generateKeyPair() override;
  std::unique_ptr<folly::IOBuf> getKeyShare() const override;
  std::unique_ptr<folly::IOBuf> generateSharedSecret(
      folly::ByteRange keyShare) const override;
  std::unique_ptr<KeyExchange> clone() const override;
  std::size_t getExpectedKeyShareSize() const override;
  bool isInitiated() const override;
  void checkChained() const override;
};

class OQSServerKeyExchange : public OQSKeyExchange {
 private:
  std::unique_ptr<folly::IOBuf, keyDeleter_> cipherText_;

 public:
  explicit OQSServerKeyExchange(const std::string& algorithm);
  ~OQSServerKeyExchange() override = default;
  /**
   * Intentionally left as blank as the server doesn't need to generate the key
   * pair because it only uses client's public key to encrypt the shared secret.
   * But if in future the shared secret becomes uni-directional and the client
   * also needs to derive its own shared secret (i.e., 1.5-RTT), then we need
   * the server to generate the key pair as well. See
   * https://www.ietf.org/id/draft-celi-wiggers-tls-authkem-01.html
   */
  void generateKeyPair() override {}
  std::unique_ptr<folly::IOBuf> getKeyShare() const override;
  std::unique_ptr<folly::IOBuf> generateSharedSecret(
      folly::ByteRange keyShare) const override;
  std::unique_ptr<KeyExchange> clone() const override;
  std::size_t getExpectedKeyShareSize() const override;
  bool isInitiated() const override;
  void checkChained() const override;
};
} // namespace fizz
