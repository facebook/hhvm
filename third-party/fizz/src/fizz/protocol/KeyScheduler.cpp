/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/protocol/KeyScheduler.h>
#include <fizz/protocol/ech/Types.h>

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

void KeyScheduler::deriveEarlySecret(folly::ByteRange psk) {
  if (secret_) {
    throw std::runtime_error("secret already set");
  }

  auto zeros = std::vector<uint8_t>(deriver_->hashLength(), 0);
  secret_.emplace(EarlySecret{deriver_->hkdfExtract(folly::range(zeros), psk)});
}

void KeyScheduler::deriveHandshakeSecret() {
  auto& earlySecret = *secret_->asEarlySecret();
  auto zeros = std::vector<uint8_t>(deriver_->hashLength(), 0);
  auto preSecret = deriver_->deriveSecret(
      folly::range(earlySecret.secret),
      kDerivedSecret,
      deriver_->blankHash(),
      deriver_->hashLength());
  secret_.emplace(HandshakeSecret{
      deriver_->hkdfExtract(folly::range(preSecret), folly::range(zeros))});
}

void KeyScheduler::deriveHandshakeSecret(folly::ByteRange ecdhe) {
  if (!secret_) {
    auto zeros = std::vector<uint8_t>(deriver_->hashLength(), 0);
    secret_.emplace(EarlySecret{
        deriver_->hkdfExtract(folly::range(zeros), folly::range(zeros))});
  }

  auto& earlySecret = secret_->tryAsEarlySecret();
  auto preSecret = deriver_->deriveSecret(
      folly::range(earlySecret.secret),
      kDerivedSecret,
      deriver_->blankHash(),
      deriver_->hashLength());
  secret_.emplace(
      HandshakeSecret{deriver_->hkdfExtract(folly::range(preSecret), ecdhe)});
}

void KeyScheduler::deriveMasterSecret() {
  auto zeros = std::vector<uint8_t>(deriver_->hashLength(), 0);
  auto& handshakeSecret = secret_->tryAsHandshakeSecret();
  auto preSecret = deriver_->deriveSecret(
      folly::range(handshakeSecret.secret),
      kDerivedSecret,
      deriver_->blankHash(),
      deriver_->hashLength());
  secret_.emplace(MasterSecret{
      deriver_->hkdfExtract(folly::range(preSecret), folly::range(zeros))});
}

void KeyScheduler::deriveAppTrafficSecrets(folly::ByteRange transcript) {
  auto& masterSecret = *secret_->asMasterSecret();
  AppTrafficSecret trafficSecret;
  trafficSecret.client = deriver_->deriveSecret(
      folly::range(masterSecret.secret),
      kClientAppTraffic,
      transcript,
      deriver_->hashLength());
  trafficSecret.server = deriver_->deriveSecret(
      folly::range(masterSecret.secret),
      kServerAppTraffic,
      transcript,
      deriver_->hashLength());
  appTrafficSecret_ = std::move(trafficSecret);
}

void KeyScheduler::clearMasterSecret() {
  if (secret_->type() != KeySchedulerSecret::Type::MasterSecret_E) {
    throw std::runtime_error("Secret isn't MasterSecret");
  }
  secret_ = folly::none;
}

uint32_t KeyScheduler::clientKeyUpdate() {
  auto& appTrafficSecret = *appTrafficSecret_;
  auto buf = deriver_->expandLabel(
      folly::range(appTrafficSecret.client),
      kTrafficKeyUpdate,
      folly::IOBuf::create(0),
      deriver_->hashLength());
  buf->coalesce();
  appTrafficSecret.client = std::vector<uint8_t>(buf->data(), buf->tail());
  return ++appTrafficSecret.clientGeneration;
}

uint32_t KeyScheduler::serverKeyUpdate() {
  auto& appTrafficSecret = *appTrafficSecret_;
  auto buf = deriver_->expandLabel(
      folly::range(appTrafficSecret.server),
      kTrafficKeyUpdate,
      folly::IOBuf::create(0),
      deriver_->hashLength());
  buf->coalesce();
  appTrafficSecret.server = std::vector<uint8_t>(buf->data(), buf->tail());
  return ++appTrafficSecret.serverGeneration;
}

DerivedSecret KeyScheduler::getSecret(
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
      LOG(FATAL) << "unknown secret";
  }

  auto& earlySecret = *secret_->asEarlySecret();
  return DerivedSecret(
      deriver_->deriveSecret(
          folly::range(earlySecret.secret), label, transcript, secretLength),
      SecretType(s));
}

DerivedSecret KeyScheduler::getSecret(
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
      LOG(FATAL) << "unknown secret";
  }

  auto& handshakeSecret = *secret_->asHandshakeSecret();
  return DerivedSecret(
      deriver_->deriveSecret(
          folly::range(handshakeSecret.secret),
          label,
          transcript,
          secretLength),
      SecretType(s));
}

DerivedSecret KeyScheduler::getSecret(
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
      LOG(FATAL) << "unknown secret";
  }

  auto& masterSecret = *secret_->asMasterSecret();
  return DerivedSecret(
      deriver_->deriveSecret(
          folly::range(masterSecret.secret),
          label,
          transcript,
          deriver_->hashLength()),
      SecretType(s));
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
      LOG(FATAL) << "unknown secret";
  }
}

TrafficKey KeyScheduler::getTrafficKey(
    folly::ByteRange trafficSecret,
    size_t keyLength,
    size_t ivLength) const {
  return getTrafficKeyWithLabel(
      trafficSecret, kTrafficKey, kTrafficIv, keyLength, ivLength);
}

TrafficKey KeyScheduler::getTrafficKeyWithLabel(
    folly::ByteRange trafficSecret,
    folly::StringPiece keyLabel,
    folly::StringPiece ivLabel,
    size_t keyLength,
    size_t ivLength) const {
  TrafficKey trafficKey;
  trafficKey.key = deriver_->expandLabel(
      trafficSecret, keyLabel, folly::IOBuf::create(0), keyLength);
  trafficKey.iv = deriver_->expandLabel(
      trafficSecret, ivLabel, folly::IOBuf::create(0), ivLength);
  return trafficKey;
}

Buf KeyScheduler::getResumptionSecret(
    folly::ByteRange resumptionMasterSecret,
    folly::ByteRange ticketNonce) const {
  return deriver_->expandLabel(
      resumptionMasterSecret,
      kResumption,
      folly::IOBuf::wrapBuffer(ticketNonce),
      deriver_->hashLength());
}

std::unique_ptr<KeyScheduler> KeyScheduler::clone() const {
  return std::unique_ptr<KeyScheduler>(
      new KeyScheduler(secret_, appTrafficSecret_, deriver_->clone()));
}
} // namespace fizz
