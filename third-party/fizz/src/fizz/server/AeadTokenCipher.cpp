/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/RandomGenerator.h>
#include <fizz/crypto/Utils.h>
#include <fizz/crypto/aead/Aead.h>
#include <fizz/server/AeadTokenCipher.h>

namespace fizz {
namespace server {

/*
 * Token structure:
 *
 * 32 bytes salt
 * 4 bytes sequence number
 * remaining data ciphertext
 *
 * secret = HKDF-Extract(Codec label, token secret)
 * if (PSK context)
 *   secret = HKDF-Extract(PSK context, secret)
 *
 * (aead key | aead iv) = HKDF-Expand(
 *     secret, salt, key length + iv length)
 *
 * The 32 byte salt is used to derive an aead key with sufficient space such
 * that the salts can be generated randomly without worry of collisions. The
 * sequence number is currently always 0 when encrypting tokens, however it
 * could be incremented to avoid an extra HKDF-Expand on every token.
 */

bool Aead128GCMTokenCipher::setSecrets(
    const std::vector<folly::ByteRange>& tokenSecrets) {
  VLOG(3) << "Updating token secrets";

  for (const auto& tokenSecret : tokenSecrets) {
    if (tokenSecret.size() < kMinTokenSecretLength) {
      LOG(ERROR) << "Token cipher secret too small - not updating.";
      return false;
    }
  }

  VLOG(4) << "Updating token secrets, num=" << tokenSecrets.size();
  clearSecrets();
  for (const auto& tokenSecret : tokenSecrets) {
    Secret extracted(tokenSecret.begin(), tokenSecret.end());
    for (const auto& contextString : contextStrings_) {
      extracted = HkdfImpl::create<HashType>().extract(
          folly::range(contextString), folly::range(extracted));
    }
    secrets_.push_back(std::move(extracted));
  }
  return true;
}

folly::Optional<Buf> Aead128GCMTokenCipher::encrypt(
    Buf plaintext,
    folly::IOBuf* associatedData) const {
  if (secrets_.empty()) {
    return folly::none;
  }

  auto salt = RandomGenerator<kSaltLength>().generateRandom();
  auto aead = createAead(folly::range(secrets_.front()), folly::range(salt));

  // For now we always use sequence number 0.
  SeqNum seqNum = 0;
  auto token = folly::IOBuf::create(kTokenHeaderLength);
  folly::io::Appender appender(token.get(), kTokenHeaderLength);
  appender.push(folly::range(salt));
  appender.writeBE(seqNum);
  token->prependChain(
      aead->encrypt(std::move(plaintext), associatedData, seqNum));

  return std::move(token);
}

folly::Optional<Buf> Aead128GCMTokenCipher::decrypt(
    Buf token,
    folly::IOBuf* associatedData) const {
  folly::io::Cursor cursor(token.get());
  if (secrets_.empty() || !cursor.canAdvance(kTokenHeaderLength)) {
    return folly::none;
  }

  Salt salt;
  cursor.pull(salt.data(), salt.size());
  auto seqNum = cursor.readBE<SeqNum>();
  Buf ciphertext;
  cursor.clone(ciphertext, cursor.totalLength());

  for (const auto& secret : secrets_) {
    auto aead = createAead(folly::range(secret), folly::range(salt));
    auto result = aead->tryDecrypt(ciphertext->clone(), associatedData, seqNum);
    if (result) {
      return result;
    }
  }

  VLOG(6) << "Failed to decrypt token.";
  return folly::none;
}

std::unique_ptr<Aead> Aead128GCMTokenCipher::createAead(
    folly::ByteRange secret,
    folly::ByteRange salt) const {
  auto aead = AeadType::makeCipher<CipherType>();
  std::unique_ptr<folly::IOBuf> info = folly::IOBuf::wrapBuffer(salt);
  auto keys = HkdfImpl::create<HashType>().expand(
      secret, *info, aead->keyLength() + aead->ivLength());
  folly::io::Cursor cursor(keys.get());
  TrafficKey key;
  cursor.clone(key.key, aead->keyLength());
  cursor.clone(key.iv, aead->ivLength());
  aead->setKey(std::move(key));
  return aead;
}

void Aead128GCMTokenCipher::clearSecrets() {
  for (auto& secret : secrets_) {
    CryptoUtils::clean(folly::range(secret));
  }
  secrets_.clear();
}
} // namespace server
} // namespace fizz
