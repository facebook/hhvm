/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/hpke/Context.h>
#include <fizz/crypto/hpke/Utils.h>

namespace fizz {
namespace hpke {

HpkeContextImpl::HpkeContextImpl(
    std::unique_ptr<Aead> cipher,
    std::unique_ptr<folly::IOBuf> exporterSecret,
    std::unique_ptr<fizz::hpke::Hkdf> hkdf,
    HpkeSuiteId suiteId,
    uint64_t seqNum,
    HpkeContext::Role role)
    : seqNum_(seqNum),
      cipher_(std::move(cipher)),
      exporterSecret_(std::move(exporterSecret)),
      hkdf_(std::move(hkdf)),
      suiteId_(std::move(suiteId)),
      role_(role) {}

HpkeContextImpl::HpkeContextImpl(
    std::unique_ptr<Aead> cipher,
    std::unique_ptr<folly::IOBuf> exporterSecret,
    std::unique_ptr<fizz::hpke::Hkdf> hkdf,
    HpkeSuiteId suiteId,
    HpkeContext::Role role)
    : HpkeContextImpl(
          std::move(cipher),
          std::move(exporterSecret),
          std::move(hkdf),
          std::move(suiteId),
          0,
          role) {}

void HpkeContextImpl::incrementSeq() {
  if (seqNum_ >= (UINT64_MAX - 1)) {
    throw std::runtime_error("NonceOverflowError: When incrementing seqNum");
  }
  seqNum_ += 1;
}

std::unique_ptr<folly::IOBuf> HpkeContextImpl::seal(
    const folly::IOBuf* aad,
    std::unique_ptr<folly::IOBuf> pt) {
  if (role_ != Role::Sender) {
    throw std::logic_error("sealing can only be done from a sender context");
  }
  std::unique_ptr<folly::IOBuf> ct =
      cipher_->encrypt(std::move(pt), aad, seqNum_);
  incrementSeq();
  return ct;
}

std::unique_ptr<folly::IOBuf> HpkeContextImpl::open(
    const folly::IOBuf* aad,
    std::unique_ptr<folly::IOBuf> ct) {
  if (role_ != Role::Receiver) {
    throw std::logic_error("opening can only be done from a receiver context");
  }
  std::unique_ptr<folly::IOBuf> pt =
      cipher_->decrypt(std::move(ct), aad, seqNum_);
  incrementSeq();
  return pt;
}

std::unique_ptr<folly::IOBuf> HpkeContextImpl::exportSecret(
    std::unique_ptr<folly::IOBuf> exporterContext,
    size_t desiredLength) const {
  auto maxL = 255 * hkdf_->hashLength();
  if (desiredLength > maxL) {
    throw std::runtime_error(
        "desired length for exported secret exceeds maximum");
  }
  return hkdf_->labeledExpand(
      exporterSecret_->coalesce(),
      folly::ByteRange(folly::StringPiece("sec")),
      std::move(exporterContext),
      desiredLength,
      suiteId_->clone());
}

std::unique_ptr<folly::IOBuf> HpkeContextImpl::getExporterSecret() {
  return exporterSecret_->clone();
}

} // namespace hpke
} // namespace fizz
