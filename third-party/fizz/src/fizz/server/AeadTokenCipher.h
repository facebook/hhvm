/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/Hkdf.h>
#include <fizz/crypto/Sha256.h>
#include <fizz/crypto/aead/OpenSSLEVPCipher.h>
#include <fizz/record/Types.h>
#include <fizz/server/TokenCipher.h>
#include <folly/Optional.h>
#include <folly/io/IOBuf.h>

namespace fizz {
namespace server {

class Aead128GCMTokenCipher : public TokenCipher {
 public:
  static constexpr size_t kMinTokenSecretLength = 32;

  using HashType = Sha256;
  using AeadType = OpenSSLEVPCipher;
  using CipherType = AESGCM128;

  /**
   * Set additional context strings for use with these tokens. The strings will
   * be used, in order, as part of the key derivation so that different contexts
   * will result in different keys, preventing keys from one context from being
   * used for another.
   */
  explicit Aead128GCMTokenCipher(std::vector<std::string> contextStrings)
      : contextStrings_(std::move(contextStrings)) {}

  ~Aead128GCMTokenCipher() override {
    clearSecrets();
  }

  /**
   * The first one will be used for encryption.
   * All secrets must be at least kMinTokenSecretLength long.
   */
  bool setSecrets(const std::vector<folly::ByteRange>& tokenSecrets) override;

  folly::Optional<Buf> encrypt(
      Buf plaintext,
      folly::IOBuf* associatedData = nullptr) const override;

  folly::Optional<Buf> decrypt(Buf, folly::IOBuf* associatedData = nullptr)
      const override;

 private:
  using Secret = std::vector<uint8_t>;
  static constexpr size_t kSaltLength = HashType::HashLen;
  using Salt = std::array<uint8_t, kSaltLength>;
  using SeqNum = uint32_t;
  static constexpr size_t kTokenHeaderLength = kSaltLength + sizeof(SeqNum);

  std::unique_ptr<Aead> createAead(
      folly::ByteRange secret,
      folly::ByteRange salt) const;

  void clearSecrets();

  // First secret is the one used to encrypt.
  std::vector<Secret> secrets_;

  std::vector<std::string> contextStrings_;
};
} // namespace server
} // namespace fizz
