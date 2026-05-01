/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/hpke/Hpke.h>
#include <fizz/crypto/hpke/Types.h>

namespace fizz {
namespace hpke {

static Status writeKeyScheduleContext(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    Mode mode,
    const std::vector<uint8_t>& pskIdHash,
    const std::vector<uint8_t>& infoHash) {
  std::unique_ptr<folly::IOBuf> keyScheduleContext = folly::IOBuf::create(0);
  folly::io::Appender appender(keyScheduleContext.get(), 20);
  FIZZ_RETURN_ON_ERROR(detail::write<uint8_t>(err, (uint8_t)mode, appender));

  detail::writeBufWithoutLength(folly::IOBuf::copyBuffer(pskIdHash), appender);
  detail::writeBufWithoutLength(folly::IOBuf::copyBuffer(infoHash), appender);
  ret = std::move(keyScheduleContext);
  return Status::Success;
}

Status keySchedule(
    std::unique_ptr<HpkeContext>& ret,
    Error& err,
    KeyScheduleParams params) {
  auto hkdf = std::move(params.hkdf);

  auto psk = params.pskInputs.hasValue()
      ? std::move(params.pskInputs.value().psk)
      : PskInputs::getDefaultPsk();
  auto pskId = params.pskInputs.hasValue()
      ? std::move(params.pskInputs.value().id)
      : PskInputs::getDefaultId();

  // Generate hashes for key schedule context
  std::vector<uint8_t> pskIdHash = hkdf->labeledExtract(
      folly::IOBuf::copyBuffer(""),
      folly::ByteRange(folly::StringPiece("psk_id_hash")),
      std::move(pskId),
      params.suiteId->clone());
  std::vector<uint8_t> infoHash = hkdf->labeledExtract(
      folly::IOBuf::copyBuffer(""),
      folly::ByteRange(folly::StringPiece("info_hash")),
      std::move(params.info),
      params.suiteId->clone());
  std::unique_ptr<folly::IOBuf> keyScheduleContext;
  FIZZ_RETURN_ON_ERROR(writeKeyScheduleContext(
      keyScheduleContext, err, params.mode, pskIdHash, infoHash));

  // Generate hashes for cipher key
  std::vector<uint8_t> secret = hkdf->labeledExtract(
      std::move(params.sharedSecret),
      folly::ByteRange(folly::StringPiece("secret")),
      std::move(psk),
      params.suiteId->clone());

  // Generate additional values needed for contructing context
  std::unique_ptr<folly::IOBuf> key;
  FIZZ_RETURN_ON_ERROR(hkdf->labeledExpand(
      key,
      err,
      secret,
      folly::ByteRange(folly::StringPiece("key")),
      keyScheduleContext->clone(),
      params.cipher->keyLength(),
      params.suiteId->clone()));
  std::unique_ptr<folly::IOBuf> nonce;
  FIZZ_RETURN_ON_ERROR(hkdf->labeledExpand(
      nonce,
      err,
      secret,
      folly::ByteRange(folly::StringPiece("base_nonce")),
      keyScheduleContext->clone(),
      params.cipher->ivLength(),
      params.suiteId->clone()));
  std::unique_ptr<folly::IOBuf> exporterSecret;
  FIZZ_RETURN_ON_ERROR(hkdf->labeledExpand(
      exporterSecret,
      err,
      secret,
      folly::ByteRange(folly::StringPiece("exp")),
      std::move(keyScheduleContext),
      hkdf->hashLength(),
      params.suiteId->clone()));

  // Configure cipher to use our generated key
  TrafficKey trafficKey;
  trafficKey.key = std::move(key);
  trafficKey.iv = std::move(nonce);
  FIZZ_RETURN_ON_ERROR(params.cipher->setKey(err, std::move(trafficKey)));

  ret = std::make_unique<HpkeContextImpl>(
      std::move(params.cipher),
      std::move(exporterSecret),
      std::move(hkdf),
      std::move(params.suiteId),
      params.seqNum,
      params.ctxRole);
  return Status::Success;
}

Status setupWithEncap(
    SetupResult& ret,
    Error& err,
    Mode mode,
    folly::ByteRange pkR,
    std::unique_ptr<folly::IOBuf> info,
    folly::Optional<PskInputs> pskInputs,
    SetupParam param) {
  DHKEM::EncapResult encapResult;
  switch (mode) {
    case Mode::Auth:
    case Mode::AuthPsk:
      FIZZ_RETURN_ON_ERROR(param.dhkem->authEncap(encapResult, err, pkR));
      break;
    case Mode::Base:
    case Mode::Psk:
      FIZZ_RETURN_ON_ERROR(param.dhkem->encap(encapResult, err, pkR));
      break;
  }

  KeyScheduleParams keyScheduleParams{
      mode,
      std::move(encapResult.sharedSecret),
      std::move(info),
      std::move(pskInputs),
      std::move(param.cipher),
      std::move(param.hkdf),
      std::move(param.suiteId),
      HpkeContext::Role::Sender,
      param.seqNum};

  std::unique_ptr<HpkeContext> context;
  FIZZ_RETURN_ON_ERROR(keySchedule(context, err, std::move(keyScheduleParams)));
  ret = SetupResult{std::move(encapResult.enc), std::move(context)};
  return Status::Success;
}

Status setupWithDecap(
    std::unique_ptr<HpkeContext>& ret,
    Error& err,
    Mode mode,
    folly::ByteRange enc,
    folly::Optional<folly::ByteRange> pkS,
    std::unique_ptr<folly::IOBuf> info,
    folly::Optional<PskInputs> pskInputs,
    SetupParam param) {
  std::unique_ptr<folly::IOBuf> sharedSecret;
  switch (mode) {
    case Mode::Auth:
    case Mode::AuthPsk:
      FIZZ_RETURN_ON_ERROR(
          param.dhkem->authDecap(sharedSecret, err, enc, *pkS));
      break;
    case Mode::Base:
    case Mode::Psk:
      FIZZ_RETURN_ON_ERROR(param.dhkem->decap(sharedSecret, err, enc));
      break;
  }
  KeyScheduleParams keyScheduleParams{
      mode,
      std::move(sharedSecret),
      std::move(info),
      std::move(pskInputs),
      std::move(param.cipher),
      std::move(param.hkdf),
      std::move(param.suiteId),
      HpkeContext::Role::Receiver,
      param.seqNum};

  return keySchedule(ret, err, std::move(keyScheduleParams));
}
} // namespace hpke
} // namespace fizz
