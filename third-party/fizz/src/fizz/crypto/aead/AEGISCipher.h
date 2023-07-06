/*
 *  Copyright (c) Meta Platforms, Inc. and affiliates.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <fizz/fizz-config.h>

#if FIZZ_BUILD_AEGIS

#include <fizz/crypto/aead/Aead.h>
#include <fizz/crypto/aead/IOBufUtil.h>
#include <fizz/third-party/libsodium-aegis/aegis.h>
#include <folly/Conv.h>
#include <folly/Memory.h>
#include <folly/Range.h>
#include <folly/String.h>
#include <folly/lang/Bits.h>
#include <folly/ssl/OpenSSLPtrTypes.h>

namespace fizz {
class AEGISCipher : public Aead {
 public:
  using AegisEVPCtx = fizz_aegis_evp_ctx;
  using EncryptFn = int (*const)(
      unsigned char* c,
      unsigned long long* clen_p,
      const unsigned char* m,
      unsigned long long mlen,
      const unsigned char* ad,
      unsigned long long adlen,
      const unsigned char* nsec,
      const unsigned char* npub,
      const unsigned char* k);
  using DecryptFn = int (*const)(
      unsigned char* m,
      unsigned long long* mlen_p,
      unsigned char* nsec,
      const unsigned char* c,
      unsigned long long clen,
      const unsigned char* ad,
      unsigned long long adlen,
      const unsigned char* npub,
      const unsigned char* k);
  using InitStateFn = int (*const)(
      const unsigned char* key,
      const unsigned char* nonce,
      AegisEVPCtx* ctx);
  using AadUpdateFn = int (*const)(
      const unsigned char* ad,
      unsigned long long adlen,
      AegisEVPCtx* ctx);
  using AadFinalFn = int (*const)(AegisEVPCtx* ctx);
  using EncryptUpdateFn = int (*const)(
      unsigned char* c,
      unsigned long long* clen_p,
      const unsigned char* m,
      unsigned long long mlen,
      AegisEVPCtx* ctx);
  using EncryptFinalFn = int (*const)(
      unsigned char* c,
      unsigned long long* c_writtenlen_p,
      unsigned long long mlen,
      unsigned long long adlen,
      AegisEVPCtx* ctx);

  static constexpr size_t kMaxIVLength = 32;

  static std::unique_ptr<Aead> make128L();
  static std::unique_ptr<Aead> make256();

  void setKey(TrafficKey trafficKey) override;
  folly::Optional<TrafficKey> getKey() const override;

  size_t keyLength() const override {
    return keyLength_;
  }

  size_t ivLength() const override {
    return ivLength_;
  }

  std::unique_ptr<folly::IOBuf> encrypt(
      std::unique_ptr<folly::IOBuf>&& plaintext,
      const folly::IOBuf* associatedData,
      uint64_t seqNum,
      Aead::AeadOptions options) const override;

  std::unique_ptr<folly::IOBuf> encrypt(
      std::unique_ptr<folly::IOBuf>&& plaintext,
      const folly::IOBuf* associatedData,
      folly::ByteRange nonce,
      Aead::AeadOptions options) const override;

  // TODO: (T136805571) We will add implementation for inplace encryption later
  std::unique_ptr<folly::IOBuf> inplaceEncrypt(
      std::unique_ptr<folly::IOBuf>&& plaintext,
      const folly::IOBuf* associatedData,
      uint64_t seqNum) const override;

  folly::Optional<std::unique_ptr<folly::IOBuf>> tryDecrypt(
      std::unique_ptr<folly::IOBuf>&& ciphertext,
      const folly::IOBuf* associatedData,
      uint64_t seqNum,
      Aead::AeadOptions options) const override;

  folly::Optional<std::unique_ptr<folly::IOBuf>> tryDecrypt(
      std::unique_ptr<folly::IOBuf>&& ciphertext,
      const folly::IOBuf* associatedData,
      folly::ByteRange nonce,
      Aead::AeadOptions options) const override;

  size_t getCipherOverhead() const override;

  void setEncryptedBufferHeadroom(size_t headroom) override {
    headroom_ = headroom;
  }

 private:
  AEGISCipher(
      DecryptFn decrypt,
      InitStateFn init,
      AadUpdateFn aadUpdate,
      AadFinalFn aadFinal_,
      EncryptUpdateFn encryptUpdate,
      EncryptFinalFn encryptFinal,
      size_t keyLength,
      size_t ivLength,
      size_t tagLength);

  TrafficKey trafficKey_;
  folly::ByteRange trafficIvKey_;
  folly::ByteRange trafficKeyKey_;
  size_t headroom_{0};

  // set by the ctor
  DecryptFn decrypt_;
  InitStateFn initstate_;
  AadUpdateFn aadUpdate_;
  AadFinalFn aadFinal_;
  EncryptUpdateFn encryptUpdate_;
  EncryptFinalFn encryptFinal_;
  AegisEVPCtx ctx_;
  size_t keyLength_;
  size_t ivLength_;
  size_t tagLength_;
};
} // namespace fizz
#endif
