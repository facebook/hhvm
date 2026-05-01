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

Status HpkeContextImpl::incrementSeq(Error& err) {
  if (seqNum_ >= (UINT64_MAX - 1)) {
    return err.error("MessageLimitReachedError: When incrementing seqNum");
  }
  seqNum_ += 1;
  return Status::Success;
}

Status HpkeContextImpl::seal(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    const folly::IOBuf* aad,
    std::unique_ptr<folly::IOBuf> pt) {
  if (role_ != Role::Sender) {
    return err.error(
        "sealing can only be done from a sender context",
        folly::none,
        Error::Category::StdLogic);
  }
  FIZZ_RETURN_ON_ERROR(cipher_->encrypt(ret, err, std::move(pt), aad, seqNum_));
  FIZZ_RETURN_ON_ERROR(incrementSeq(err));
  return Status::Success;
}

Status HpkeContextImpl::open(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    const folly::IOBuf* aad,
    std::unique_ptr<folly::IOBuf> ct) {
  if (role_ != Role::Receiver) {
    return err.error(
        "opening can only be done from a receiver context",
        folly::none,
        Error::Category::StdLogic);
  }
  FIZZ_RETURN_ON_ERROR(cipher_->decrypt(ret, err, std::move(ct), aad, seqNum_));
  FIZZ_RETURN_ON_ERROR(incrementSeq(err));
  return Status::Success;
}

Status HpkeContextImpl::exportSecret(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    std::unique_ptr<folly::IOBuf> exporterContext,
    size_t desiredLength) const {
  auto maxL = 255 * hkdf_->hashLength();
  if (desiredLength > maxL) {
    return err.error("desired length for exported secret exceeds maximum");
  }
  FIZZ_RETURN_ON_ERROR(hkdf_->labeledExpand(
      ret,
      err,
      exporterSecret_->coalesce(),
      folly::ByteRange(folly::StringPiece("sec")),
      std::move(exporterContext),
      desiredLength,
      suiteId_->clone()));
  return Status::Success;
}

std::unique_ptr<folly::IOBuf> HpkeContextImpl::getExporterSecret() {
  return exporterSecret_->clone();
}

} // namespace hpke
} // namespace fizz
