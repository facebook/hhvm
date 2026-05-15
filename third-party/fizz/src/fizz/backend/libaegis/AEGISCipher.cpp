/*
 *  Copyright (c) Meta Platforms, Inc. and affiliates.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/backend/libaegis/LibAEGIS.h>
#include <fizz/fizz-config.h>

#if FIZZ_HAVE_LIBAEGIS
#include <aegis.h>
#include <fizz/crypto/aead/CryptoUtil.h>
#include <folly/lang/CheckedMath.h>
#include <cstring>

namespace fizz::libaegis {
namespace {
class LibAegisCipherBase {
 public:
  virtual ~LibAegisCipherBase() = default;
  virtual void stateInit(
      const uint8_t* ad,
      size_t adlen,
      const uint8_t* npub,
      const uint8_t* k) = 0;
  virtual int encryptUpdate(uint8_t* c, const uint8_t* m, size_t mlen) = 0;
  virtual int encryptFinal(uint8_t* m, size_t maclen) = 0;
  virtual int decryptUpdate(uint8_t* m, const uint8_t* c, size_t clen) = 0;
  virtual int decryptFinal(const uint8_t* mac, size_t maclen) = 0;
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
      const uint8_t* k) override {
    return Impl::stateInit(&state_, ad, adlen, npub, k);
  }
  int encryptUpdate(uint8_t* c, const uint8_t* m, size_t mlen) override {
    return Impl::encryptUpdate(&state_, c, m, mlen);
  }
  int encryptFinal(uint8_t* mac, size_t maclen) override {
    return Impl::encryptFinal(&state_, mac, maclen);
  }
  int decryptUpdate(uint8_t* m, const uint8_t* c, size_t clen) override {
    return Impl::decryptUpdate(&state_, m, c, clen);
  }
  int decryptFinal(const uint8_t* mac, size_t maclen) override {
    return Impl::decryptFinal(&state_, mac, maclen);
  }
  typename Impl::stateType state_;
};
#define DEFINE_LIBAEGIS_CIPHER(STRUCT_NAME, CIPHER)                     \
  struct STRUCT_NAME {                                                  \
    using stateType = CIPHER##_state;                                   \
    static constexpr auto stateInit{CIPHER##_state_init};               \
    static constexpr auto encryptUpdate{CIPHER##_state_encrypt_update}; \
    static constexpr auto encryptFinal{CIPHER##_state_encrypt_final};   \
    static constexpr auto decryptUpdate{CIPHER##_state_decrypt_update}; \
    static constexpr auto decryptFinal{CIPHER##_state_decrypt_final};   \
  };

DEFINE_LIBAEGIS_CIPHER(AEGIS128L, aegis128l)
DEFINE_LIBAEGIS_CIPHER(AEGIS256, aegis256)

class AEGISCipher : public Aead {
 public:
  static constexpr size_t kMaxIVLength = 32;
  static constexpr size_t kTagLength = 16;

  static Status create(
      std::unique_ptr<Aead>& ret,
      Error& err,
      std::unique_ptr<LibAegisCipherBase> impl,
      size_t keyLength,
      size_t ivLength);

  Status setKey(Error& err, TrafficKey trafficKey) override;
  folly::Optional<TrafficKey> getKey() const override;

  size_t keyLength() const override {
    return keyLength_;
  }

  size_t ivLength() const override {
    return ivLength_;
  }

  Status doEncrypt(
      std::unique_ptr<folly::IOBuf>& ret,
      Error& err,
      std::unique_ptr<folly::IOBuf>&& plaintext,
      const folly::IOBuf* associatedData,
      folly::ByteRange iv,
      Aead::AeadOptions options) const;

  Status doDecrypt(
      folly::Optional<std::unique_ptr<folly::IOBuf>>& ret,
      Error& err,
      std::unique_ptr<folly::IOBuf>&& ciphertext,
      const folly::IOBuf* associatedData,
      folly::ByteRange iv,
      folly::MutableByteRange tagOut,
      bool inPlace) const;

  Status encrypt(
      std::unique_ptr<folly::IOBuf>& ret,
      Error& err,
      std::unique_ptr<folly::IOBuf>&& plaintext,
      const folly::IOBuf* associatedData,
      uint64_t seqNum,
      Aead::AeadOptions options) const override;

  Status encrypt(
      std::unique_ptr<folly::IOBuf>& ret,
      Error& err,
      std::unique_ptr<folly::IOBuf>&& plaintext,
      const folly::IOBuf* associatedData,
      folly::ByteRange nonce,
      Aead::AeadOptions options) const override;

  Status inplaceEncrypt(
      std::unique_ptr<folly::IOBuf>& ret,
      Error& err,
      std::unique_ptr<folly::IOBuf>&& plaintext,
      const folly::IOBuf* associatedData,
      uint64_t seqNum) const override;

  Status tryDecrypt(
      folly::Optional<std::unique_ptr<folly::IOBuf>>& ret,
      Error& err,
      std::unique_ptr<folly::IOBuf>&& ciphertext,
      const folly::IOBuf* associatedData,
      uint64_t seqNum,
      Aead::AeadOptions options) const override;

  Status tryDecrypt(
      folly::Optional<std::unique_ptr<folly::IOBuf>>& ret,
      Error& err,
      std::unique_ptr<folly::IOBuf>&& ciphertext,
      const folly::IOBuf* associatedData,
      folly::ByteRange nonce,
      Aead::AeadOptions options) const override;

  size_t getCipherOverhead() const override;

  void setEncryptedBufferHeadroom(size_t headroom) override {
    headroom_ = headroom;
  }

 private:
  std::unique_ptr<LibAegisCipherBase> impl_;
  TrafficKey trafficKey_;
  folly::ByteRange trafficIvKey_;
  folly::ByteRange trafficKeyKey_;
  /* When allocating a new IOBUF for encryption, we need to save 5 bytes of
  headroom for TLS 1.3 record header. */
  size_t headroom_{5};

  // set by the ctor
  size_t keyLength_;
  size_t ivLength_;

  AEGISCipher(
      std::unique_ptr<LibAegisCipherBase> impl,
      size_t keyLength,
      size_t ivLength);
};

static_assert(
    AEGISCipher::kMaxIVLength == aegis256_NPUBBYTES,
    "Invalid AEGISCipher::kMaxIVLength");

Status AEGISCipher::doEncrypt(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    std::unique_ptr<folly::IOBuf>&& plaintext,
    const folly::IOBuf* associatedData,
    folly::ByteRange iv,
    Aead::AeadOptions options) const {
  struct AeadImpl {
    const AEGISCipher& self;
    unsigned char tagTemp[kTagLength];

    explicit AeadImpl(const AEGISCipher& s) : self(s) {}
    Status init(
        Error& /* err */,
        folly::ByteRange iv,
        const folly::IOBuf* associatedData,
        size_t /* plaintextLength */) {
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
      // the stateInit function will skip adding aad to the state when adData is
      // null and adLen is 0
      // @lint-ignore CLANGTIDY facebook-hte-NullableDereference
      self.impl_->stateInit(
          adData, adLen, iv.data(), self.trafficKeyKey_.data());
      return Status::Success;
    }

    Status
    encrypt(Error& err, folly::IOBuf& plaintext, folly::IOBuf& ciphertext) {
      Status status = Status::Success;
      transformBuffer(
          plaintext,
          ciphertext,
          [&](uint8_t* out, const uint8_t* in, size_t len) {
            if (status != Status::Success) {
              return;
            }
            if (self.impl_->encryptUpdate(out, in, len) != 0) {
              status = err.error("Encryption error");
              return;
            }
          });
      if (status != Status::Success) {
        return status;
      }

      if (self.impl_->encryptFinal(tagTemp, kTagLength) != 0) {
        return err.error("Encryption error");
      }
      return Status::Success;
    }

    Status final(Error& /* err */, int tagLen, void* tagOut) {
      memcpy(tagOut, tagTemp, tagLen);
      return Status::Success;
    }
  };

  FIZZ_RETURN_ON_ERROR(encryptHelper(
      ret,
      err,
      AeadImpl{*this},
      std::move(plaintext),
      associatedData,
      iv,
      kTagLength,
      headroom_,
      options));
  return Status::Success;
}

Status AEGISCipher::doDecrypt(
    folly::Optional<std::unique_ptr<folly::IOBuf>>& ret,
    Error& err,
    std::unique_ptr<folly::IOBuf>&& ciphertext,
    const folly::IOBuf* associatedData,
    folly::ByteRange iv,
    folly::MutableByteRange tagOut,
    bool inPlace) const {
  struct AeadImpl {
    const AEGISCipher& self;

    explicit AeadImpl(const AEGISCipher& s) : self(s) {}
    Status init(
        Error& /* err */,
        folly::ByteRange iv,
        const folly::IOBuf* associatedData,
        size_t /* ciphertextLength */) {
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

      // the stateInit function will skip adding aad to the state when adData is
      // null and adLen is 0
      // @lint-ignore CLANGTIDY facebook-hte-NullableDereference
      self.impl_->stateInit(
          adData, adLen, iv.data(), self.trafficKeyKey_.data());
      return Status::Success;
    }

    Status decryptAndFinal(
        bool& ret,
        Error& err,
        folly::IOBuf& ciphertext,
        folly::IOBuf& plaintext,
        folly::MutableByteRange tagOut) {
      Status status = Status::Success;
      transformBuffer(
          ciphertext,
          plaintext,
          [&](uint8_t* out, const uint8_t* in, size_t len) {
            if (status != Status::Success) {
              return;
            }
            if (self.impl_->decryptUpdate(out, in, len) != 0) {
              status = err.error("Decryption error");
              return;
            }
          });
      if (status != Status::Success) {
        return status;
      }

      ret = self.impl_->decryptFinal(tagOut.data(), kTagLength) == 0;
      return Status::Success;
    }
  };
  FIZZ_RETURN_ON_ERROR(decryptHelper(
      ret,
      err,
      AeadImpl{*this},
      std::move(ciphertext),
      associatedData,
      iv,
      tagOut,
      inPlace));
  return Status::Success;
}

AEGISCipher::AEGISCipher(
    std::unique_ptr<LibAegisCipherBase> impl,
    size_t keyLength,
    size_t ivLength)
    : impl_(std::move(impl)), keyLength_(keyLength), ivLength_(ivLength) {}

Status AEGISCipher::create(
    std::unique_ptr<Aead>& ret,
    Error& err,
    std::unique_ptr<LibAegisCipherBase> impl,
    size_t keyLength,
    size_t ivLength) {
  // aegis_init is safe to call multiple times. It populates libaegis's cpu
  // feature vector
  static int initResult = aegis_init();
  if (initResult == -1) {
    return err.error("failed to initialize libaegis");
  }
  ret = std::unique_ptr<Aead>(
      new AEGISCipher(std::move(impl), keyLength, ivLength));
  return Status::Success;
}

Status AEGISCipher::setKey(Error& err, TrafficKey trafficKey) {
  trafficKey.key->coalesce();
  trafficKey.iv->coalesce();
  if (trafficKey.key->length() != keyLength_) {
    return err.error("Invalid key");
  }
  if (trafficKey.iv->length() != ivLength_) {
    return err.error("Invalid IV");
  }
  trafficKey_ = std::move(trafficKey);
  // Cache the iv and key. calling coalesce() is not free.
  trafficIvKey_ = trafficKey_.iv->coalesce();
  trafficKeyKey_ = trafficKey_.key->coalesce();
  return Status::Success;
}

folly::Optional<TrafficKey> AEGISCipher::getKey() const {
  if (!trafficKey_.key || !trafficKey_.iv) {
    return folly::none;
  }
  return trafficKey_.clone();
}

Status AEGISCipher::encrypt(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    std::unique_ptr<folly::IOBuf>&& plaintext,
    const folly::IOBuf* associatedData,
    uint64_t seqNum,
    Aead::AeadOptions options) const {
  auto iv = ::fizz::createIV<AEGISCipher::kMaxIVLength>(
      seqNum, ivLength_, trafficIvKey_);
  return encrypt(
      ret,
      err,
      std::move(plaintext),
      associatedData,
      folly::ByteRange(iv.data(), ivLength_),
      options);
}

Status AEGISCipher::encrypt(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    std::unique_ptr<folly::IOBuf>&& plaintext,
    const folly::IOBuf* associatedData,
    folly::ByteRange nonce,
    Aead::AeadOptions options) const {
  return doEncrypt(
      ret, err, std::move(plaintext), associatedData, nonce, options);
}

// TODO: (T136805571) We will add implementation for inplace encryption later
Status AEGISCipher::inplaceEncrypt(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    std::unique_ptr<folly::IOBuf>&& plaintext,
    const folly::IOBuf* associatedData,
    uint64_t seqNum) const {
  auto iv =
      createIV<AEGISCipher::kMaxIVLength>(seqNum, ivLength_, trafficIvKey_);
  return doEncrypt(
      ret,
      err,
      std::move(plaintext),
      associatedData,
      folly::ByteRange(iv.data(), ivLength_),
      {Aead::BufferOption::AllowFullModification,
       Aead::AllocationOption::Deny});
}

Status AEGISCipher::tryDecrypt(
    folly::Optional<std::unique_ptr<folly::IOBuf>>& ret,
    Error& err,
    std::unique_ptr<folly::IOBuf>&& ciphertext,
    const folly::IOBuf* associatedData,
    uint64_t seqNum,
    Aead::AeadOptions options) const {
  auto iv = ::fizz::createIV<AEGISCipher::kMaxIVLength>(
      seqNum, ivLength_, trafficIvKey_);
  return tryDecrypt(
      ret,
      err,
      std::move(ciphertext),
      associatedData,
      folly::ByteRange(iv.data(), ivLength_),
      options);
}

Status AEGISCipher::tryDecrypt(
    folly::Optional<std::unique_ptr<folly::IOBuf>>& ret,
    Error& err,
    std::unique_ptr<folly::IOBuf>&& ciphertext,
    const folly::IOBuf* associatedData,
    folly::ByteRange nonce,
    Aead::AeadOptions options) const {
  if (kTagLength > ciphertext->computeChainDataLength()) {
    ret = folly::none;
    return Status::Success;
  }

  auto inPlace =
      (!ciphertext->isShared() ||
       options.bufferOpt != Aead::BufferOption::RespectSharedPolicy);

  if (!inPlace && options.allocOpt == Aead::AllocationOption::Deny) {
    return err.error("Unable to decrypt (no-alloc requires in-place)");
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
      return err.error(
          "Unable to decrypt (tag is fragmented and no allocation allowed)");
    }
    // buffer to copy the tag into when we decrypt
    tagOut = {tag.data(), kTagLength};
    trimBytes(*ciphertext, tagOut);
  }
  return doDecrypt(
      ret, err, std::move(ciphertext), associatedData, nonce, tagOut, inPlace);
}

size_t AEGISCipher::getCipherOverhead() const {
  return kTagLength;
}
} // namespace

template <>
Status makeCipher<fizz::AEGIS128L>(std::unique_ptr<Aead>& ret, Error& err) {
  auto impl =
      std::unique_ptr<LibAegisCipherBase>(new LibAegisCipher<AEGIS128L>());
  return AEGISCipher::create(
      ret, err, std::move(impl), aegis128l_KEYBYTES, aegis128l_NPUBBYTES);
}

template <>
Status makeCipher<fizz::AEGIS256>(std::unique_ptr<Aead>& ret, Error& err) {
  auto impl =
      std::unique_ptr<LibAegisCipherBase>(new LibAegisCipher<AEGIS256>());
  return AEGISCipher::create(
      ret, err, std::move(impl), aegis256_KEYBYTES, aegis256_NPUBBYTES);
}
} // namespace fizz::libaegis
#endif
