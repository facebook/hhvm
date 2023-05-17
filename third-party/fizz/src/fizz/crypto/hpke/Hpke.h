/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/aead/Aead.h>
#include <fizz/crypto/hpke/Context.h>
#include <fizz/crypto/hpke/DHKEM.h>

namespace fizz {
namespace hpke {

struct PskInputs {
  Mode mode;
  std::unique_ptr<folly::IOBuf> psk;
  std::unique_ptr<folly::IOBuf> id;

  PskInputs(
      Mode givenMode,
      std::unique_ptr<folly::IOBuf> givenPsk,
      std::unique_ptr<folly::IOBuf> givenId)
      : mode(givenMode), psk(std::move(givenPsk)), id(std::move(givenId)) {
    bool gotPsk = folly::IOBufNotEqualTo()(psk, getDefaultPsk());
    bool gotPskId = folly::IOBufNotEqualTo()(id, getDefaultId());

    if (gotPsk != gotPskId) {
      throw std::runtime_error("Inconsistent PSK inputs");
    }

    if (gotPsk && (mode == Mode::Base || mode == Mode::Auth)) {
      throw std::runtime_error("PSK input provided when not needed");
    }

    if (!gotPsk && (mode == Mode::Psk || mode == Mode::AuthPsk)) {
      throw std::runtime_error("Missing required PSK input");
    }
  }

  static std::unique_ptr<folly::IOBuf> getDefaultPsk() {
    return folly::IOBuf::copyBuffer("");
  }

  static std::unique_ptr<folly::IOBuf> getDefaultId() {
    return folly::IOBuf::copyBuffer("");
  }
};

struct KeyScheduleParams {
  Mode mode{Mode::Base};
  std::unique_ptr<folly::IOBuf> sharedSecret;
  std::unique_ptr<folly::IOBuf> info;
  folly::Optional<PskInputs> pskInputs;
  std::unique_ptr<Aead> cipher;
  std::unique_ptr<fizz::hpke::Hkdf> hkdf;
  std::unique_ptr<folly::IOBuf> suiteId;
  fizz::hpke::HpkeContext::Role ctxRole{fizz::hpke::HpkeContext::Role::Sender};
  uint64_t seqNum{0};
};

std::unique_ptr<HpkeContext> keySchedule(KeyScheduleParams params);

struct SetupResult {
  std::unique_ptr<folly::IOBuf> enc;
  std::unique_ptr<HpkeContext> context;
};

struct SetupParam {
  std::unique_ptr<DHKEM> dhkem;
  std::unique_ptr<Aead> cipher;
  std::unique_ptr<fizz::hpke::Hkdf> hkdf;
  std::unique_ptr<folly::IOBuf> suiteId;
  uint64_t seqNum{0};
};

SetupResult setupWithEncap(
    Mode mode,
    folly::ByteRange pkR,
    std::unique_ptr<folly::IOBuf> info,
    folly::Optional<PskInputs> pskInputs,
    SetupParam param);

std::unique_ptr<HpkeContext> setupWithDecap(
    Mode mode,
    folly::ByteRange enc,
    folly::Optional<folly::ByteRange> pkS,
    std::unique_ptr<folly::IOBuf> info,
    folly::Optional<PskInputs> pskInputs,
    SetupParam param);

/**
 * Deserialize a public key from a hex or DER encoded string.
 * Note, Curve25519 based KEMs only support hex endoded strings.
 * EC curves support DER encoded strings.
 * @param kemId kem ID to deserialize
 * @param publicKey hex or DER encoded string
 * @return deserialized public key
 **/
std::unique_ptr<folly::IOBuf> deserializePublicKey(
    fizz::hpke::KEMId kemId,
    const std::string& publicKey);

} // namespace hpke
} // namespace fizz
