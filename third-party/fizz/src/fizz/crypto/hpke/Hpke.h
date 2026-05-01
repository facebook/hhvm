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

  static Status create(
      folly::Optional<PskInputs>& ret,
      Error& err,
      Mode givenMode,
      std::unique_ptr<folly::IOBuf> givenPsk,
      std::unique_ptr<folly::IOBuf> givenId) {
    bool gotPsk = folly::IOBufNotEqualTo()(givenPsk, getDefaultPsk());
    bool gotPskId = folly::IOBufNotEqualTo()(givenId, getDefaultId());

    if (gotPsk != gotPskId) {
      return err.error("Inconsistent PSK inputs");
    }

    if (gotPsk && (givenMode == Mode::Base || givenMode == Mode::Auth)) {
      return err.error("PSK input provided when not needed");
    }

    if (!gotPsk && (givenMode == Mode::Psk || givenMode == Mode::AuthPsk)) {
      return err.error("Missing required PSK input");
    }
    ret = PskInputs(givenMode, std::move(givenPsk), std::move(givenId));
    return Status::Success;
  }

  static std::unique_ptr<folly::IOBuf> getDefaultPsk() {
    return folly::IOBuf::copyBuffer("");
  }

  static std::unique_ptr<folly::IOBuf> getDefaultId() {
    return folly::IOBuf::copyBuffer("");
  }

 private:
  PskInputs(
      Mode givenMode,
      std::unique_ptr<folly::IOBuf> givenPsk,
      std::unique_ptr<folly::IOBuf> givenId)
      : mode(givenMode), psk(std::move(givenPsk)), id(std::move(givenId)) {}
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

Status keySchedule(
    std::unique_ptr<HpkeContext>& ret,
    Error& err,
    KeyScheduleParams params);

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

Status setupWithEncap(
    SetupResult& ret,
    Error& err,
    Mode mode,
    folly::ByteRange pkR,
    std::unique_ptr<folly::IOBuf> info,
    folly::Optional<PskInputs> pskInputs,
    SetupParam param);

Status setupWithDecap(
    std::unique_ptr<HpkeContext>& ret,
    Error& err,
    Mode mode,
    folly::ByteRange enc,
    folly::Optional<folly::ByteRange> pkS,
    std::unique_ptr<folly::IOBuf> info,
    folly::Optional<PskInputs> pskInputs,
    SetupParam param);

} // namespace hpke
} // namespace fizz
