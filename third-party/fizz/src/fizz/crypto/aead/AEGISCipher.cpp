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
namespace detail {
class LibAegisCipherBase {
 public:
  virtual ~LibAegisCipherBase() = default;
  virtual void stateInit(
      const uint8_t* ad,
      size_t adlen,
      const uint8_t* npub,
      const uint8_t* k,
      size_t inputLength) = 0;
  virtual int
  encryptUpdate(uint8_t* c, size_t* written, const uint8_t* m, size_t mlen) = 0;
  virtual int
  encryptFinal(uint8_t* c, size_t* written, uint8_t* m, size_t maclen) = 0;
  virtual int
  decryptUpdate(uint8_t* m, size_t* written, const uint8_t* c, size_t clen) = 0;
  virtual int decryptFinal(
      uint8_t* m,
      size_t* written,
      const uint8_t* mac,
      size_t maclen) = 0;
};

template <typename Impl>
class LibAegisCipher : public LibAegisCipherBase {
 public:
  ~LibAegisCipher() override = default;
  LibAegisCipher() : state_({}) {}
  void stateInit(
      const uint8_t* ad,
      size_t adlen,
      const uint8_t* npub,
      const uint8_t* k,
      size_t capacity) override {
    capacity_ = capacity;
    return Impl::stateInit(&state_, ad, adlen, npub, k);
  }
  int encryptUpdate(uint8_t* c, size_t* written, const uint8_t* m, size_t mlen)
      override {
    return Impl::encryptUpdate(&state_, c, capacity_, written, m, mlen);
  }
  int encryptFinal(uint8_t* c, size_t* written, uint8_t* m, size_t maclen)
      override {
    return Impl::encryptFinal(&state_, c, capacity_, written, m, maclen);
  }
  int decryptUpdate(uint8_t* m, size_t* written, const uint8_t* c, size_t clen)
      override {
    return Impl::decryptUpdate(&state_, m, capacity_, written, c, clen);
  }
  int decryptFinal(
      uint8_t* m,
      size_t* written,
      const uint8_t* mac,
      size_t maclen) override {
    return Impl::decryptFinal(&state_, m, capacity_, written, mac, maclen);
  }
  typename Impl::stateType state_;
  size_t capacity_{0};
};
} // namespace detail
#define DEFINE_LIBAEGIS_CIPHER(STRUCT_NAME, CIPHER)                            \
  struct STRUCT_NAME {                                                         \
    using stateType = CIPHER##_state;                                          \
    static constexpr auto stateInit{CIPHER##_state_init};                      \
    static constexpr auto encryptUpdate{CIPHER##_state_encrypt_update};        \
    static constexpr auto encryptFinal{CIPHER##_state_encrypt_detached_final}; \
    static constexpr auto decryptUpdate{                                       \
        CIPHER##_state_decrypt_detached_update};                               \
    static constexpr auto decryptFinal{CIPHER##_state_decrypt_detached_final}; \
  };

DEFINE_LIBAEGIS_CIPHER(AEGIS128L, aegis128l)
DEFINE_LIBAEGIS_CIPHER(AEGIS256, aegis256)

static_assert(
    fizz::AEGISCipher::kMaxIVLength == aegis256_NPUBBYTES,
    "Invalid AEGISCipher::kMaxIVLength");

std::unique_ptr<folly::IOBuf> AEGISCipher::doEncrypt(
    std::unique_ptr<folly::IOBuf>&& plaintext,
    const folly::IOBuf* associatedData,
    folly::ByteRange iv,
    folly::ByteRange key,
    Aead::AeadOptions options) const {
  const uint8_t* adData = nullptr;
  size_t adLen = 0;
  std::unique_ptr<folly::IOBuf> ad;
  if (associatedData) {
    if (associatedData->isChained()) {
      ad = associatedData->cloneCoalesced();
      adData = ad->data();
      adLen = ad->length();
    } else {
      adData = associatedData->data();
      adLen = associatedData->length();
    }
  }

  auto inputLength = plaintext->computeChainDataLength();
  // the stateInit function will skip adding aad to the state when adData is
  // null and adLen is 0
  impl_->stateInit(adData, adLen, iv.data(), key.data(), inputLength);

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

  struct Impl {
    const AEGISCipher& self;
    unsigned char* tag;

    Impl(const AEGISCipher& s, unsigned char* t) : self(s), tag(t) {}
    bool encryptUpdate(
        uint8_t* cipher,
        int* outLen,
        const uint8_t* plain,
        size_t len) {
      size_t tempOutLen;
      auto ret = self.impl_->encryptUpdate(cipher, &tempOutLen, plain, len);
      *outLen = tempOutLen;
      return ret == 0;
    }
    bool encryptFinal(unsigned char* outm, int* outLen) {
      size_t tempOutLen;
      auto ret = self.impl_->encryptFinal(outm, &tempOutLen, tag, kTagLength);
      *outLen = tempOutLen;
      return ret == 0;
    }
  };
  unsigned char tag[kTagLength];
  if (mms_ == AEGISCipher::kAEGIS128LMMS) {
    encFuncBlocks<AEGISCipher::kAEGIS128LMMS>(
        Impl(*this, tag), *input, *output);
  } else if (mms_ == AEGISCipher::kAEGIS256MMS) {
    encFuncBlocks<AEGISCipher::kAEGIS256MMS>(Impl(*this, tag), *input, *output);
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
  const uint8_t* adData = nullptr;
  size_t adLen = 0;
  std::unique_ptr<folly::IOBuf> ad;
  if (associatedData) {
    if (associatedData->isChained()) {
      ad = associatedData->cloneCoalesced();
      adData = ad->data();
      adLen = ad->length();
    } else {
      adData = associatedData->data();
      adLen = associatedData->length();
    }
  }

  auto inputLength = ciphertext->computeChainDataLength();
  // the stateInit function will skip adding aad to the state when adData is
  // null and adLen is 0
  impl_->stateInit(adData, adLen, iv.data(), key.data(), inputLength);

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

  struct Impl {
    const AEGISCipher& self;
    const unsigned char* expectedTag{nullptr};

    explicit Impl(const AEGISCipher& s) : self(s) {}
    bool decryptUpdate(
        uint8_t* plain,
        const uint8_t* cipher,
        size_t len,
        int* outLen) {
      size_t tempOutLen;
      auto ret = self.impl_->decryptUpdate(plain, &tempOutLen, cipher, len);
      *outLen = static_cast<int>(tempOutLen);
      return ret == 0;
    }
    bool setExpectedTag(int /*tagSize*/, const unsigned char* tag) {
      this->expectedTag = tag;
      return true;
    }
    bool decryptFinal(unsigned char* outm, int* outLen) {
      size_t tempOutLen;
      auto ret =
          self.impl_->decryptFinal(outm, &tempOutLen, expectedTag, kTagLength);
      *outLen = static_cast<int>(tempOutLen);
      return ret == 0;
    }
  };

  bool decrypted;
  if (mms_ == AEGISCipher::kAEGIS128LMMS) {
    decrypted = decFuncBlocks<AEGISCipher::kAEGIS128LMMS>(
        Impl(*this), *input, *output, tagOut);
  } else if (mms_ == AEGISCipher::kAEGIS256MMS) {
    decrypted = decFuncBlocks<AEGISCipher::kAEGIS256MMS>(
        Impl(*this), *input, *output, tagOut);
  } else {
    throw std::runtime_error("Unsupported AEGIS state size");
  }
  if (!decrypted) {
    return folly::none;
  }
  return output;
}

AEGISCipher::AEGISCipher(
    std::unique_ptr<detail::LibAegisCipherBase> impl,
    size_t keyLength,
    size_t ivLength,
    size_t mms)
    : impl_(std::move(impl)),
      keyLength_(keyLength),
      ivLength_(ivLength),
      mms_(mms) {
  static int dummy = []() -> int {
    // aegis_init is safe to call multiple times. It populates libaegis's cpu
    // feature vector
    if (aegis_init() == -1) {
      throw std::runtime_error("failed to initialize libaegis");
    }
    return 0;
  }();
  (void)dummy;
}

std::unique_ptr<Aead> AEGISCipher::make128L() {
  auto impl = std::unique_ptr<detail::LibAegisCipherBase>(
      new detail::LibAegisCipher<fizz::AEGIS128L>());
  return std::unique_ptr<Aead>(new AEGISCipher(
      std::move(impl),
      aegis128l_KEYBYTES,
      aegis128l_NPUBBYTES,
      AEGISCipher::kAEGIS128LMMS));
}

std::unique_ptr<Aead> AEGISCipher::make256() {
  auto impl = std::unique_ptr<detail::LibAegisCipherBase>(
      new detail::LibAegisCipher<fizz::AEGIS256>());
  return std::unique_ptr<Aead>(new AEGISCipher(
      std::move(impl),
      aegis256_KEYBYTES,
      aegis256_NPUBBYTES,
      AEGISCipher::kAEGIS256MMS));
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
