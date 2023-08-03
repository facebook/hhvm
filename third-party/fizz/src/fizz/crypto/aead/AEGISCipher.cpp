/*
 *  Copyright (c) Meta Platforms, Inc. and affiliates.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/aead/AEGISCipher.h>
#include <fizz/fizz-config.h>
#include <cstring>
#if FIZZ_BUILD_AEGIS

#include <fizz/crypto/aead/CryptoUtil.h>
#include <folly/lang/CheckedMath.h>
#include <functional>

namespace fizz {

static_assert(
    fizz::AEGISCipher::kMaxIVLength == fizz_aegis256_NPUBBYTES,
    "Invalid AEGISCipher::kMaxIVLength");

std::unique_ptr<folly::IOBuf> AEGISCipher::doEncrypt(
    std::unique_ptr<folly::IOBuf>&& plaintext,
    const folly::IOBuf* associatedData,
    folly::ByteRange iv,
    folly::ByteRange key,
    Aead::AeadOptions options) const {
  AegisEVPCtx ctx;
  if (initstate_(key.data(), iv.data(), &ctx) != 0) {
    throw std::runtime_error("Initiate encryption state error");
  }

  auto inputLength = plaintext->computeChainDataLength();
  const auto& bufOption = options.bufferOpt;
  const auto& allocOption = options.allocOpt;
  std::unique_ptr<folly::IOBuf> output;
  folly::IOBuf* input;

  // Whether to allow modifying buffer contents directly
  bool allowInplaceEdit = !plaintext->isShared() ||
      bufOption != Aead::BufferOption::RespectSharedPolicy;

  // Whether to allow growing tailRoom
  bool allowGrowth =
      (bufOption ==
           Aead::BufferOption::AllowFullModification || // When explicitly
                                                        // requested
       !plaintext->isShared() || // When plaintext is unique
       !allowInplaceEdit); // When not in-place (new buffer)

  // Whether to allow memory allocations (throws if needs more memory)
  bool allowAlloc = allocOption == Aead::AllocationOption::Allow;

  if (!allowInplaceEdit && !allowAlloc) {
    throw std::runtime_error(
        "Cannot encrypt (must be in-place or allow allocation)");
  }

  if (allowInplaceEdit) {
    output = std::move(plaintext);
    input = output.get();
  } else {
    // create enough to also fit the tag and headroom
    size_t totalSize{0};
    if (!folly::checked_add<size_t>(
            &totalSize, headroom_, inputLength, kTagLength)) {
      throw std::overflow_error("Output buffer size");
    }
    output = folly::IOBuf::create(totalSize);
    output->advance(headroom_);
    output->append(inputLength);
    input = plaintext.get();
  }

  if (associatedData) {
    for (auto current : *associatedData) {
      if (current.size() > std::numeric_limits<int>::max()) {
        throw std::runtime_error("too much associated data");
      }
      if (aadUpdate_(current.data(), current.size(), &ctx) != 0) {
        throw std::runtime_error("Encryption aad update error");
      }
    }
    if (aadFinal_(&ctx) != 0) {
      throw std::runtime_error("Encryption aad final error");
    }
  }

  struct Impl {
    const AEGISCipher& self;
    AegisEVPCtx& ctx;
    unsigned char* tag;

    Impl(const AEGISCipher& s, AegisEVPCtx& c, unsigned char* t)
        : self(s), ctx(c), tag(t) {}
    bool encryptUpdate(
        uint8_t* cipher,
        int* outLen,
        const uint8_t* plain,
        size_t len) {
      unsigned long long tempOutLen;
      auto ret = self.encryptUpdate_(cipher, &tempOutLen, plain, len, &ctx);
      *outLen = static_cast<int>(tempOutLen);
      return ret == 0;
    }
    bool encryptFinal(unsigned char* outm, int* outLen) {
      unsigned long long tempOutLen;
      auto ret = self.encryptFinal_(outm, &tempOutLen, tag, &ctx);
      *outLen = static_cast<int>(tempOutLen) - kTagLength;
      return ret == 0;
    }
  };
  unsigned char tag[kTagLength];
  if (mms_ == AEGISCipher::kAEGIS128LMMS) {
    encFuncBlocks<AEGISCipher::kAEGIS128LMMS>(
        Impl(*this, ctx, tag), *input, *output);
  } else if (mms_ == AEGISCipher::kAEGIS256MMS) {
    encFuncBlocks<AEGISCipher::kAEGIS256MMS>(
        Impl(*this, ctx, tag), *input, *output);
  } else {
    throw std::runtime_error("Unsupported AEGIS state size");
  }

  // output is always something we can modify
  auto tailRoom = output->prev()->tailroom();
  if (tailRoom < kTagLength || !allowGrowth) {
    if (!allowAlloc) {
      throw std::runtime_error("Cannot encrypt (insufficient space for tag)");
    }
    std::unique_ptr<folly::IOBuf> tag_iobuf = folly::IOBuf::create(kTagLength);
    tag_iobuf->append(kTagLength);
    memcpy(tag_iobuf->writableData(), tag, kTagLength);
    output->prependChain(std::move(tag_iobuf));
  } else {
    auto lastBuf = output->prev();
    lastBuf->append(kTagLength);
    memcpy(lastBuf->writableTail() - kTagLength, tag, kTagLength);
  }
  return output;
}

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
    AegisEVPCtx& ctx;
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
  if (mms_ == AEGISCipher::kAEGIS128LMMS) {
    decrypted = decFuncBlocks<AEGISCipher::kAEGIS128LMMS>(
        Impl(*this, ctx), *input, *output, tagOut);
  } else if (mms_ == AEGISCipher::kAEGIS256MMS) {
    decrypted = decFuncBlocks<AEGISCipher::kAEGIS256MMS>(
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
    size_t mms)
    : initstate_(init),
      aadUpdate_(aadUpdate),
      aadFinal_(addFinal),
      encryptUpdate_(encryptUpdate),
      encryptFinal_(encryptFinal),
      decryptUpdate_(decryptUpdate),
      decryptFinal_(decryptFinal),
      keyLength_(keyLength),
      ivLength_(ivLength),
      mms_(mms) {
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
      kAEGIS128LMMS));
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
      kAEGIS256MMS));
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
    Aead::AeadOptions options) const {
  return doEncrypt(
      std::move(plaintext), associatedData, nonce, trafficKeyKey_, options);
}

// TODO: (T136805571) We will add implementation for inplace encryption later
std::unique_ptr<folly::IOBuf> AEGISCipher::inplaceEncrypt(
    std::unique_ptr<folly::IOBuf>&& plaintext,
    const folly::IOBuf* associatedData,
    uint64_t seqNum) const {
  auto iv =
      createIV<AEGISCipher::kMaxIVLength>(seqNum, ivLength_, trafficIvKey_);
  return doEncrypt(
      std::move(plaintext),
      associatedData,
      folly::ByteRange(iv.data(), ivLength_),
      trafficKeyKey_,
      {Aead::BufferOption::AllowFullModification,
       Aead::AllocationOption::Deny});
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
  if (kTagLength > ciphertext->computeChainDataLength()) {
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
  if (lastBuf->length() >= kTagLength) {
    // We can directly carve out this buffer from the last IOBuf
    // Adjust buffer sizes
    lastBuf->trimEnd(kTagLength);

    tagOut = {lastBuf->writableTail(), kTagLength};
  } else {
    // Tag is fragmented so we need to copy it out.
    if (options.allocOpt == Aead::AllocationOption::Deny) {
      throw std::runtime_error(
          "Unable to decrypt (tag is fragmented and no allocation allowed)");
    }
    // buffer to copy the tag into when we decrypt
    tagOut = {tag.data(), kTagLength};
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
  return kTagLength;
}
} // namespace fizz

#endif
