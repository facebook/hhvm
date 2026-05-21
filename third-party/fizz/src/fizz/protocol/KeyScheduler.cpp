/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/protocol/KeyScheduler.h>
#include <fizz/protocol/ech/Types.h>
#include <fizz/util/Logging.h>

using folly::StringPiece;

static constexpr StringPiece kTrafficKey{"key"};
static constexpr StringPiece kTrafficIv{"iv"};

static constexpr StringPiece kExternalPskBinder{"ext binder"};
static constexpr StringPiece kResumptionPskBinder{"res binder"};
static constexpr StringPiece kClientEarlyTraffic{"c e traffic"};
static constexpr StringPiece kEarlyExporter{"e exp master"};
static constexpr StringPiece kClientHandshakeTraffic{"c hs traffic"};
static constexpr StringPiece kServerHandshakeTraffic{"s hs traffic"};
static constexpr StringPiece kClientAppTraffic{"c ap traffic"};
static constexpr StringPiece kServerAppTraffic{"s ap traffic"};
static constexpr StringPiece kExporterMaster{"exp master"};
static constexpr StringPiece kResumptionMaster{"res master"};
static constexpr StringPiece kDerivedSecret{"derived"};
static constexpr StringPiece kTrafficKeyUpdate{"traffic upd"};
static constexpr StringPiece kResumption{"resumption"};
static constexpr StringPiece kECHAcceptConfirmation{"ech accept confirmation"};
static constexpr StringPiece kHRRECHAcceptConfirmation{
    "hrr ech accept confirmation"};

namespace fizz {

Status KeyScheduler::deriveEarlySecret(Error& err, folly::ByteRange psk) {
  if (secret_) {
    return err.error("secret already set", folly::none);
  }

  auto zeros = std::vector<uint8_t>(deriver_->hashLength(), 0);
  std::vector<uint8_t> extracted;
  FIZZ_RETURN_ON_ERROR(
      deriver_->hkdfExtract(extracted, err, folly::range(zeros), psk));
  secret_.emplace(EarlySecret{std::move(extracted)});
  return Status::Success;
}

Status KeyScheduler::deriveHandshakeSecret(Error& err) {
  auto& earlySecret = *secret_->asEarlySecret();
  auto zeros = std::vector<uint8_t>(deriver_->hashLength(), 0);
  std::vector<uint8_t> preSecret;
  FIZZ_RETURN_ON_ERROR(deriver_->deriveSecret(
      preSecret,
      err,
      folly::range(earlySecret.secret),
      kDerivedSecret,
      deriver_->blankHash(),
      deriver_->hashLength()));
  std::vector<uint8_t> extracted;
  FIZZ_RETURN_ON_ERROR(deriver_->hkdfExtract(
      extracted, err, folly::range(preSecret), folly::range(zeros)));
  secret_.emplace(HandshakeSecret{std::move(extracted)});
  return Status::Success;
}

Status KeyScheduler::deriveHandshakeSecret(Error& err, folly::ByteRange ecdhe) {
  if (!secret_) {
    auto zeros = std::vector<uint8_t>(deriver_->hashLength(), 0);
    std::vector<uint8_t> extracted;
    FIZZ_RETURN_ON_ERROR(deriver_->hkdfExtract(
        extracted, err, folly::range(zeros), folly::range(zeros)));
    secret_.emplace(EarlySecret{std::move(extracted)});
  }

  auto& earlySecret = secret_->tryAsEarlySecret();
  std::vector<uint8_t> preSecret;
  FIZZ_RETURN_ON_ERROR(deriver_->deriveSecret(
      preSecret,
      err,
      folly::range(earlySecret.secret),
      kDerivedSecret,
      deriver_->blankHash(),
      deriver_->hashLength()));
  std::vector<uint8_t> hsExtracted;
  FIZZ_RETURN_ON_ERROR(
      deriver_->hkdfExtract(hsExtracted, err, folly::range(preSecret), ecdhe));
  secret_.emplace(HandshakeSecret{std::move(hsExtracted)});
  return Status::Success;
}

Status KeyScheduler::deriveMasterSecret(Error& err) {
  auto zeros = std::vector<uint8_t>(deriver_->hashLength(), 0);
  auto& handshakeSecret = secret_->tryAsHandshakeSecret();
  std::vector<uint8_t> preSecret;
  FIZZ_RETURN_ON_ERROR(deriver_->deriveSecret(
      preSecret,
      err,
      folly::range(handshakeSecret.secret),
      kDerivedSecret,
      deriver_->blankHash(),
      deriver_->hashLength()));
  std::vector<uint8_t> msExtracted;
  FIZZ_RETURN_ON_ERROR(deriver_->hkdfExtract(
      msExtracted, err, folly::range(preSecret), folly::range(zeros)));
  secret_.emplace(MasterSecret{std::move(msExtracted)});
  return Status::Success;
}

Status KeyScheduler::deriveAppTrafficSecrets(
    Error& err,
    folly::ByteRange transcript) {
  auto& masterSecret = *secret_->asMasterSecret();
  AppTrafficSecret trafficSecret;
  FIZZ_RETURN_ON_ERROR(deriver_->deriveSecret(
      trafficSecret.client,
      err,
      folly::range(masterSecret.secret),
      kClientAppTraffic,
      transcript,
      deriver_->hashLength()));
  FIZZ_RETURN_ON_ERROR(deriver_->deriveSecret(
      trafficSecret.server,
      err,
      folly::range(masterSecret.secret),
      kServerAppTraffic,
      transcript,
      deriver_->hashLength()));
  appTrafficSecret_ = std::move(trafficSecret);
  return Status::Success;
}

Status KeyScheduler::clearMasterSecret(Error& err) {
  if (secret_->type() != KeySchedulerSecret::Type::MasterSecret_E) {
    return err.error("Secret isn't MasterSecret", folly::none);
  }
  secret_ = folly::none;
  return Status::Success;
}

Status KeyScheduler::clientKeyUpdate(uint32_t& ret, Error& err) {
  auto& appTrafficSecret = *appTrafficSecret_;
  Buf buf;
  FIZZ_RETURN_ON_ERROR(deriver_->expandLabel(
      buf,
      err,
      folly::range(appTrafficSecret.client),
      kTrafficKeyUpdate,
      folly::IOBuf::create(0),
      deriver_->hashLength()));
  buf->coalesce();
  appTrafficSecret.client = std::vector<uint8_t>(buf->data(), buf->tail());
  ret = ++appTrafficSecret.clientGeneration;
  return Status::Success;
}

Status KeyScheduler::serverKeyUpdate(uint32_t& ret, Error& err) {
  auto& appTrafficSecret = *appTrafficSecret_;
  Buf buf;
  FIZZ_RETURN_ON_ERROR(deriver_->expandLabel(
      buf,
      err,
      folly::range(appTrafficSecret.server),
      kTrafficKeyUpdate,
      folly::IOBuf::create(0),
      deriver_->hashLength()));
  buf->coalesce();
  appTrafficSecret.server = std::vector<uint8_t>(buf->data(), buf->tail());
  ret = ++appTrafficSecret.serverGeneration;
  return Status::Success;
}

Status KeyScheduler::getSecret(
    DerivedSecret& ret,
    Error& err,
    EarlySecrets s,
    folly::ByteRange transcript) const {
  StringPiece label;
  uint16_t secretLength = deriver_->hashLength();
  switch (s) {
    case EarlySecrets::ExternalPskBinder:
      label = kExternalPskBinder;
      break;
    case EarlySecrets::ResumptionPskBinder:
      label = kResumptionPskBinder;
      break;
    case EarlySecrets::ClientEarlyTraffic:
      label = kClientEarlyTraffic;
      break;
    case EarlySecrets::EarlyExporter:
      label = kEarlyExporter;
      break;
    case EarlySecrets::ECHAcceptConfirmation:
      label = kECHAcceptConfirmation;
      secretLength = ech::kEchAcceptConfirmationSize;
      break;
    case EarlySecrets::HRRECHAcceptConfirmation:
      label = kHRRECHAcceptConfirmation;
      secretLength = ech::kEchAcceptConfirmationSize;
      break;
    default:
      FIZZ_LOG(FATAL) << "unknown secret";
  }

  auto& earlySecret = *secret_->asEarlySecret();
  std::vector<uint8_t> secret;
  FIZZ_RETURN_ON_ERROR(deriver_->deriveSecret(
      secret,
      err,
      folly::range(earlySecret.secret),
      label,
      transcript,
      secretLength));
  ret = DerivedSecret(std::move(secret), SecretType(s));
  return Status::Success;
}

Status KeyScheduler::getSecret(
    DerivedSecret& ret,
    Error& err,
    HandshakeSecrets s,
    folly::ByteRange transcript) const {
  StringPiece label;
  uint16_t secretLength = deriver_->hashLength();
  switch (s) {
    case HandshakeSecrets::ClientHandshakeTraffic:
      label = kClientHandshakeTraffic;
      break;
    case HandshakeSecrets::ServerHandshakeTraffic:
      label = kServerHandshakeTraffic;
      break;
    case HandshakeSecrets::ECHAcceptConfirmation:
      label = kECHAcceptConfirmation;
      secretLength = ech::kEchAcceptConfirmationSize;
      break;
    default:
      FIZZ_LOG(FATAL) << "unknown secret";
  }

  auto& handshakeSecret = *secret_->asHandshakeSecret();
  std::vector<uint8_t> secret;
  FIZZ_RETURN_ON_ERROR(deriver_->deriveSecret(
      secret,
      err,
      folly::range(handshakeSecret.secret),
      label,
      transcript,
      secretLength));
  ret = DerivedSecret(std::move(secret), SecretType(s));
  return Status::Success;
}

Status KeyScheduler::getSecret(
    DerivedSecret& ret,
    Error& err,
    MasterSecrets s,
    folly::ByteRange transcript) const {
  StringPiece label;
  switch (s) {
    case MasterSecrets::ExporterMaster:
      label = kExporterMaster;
      break;
    case MasterSecrets::ResumptionMaster:
      label = kResumptionMaster;
      break;
    default:
      FIZZ_LOG(FATAL) << "unknown secret";
  }

  auto& masterSecret = *secret_->asMasterSecret();
  std::vector<uint8_t> secret;
  FIZZ_RETURN_ON_ERROR(deriver_->deriveSecret(
      secret,
      err,
      folly::range(masterSecret.secret),
      label,
      transcript,
      deriver_->hashLength()));
  ret = DerivedSecret(std::move(secret), SecretType(s));
  return Status::Success;
}

DerivedSecret KeyScheduler::getSecret(AppTrafficSecrets s) const {
  auto& appTrafficSecret = *appTrafficSecret_;
  switch (s) {
    case AppTrafficSecrets::ClientAppTraffic:
      return DerivedSecret(
          appTrafficSecret.client,
          SecretType(AppTrafficSecrets::ClientAppTraffic));
    case AppTrafficSecrets::ServerAppTraffic:
      return DerivedSecret(
          appTrafficSecret.server,
          SecretType(AppTrafficSecrets::ServerAppTraffic));
    default:
      FIZZ_LOG(FATAL) << "unknown secret";
  }
}

Status KeyScheduler::getTrafficKey(
    TrafficKey& ret,
    Error& err,
    folly::ByteRange trafficSecret,
    size_t keyLength,
    size_t ivLength) const {
  return getTrafficKeyWithLabel(
      ret, err, trafficSecret, kTrafficKey, kTrafficIv, keyLength, ivLength);
}

Status KeyScheduler::getTrafficKeyWithLabel(
    TrafficKey& ret,
    Error& err,
    folly::ByteRange trafficSecret,
    folly::StringPiece keyLabel,
    folly::StringPiece ivLabel,
    size_t keyLength,
    size_t ivLength) const {
  FIZZ_RETURN_ON_ERROR(deriver_->expandLabel(
      ret.key,
      err,
      trafficSecret,
      keyLabel,
      folly::IOBuf::create(0),
      keyLength));
  FIZZ_RETURN_ON_ERROR(deriver_->expandLabel(
      ret.iv, err, trafficSecret, ivLabel, folly::IOBuf::create(0), ivLength));
  return Status::Success;
}

Status KeyScheduler::getResumptionSecret(
    Buf& ret,
    Error& err,
    folly::ByteRange resumptionMasterSecret,
    folly::ByteRange ticketNonce) const {
  FIZZ_RETURN_ON_ERROR(deriver_->expandLabel(
      ret,
      err,
      resumptionMasterSecret,
      kResumption,
      folly::IOBuf::wrapBuffer(ticketNonce),
      deriver_->hashLength()));
  return Status::Success;
}

std::unique_ptr<KeyScheduler> KeyScheduler::clone() const {
  return std::unique_ptr<KeyScheduler>(
      new KeyScheduler(secret_, appTrafficSecret_, deriver_->clone()));
}
} // namespace fizz
