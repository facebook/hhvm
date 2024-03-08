/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/aead/CryptoUtil.h>
#include <fizz/crypto/aead/OpenSSLEVPCipher.h>
#include <folly/lang/CheckedMath.h>
#include <functional>

namespace fizz {

namespace {
void encFunc(
    EVP_CIPHER_CTX* encryptCtx,
    const folly::IOBuf& plaintext,
    folly::IOBuf& output) {
  int numWritten = 0;
  int outLen = 0;
  transformBuffer(
      plaintext,
      output,
      [&](uint8_t* cipher, const uint8_t* plain, size_t len) {
        if (len > std::numeric_limits<int>::max()) {
          throw std::runtime_error("Encryption error: too much plain text");
        }
        if (len == 0) {
          return;
        }
        if (EVP_EncryptUpdate(
                encryptCtx, cipher, &outLen, plain, static_cast<int>(len)) !=
            1) {
          throw std::runtime_error("Encryption error");
        }
        numWritten += outLen;
      });
  // We don't expect any writes at the end
  if (EVP_EncryptFinal_ex(
          encryptCtx, output.writableData() + numWritten, &outLen) != 1) {
    throw std::runtime_error("Encryption error");
  }
}

bool decFunc(
    EVP_CIPHER_CTX* decryptCtx,
    const folly::IOBuf& ciphertext,
    folly::IOBuf& output,
    folly::MutableByteRange tagOut) {
  int numWritten = 0;
  int outLen = 0;
  transformBuffer(
      ciphertext,
      output,
      [&](uint8_t* plain, const uint8_t* cipher, size_t len) {
        if (len > std::numeric_limits<int>::max()) {
          throw std::runtime_error("Decryption error: too much cipher text");
        }
        if (EVP_DecryptUpdate(
                decryptCtx, plain, &outLen, cipher, static_cast<int>(len)) !=
            1) {
          throw std::runtime_error("Decryption error");
        }
        numWritten += outLen;
      });

  auto tagLen = tagOut.size();
  if (EVP_CIPHER_CTX_ctrl(
          decryptCtx,
          EVP_CTRL_GCM_SET_TAG,
          tagLen,
          static_cast<void*>(tagOut.begin())) != 1) {
    throw std::runtime_error("Decryption error");
  }
  return EVP_DecryptFinal_ex(
             decryptCtx, output.writableData() + numWritten, &outLen) == 1;
}

std::unique_ptr<folly::IOBuf> evpEncrypt(
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

    void init(
        folly::ByteRange iv,
        const folly::IOBuf* associatedData,
        size_t /*plaintextLength*/) {
      if (EVP_EncryptInit_ex(
              encryptCtx, nullptr, nullptr, nullptr, iv.data()) != 1) {
        throw std::runtime_error("Encryption error");
      }
      if (associatedData) {
        for (auto current : *associatedData) {
          if (current.size() > std::numeric_limits<int>::max()) {
            throw std::runtime_error("too much associated data");
          }
          int len;
          if (EVP_EncryptUpdate(
                  encryptCtx,
                  nullptr,
                  &len,
                  current.data(),
                  static_cast<int>(current.size())) != 1) {
            throw std::runtime_error("Encryption error");
          }
        }
      }
    }

    void encrypt(folly::IOBuf& plaintext, folly::IOBuf& ciphertext) {
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
        // TODO(T180781189): make transformBufferBlocks not take a template
        // parameter for the block size.
        encFuncBlocks<16>(EVPEncImpl{encryptCtx}, plaintext, ciphertext);
      } else {
        encFunc(encryptCtx, plaintext, ciphertext);
      }
    }

    void final(int tagLen, void* tagOut) {
      if (EVP_CIPHER_CTX_ctrl(
              encryptCtx, EVP_CTRL_GCM_GET_TAG, tagLen, tagOut) != 1) {
        throw std::runtime_error("Encryption error");
      }
    }
  };
  return encryptHelper(
      AeadImpl{encryptCtx, useBlockOps},
      std::move(plaintext),
      associatedData,
      iv,
      tagLen,
      headroom,
      options);
}

folly::Optional<std::unique_ptr<folly::IOBuf>> evpDecrypt(
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

    void init(
        folly::ByteRange iv,
        const folly::IOBuf* associatedData,
        size_t /*ciphertextLength*/) {
      if (EVP_DecryptInit_ex(
              decryptCtx, nullptr, nullptr, nullptr, iv.data()) != 1) {
        throw std::runtime_error("Decryption error");
      }

      if (associatedData) {
        for (auto current : *associatedData) {
          if (current.size() > std::numeric_limits<int>::max()) {
            throw std::runtime_error("too much associated data");
          }
          int len;
          if (EVP_DecryptUpdate(
                  decryptCtx,
                  nullptr,
                  &len,
                  current.data(),
                  static_cast<int>(current.size())) != 1) {
            throw std::runtime_error("Decryption error");
          }
        }
      }
    }

    bool decryptAndFinal(
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
        return decFuncBlocks<16>(
            EVPDecImpl{decryptCtx}, ciphertext, plaintext, tagOut);
      } else {
        return decFunc(decryptCtx, ciphertext, plaintext, tagOut);
      }
    }
  };

  return decryptHelper(
      AeadImpl{decryptCtx, useBlockOps},
      std::move(ciphertext),
      associatedData,
      iv,
      tagOut,
      inPlace);
}

} // namespace

OpenSSLEVPCipher::OpenSSLEVPCipher(
    size_t keyLength,
    size_t ivLength,
    size_t tagLength,
    const EVP_CIPHER* cipher,
    bool operatesInBlocks,
    bool requiresPresetTagLen)
    : keyLength_(keyLength),
      ivLength_(ivLength),
      tagLength_(tagLength),
      cipher_(cipher),
      operatesInBlocks_(operatesInBlocks),
      requiresPresetTagLen_(requiresPresetTagLen) {
  encryptCtx_.reset(EVP_CIPHER_CTX_new());
  if (encryptCtx_ == nullptr) {
    throw std::runtime_error("Unable to allocate an EVP_CIPHER_CTX object");
  }
  decryptCtx_.reset(EVP_CIPHER_CTX_new());
  if (decryptCtx_ == nullptr) {
    throw std::runtime_error("Unable to allocate an EVP_CIPHER_CTX object");
  }
  if (EVP_EncryptInit_ex(
          encryptCtx_.get(), cipher_, nullptr, nullptr, nullptr) != 1) {
    throw std::runtime_error("Init error");
  }
  if (EVP_CIPHER_CTX_ctrl(
          encryptCtx_.get(), EVP_CTRL_GCM_SET_IVLEN, ivLength_, nullptr) != 1) {
    throw std::runtime_error("Error setting iv length");
  }
  if (EVP_DecryptInit_ex(
          decryptCtx_.get(), cipher_, nullptr, nullptr, nullptr) != 1) {
    throw std::runtime_error("Init error");
  }
  if (EVP_CIPHER_CTX_ctrl(
          decryptCtx_.get(), EVP_CTRL_GCM_SET_IVLEN, ivLength_, nullptr) != 1) {
    throw std::runtime_error("Error setting iv length");
  }

  if (requiresPresetTagLen_) {
    if (EVP_CIPHER_CTX_ctrl(
            encryptCtx_.get(), EVP_CTRL_GCM_SET_TAG, tagLength_, nullptr) !=
        1) {
      throw std::runtime_error("Error setting enc tag length");
    }

    if (EVP_CIPHER_CTX_ctrl(
            decryptCtx_.get(), EVP_CTRL_GCM_SET_TAG, tagLength_, nullptr) !=
        1) {
      throw std::runtime_error("Error setting dec tag length");
    }
  }
}

void OpenSSLEVPCipher::setKey(TrafficKey trafficKey) {
  trafficKey.key->coalesce();
  trafficKey.iv->coalesce();
  if (trafficKey.key->length() != keyLength_) {
    throw std::runtime_error("Invalid key");
  }
  if (trafficKey.iv->length() != ivLength_) {
    throw std::runtime_error("Invalid IV");
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
    throw std::runtime_error("Error setting encrypt key");
  }
  if (EVP_DecryptInit_ex(
          decryptCtx_.get(),
          nullptr,
          nullptr,
          trafficKey_.key->data(),
          nullptr) != 1) {
    throw std::runtime_error("Error setting decrypt key");
  }
}

folly::Optional<TrafficKey> OpenSSLEVPCipher::getKey() const {
  if (!trafficKey_.key || !trafficKey_.iv) {
    return folly::none;
  }
  return trafficKey_.clone();
}

std::unique_ptr<folly::IOBuf> OpenSSLEVPCipher::encrypt(
    std::unique_ptr<folly::IOBuf>&& plaintext,
    const folly::IOBuf* associatedData,
    uint64_t seqNum,
    Aead::AeadOptions options) const {
  auto iv = createIV<OpenSSLEVPCipher::kMaxIVLength>(
      seqNum, ivLength_, trafficIvKey_);
  return encrypt(
      std::move(plaintext),
      associatedData,
      folly::ByteRange(iv.data(), ivLength_),
      options);
}

std::unique_ptr<folly::IOBuf> OpenSSLEVPCipher::encrypt(
    std::unique_ptr<folly::IOBuf>&& plaintext,
    const folly::IOBuf* associatedData,
    folly::ByteRange nonce,
    Aead::AeadOptions options) const {
  return evpEncrypt(
      std::move(plaintext),
      associatedData,
      nonce,
      tagLength_,
      operatesInBlocks_,
      headroom_,
      encryptCtx_.get(),
      options);
}

std::unique_ptr<folly::IOBuf> OpenSSLEVPCipher::inplaceEncrypt(
    std::unique_ptr<folly::IOBuf>&& plaintext,
    const folly::IOBuf* associatedData,
    uint64_t seqNum) const {
  auto iv = createIV<OpenSSLEVPCipher::kMaxIVLength>(
      seqNum, ivLength_, trafficIvKey_);
  return evpEncrypt(
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

folly::Optional<std::unique_ptr<folly::IOBuf>> OpenSSLEVPCipher::tryDecrypt(
    std::unique_ptr<folly::IOBuf>&& ciphertext,
    const folly::IOBuf* associatedData,
    uint64_t seqNum,
    Aead::AeadOptions options) const {
  auto iv = createIV<OpenSSLEVPCipher::kMaxIVLength>(
      seqNum, ivLength_, trafficIvKey_);
  return tryDecrypt(
      std::move(ciphertext),
      associatedData,
      folly::ByteRange(iv.data(), ivLength_),
      options);
}

folly::Optional<std::unique_ptr<folly::IOBuf>> OpenSSLEVPCipher::tryDecrypt(
    std::unique_ptr<folly::IOBuf>&& ciphertext,
    const folly::IOBuf* associatedData,
    folly::ByteRange nonce,
    Aead::AeadOptions options) const {
  // Check that there's enough data to decrypt
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
  if (lastBuf->length() >= tagLength_) {
    // We can directly carve out this buffer from the last IOBuf
    // Adjust buffer sizes
    lastBuf->trimEnd(tagLength_);

    folly::MutableByteRange tagOut{lastBuf->writableTail(), tagLength_};
    return evpDecrypt(
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
      throw std::runtime_error(
          "Unable to decrypt (tag is fragmented and no allocation allowed)");
    }
    std::array<uint8_t, kMaxTagLength> tag;
    // buffer to copy the tag into when we decrypt
    folly::MutableByteRange tagOut{tag.data(), tagLength_};
    trimBytes(*ciphertext, tagOut);
    return evpDecrypt(
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
} // namespace fizz
