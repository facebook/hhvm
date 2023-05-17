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

static std::unique_ptr<folly::IOBuf> writeKeyScheduleContext(
    Mode mode,
    const std::vector<uint8_t>& pskIdHash,
    const std::vector<uint8_t>& infoHash) {
  std::unique_ptr<folly::IOBuf> keyScheduleContext = folly::IOBuf::create(0);
  folly::io::Appender appender(keyScheduleContext.get(), 20);
  detail::write<uint8_t>((uint8_t)mode, appender);

  detail::writeBufWithoutLength(folly::IOBuf::copyBuffer(pskIdHash), appender);
  detail::writeBufWithoutLength(folly::IOBuf::copyBuffer(infoHash), appender);
  return keyScheduleContext;
}

std::unique_ptr<HpkeContext> keySchedule(KeyScheduleParams params) {
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
  std::unique_ptr<folly::IOBuf> keyScheduleContext =
      writeKeyScheduleContext(params.mode, pskIdHash, infoHash);

  // Generate hashes for cipher key
  std::vector<uint8_t> secret = hkdf->labeledExtract(
      std::move(params.sharedSecret),
      folly::ByteRange(folly::StringPiece("secret")),
      std::move(psk),
      params.suiteId->clone());

  // Generate additional values needed for contructing context
  std::unique_ptr<folly::IOBuf> key = hkdf->labeledExpand(
      secret,
      folly::ByteRange(folly::StringPiece("key")),
      keyScheduleContext->clone(),
      params.cipher->keyLength(),
      params.suiteId->clone());
  std::unique_ptr<folly::IOBuf> nonce = hkdf->labeledExpand(
      secret,
      folly::ByteRange(folly::StringPiece("base_nonce")),
      keyScheduleContext->clone(),
      params.cipher->ivLength(),
      params.suiteId->clone());
  std::unique_ptr<folly::IOBuf> exporterSecret = hkdf->labeledExpand(
      secret,
      folly::ByteRange(folly::StringPiece("exp")),
      std::move(keyScheduleContext),
      hkdf->hashLength(),
      params.suiteId->clone());

  // Configure cipher to use our generated key
  TrafficKey trafficKey;
  trafficKey.key = std::move(key);
  trafficKey.iv = std::move(nonce);
  params.cipher->setKey(std::move(trafficKey));

  return std::make_unique<HpkeContextImpl>(
      std::move(params.cipher),
      std::move(exporterSecret),
      std::move(hkdf),
      std::move(params.suiteId),
      params.seqNum,
      params.ctxRole);
}

SetupResult setupWithEncap(
    Mode mode,
    folly::ByteRange pkR,
    std::unique_ptr<folly::IOBuf> info,
    folly::Optional<PskInputs> pskInputs,
    SetupParam param) {
  DHKEM::EncapResult encapResult;
  switch (mode) {
    case Mode::Auth:
    case Mode::AuthPsk:
      encapResult = param.dhkem->authEncap(pkR);
      break;
    case Mode::Base:
    case Mode::Psk:
      encapResult = param.dhkem->encap(pkR);
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

  SetupResult result{
      std::move(encapResult.enc), keySchedule(std::move(keyScheduleParams))};
  return result;
}

std::unique_ptr<HpkeContext> setupWithDecap(
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
      sharedSecret = param.dhkem->authDecap(enc, *pkS);
      break;
    case Mode::Base:
    case Mode::Psk:
      sharedSecret = param.dhkem->decap(enc);
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

  return keySchedule(std::move(keyScheduleParams));
}

std::unique_ptr<folly::IOBuf> deserializePublicKey(
    fizz::hpke::KEMId kemId,
    const std::string& publicKey) {
  switch (kemId) {
    case fizz::hpke::KEMId::x25519:
    case fizz::hpke::KEMId::x448: {
      return folly::IOBuf::copyBuffer(folly::unhexlify(publicKey));
    }
    case fizz::hpke::KEMId::secp256r1:
    case fizz::hpke::KEMId::secp384r1:
    case fizz::hpke::KEMId::secp521r1: {
      folly::ssl::BioUniquePtr bio(BIO_new(BIO_s_mem()));
      BIO_write(bio.get(), publicKey.data(), publicKey.size());
      folly::ssl::EvpPkeyUniquePtr pkey(
          PEM_read_bio_PUBKEY(bio.get(), nullptr, nullptr, nullptr));
      return fizz::detail::encodeECPublicKey(pkey);
    }
    default:
      throw std::runtime_error("Unsupported KEM ID");
  }
}

} // namespace hpke
} // namespace fizz
