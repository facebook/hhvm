/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/backend/openssl/crypto/aead/OpenSSLEVPCipher.h>
#include <fizz/crypto/aead/CryptoUtil.h>
#include <folly/lang/CheckedMath.h>

namespace fizz {
namespace openssl {

namespace {
Status encFunc(
    Error& err,
    EVP_CIPHER_CTX* encryptCtx,
    const folly::IOBuf& plaintext,
    folly::IOBuf& output) {
  int numWritten = 0;
  int outLen = 0;
  Status status = Status::Success;
  transformBuffer(
      plaintext,
      output,
      [&](uint8_t* cipher, const uint8_t* plain, size_t len) {
        if (status != Status::Success) {
          return;
        }
        if (len > std::numeric_limits<int>::max()) {
          status = err.error("Encryption error: too much plain text");
          return;
        }
        if (len == 0) {
          return;
        }
        if (EVP_EncryptUpdate(
                encryptCtx, cipher, &outLen, plain, static_cast<int>(len)) !=
            1) {
          status = err.error("Encryption error");
          return;
        }
        numWritten += outLen;
      });
  if (status != Status::Success) {
    return status;
  }
  // We don't expect any writes at the end
  if (EVP_EncryptFinal_ex(
          encryptCtx, output.writableData() + numWritten, &outLen) != 1) {
    return err.error("Encryption error");
  }
  return Status::Success;
}

Status decFunc(
    bool& ret,
    Error& err,
    EVP_CIPHER_CTX* decryptCtx,
    const folly::IOBuf& ciphertext,
    folly::IOBuf& output,
    folly::MutableByteRange tagOut) {
  int numWritten = 0;
  int outLen = 0;
  Status status = Status::Success;
  transformBuffer(
      ciphertext,
      output,
      [&](uint8_t* plain, const uint8_t* cipher, size_t len) {
        if (status != Status::Success) {
          return;
        }
        if (len > std::numeric_limits<int>::max()) {
          status = err.error("Decryption error: too much cipher text");
          return;
        }
        if (EVP_DecryptUpdate(
                decryptCtx, plain, &outLen, cipher, static_cast<int>(len)) !=
            1) {
          status = err.error("Decryption error");
          return;
        }
        numWritten += outLen;
      });
  if (status != Status::Success) {
    return status;
  }

  auto tagLen = tagOut.size();
  if (EVP_CIPHER_CTX_ctrl(
          decryptCtx,
          EVP_CTRL_GCM_SET_TAG,
          tagLen,
          static_cast<void*>(tagOut.begin())) != 1) {
    return err.error("Decryption error");
  }
  ret = EVP_DecryptFinal_ex(
            decryptCtx, output.writableData() + numWritten, &outLen) == 1;
  return Status::Success;
}

Status evpEncrypt(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    std::unique_ptr<folly::IOBuf>&& plaintext,
    const folly::IOBuf* associatedData,
    folly::ByteRange iv,
    size_t tagLen,
    bool useBlockOps,
    size_t headroom,
    EVP_CIPHER_CTX* encryptCtx,
    Aead::AeadOptions options) {
  struct AeadImpl {
    EVP_CIPHER_CTX* encryptCtx;
    bool useBlockOps;

    AeadImpl(EVP_CIPHER_CTX* e, bool u) : encryptCtx(e), useBlockOps(u) {}

    Status init(
        Error& err,
        folly::ByteRange iv,
        const folly::IOBuf* associatedData,
        size_t /*plaintextLength*/) {
      if (EVP_EncryptInit_ex(
              encryptCtx, nullptr, nullptr, nullptr, iv.data()) != 1) {
        return err.error("Encryption error");
      }
      if (associatedData) {
        for (auto current : *associatedData) {
          if (current.size() > std::numeric_limits<int>::max()) {
            return err.error("too much associated data");
          }
          int len;
          if (EVP_EncryptUpdate(
                  encryptCtx,
                  nullptr,
                  &len,
                  current.data(),
                  static_cast<int>(current.size())) != 1) {
            return err.error("Encryption error");
          }
        }
      }
      return Status::Success;
    }

    Status
    encrypt(Error& err, folly::IOBuf& plaintext, folly::IOBuf& ciphertext) {
      if (useBlockOps) {
        struct EVPEncImpl {
          EVP_CIPHER_CTX* encryptCtx;

          explicit EVPEncImpl(EVP_CIPHER_CTX* e) : encryptCtx(e) {}
          bool encryptUpdate(
              uint8_t* cipher,
              int* outLen,
              const uint8_t* plain,
              size_t len) {
            return EVP_EncryptUpdate(
                       encryptCtx,
                       cipher,
                       outLen,
                       plain,
                       static_cast<int>(len)) == 1;
          }
          bool encryptFinal(uint8_t* cipher, int* outLen) {
            return EVP_EncryptFinal_ex(encryptCtx, cipher, outLen) == 1;
          }
        };
        FIZZ_RETURN_ON_ERROR(encFuncBlocks(
            err, EVPEncImpl{encryptCtx}, plaintext, ciphertext, 16));
      } else {
        FIZZ_RETURN_ON_ERROR(encFunc(err, encryptCtx, plaintext, ciphertext));
      }
      return Status::Success;
    }

    Status final(Error& err, int tagLen, void* tagOut) {
      if (EVP_CIPHER_CTX_ctrl(
              encryptCtx, EVP_CTRL_GCM_GET_TAG, tagLen, tagOut) != 1) {
        return err.error("Encryption error");
      }
      return Status::Success;
    }
  };
  FIZZ_RETURN_ON_ERROR(encryptHelper(
      ret,
      err,
      AeadImpl{encryptCtx, useBlockOps},
      std::move(plaintext),
      associatedData,
      iv,
      tagLen,
      headroom,
      options));
  return Status::Success;
}

Status evpDecrypt(
    folly::Optional<std::unique_ptr<folly::IOBuf>>& ret,
    Error& err,
    std::unique_ptr<folly::IOBuf>&& ciphertext,
    const folly::IOBuf* associatedData,
    folly::ByteRange iv,
    folly::MutableByteRange tagOut,
    bool useBlockOps,
    EVP_CIPHER_CTX* decryptCtx,
    bool inPlace) {
  struct AeadImpl {
    EVP_CIPHER_CTX* decryptCtx;
    bool useBlockOps;

    AeadImpl(EVP_CIPHER_CTX* d, bool u) : decryptCtx(d), useBlockOps(u) {}

    Status init(
        Error& err,
        folly::ByteRange iv,
        const folly::IOBuf* associatedData,
        size_t /*ciphertextLength*/) {
      if (EVP_DecryptInit_ex(
              decryptCtx, nullptr, nullptr, nullptr, iv.data()) != 1) {
        return err.error("Decryption error");
      }

      if (associatedData) {
        for (auto current : *associatedData) {
          if (current.size() > std::numeric_limits<int>::max()) {
            return err.error("too much associated data");
          }
          int len;
          if (EVP_DecryptUpdate(
                  decryptCtx,
                  nullptr,
                  &len,
                  current.data(),
                  static_cast<int>(current.size())) != 1) {
            return err.error("Decryption error");
          }
        }
      }
      return Status::Success;
    }

    Status decryptAndFinal(
        bool& ret,
        Error& err,
        folly::IOBuf& ciphertext,
        folly::IOBuf& plaintext,
        folly::MutableByteRange tagOut) {
      if (useBlockOps) {
        struct EVPDecImpl {
          EVP_CIPHER_CTX* decryptCtx;

          explicit EVPDecImpl(EVP_CIPHER_CTX* d) : decryptCtx(d) {}
          bool decryptUpdate(
              uint8_t* plain,
              const uint8_t* cipher,
              size_t len,
              int* outLen) {
            return EVP_DecryptUpdate(
                       decryptCtx,
                       plain,
                       outLen,
                       cipher,
                       static_cast<int>(len)) == 1;
          }
          bool setExpectedTag(int tagSize, unsigned char* tag) {
            return EVP_CIPHER_CTX_ctrl(
                       decryptCtx,
                       EVP_CTRL_GCM_SET_TAG,
                       tagSize,
                       static_cast<void*>(tag)) == 1;
          }
          bool decryptFinal(unsigned char* outm, int* outLen) {
            return EVP_DecryptFinal_ex(decryptCtx, outm, outLen) == 1;
          }
        };
        FIZZ_RETURN_ON_ERROR(decFuncBlocks(
            ret,
            err,
            EVPDecImpl{decryptCtx},
            ciphertext,
            plaintext,
            tagOut,
            16));
      } else {
        FIZZ_RETURN_ON_ERROR(
            decFunc(ret, err, decryptCtx, ciphertext, plaintext, tagOut));
      }
      return Status::Success;
    }
  };

  FIZZ_RETURN_ON_ERROR(decryptHelper(
      ret,
      err,
      AeadImpl{decryptCtx, useBlockOps},
      std::move(ciphertext),
      associatedData,
      iv,
      tagOut,
      inPlace));
  return Status::Success;
}
} // namespace

OpenSSLEVPCipher::OpenSSLEVPCipher(
    size_t keyLength,
    size_t ivLength,
    size_t tagLength,
    bool operatesInBlocks)
    : keyLength_(keyLength),
      ivLength_(ivLength),
      tagLength_(tagLength),
      operatesInBlocks_(operatesInBlocks) {}

Status OpenSSLEVPCipher::create(
    std::unique_ptr<OpenSSLEVPCipher>& ret,
    Error& err,
    size_t keyLength,
    size_t ivLength,
    size_t tagLength,
    const EVP_CIPHER* cipher,
    bool operatesInBlocks,
    bool requiresPresetTagLen) {
  auto obj = std::unique_ptr<OpenSSLEVPCipher>(
      new OpenSSLEVPCipher(keyLength, ivLength, tagLength, operatesInBlocks));
  obj->encryptCtx_.reset(EVP_CIPHER_CTX_new());
  if (obj->encryptCtx_ == nullptr) {
    return err.error("Unable to allocate an EVP_CIPHER_CTX object");
  }
  obj->decryptCtx_.reset(EVP_CIPHER_CTX_new());
  if (obj->decryptCtx_ == nullptr) {
    return err.error("Unable to allocate an EVP_CIPHER_CTX object");
  }
  if (EVP_EncryptInit_ex(
          obj->encryptCtx_.get(), cipher, nullptr, nullptr, nullptr) != 1) {
    return err.error("Init error");
  }
  if (EVP_CIPHER_CTX_ctrl(
          obj->encryptCtx_.get(), EVP_CTRL_GCM_SET_IVLEN, ivLength, nullptr) !=
      1) {
    return err.error("Error setting iv length");
  }
  if (EVP_DecryptInit_ex(
          obj->decryptCtx_.get(), cipher, nullptr, nullptr, nullptr) != 1) {
    return err.error("Init error");
  }
  if (EVP_CIPHER_CTX_ctrl(
          obj->decryptCtx_.get(), EVP_CTRL_GCM_SET_IVLEN, ivLength, nullptr) !=
      1) {
    return err.error("Error setting iv length");
  }

  if (requiresPresetTagLen) {
    if (EVP_CIPHER_CTX_ctrl(
            obj->encryptCtx_.get(), EVP_CTRL_GCM_SET_TAG, tagLength, nullptr) !=
        1) {
      return err.error("Error setting enc tag length");
    }

    if (EVP_CIPHER_CTX_ctrl(
            obj->decryptCtx_.get(), EVP_CTRL_GCM_SET_TAG, tagLength, nullptr) !=
        1) {
      return err.error("Error setting dec tag length");
    }
  }
  ret = std::move(obj);
  return Status::Success;
}

Status OpenSSLEVPCipher::setKey(Error& err, TrafficKey trafficKey) {
  trafficKey.key->coalesce();
  trafficKey.iv->coalesce();
  if (trafficKey.key->length() != keyLength_) {
    return err.error("Invalid key");
  }
  if (trafficKey.iv->length() != ivLength_) {
    return err.error("Invalid IV");
  }
  trafficKey_ = std::move(trafficKey);
  // Cache the IV key. calling coalesce() is not free.
  trafficIvKey_ = trafficKey_.iv->coalesce();
  if (EVP_EncryptInit_ex(
          encryptCtx_.get(),
          nullptr,
          nullptr,
          trafficKey_.key->data(),
          nullptr) != 1) {
    return err.error("Error setting encrypt key");
  }
  if (EVP_DecryptInit_ex(
          decryptCtx_.get(),
          nullptr,
          nullptr,
          trafficKey_.key->data(),
          nullptr) != 1) {
    return err.error("Error setting decrypt key");
  }
  return Status::Success;
}

folly::Optional<TrafficKey> OpenSSLEVPCipher::getKey() const {
  if (!trafficKey_.key || !trafficKey_.iv) {
    return folly::none;
  }
  return trafficKey_.clone();
}

Status OpenSSLEVPCipher::encrypt(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    std::unique_ptr<folly::IOBuf>&& plaintext,
    const folly::IOBuf* associatedData,
    uint64_t seqNum,
    Aead::AeadOptions options) const {
  auto iv = createIV<OpenSSLEVPCipher::kMaxIVLength>(
      seqNum, ivLength_, trafficIvKey_);
  return encrypt(
      ret,
      err,
      std::move(plaintext),
      associatedData,
      folly::ByteRange(iv.data(), ivLength_),
      options);
}

Status OpenSSLEVPCipher::encrypt(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    std::unique_ptr<folly::IOBuf>&& plaintext,
    const folly::IOBuf* associatedData,
    folly::ByteRange nonce,
    Aead::AeadOptions options) const {
  return evpEncrypt(
      ret,
      err,
      std::move(plaintext),
      associatedData,
      nonce,
      tagLength_,
      operatesInBlocks_,
      headroom_,
      encryptCtx_.get(),
      options);
}

Status OpenSSLEVPCipher::inplaceEncrypt(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    std::unique_ptr<folly::IOBuf>&& plaintext,
    const folly::IOBuf* associatedData,
    uint64_t seqNum) const {
  auto iv = createIV<OpenSSLEVPCipher::kMaxIVLength>(
      seqNum, ivLength_, trafficIvKey_);
  return evpEncrypt(
      ret,
      err,
      std::move(plaintext),
      associatedData,
      folly::ByteRange(iv.data(), ivLength_),
      tagLength_,
      operatesInBlocks_,
      headroom_,
      encryptCtx_.get(),
      {Aead::BufferOption::AllowFullModification,
       Aead::AllocationOption::Deny});
}

Status OpenSSLEVPCipher::tryDecrypt(
    folly::Optional<std::unique_ptr<folly::IOBuf>>& ret,
    Error& err,
    std::unique_ptr<folly::IOBuf>&& ciphertext,
    const folly::IOBuf* associatedData,
    uint64_t seqNum,
    Aead::AeadOptions options) const {
  auto iv = createIV<OpenSSLEVPCipher::kMaxIVLength>(
      seqNum, ivLength_, trafficIvKey_);
  return tryDecrypt(
      ret,
      err,
      std::move(ciphertext),
      associatedData,
      folly::ByteRange(iv.data(), ivLength_),
      options);
}

Status OpenSSLEVPCipher::tryDecrypt(
    folly::Optional<std::unique_ptr<folly::IOBuf>>& ret,
    Error& err,
    std::unique_ptr<folly::IOBuf>&& ciphertext,
    const folly::IOBuf* associatedData,
    folly::ByteRange nonce,
    Aead::AeadOptions options) const {
  // Check that there's enough data to decrypt
  if (tagLength_ > ciphertext->computeChainDataLength()) {
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
  if (lastBuf->length() >= tagLength_) {
    // We can directly carve out this buffer from the last IOBuf
    // Adjust buffer sizes
    lastBuf->trimEnd(tagLength_);

    folly::MutableByteRange tagOut{lastBuf->writableTail(), tagLength_};
    return evpDecrypt(
        ret,
        err,
        std::move(ciphertext),
        associatedData,
        nonce,
        tagOut,
        operatesInBlocks_,
        decryptCtx_.get(),
        inPlace);
  } else {
    // Tag is fragmented so we need to copy it out.
    if (options.allocOpt == Aead::AllocationOption::Deny) {
      return err.error(
          "Unable to decrypt (tag is fragmented and no allocation allowed)");
    }
    std::array<uint8_t, kMaxTagLength> tag;
    // buffer to copy the tag into when we decrypt
    folly::MutableByteRange tagOut{tag.data(), tagLength_};
    trimBytes(*ciphertext, tagOut);
    return evpDecrypt(
        ret,
        err,
        std::move(ciphertext),
        associatedData,
        nonce,
        tagOut,
        operatesInBlocks_,
        decryptCtx_.get(),
        inPlace);
  }
}

size_t OpenSSLEVPCipher::getCipherOverhead() const {
  return tagLength_;
}
} // namespace openssl
} // namespace fizz
