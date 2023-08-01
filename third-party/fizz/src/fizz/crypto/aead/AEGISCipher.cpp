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
#include <folly/lang/CheckedMath.h>
#include <functional>

namespace fizz {

static_assert(
    fizz::AEGISCipher::kMaxIVLength == fizz_aegis256_NPUBBYTES,
    "Invalid AEGISCipher::kMaxIVLength");

namespace {

std::unique_ptr<folly::IOBuf> aegisEncrypt(
    AEGISCipher::InitStateFn initstate,
    AEGISCipher::AadUpdateFn aadUpdate,
    AEGISCipher::AadFinalFn aadFinal,
    AEGISCipher::EncryptUpdateFn encryptUpdate,
    AEGISCipher::EncryptFinalFn encryptFinal,
    std::unique_ptr<folly::IOBuf>&& plaintext,
    const folly::IOBuf* associatedData,
    folly::ByteRange iv,
    folly::ByteRange key,
    size_t tagLen,
    size_t headroom) {
  AEGISCipher::AegisEVPCtx ctx;
  if (initstate(key.data(), iv.data(), &ctx) != 0) {
    throw std::runtime_error("Initiate encryption state error");
  }

  auto inputLength = plaintext->computeChainDataLength();
  std::unique_ptr<folly::IOBuf> output;
  folly::IOBuf* input;
  // create enough to also fit the tag and headroom
  size_t totalSize{0};
  if (!folly::checked_add<size_t>(&totalSize, headroom, inputLength, tagLen)) {
    throw std::overflow_error("Output buffer size");
  }
  output = folly::IOBuf::create(totalSize);
  output->advance(headroom);
  output->append(inputLength + tagLen);
  input = plaintext.get();

  if (associatedData) {
    for (auto current : *associatedData) {
      if (current.size() > std::numeric_limits<int>::max()) {
        throw std::runtime_error("too much associated data");
      }
      if (aadUpdate(current.data(), current.size(), &ctx) != 0) {
        throw std::runtime_error("Encryption aad update error");
      }
    }
    if (aadFinal(&ctx) != 0) {
      throw std::runtime_error("Encryption aad final error");
    }
  }

  unsigned long long writtenlen = 0;
  unsigned long long totalWritten = 0;
  for (auto current : *input) {
    if (current.size() > std::numeric_limits<int>::max()) {
      throw std::runtime_error("too much plaintext data");
    }
    if (encryptUpdate(
            output->writableData() + totalWritten,
            &writtenlen,
            current.data(),
            current.size(),
            &ctx) != 0) {
      throw std::runtime_error("Encryption update error");
    }
    totalWritten += writtenlen;
  }
  if (encryptFinal(
          output->writableData() + totalWritten,
          &writtenlen,
          output->writableData() + inputLength,
          &ctx) != 0 ||
      totalWritten + writtenlen != (inputLength + tagLen)) {
    throw std::runtime_error("Encryption error");
  }
  return output;
}

} // namespace

folly::Optional<std::unique_ptr<folly::IOBuf>> AEGISCipher::doDecrypt(
    std::unique_ptr<folly::IOBuf>&& ciphertext,
    const folly::IOBuf* associatedData,
    folly::ByteRange iv,
    folly::ByteRange key,
    folly::MutableByteRange tagOut,
    bool inPlace) const {
  AegisEVPCtx ctx;
  if (initstate_(key.data(), iv.data(), &ctx) != 0) {
    throw std::runtime_error("Initiate encryption state error");
  }

  auto inputLength = ciphertext->computeChainDataLength();
  folly::IOBuf* input;
  std::unique_ptr<folly::IOBuf> output;
  if (!inPlace) {
    output = folly::IOBuf::create(inputLength);
    output->append(inputLength);
    input = ciphertext.get();
  } else {
    output = std::move(ciphertext);
    input = output.get();
  }

  if (associatedData) {
    for (auto current : *associatedData) {
      if (current.size() > std::numeric_limits<int>::max()) {
        throw std::runtime_error("too much associated data");
      }
      if (aadUpdate_(current.data(), current.size(), &ctx) != 0) {
        throw std::runtime_error("Decryption aad update error");
      }
    }
    if (aadFinal_(&ctx) != 0) {
      throw std::runtime_error("Decryption aad final error");
    }
  }
  struct Impl {
    const AEGISCipher& self;
    AEGISCipher::AegisEVPCtx& ctx;
    const unsigned char* expectedTag{nullptr};

    Impl(const AEGISCipher& s, AegisEVPCtx& c) : self(s), ctx(c) {}
    bool decryptUpdate(
        uint8_t* plain,
        const uint8_t* cipher,
        size_t len,
        int* outLen) {
      unsigned long long tempOutLen;
      auto ret = self.decryptUpdate_(plain, &tempOutLen, cipher, len, &ctx);
      *outLen = static_cast<int>(tempOutLen);
      return ret == 0;
    }
    bool setExpectedTag(int /*tagSize*/, const unsigned char* tag) {
      this->expectedTag = tag;
      return true;
    }
    bool decryptFinal(unsigned char* outm, int* outLen) {
      unsigned long long tempOutLen;
      auto ret = self.decryptFinal_(outm, &tempOutLen, expectedTag, &ctx);
      *outLen = static_cast<int>(tempOutLen);
      return ret == 0;
    }
  };

  bool decrypted;
  if (stateSize_ == AEGISCipher::kAEGIS28LStateSize) {
    decrypted = decFuncBlocks<AEGISCipher::kAEGIS28LStateSize>(
        Impl(*this, ctx), *input, *output, tagOut);
  } else if (stateSize_ == AEGISCipher::kAEGIS256StateSize) {
    decrypted = decFuncBlocks<AEGISCipher::kAEGIS256StateSize>(
        Impl(*this, ctx), *input, *output, tagOut);
  } else {
    throw std::runtime_error("Unsupported AEGIS state size");
  }
  if (!decrypted) {
    return folly::none;
  }
  return output;
}

AEGISCipher::AEGISCipher(
    InitStateFn init,
    AadUpdateFn aadUpdate,
    AadFinalFn addFinal,
    EncryptUpdateFn encryptUpdate,
    EncryptFinalFn encryptFinal,
    DecryptUpdateFn decryptUpdate,
    DecryptFinalFn decryptFinal,
    size_t keyLength,
    size_t ivLength,
    size_t tagLength,
    size_t stateSize)
    : initstate_(init),
      aadUpdate_(aadUpdate),
      aadFinal_(addFinal),
      encryptUpdate_(encryptUpdate),
      encryptFinal_(encryptFinal),
      decryptUpdate_(decryptUpdate),
      decryptFinal_(decryptFinal),
      keyLength_(keyLength),
      ivLength_(ivLength),
      tagLength_(tagLength),
      stateSize_(stateSize) {
  static int dummy = []() -> int {
    // The application should be doing this, sodium_init is safe to call
    // multiple times, and `fizz_aegis*_pick_best_implementation` relies on
    // sodium's cpu feature vector being populated (which is done in init)
    if (sodium_init() == -1) {
      throw std::runtime_error("failed to initialize libsodium");
    }
    (void)fizz_aegis128l_pick_best_implementation();
    (void)fizz_aegis256_pick_best_implementation();
    return 0;
  }();
  (void)dummy;
}

std::unique_ptr<Aead> AEGISCipher::make128L() {
  return std::unique_ptr<Aead>(new AEGISCipher(
      aegis128l_init_state,
      aegis128l_aad_update,
      aegis128l_aad_final,
      aegis128l_encrypt_update,
      aegis128l_encrypt_final,
      aegis128l_decrypt_update,
      aegis128l_decrypt_final,
      fizz_aegis128l_KEYBYTES,
      fizz_aegis128l_NPUBBYTES,
      fizz_aegis128l_ABYTES,
      kAEGIS28LStateSize));
}

std::unique_ptr<Aead> AEGISCipher::make256() {
  return std::unique_ptr<Aead>(new AEGISCipher(
      aegis256_init_state,
      aegis256_aad_update,
      aegis256_aad_final,
      aegis256_encrypt_update,
      aegis256_encrypt_final,
      aegis256_decrypt_update,
      aegis256_decrypt_final,
      fizz_aegis256_KEYBYTES,
      fizz_aegis256_NPUBBYTES,
      fizz_aegis256_ABYTES,
      kAEGIS256StateSize));
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
      initstate_,
      aadUpdate_,
      aadFinal_,
      encryptUpdate_,
      encryptFinal_,
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
    Aead::AeadOptions options) const {
  if (tagLength_ > ciphertext->computeChainDataLength()) {
    return folly::none;
  }

  auto inPlace =
      (!ciphertext->isShared() ||
       options.bufferOpt != Aead::BufferOption::RespectSharedPolicy);

  if (!inPlace && options.allocOpt == Aead::AllocationOption::Deny) {
    throw std::runtime_error("Unable to decrypt (no-alloc requires in-place)");
  }

  // Set up the tag buffer now
  const auto& lastBuf = ciphertext->prev();
  folly::MutableByteRange tagOut;
  std::array<uint8_t, kTagLength> tag;
  if (lastBuf->length() >= tagLength_) {
    // We can directly carve out this buffer from the last IOBuf
    // Adjust buffer sizes
    lastBuf->trimEnd(tagLength_);

    tagOut = {lastBuf->writableTail(), tagLength_};
  } else {
    // Tag is fragmented so we need to copy it out.
    if (options.allocOpt == Aead::AllocationOption::Deny) {
      throw std::runtime_error(
          "Unable to decrypt (tag is fragmented and no allocation allowed)");
    }
    // buffer to copy the tag into when we decrypt
    tagOut = {tag.data(), tagLength_};
    trimBytes(*ciphertext, tagOut);
  }
  return doDecrypt(
      std::move(ciphertext),
      associatedData,
      nonce,
      trafficKeyKey_,
      tagOut,
      inPlace);
}

size_t AEGISCipher::getCipherOverhead() const {
  return tagLength_;
}
} // namespace fizz

#endif
