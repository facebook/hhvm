/*
 *  Copyright (c) Meta Platforms, Inc. and affiliates.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/protocol/ech/GreaseECH.h>

#include <fizz/crypto/hpke/Utils.h>

namespace fizz {
namespace ech {
namespace {

/**
 * A random number generator based on the Factory.
 * Current usage is limited to type size_t.
 */
class RandomNumberGenerator {
 public:
  explicit RandomNumberGenerator(const Factory& factory) : factory_{factory} {}

  Status generate(size_t& ret, Error& err) const {
    size_t number = 0;
    FIZZ_RETURN_ON_ERROR(factory_.makeRandomBytes(
        err, reinterpret_cast<unsigned char*>(&number), sizeof(number)));
    ret = number;
    return Status::Success;
  }

 private:
  const Factory& factory_;
};

class RandomSelector {
 public:
  explicit RandomSelector(RandomNumberGenerator&& generator)
      : generator_{std::move(generator)} {}

  Status genNumber(size_t& ret, Error& err, size_t min, size_t max) const {
    if (min > max) {
      return err.error("min > max");
    }
    if (max - min == std::numeric_limits<size_t>::max()) {
      return err.error("The range exeeds size_t");
    }
    size_t raw;
    FIZZ_RETURN_ON_ERROR(generator_.generate(raw, err));
    size_t range = max - min + 1;
    ret = min + (raw % range);
    return Status::Success;
  }

  template <typename T>
  Status select(T& ret, Error& err, const std::vector<T>& elems) const {
    size_t idx;
    FIZZ_RETURN_ON_ERROR(genNumber(idx, err, 0, elems.size() - 1));
    ret = elems[idx];
    return Status::Success;
  }

 private:
  const RandomNumberGenerator generator_;
};
} // namespace

Status generateGreaseECH(
    OuterECHClientHello& ret,
    Error& err,
    const GreaseECHSetting& setting,
    const Factory& factory,
    size_t encodedChloSize) {
  RandomSelector selector{RandomNumberGenerator{factory}};
  hpke::KDFId kdf;
  FIZZ_RETURN_ON_ERROR(selector.select(kdf, err, setting.kdfs));
  hpke::AeadId aead;
  FIZZ_RETURN_ON_ERROR(selector.select(aead, err, setting.aeads));
  ret.cipher_suite = HpkeSymmetricCipherSuite{kdf, aead};

  size_t configId;
  FIZZ_RETURN_ON_ERROR(selector.genNumber(
      configId, err, setting.minConfigId, setting.maxConfigId));
  ret.config_id = static_cast<uint8_t>(configId);

  uint16_t keySize;
  FIZZ_RETURN_ON_ERROR(selector.select(keySize, err, setting.keySizes));
  FIZZ_RETURN_ON_ERROR(factory.makeRandomIOBuf(ret.enc, err, keySize));

  size_t payloadSize;
  FIZZ_RETURN_ON_ERROR(selector.genNumber(
      payloadSize, err, setting.minPayloadSize, setting.maxPayloadSize));
  if (setting.payloadStrategy == PayloadGenerationStrategy::Computed) {
    size_t cipherOverhead;
    FIZZ_RETURN_ON_ERROR(
        hpke::getCipherOverhead(cipherOverhead, err, ret.cipher_suite.aead_id));
    payloadSize += encodedChloSize + cipherOverhead;
  }
  FIZZ_RETURN_ON_ERROR(factory.makeRandomIOBuf(ret.payload, err, payloadSize));
  return Status::Success;
}
} // namespace ech
} // namespace fizz
