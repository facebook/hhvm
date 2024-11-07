/*
 *  Copyright (c) Meta Platforms, Inc. and affiliates.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <random>

#include <fizz/protocol/ech/GreaseECH.h>

#include <fizz/crypto/hpke/Utils.h>

namespace fizz {
namespace ech {
namespace {

/**
 * A random number generator adaptor based on the Factory.
 * Current usage is limited to type size_t.
 */
class RandomNumberGenerator {
 public:
  typedef size_t result_type;

  static constexpr size_t min() {
    return 0;
  }

  static constexpr size_t max() {
    return std::numeric_limits<size_t>::max();
  }

  explicit RandomNumberGenerator(const Factory& factory) : factory_{factory} {}

  size_t operator()() const {
    size_t number = 0;
    factory_.makeRandomBytes(
        reinterpret_cast<unsigned char*>(&number), sizeof(number));
    return number;
  }

 private:
  const Factory& factory_;
};

class RandomSelector {
 public:
  explicit RandomSelector(RandomNumberGenerator&& generator)
      : generator_{std::move(generator)} {}

  size_t genNumber(size_t min, size_t max) const {
    std::uniform_int_distribution<size_t> distribution(min, max);
    return distribution(generator_);
  }

  template <typename T>
  T select(const std::vector<T>& elems) const {
    return elems[genNumber(0, elems.size() - 1)];
  }

 private:
  const RandomNumberGenerator generator_;
};
} // namespace

OuterECHClientHello generateGreaseECH(
    const GreaseECHSetting& setting,
    const Factory& factory,
    size_t encodedChloSize) {
  RandomSelector selector{RandomNumberGenerator{factory}};
  OuterECHClientHello echExtension;
  echExtension.cipher_suite = HpkeSymmetricCipherSuite{
      selector.select(setting.kdfs), selector.select(setting.aeads)};
  echExtension.config_id =
      selector.genNumber(setting.minConfigId, setting.maxConfigId);
  echExtension.enc = factory.makeRandomIOBuf(selector.select(setting.keySizes));
  size_t payloadSize =
      selector.genNumber(setting.minPayloadSize, setting.maxPayloadSize);
  if (setting.payloadStrategy == PayloadGenerationStrategy::Computed) {
    payloadSize += encodedChloSize +
        hpke::getCipherOverhead(echExtension.cipher_suite.aead_id);
  }
  echExtension.payload = factory.makeRandomIOBuf(payloadSize);
  return echExtension;
}
} // namespace ech
} // namespace fizz
