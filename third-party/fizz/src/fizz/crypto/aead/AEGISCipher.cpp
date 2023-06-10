/*
 *  Copyright (c) Meta Platforms, Inc. and affiliates.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/aead/AEGISCipher.h>
#include <fizz/fizz-config.h>
#if FIZZ_BUILD_AEGIS

#include <fizz/crypto/aead/CryptoUtil.h>
#include <fizz/third-party/libsodium-aegis/aegis.h>
#include <folly/lang/CheckedMath.h>
#include <functional>

namespace fizz {

static_assert(
    fizz::AEGISCipher::kMaxIVLength == fizz_aegis256_NPUBBYTES,
    "Invalid AEGISCipher::kMaxIVLength");

namespace {

std::unique_ptr<folly::IOBuf> aegisEncrypt(
    AEGISCipher::EncryptFn encrypt,
    std::unique_ptr<folly::IOBuf>&& plaintext,
    const folly::IOBuf* associatedData,
    folly::ByteRange iv,
    folly::ByteRange key,
    size_t tagLen,
    size_t headroom) {
  folly::ByteRange input = plaintext->coalesce();

  std::unique_ptr<folly::IOBuf> output;
  // create enough to also fit the tag and headroom
  size_t totalSize{0};
  if (!folly::checked_add<size_t>(&totalSize, headroom, input.size(), tagLen)) {
    throw std::overflow_error("Output buffer size");
  }
  output = folly::IOBuf::create(totalSize);
  output->advance(headroom);
  output->append(input.size() + tagLen);

  const unsigned char* ad;
  unsigned long long adlen;
  if (associatedData) {
    if (associatedData->isChained()) {
      throw std::overflow_error("associated data is chained or null");
    }
    ad = associatedData->data();
    adlen = associatedData->length();
  } else {
    ad = nullptr;
    adlen = 0;
  }

  unsigned long long ciphertextLength;
  int ret = encrypt(
      output->writableData(),
      &ciphertextLength,
      input.data(),
      input.size(),
      ad,
      adlen,
      nullptr,
      iv.data(),
      key.data());
  if (ret != 0 || ciphertextLength != (input.size() + tagLen)) {
    throw std::runtime_error("Encryption error");
  }
  return output;
}

folly::Optional<std::unique_ptr<folly::IOBuf>> aegisDecrypt(
    AEGISCipher::DecryptFn decrypt,
    std::unique_ptr<folly::IOBuf>&& ciphertext,
    const folly::IOBuf* associatedData,
    folly::ByteRange iv,
    size_t tagLen,
    folly::ByteRange key) {
  folly::ByteRange input = ciphertext->coalesce();

  std::unique_ptr<folly::IOBuf> output;
  output = folly::IOBuf::create(input.size() - tagLen);
  output->append(input.size() - tagLen);

  const unsigned char* ad;
  unsigned long long adlen;
  if (associatedData) {
    if (associatedData->isChained()) {
      throw std::overflow_error("associated data is chained or null");
    }
    ad = associatedData->data();
    adlen = associatedData->length();
  } else {
    ad = nullptr;
    adlen = 0;
  }

  unsigned long long decryptedLength;
  if (decrypt(
          output->writableData(),
          &decryptedLength,
          nullptr,
          input.data(),
          input.size(),
          ad,
          adlen,
          iv.data(),
          key.data()) != 0) {
    return folly::none;
  }
  return output;
}

} // namespace

AEGISCipher::AEGISCipher(
    EncryptFn encrypt,
    DecryptFn decrypt,
    size_t keyLength,
    size_t ivLength,
    size_t tagLength)
    : encrypt_(encrypt),
      decrypt_(decrypt),
      keyLength_(keyLength),
      ivLength_(ivLength),
      tagLength_(tagLength) {}

std::unique_ptr<Aead> AEGISCipher::make128L() {
  return std::unique_ptr<Aead>(new AEGISCipher(
      fizz_aegis128l_encrypt,
      fizz_aegis128l_decrypt,
      fizz_aegis128l_KEYBYTES,
      fizz_aegis128l_NPUBBYTES,
      fizz_aegis128l_ABYTES));
}

std::unique_ptr<Aead> AEGISCipher::make256() {
  return std::unique_ptr<Aead>(new AEGISCipher(
      fizz_aegis256_encrypt,
      fizz_aegis256_decrypt,
      fizz_aegis256_KEYBYTES,
      fizz_aegis256_NPUBBYTES,
      fizz_aegis256_ABYTES));
}

void AEGISCipher::setKey(TrafficKey trafficKey) {
  trafficKey.key->coalesce();
  trafficKey.iv->coalesce();
  if (trafficKey.key->length() != keyLength_) {
    throw std::runtime_error("Invalid key");
  }
  if (trafficKey.iv->length() != ivLength_) {
    throw std::runtime_error("Invalid IV");
  }
  trafficKey_ = std::move(trafficKey);
  // Cache the iv and key. calling coalesce() is not free.
  trafficIvKey_ = trafficKey_.iv->coalesce();
  trafficKeyKey_ = trafficKey_.key->coalesce();
}

folly::Optional<TrafficKey> AEGISCipher::getKey() const {
  if (!trafficKey_.key || !trafficKey_.iv) {
    return folly::none;
  }
  return trafficKey_.clone();
}

std::unique_ptr<folly::IOBuf> AEGISCipher::encrypt(
    std::unique_ptr<folly::IOBuf>&& plaintext,
    const folly::IOBuf* associatedData,
    uint64_t seqNum,
    Aead::AeadOptions options) const {
  auto iv = ::fizz::createIV<AEGISCipher::kMaxIVLength>(
      seqNum, ivLength_, trafficIvKey_);
  return encrypt(
      std::move(plaintext),
      associatedData,
      folly::ByteRange(iv.data(), ivLength_),
      options);
}

std::unique_ptr<folly::IOBuf> AEGISCipher::encrypt(
    std::unique_ptr<folly::IOBuf>&& plaintext,
    const folly::IOBuf* associatedData,
    folly::ByteRange nonce,
    Aead::AeadOptions /*options*/) const {
  return aegisEncrypt(
      encrypt_,
      std::move(plaintext),
      associatedData,
      nonce,
      trafficKeyKey_,
      tagLength_,
      headroom_);
}

// TODO: (T136805571) We will add implementation for inplace encryption later
std::unique_ptr<folly::IOBuf> AEGISCipher::inplaceEncrypt(
    std::unique_ptr<folly::IOBuf>&& /*plaintext*/,
    const folly::IOBuf* /*associatedData*/,
    uint64_t /*seqNum*/) const {
  throw std::runtime_error("inplace encryption not implemented");
}

folly::Optional<std::unique_ptr<folly::IOBuf>> AEGISCipher::tryDecrypt(
    std::unique_ptr<folly::IOBuf>&& ciphertext,
    const folly::IOBuf* associatedData,
    uint64_t seqNum,
    Aead::AeadOptions options) const {
  auto iv = ::fizz::createIV<AEGISCipher::kMaxIVLength>(
      seqNum, ivLength_, trafficIvKey_);
  return tryDecrypt(
      std::move(ciphertext),
      associatedData,
      folly::ByteRange(iv.data(), ivLength_),
      options);
}

folly::Optional<std::unique_ptr<folly::IOBuf>> AEGISCipher::tryDecrypt(
    std::unique_ptr<folly::IOBuf>&& ciphertext,
    const folly::IOBuf* associatedData,
    folly::ByteRange nonce,
    Aead::AeadOptions /*options*/) const {
  return aegisDecrypt(
      decrypt_,
      std::move(ciphertext),
      associatedData,
      nonce,
      tagLength_,
      trafficKeyKey_);
}

size_t AEGISCipher::getCipherOverhead() const {
  return tagLength_;
}
} // namespace fizz

#endif
