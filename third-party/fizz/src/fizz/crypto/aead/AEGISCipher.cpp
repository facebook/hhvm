/*
 *  Copyright (c) Meta Platforms, Inc. and affiliates.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/aead/AEGISCipher.h>

#if FIZZ_HAS_AEGIS

#include <folly/lang/CheckedMath.h>
#include <sodium.h>
#include <sodium/crypto_aead_aegis128l.h>
#include <sodium/crypto_aead_aegis256.h>
#include <functional>

namespace fizz {

static_assert(
    fizz::AEGISCipher::kMaxIVLength == crypto_aead_aegis256_NPUBBYTES,
    "Invalid AEGISCipher::kMaxIVLength");

namespace {

std::unique_ptr<folly::IOBuf> aegisEncrypt(
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
  int ret = crypto_aead_aegis128l_encrypt(
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
  if (crypto_aead_aegis128l_decrypt(
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

std::unique_ptr<Aead> AEGISCipher::makeCipher() {
  return std::unique_ptr<Aead>(new AEGISCipher(
      crypto_aead_aegis128l_KEYBYTES,
      crypto_aead_aegis128l_NPUBBYTES,
      crypto_aead_aegis128l_ABYTES));
}

AEGISCipher::AEGISCipher(size_t keyLength, size_t ivLength, size_t tagLength)
    : keyLength_(keyLength), ivLength_(ivLength), tagLength_(tagLength) {}

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
  auto iv = createIV(seqNum);
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
  auto iv = createIV(seqNum);
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
      std::move(ciphertext), associatedData, nonce, tagLength_, trafficKeyKey_);
}

size_t AEGISCipher::getCipherOverhead() const {
  return tagLength_;
}

std::array<uint8_t, AEGISCipher::kMaxIVLength> AEGISCipher::createIV(
    uint64_t seqNum) const {
  std::array<uint8_t, kMaxIVLength> iv;
  uint64_t bigEndianSeqNum = folly::Endian::big(seqNum);
  const size_t prefixLength = ivLength_ - sizeof(uint64_t);
  memset(iv.data(), 0, prefixLength);
  memcpy(iv.data() + prefixLength, &bigEndianSeqNum, sizeof(uint64_t));
  folly::MutableByteRange mutableIv{iv.data(), ivLength_};
  XOR(trafficIvKey_, mutableIv);
  return iv;
}
} // namespace fizz

#endif
