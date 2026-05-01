/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/hpke/DHKEM.h>
#include <folly/io/Cursor.h>

namespace fizz {

static Status getDHKEMId(uint16_t& ret, Error& err, NamedGroup group) {
  switch (group) {
    case NamedGroup::secp256r1:
      ret = 0x0010;
      return Status::Success;
    case NamedGroup::secp384r1:
      ret = 0x0011;
      return Status::Success;
    case NamedGroup::secp521r1:
      ret = 0x0012;
      return Status::Success;
    case NamedGroup::x25519:
      ret = 0x0020;
      return Status::Success;
    default:
      return err.error("ke: not implemented");
  }
}

static Status generateSuiteId(Buf& ret, Error& err, NamedGroup group) {
  std::unique_ptr<folly::IOBuf> buf = folly::IOBuf::copyBuffer("KEM");
  folly::io::Appender appender(buf.get(), 2);
  uint16_t kemId = 0;
  FIZZ_RETURN_ON_ERROR(getDHKEMId(kemId, err, group));
  appender.writeBE<uint16_t>(kemId);
  ret = std::move(buf);
  return Status::Success;
}

DHKEM::DHKEM(
    std::unique_ptr<KeyExchange> kex,
    NamedGroup group,
    std::unique_ptr<fizz::hpke::Hkdf> hkdf)
    : kex_(std::move(kex)), group_(group), hkdf_(std::move(hkdf)) {}

DHKEM::DHKEM(
    std::unique_ptr<KeyExchange> kex,
    std::unique_ptr<KeyExchange> authKex,
    NamedGroup group,
    std::unique_ptr<fizz::hpke::Hkdf> hkdf)
    : kex_(std::move(kex)),
      authKex_(std::move(authKex)),
      group_(group),
      hkdf_(std::move(hkdf)) {}

Status DHKEM::extractAndExpand(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    std::unique_ptr<folly::IOBuf> dh,
    std::unique_ptr<folly::IOBuf> kemContext) {
  Buf suiteId;
  FIZZ_RETURN_ON_ERROR(generateSuiteId(suiteId, err, group_));
  std::vector<uint8_t> eae_prkVec = hkdf_->labeledExtract(
      folly::IOBuf::copyBuffer(""),
      folly::ByteRange(folly::StringPiece("eae_prk")),
      std::move(dh),
      suiteId->clone());
  folly::ByteRange eaePrk(eae_prkVec.data(), eae_prkVec.size());
  FIZZ_RETURN_ON_ERROR(hkdf_->labeledExpand(
      ret,
      err,
      eaePrk,
      folly::ByteRange(folly::StringPiece("shared_secret")),
      std::move(kemContext),
      hkdf_->hashLength(),
      std::move(suiteId)));
  return Status::Success;
}

Status DHKEM::getKEMId(hpke::KEMId& ret, Error& err) const {
  uint16_t id = 0;
  FIZZ_RETURN_ON_ERROR(getDHKEMId(id, err, group_));
  ret = static_cast<hpke::KEMId>(id);
  return Status::Success;
}

Status DHKEM::encap(EncapResult& ret, Error& err, folly::ByteRange pkR) {
  FIZZ_RETURN_ON_ERROR(kex_->generateKeyPair(err));
  std::unique_ptr<folly::IOBuf> dh;
  FIZZ_RETURN_ON_ERROR(kex_->generateSharedSecret(dh, err, pkR));
  std::unique_ptr<folly::IOBuf> enc = kex_->getKeyShare();

  std::unique_ptr<folly::IOBuf> kemContext = enc->clone();
  kemContext->prependChain(folly::IOBuf::copyBuffer(pkR));
  std::unique_ptr<folly::IOBuf> sharedSecret;
  FIZZ_RETURN_ON_ERROR(extractAndExpand(
      sharedSecret, err, std::move(dh), std::move(kemContext)));

  ret = EncapResult{std::move(sharedSecret), std::move(enc)};
  return Status::Success;
}

Status DHKEM::authEncap(EncapResult& ret, Error& err, folly::ByteRange pkR) {
  if (!authKex_) {
    return err.error("DHKEM has no sender key exchange set up");
  }
  FIZZ_RETURN_ON_ERROR(kex_->generateKeyPair(err));
  std::unique_ptr<folly::IOBuf> dh;
  FIZZ_RETURN_ON_ERROR(kex_->generateSharedSecret(dh, err, pkR));
  std::unique_ptr<folly::IOBuf> authDh;
  FIZZ_RETURN_ON_ERROR(authKex_->generateSharedSecret(authDh, err, pkR));
  dh->prependChain(std::move(authDh));
  std::unique_ptr<folly::IOBuf> enc = kex_->getKeyShare();

  std::unique_ptr<folly::IOBuf> kemContext = enc->clone();
  kemContext->prependChain(folly::IOBuf::copyBuffer(pkR));
  kemContext->prependChain(authKex_->getKeyShare());

  std::unique_ptr<folly::IOBuf> sharedSecret;
  FIZZ_RETURN_ON_ERROR(extractAndExpand(
      sharedSecret, err, std::move(dh), std::move(kemContext)));

  ret = EncapResult{std::move(sharedSecret), std::move(enc)};
  return Status::Success;
}

Status DHKEM::decap(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    folly::ByteRange enc) {
  std::unique_ptr<folly::IOBuf> dh;
  FIZZ_RETURN_ON_ERROR(kex_->generateSharedSecret(dh, err, enc));
  std::unique_ptr<folly::IOBuf> pkRm = kex_->getKeyShare();

  std::unique_ptr<folly::IOBuf> kemContext = folly::IOBuf::copyBuffer(enc);
  kemContext->prependChain(std::move(pkRm));

  FIZZ_RETURN_ON_ERROR(
      extractAndExpand(ret, err, std::move(dh), std::move(kemContext)));
  return Status::Success;
}

Status DHKEM::authDecap(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    folly::ByteRange enc,
    folly::ByteRange pkS) {
  std::unique_ptr<folly::IOBuf> dh;
  FIZZ_RETURN_ON_ERROR(kex_->generateSharedSecret(dh, err, enc));
  std::unique_ptr<folly::IOBuf> authDh;
  FIZZ_RETURN_ON_ERROR(kex_->generateSharedSecret(authDh, err, pkS));
  dh->prependChain(std::move(authDh));
  std::unique_ptr<folly::IOBuf> pkRm = kex_->getKeyShare();

  std::unique_ptr<folly::IOBuf> kemContext = folly::IOBuf::copyBuffer(enc);
  kemContext->prependChain(std::move(pkRm));
  kemContext->prependChain(folly::IOBuf::copyBuffer(pkS));

  FIZZ_RETURN_ON_ERROR(
      extractAndExpand(ret, err, std::move(dh), std::move(kemContext)));
  return Status::Success;
}

} // namespace fizz
